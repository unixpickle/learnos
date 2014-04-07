#include "alloc.h"
#include "system.h"
#include <keyedbits/buff_encoder.h>
#include <keyedbits/buff_decoder.h>
#include <keyedbits/validation.h>

static uint64_t pageSocket = UINT64_MAX;

static bool _manual_alloc_pages(uint64_t idx, uint64_t count);
static bool _manual_free_pages(uint64_t idx, uint64_t count);
static bool _send_req(const char * type, uint64_t pageIndex, uint64_t count);

bool alloc_pages(uint64_t pageIndex, uint64_t count) {
  if (sys_self_pid() == 0) {
    return _manual_alloc_pages(pageIndex, count);   
  }
  return _send_req("alloc", pageIndex, count);
}

bool free_pages(uint64_t pageIndex, uint64_t count) {
  if (sys_self_pid() == 0) {
    return _manual_free_pages(pageIndex, count);   
  }
  return _send_req("free", pageIndex, count);
}

static bool _manual_alloc_pages(uint64_t idx, uint64_t count) {
  uint64_t i, start = idx + (((uint64_t)ALLOC_DATA_BASE) >> 12);
  uint64_t vmFlags = 7;
  for (i = 0; i < count; i++) {
    uint64_t pg = sys_alloc_page();
    if (!pg) {
      if (i > 0) _manual_free_pages(start, i);
      return false;
    }
    sys_self_vmmap(i + start, (pg << 12) | vmFlags);
  }
  return true;
}

static bool _manual_free_pages(uint64_t idx, uint64_t count) {
  uint64_t i, start = idx + (((uint64_t)ALLOC_DATA_BASE) >> 12);
  for (i = 0; i < count; i++) {
    uint64_t entry = sys_vmread(start + i);
    if ((entry & 7) != 7) return false;
    sys_free_page(entry & 0xFFFFFFFFFFFFF000);
    sys_self_vmunmap(start + i);
  }
  return true;
}

static bool _send_req(const char * type, uint64_t pageIndex, uint64_t count) {
  if (!(pageSocket + 1)) {
    pageSocket = sys_open();
    if (!(pageSocket + 1)) return false;
    if (!sys_open(pageSocket, 0)) {
      sys_close(pageSocket);
      return false;
    }
  }

  char buff[0xfe8];
  kb_buff_t kb;
  kb_buff_initialize_encode(&kb, buff, 0xfe8);
  if (!kb_buff_write_dict(&kb)) return false;
  if (!kb_buff_write_key(&kb, "type")) return false;
  if (!kb_buff_write_string(&kb, type)) return false;
  if (!kb_buff_write_key(&kb, "start")) return false;
  if (!kb_buff_write_int(&kb, (int64_t)pageIndex)) return false;
  if (!kb_buff_write_key(&kb, "count")) return false;
  if (!kb_buff_write_int(&kb, (int64_t)count)) return false;
  if (!kb_buff_write_terminator(&kb)) return false;
  if (!sys_write(pageSocket, buff, kb.off)) return false;

  // wait for a response from the allocator
  msg_t msg;
  while (!sys_read(pageSocket, &msg)) sys_sleep(0x10000);
  if (msg.type != 1) return false;

  // we should have gotten an integer back; 0 = false, otherwise true
  kb_buff_initialize_decode(&kb, msg.message, msg.len);
  kb_header_t header;
  if (!kb_buff_read_header(&kb, &header)) return false;
  if (!kb_validate_header(&header)) return false;
  if (header.typeField != KeyedBitsTypeInteger) return false;

  int64_t number;
  if (!kb_buff_read_int(&kb, header.lenLen, &number)) return false;
  return number == 0 ? false : true;
}

