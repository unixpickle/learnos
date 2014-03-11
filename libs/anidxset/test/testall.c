#include <anidxset.h>
#include <stdlib.h>
#include <stdio.h>

#define MASSIVE_COUNT 0x1000000

void fail_test(const char * msg);
void begin_test(const char * name);
void end_test();

void * block_alloc();
void block_free(void * ptr);

void test_initialize();
void test_massive_gets();
void test_massive_puts();
void test_get_loopback();

static anidxset_root_t root;

int main(int argc, const char * argv[]) {
  test_initialize();
  test_massive_gets();
  test_massive_puts();
  test_get_loopback();
  anidxset_free(&root);
  return 0;
}

// fail, start, pass

void fail_test(const char * msg) {
  fprintf(stderr, "[failed]: %s\n", msg);
  exit(1);
}

void begin_test(const char * name) {
  printf("[beginning]: %s\n", name);
}

void end_test() {
  printf("[succeeded]\n");
}

// allocation methods

void * block_alloc() {
  return malloc(0x1000);
}

void block_free(void * ptr) {
  free(ptr);
}

void test_initialize() {
  begin_test("initialization");
  
  anidxset_initialize(&root, block_alloc, block_free);
  
  // check on the first node
  anidxset_node_t * node = root.first;
  if (node->count != 0x1fe) fail_test("invalid initial count.");
  if (node->next != 0) fail_test("invalid `next` ptr");
  int i;
  for (i = 0; i < 0x1fe; i++) {
    if (node->indexes[i] != 0x1fe - i - 1) fail_test("invalid index");
  }
  
  end_test();
}

void test_massive_gets() {
  begin_test("get()");
  int i;
  for (i = 0; i < MASSIVE_COUNT; i++) {
    if (anidxset_get(&root) != i) fail_test("got invalid value");
  }
  end_test();
}

void test_massive_puts() {
  begin_test("put()");
  int i;
  for (i = 0; i < MASSIVE_COUNT; i++) {
    if (!anidxset_put(&root, i)) fail_test("failed to put");
  }
  end_test();
}

void test_get_loopback() {
  begin_test("more get()");
  int i;
  for (i = 0; i < MASSIVE_COUNT; i++) {
    if (anidxset_get(&root) != MASSIVE_COUNT - i - 1) {
      fail_test("got invalid loopback value");
    }
  }
  for (i = 0; i < MASSIVE_COUNT; i++) {
    if (anidxset_get(&root) != MASSIVE_COUNT + i) {
      fail_test("got invalid new value");
    }
  }
  end_test();
}
