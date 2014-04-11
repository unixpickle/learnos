#include <base/msgd.h>
#include <stdio.h>
#include <string.h>

#include "echo.h"
#include "count.h"
#include "mem.h"
#include "sleep.h"
#include "fault.h"
#include "aborter.h"
#include "allocer.h"

#define BUFF_SIZE 0xff

static uint64_t keyboard = 0;
static char * buffer = NULL;
static uint64_t bufferCount = 0;
static uint64_t taskFd = 0xffffffffffffffffL;

static void handle_chars(const char * chrs, uint64_t len);
static void process_cmd();
static void prompt();
static bool is_command(const char * cmd);

int main() {
  char _buffer[BUFF_SIZE + 1];
  buffer = _buffer;

  char * keyboardName = "keyboard";
  msgd_connect_services(1, (const char **)&keyboardName, &keyboard, 3);
  if (!(keyboard + 1)) {
    printf("[terminal]: error: failed to connect to keyboard\n");
    return 0;
  }
  printf("[terminal]: connected to keyboard.\n");

  prompt();

  while (1) {
    uint64_t res = sys_poll();
    if (!(res + 1)) continue;
    if (res == keyboard) {
      msg_t msg;
      while (sys_read(keyboard, &msg)) {
        if (taskFd + 1) {
          sys_write(taskFd, msg.message, msg.len);
          // see if any chars were Control+C = 0x3
          uint64_t i;
          for (i = 0; i < msg.len; i++) {
            if (msg.message[i] == 3) {
              sys_kill(sys_remote_pid(taskFd));
              printf("^C");
            }
          }
        } else {
          handle_chars((const char *)msg.message, msg.len);
        }
      }
    } else if (res == taskFd) {
      msg_t msg;
      while (sys_read(taskFd, &msg)) {
        if (msg.type == 2) {
          sys_close(taskFd);

          // possibly print out the kill reason
          void * data = msg.message;
          uint64_t killReason = *((uint64_t *)data);
          if ((killReason & 1) && killReason != 1) {
            sys_color(0x0c);
            printf("[terminal]: task kill with code 0x%x\n", killReason >> 1);
          } else if (!killReason) {
            sys_color(0x0e);
            printf("[terminal]: task closed link.\n");
          }
          taskFd = 0xffffffffffffffffL;
          prompt();
          break;
        }
        char message[0x1000];
        memcpy(message, msg.message, msg.len);
        message[msg.len] = 0;
        sys_print(message);
      }
    }
  }
  
  return 0;
}

static void handle_chars(const char * chrs, uint64_t len) {
  uint64_t i;
  for (i = 0; i < len; i++) {
    char ch = chrs[i];
    if (ch == '\n') {
      sys_print("\n");
      process_cmd();
    } else if (ch == '\b') {
      if (!bufferCount) continue;
      bufferCount--;
      sys_print("\b");
    } else {
      if (bufferCount == BUFF_SIZE) continue;
      buffer[bufferCount++] = ch;
      printf("%c", chrs[i]);
    }
  }
}

static void process_cmd() {
  if (!bufferCount) {
    prompt();
    return;
  }
  // command is in buffer
  buffer[bufferCount] = 0;
  uint64_t method = 0;
  if (is_command("echo")) {
    method = (uint64_t)command_echo;
  } else if (is_command("count")) {
    method = (uint64_t)command_count;
  } else if (is_command("memusage")) {
    method = (uint64_t)command_memusage;
  } else if (is_command("sleep")) {
    method = (uint64_t)command_sleep;
  } else if (is_command("fault")) {
    method = (uint64_t)command_fault;
  } else if (is_command("abort")) {
    method = (uint64_t)command_abort;
  } else if (is_command("allocer")) {
    method = (uint64_t)command_allocer;
  } else {
    printf("[terminal]: `%s` unknown command\n", buffer);
    prompt();
    return;
  }
  taskFd = sys_fork((uint64_t)method);
  if (!(taskFd + 1)) {
    sys_color(0x7);
    printf("[terminal]: could not launch task\n", buffer);
    prompt();
  } else {
    sys_write(taskFd, buffer, bufferCount);
  }
}

static void prompt() {
  bufferCount = 0;
  sys_color(0x7);
  sys_print("[terminal] $");
  sys_color(0xf);
  sys_print(" ");
}

static bool is_command(const char * cmd) {
  if (bufferCount < strlen(cmd)) {
    return false;
  }
  int i;
  for (i = 0; i < strlen(cmd); i++) {
    if (buffer[i] != cmd[i]) return false;
  }
  if (bufferCount == strlen(cmd)) return true;
  return buffer[strlen(cmd)] == ' ';
}

