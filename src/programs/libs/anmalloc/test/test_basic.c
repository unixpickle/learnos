#include <anmalloc/anmalloc.h>
#include <stdio.h>
#include <assert.h>

void test_units();
void test_breaking();
void test_realloc();
void test_alignment();

int main(int argc, const char * argv[]) {
  test_units();
  test_breaking();
  test_realloc();
  test_alignment();
  return 0;
}

void test_units() {
  printf("testing unit-sized allocation... ");
  
  void * buff1 = anmalloc_alloc(0x20);
  void * buff2 = anmalloc_alloc(0x20);
  assert(buff2 == buff1 + 0x20);
  anmalloc_free(buff2);
  void * buff3 = anmalloc_alloc(0x20);
  assert(buff2 == buff3);
  anmalloc_free(buff3);
  anmalloc_free(buff1);
  
  printf(" passed!\n");
}

void test_breaking() {
  printf("testing break sizes... ");
  
  assert(__anmalloc_brk_size() == 0);
  void * buff = anmalloc_alloc(0x80000);
  assert(__anmalloc_brk_size() == 0x100000);
  anmalloc_free(buff);
  assert(__anmalloc_brk_size() == 0);
  buff = anmalloc_alloc(0x80001);
  assert(__anmalloc_brk_size() == 0x400000);
  
  void * buff1 = anmalloc_alloc(0x80000);
  void * buff2 = anmalloc_alloc(0x80000);
  assert(__anmalloc_brk_size() == 0x400000);
  
  anmalloc_free(buff1);
  anmalloc_free(buff2);
  assert(__anmalloc_brk_size() == 0x400000);
  
  anmalloc_free(buff);
  assert(__anmalloc_brk_size() == 0);
  
  printf(" passed!\n");
}

void test_realloc() {
  printf("testing realloc... ");
  
  void * buff = anmalloc_alloc(0x80000);
  buff = anmalloc_realloc(buff, 0x40000);
  assert(buff != NULL);
  
  void * buff1 = anmalloc_alloc(0x40000);
  assert(buff1 == buff + 0x40000);
  anmalloc_free(buff);
  anmalloc_free(buff1);
  
  buff = anmalloc_alloc(0x40000);
  buff1 = anmalloc_alloc(0x40000);
  assert(__anmalloc_brk_size() == 0x100000);
  buff = anmalloc_realloc(buff, 0x80000);
  assert(__anmalloc_brk_size() == 0x200000);
  buff1 = anmalloc_realloc(buff1, 0x80000);
  assert(__anmalloc_brk_size() == 0x200000);
  anmalloc_free(buff);
  anmalloc_free(buff1);
  
  printf(" passed!\n");
}

void test_alignment() {
  printf("testing alignment... ");
  
  void * buff = anmalloc_aligned(0x10000, 0x20000);
  void * buff1 = anmalloc_aligned(0x10000, 0x30000);
  void * buff2 = anmalloc_aligned(0x10000, 0x80001);
  void * buff3 = anmalloc_aligned(0x10000, 0x123);
  
  assert(buff != NULL);
  assert(buff1 != NULL);
  assert(buff2 != NULL);
  assert(buff3 != NULL);
  assert(0 == (0xffff & (uint64_t)buff));
  assert(0 == (0xffff & (uint64_t)buff1));
  assert(0 == (0xffff & (uint64_t)buff2));
  assert(0 == (0xffff & (uint64_t)buff3));
  
  anmalloc_free(buff);
  anmalloc_free(buff1);
  anmalloc_free(buff2);
  anmalloc_free(buff3);
  
  buff = anmalloc_aligned(0x400000, 1);
  assert(buff != NULL);
  assert(0 == (0x3fffff & (uint64_t)buff));
  anmalloc_free(buff);
  
  buff = anmalloc_aligned(0x200001, 1);
  assert(buff == NULL);
  assert(__anmalloc_brk_size() == 0);
  
  buff = anmalloc_aligned(0x100001, 1);
  assert(buff != NULL);
  assert(__anmalloc_brk_size() == 0x1000000);
  anmalloc_free(buff);
  assert(__anmalloc_brk_size() == 0);
  
  printf(" passed!\n");
}
