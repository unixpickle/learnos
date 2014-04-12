#include "client.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <assert.h>
#include <keyedbits/buff_decoder.h>
#include <keyedbits/buff_encoder.h>
#include <keyedbits/validation.h>

#define ANSCHEDULER_TASK_DATA_PAGE 0x10200000

void handle_messages(uint64_t fd);
void handle_faults();
void handle_client_fault(client_t * cli, pgf_t * fault);

const char * client_request(kb_buff_t * kb, uint64_t * start, uint64_t * count);
void handle_client_request(client_t * cli,
                           const char * action,
                           uint64_t start,
                           uint64_t count);
bool handle_client_alloc(client_t * cli, uint64_t start, uint64_t count);
bool handle_client_free(client_t * cli, uint64_t start, uint64_t count);
void handle_client_death(client_t * cli);

void main() {
  sys_become_pager();

  while (1) {
    uint64_t fd = sys_poll();
    if (fd + 1) handle_messages(fd);
    handle_faults();
  }
}

void handle_messages(uint64_t fd) {
  client_t * cli = client_get(fd);
  if (!cli) return;

  msg_t msg;
  while (sys_read(fd, &msg)) {
    if (msg.type == 2) {
      handle_client_death(cli);
      client_delete(cli);
      sys_close(fd);
      return;
    }
    if (msg.type == 0) continue;

    kb_buff_t kb;
    kb_buff_initialize_decode(&kb, (void *)msg.message, msg.len);
    uint64_t start, count;
    const char * type = client_request(&kb, &start, &count);
    if (!type) {
      handle_client_death(cli);
      client_delete(cli);
      sys_close(fd);
      return;
    }
    handle_client_request(cli, type, start, count);
  }
}

void handle_faults() {
  pgf_t fault;
  while (sys_get_fault(&fault)) {
    // map in the page or kill the task
    client_t * client = client_find(fault.taskId);
    if (!client) {
      sys_mem_fault(fault.taskId);
      sys_shift_fault();
      continue;
    }
    handle_client_fault(client, &fault);
  }
}

void handle_client_fault(client_t * cli, pgf_t * fault) {
  uint64_t index = fault->address >> 12;
  if (index < ANSCHEDULER_TASK_DATA_PAGE ||
      index >= ANSCHEDULER_TASK_DATA_PAGE + cli->pageCount) {
    sys_mem_fault(cli->pid);
    sys_shift_fault();
    return;
  }

  sys_shift_fault();

  // if the page has already been allocated, retry
  uint64_t pg = index - ANSCHEDULER_TASK_DATA_PAGE;
  if (cli->pages[pg]) return;

  uint64_t grabCount;
  uint64_t maxCount = 0x20;
  if (cli->pageCount - pg < maxCount) maxCount = cli->pageCount - pg;

  for (grabCount = 1; grabCount < maxCount; grabCount++) {
    if (cli->pages[pg + grabCount]) break;
  }

  uint64_t addrs[0x21];
  sys_batch_alloc(&addrs[1], grabCount);
  uint64_t i;
  for (i = 0; i < grabCount; i++) {
    cli->pages[pg + i] = addrs[i + 1];
    addrs[i + 1] |= 7;
  }
  addrs[0] = index;

  sys_batch_vmmap(cli->fd, addrs, grabCount);
  sys_invlpg(cli->fd);
  sys_wake_thread(cli->fd, fault->threadId);
}

const char * client_request(kb_buff_t * kb,
                            uint64_t * start,
                            uint64_t * count) {
  const char * typeResult = NULL;
  int found = 0;

  kb_header_t header;
  if (!kb_buff_read_header(kb, &header)) return NULL;
  if (!kb_validate_header(&header)) return NULL;
  if (header.typeField != KeyedBitsTypeDictionary) return NULL;

  char key[32];
  while (1) {
    if (!kb_buff_read_key(kb, key, 32)) return NULL;
    if (!key[0]) break;
    found++;
    if (!strcmp(key, "type")) {
      if (!kb_buff_read_header(kb, &header)) return NULL;
      if (!kb_validate_header(&header)) return NULL;
      if (header.typeField != KeyedBitsTypeString) return NULL;
      const char * value = 0;
      uint64_t unused;
      if (!kb_buff_read_string(kb, &value, &unused)) return NULL;
      if (!strcmp(value, "alloc")) typeResult = "alloc";
      else if (!strcmp(value, "free")) typeResult = "free";
      else return NULL;
    } else if (!strcmp(key, "start") || !strcmp(key, "count")) {
      int64_t value;
      if (!kb_buff_read_header(kb, &header)) return NULL;
      if (!kb_validate_header(&header)) return NULL;
      if (header.typeField != KeyedBitsTypeInteger) return NULL;
      if (!kb_buff_read_int(kb, header.lenLen, &value)) return NULL;
      if (value < 0) return NULL;
      if (!strcmp(key, "start")) *start = (uint64_t)value;
      else *count = (uint64_t)value;
    } else {
      return NULL;
    }
  }

  if (found != 3) return NULL;
  return typeResult;
}

void handle_client_request(client_t * cli,
                           const char * action,
                           uint64_t start,
                           uint64_t count) {
  bool res = false;
  if (!strcmp(action, "alloc")) {
    res = handle_client_alloc(cli, start, count);
  } else if (!strcmp(action, "free")) {
    res = handle_client_free(cli, start, count);
  }

  int64_t number = res ? 1 : 0;
  char result[0x10];
  kb_buff_t kb;
  kb_buff_initialize_encode(&kb, result, 0x10);
  kb_buff_write_int(&kb, number);
  sys_write(cli->fd, result, kb.off);
}

bool handle_client_alloc(client_t * cli, uint64_t start, uint64_t count) {
  if (start != cli->pageCount) return false;
  if (!count) return true;

  cli->pageCount += count;
  cli->pages = realloc(cli->pages, sizeof(uint64_t) * cli->pageCount);
  assert(cli->pages != NULL);
  bzero(&cli->pages[start], 8 * count);
  return true;
}

bool handle_client_free(client_t * cli, uint64_t start, uint64_t count) {
  if (start + count != cli->pageCount) return false;
  if (count > cli->pageCount) return false;
  if (!count) return true;

  uint64_t i = 0;
  while (i < count) {
    uint64_t pg = start + i;
    if (i + 0x20 > count) {
      sys_batch_vmunmap(cli->fd, ANSCHEDULER_TASK_DATA_PAGE + pg, 0x20);
      i += 0x20;
    } else {
      sys_vmunmap(cli->fd, ANSCHEDULER_TASK_DATA_PAGE + pg);
      i += 1;
    }
  }
  sys_invlpg(cli->fd);

  for (i = 0; i < count; i++) {
    uint64_t pg = start + i;
    if (cli->pages[pg]) {
      sys_free_page(cli->pages[pg]);
    }
  }
  

  cli->pageCount -= count;
  if (!cli->pageCount && cli->pages) {
    free(cli->pages);
    cli->pages = NULL;
  } else {
    cli->pages = realloc(cli->pages, sizeof(uint64_t) * cli->pageCount);
    assert(cli->pages != NULL);
  }
  return true;
}

void handle_client_death(client_t * cli) {
  handle_client_free(cli, 0, cli->pageCount);
}

