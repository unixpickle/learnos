#include "anbtree.h"

static anbtree_path _anbtree_search(anbtree_ptr tree,
                                    anbtree_path path,
                                    uint64_t maxDepth,
                                    uint8_t rightFirst);
static uint8_t _anbtree_get_bit(anbtree_ptr tree, uint64_t index);
static void _anbtree_set_bit(anbtree_ptr tree, uint64_t index, uint8_t val);

uint64_t anbtree_size(uint8_t depth) {
  // there will always be 2^(depth + 1) - 1 nodes
  // ceil(nodes/8) is how many bytes we need
  
  if (depth < 3) return 1;
  return 1L << (depth - 2);
}

void anbtree_initialize(anbtree_ptr tree, uint64_t size) {
  uint64_t i;
  for (i = 0; i < size; i++) {
    tree[i] = 0;
  }
}

anbtree_path anbtree_path_to_leaf(anbtree_ptr tree, uint64_t depth) {
  // search the tree until we find a free node
  return _anbtree_search(tree, anbtree_path_root, depth, 0);
}

anbtree_path anbtree_high_path_to_leaf(anbtree_ptr tree, uint64_t depth) {
  return _anbtree_search(tree, anbtree_path_root, depth, 1);
}

void anbtree_free_node(anbtree_ptr tree, anbtree_path path) {
  _anbtree_set_bit(tree, path, 0);
}

void anbtree_alloc_node(anbtree_ptr tree, anbtree_path path) {
  _anbtree_set_bit(tree, path, 1);
}

uint8_t anbtree_is_allocated(anbtree_ptr tree, anbtree_path path) {
  return _anbtree_get_bit(tree, path);
}

/********************
 * Path information *
 ********************/

uint64_t anbtree_path_depth(anbtree_path path) {
  char depth;
  for (depth = 63; depth >= 0; depth--) {
    if ((path + 1) & (1L << depth)) break;
  }
  return depth;
}

uint64_t anbtree_path_local_index(anbtree_path path) {
  uint64_t depth = anbtree_path_depth(path);
  return (path + 1) ^ (1L << depth);
}

anbtree_path anbtree_path_left(anbtree_path path) {
  uint64_t depth = anbtree_path_depth(path);
  
  // get the index in this particular depth
  uint64_t localIdx = (path + 1) ^ (1L << depth);
  
  // double the local index for the next depth
  uint64_t nextIdx = localIdx << 1;
  return (nextIdx | (1L << (depth + 1))) - 1;
}

anbtree_path anbtree_path_right(anbtree_path path) {
  uint64_t depth = anbtree_path_depth(path);
  
  // get the index in this particular depth
  uint64_t localIdx = (path + 1) ^ (1L << depth);
  
  // double the local index for the next depth
  uint64_t nextIdx = localIdx << 1;
  return (nextIdx | (1L << (depth + 1))); // don't subtract 0 for right
}

anbtree_path anbtree_path_parent(anbtree_path path) {
  if (path == anbtree_path_root) return anbtree_path_none;
  uint64_t depth = anbtree_path_depth(path);
  uint64_t localIdx = (path + 1) ^ (1 << depth);
  
  uint64_t newIdx = localIdx >> 1;
  return (newIdx | (1L << (depth - 1))) - 1;
}

anbtree_path anbtree_path_sibling(anbtree_path path) {
  if (path == anbtree_path_root) return anbtree_path_none;
  
  // odd global index = even local index = left node
  if (path & 1) return path + 1;
  return path - 1;
}

anbtree_path anbtree_path_from_info(uint64_t depth, uint64_t localIdx) {
  return (localIdx | (1L << depth)) - 1;
}

/***********
 * Private *
 ***********/

static anbtree_path _anbtree_search(anbtree_ptr tree,
                                    anbtree_path path,
                                    uint64_t maxDepth,
                                    uint8_t rightFirst) {
  // if a node that we've reached has a value of 0, it's free
  if (!_anbtree_get_bit(tree, path)) return path;
  
  // if we can go deeper, we may seek out a deeper node
  if (anbtree_path_depth(path) < maxDepth) {
    anbtree_path first, second;
    if (rightFirst) {
      first = anbtree_path_right(path);
      second = anbtree_path_left(path);
    } else {
      first = anbtree_path_left(path);
      second = anbtree_path_right(path);
    }
    
    if (!_anbtree_get_bit(tree, first) && !_anbtree_get_bit(tree, second)) {
      // if we're 1 and both children are 0, we're a data node.
      return anbtree_path_none;
    }
    
    first = _anbtree_search(tree, first, maxDepth, rightFirst);
    if (first != anbtree_path_none) return first;
    
    second = _anbtree_search(tree, second, maxDepth, rightFirst);
    if (second != anbtree_path_none) return second;
  }
  
  return anbtree_path_none;
}

static uint8_t _anbtree_get_bit(anbtree_ptr tree, uint64_t index) {
  uint64_t byteIndex = index >> 3;
  uint64_t bitIndex = index & 0x7;
  return (tree[byteIndex] & (1 << bitIndex)) != 0;
}

static void _anbtree_set_bit(anbtree_ptr tree,
                             uint64_t index,
                             uint8_t val) {
  uint64_t byteIndex = index >> 3;
  uint64_t bitIndex = index & 0x7;
  if (val) {
    tree[byteIndex] |= 1 << bitIndex;
  } else {
    tree[byteIndex] &= 0xff ^ (1 << bitIndex);
  }
}
