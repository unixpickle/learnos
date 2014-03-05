#include "anidxset.h"

static void _allocate_next_page(anidxset_root_t * root);

uint8_t anidxset_initialize(anidxset_root_t * root,
                            anidxset_alloc_t alloc,
                            anidxset_free_t free) {
  root->alloc = alloc;
  root->free = free;
  root->used = ANIDXSET_NODE_CAPACITY;
  root->first = root->alloc();
  if (!root->first) return 0;
  anidxset_node_t * node = (anidxset_node_t *)root->first;
  node->count = ANIDXSET_NODE_CAPACITY;
  node->next = 0;
  int i;
  for (i = 0; i < ANIDXSET_NODE_CAPACITY; i++) {
    node->indexes[i] = ANIDXSET_NODE_CAPACITY - 1 - i;
  }
  return 1;
}

uint64_t anidxset_get(anidxset_root_t * root) {
  anidxset_node_t * node = (anidxset_node_t *)root->first;
  if (!node->count) {
    if (!node->next) {
      // allocate a new chunk
      _allocate_next_page(root);
    } else {
      node = (root->first = (anidxset_node_t *)node->next);
    }
  }
  return node->indexes[--node->count];
}

uint8_t anidxset_put(anidxset_root_t * root, uint64_t value) {
  anidxset_node_t * node = root->first;
  // if this node is full, we will allocate a new node
  if (node->count == ANIDXSET_NODE_CAPACITY) {
    anidxset_node_t * newNode = (anidxset_node_t *)root->alloc();
    if (!newNode) return 0;
    newNode->count = 1;
    newNode->next = node;
    newNode->indexes[0] = value;
    root->first = newNode;
  } else {
    node->indexes[node->count++] = value;
  }
  return 1;
}

static void _allocate_next_page(anidxset_root_t * root) {
  uint64_t nextCount = ANIDXSET_NODE_CAPACITY;
  anidxset_node_t * node = (anidxset_node_t *)root->first;
  node->next = 0;
  node->count = nextCount;
  int i;
  for (i = 0; i < nextCount; i++) {
    node->indexes[nextCount - i - 1] = root->used++;
  }
}

