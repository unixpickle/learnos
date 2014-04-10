#include "analloc.h"

static uint64_t _analloc_log_page(analloc_t alloc, uint64_t size);
static uint8_t _analloc_grab_first(analloc_t alloc, uint64_t size);
static anbtree_path _analloc_ptr_path(analloc_t alloc,
                                      void * buffer,
                                      uint64_t size);
static void _analloc_memcpy(uint8_t * dest, uint8_t * source, uint64_t size);

uint8_t analloc_with_chunk(analloc_t alloc,
                           void * ptr,
                           uint64_t total,
                           uint64_t used,
                           uint64_t page) {
  if (total < page) return 0;
  // calculate the actual size of memory to use
  uint64_t depth = 0;
  while (page << depth <= total) depth++;
  depth--;
  
  // make sure they haven't used too much memory already
  uint64_t size = page << depth;
  if (size < used) return 0;
  
  uint64_t treeSize = anbtree_size(depth);
  uint64_t realUsed = used + treeSize;
  
  // setup the allocator
  alloc->mem = ptr;
  alloc->tree = ((uint8_t *)alloc->mem) + used;
  alloc->page = page;
  alloc->depth = depth;
  
  // initialize the tree and allocate our used data
  anbtree_initialize(alloc->tree, treeSize);
  return _analloc_grab_first(alloc, realUsed);
}

void * analloc_alloc(analloc_t alloc, uint64_t * sizeInOut, uint8_t high) {
  uint64_t twoPower = _analloc_log_page(alloc, *sizeInOut);
  if (twoPower > alloc->depth) {
    (*sizeInOut) = 0;
    return (void *)0;
  }
  
  uint64_t depth = alloc->depth - twoPower;
  anbtree_path path;
  if (high) path = anbtree_high_path_to_leaf(alloc->tree, depth);
  else path = anbtree_path_to_leaf(alloc->tree, depth);
  
  if (path == anbtree_path_none) {
    (*sizeInOut) = 0;
    return (void *)0;
  }
  
  // split down our `depth` until we have the actual buddy we want.
  uint64_t baseDepth = anbtree_path_depth(path);
  while (baseDepth < depth) {
    anbtree_alloc_node(alloc->tree, path);
    baseDepth++;
    if (high) { 
      anbtree_free_node(alloc->tree, anbtree_path_left(path));
      path = anbtree_path_right(path);
    } else {
      anbtree_free_node(alloc->tree, anbtree_path_right(path));
      path = anbtree_path_left(path);
    } 
  }
  
  // allocate the base node and set it as a data node.
  anbtree_alloc_node(alloc->tree, path);
  
  // the node should by DEFAULT be a child node based on the data.
  /*
  if (depth < alloc->depth) {
    anbtree_free_node(alloc->tree, anbtree_path_left(path));
    anbtree_free_node(alloc->tree, anbtree_path_right(path));
  }
  */
  
  uint64_t index = anbtree_path_local_index(path);
  uint64_t finalSize = alloc->page << twoPower;
  (*sizeInOut) = finalSize;
  return (void *)((index * finalSize) + (uint64_t)alloc->mem);
}

void analloc_free(analloc_t alloc, void * buffer, uint64_t length) {
  anbtree_path path = _analloc_ptr_path(alloc, buffer, length);
  
  // keep freeing paths until we can return
  while (path != anbtree_path_none) {
    anbtree_free_node(alloc->tree, path);
    anbtree_path parent = anbtree_path_parent(path);
    if (parent == anbtree_path_none) break;
    
    // if both sibling and us are now free, the parent node can be free'd.
    anbtree_path sibling = anbtree_path_sibling(path);
    if (anbtree_is_allocated(alloc->tree, sibling)) break;
    path = parent;
  }
}

/***********
 * Realloc *
 ***********/

