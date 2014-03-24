#include <anscheduler/types.h>

uint64_t anscheduler_vm_physical(uint64_t virt); // global VM translation
uint64_t anscheduler_vm_virtual(uint64_t phys); // global VM translation
void * anscheduler_vm_root_alloc();
bool anscheduler_vm_map(void * root,
                        uint64_t vpage,
                        uint64_t dpage,
                        uint16_t flags);
void anscheduler_vm_unmap(void * root, uint64_t vpage);
uint64_t anscheduler_vm_lookup(void * root,
                               uint64_t vpage,
                               uint16_t * flags);
void anscheduler_vm_root_free(void * root);
void anscheduler_vm_root_free_async(void * root);