void * analloc_realloc(analloc_t alloc,
                       void * buffer,
                       uint64_t length,
                       uint64_t * newLen,
                       uint8_t high) {
  uint64_t oldPower = _analloc_log_page(alloc, length);
  uint64_t newPower = _analloc_log_page(alloc, *newLen);
  if (oldPower == newPower) {
    *newLen = alloc->page << oldPower;
    return buffer;
  }
  
  // freeing the buffer does not change its contents or corrupt it,
  // it just marks it off in the tree.
  analloc_free(alloc, buffer, length);
  
  // likewise, allocation does not actually modify the contents of
  // the buffer, making it super easy to reallocate memory.
  void * newBuff = analloc_alloc(alloc, newLen, high);
  
  // of course, the re-allocation may have failed, in which case we
  // should undo our initial analloc_free().
  if (!(*newLen)) {
    // re-reserve our old buffer and reverse the effects of
    // analloc_free().
    (*newLen) = 0;
    anbtree_path path = _analloc_ptr_path(alloc, buffer, length);
    while (path != anbtree_path_none) {
      if (anbtree_is_allocated(alloc->tree, path)) break;
      anbtree_alloc_node(alloc->tree, path);
      path = anbtree_path_parent(path);
    }
    return (void *)0;
  }
  
  // copy contents of our old buffer to our new buffer for safe keeping!
  if (oldPower > newPower) {
    _analloc_memcpy(newBuff, buffer, alloc->page << newPower);
  } else {
    _analloc_memcpy(newBuff, buffer, alloc->page << oldPower);
  }
  
  (*newLen) = alloc->page << newPower;
  return newBuff;
}

uint64_t analloc_mem_size(analloc_t alloc, void * buffer) {
  anbtree_path path = _analloc_ptr_path(alloc, buffer, alloc->page);
  // keep incrementing until it's allocated
  uint64_t size = alloc->page;
  while (path != anbtree_path_none) {
    if (anbtree_is_allocated(alloc->tree, path)) {
      return size;
    }
    size <<= 1;
    path = anbtree_path_parent(path);
  }
  return 0;
}

void * analloc_mem_start(analloc_t alloc, void * buffer, uint64_t * _size) {
  anbtree_path path = _analloc_ptr_path(alloc, buffer, alloc->page);
  
  // go up the tree until we find the parent data node
  uint64_t size = alloc->page;
  while (path != anbtree_path_none) {
    if (anbtree_is_allocated(alloc->tree, path)) {
      uint64_t index = anbtree_path_local_index(path);
      if (_size) *_size = size;
      return (void *)((index * size) + (uint64_t)alloc->mem);
    }
    size <<= 1;
    path = anbtree_path_parent(path);
  }
  return (void *)0;
}

/***********
 * Private *
 ***********/

static uint64_t _analloc_log_page(analloc_t alloc, uint64_t size) {
  uint64_t result = alloc->page;
  uint64_t power = 0;
  while (result < size) {
    result <<= 1L;
    power += 1;
  }
  return power;
}

static uint8_t _analloc_grab_first(analloc_t alloc, uint64_t size) {
  uint64_t pageExp = _analloc_log_page(alloc, size);
  uint64_t sizeOut = size;
  
  if (pageExp == 0) {
    analloc_alloc(alloc, &sizeOut, 0);
    return sizeOut >= size;
  } else if (alloc->page << pageExp == size) {
    analloc_alloc(alloc, &sizeOut, 0);
    return sizeOut == size;
  } else {
    uint64_t newSize = alloc->page << (pageExp - 1);
    sizeOut = newSize;
    analloc_alloc(alloc, &sizeOut, 0);
    
    if (sizeOut != newSize) return 0;
    return _analloc_grab_first(alloc, size - newSize);
  }
}

static anbtree_path _analloc_ptr_path(analloc_t alloc,
                                      void * buffer,
                                      uint64_t length) {
  uint64_t twoPower = _analloc_log_page(alloc, length);
  uint64_t depth = alloc->depth - twoPower;
  
  uint64_t index = (uint64_t)buffer - (uint64_t)alloc->mem;
  index /= alloc->page << twoPower;
  
  return anbtree_path_from_info(depth, index);
}

static void _analloc_memcpy(uint8_t * dest, uint8_t * source, uint64_t size) {
  uint64_t i;
  if (dest < source) {
    for (i = 0; i < size; i++) {
      dest[i] = source[i];
    }
  } else {
    for (i = 0; i < size; i++) {
      dest[size - i - 1] = source[size - i - 1];
    }
  }
}
