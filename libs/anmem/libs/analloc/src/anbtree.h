#ifndef __ANBTREE_H__
#define __ANBTREE_H__

#include <stdint.h>

/**
 * The anbtree_ptr type is a pointer to a binary tree used for memory
 * allocation.
 */
typedef uint8_t * anbtree_ptr;

/**
 * The anbtree_path type represents a path to a node/leaf in a b-tree.
 */
typedef uint64_t anbtree_path;

/**
 * Result of functions when a path does not exist.
 */
#define anbtree_path_none 0xffffffffffffffffL
#define anbtree_path_root 0

/**
 * Calculates the size in bytes needed to store a binary tree which may
 * extend to a certain depth.
 * @param depth - The maximum depth to be used by the tree. For instance,
 * if the tree must be able to have a root node and two immediate children,
 * this value would be 1.
 */
uint64_t anbtree_size(uint8_t depth);

/**
 * Creates an empty memory b-tree.
 */
void anbtree_initialize(anbtree_ptr tree, uint64_t size);

/**
 * Returns the path to the first leaf of depth <= `depth`
 * @param tree - The tree to search.
 * @param depth - The maximum depth of the leaf to find.
 * @return The path.  If no such leaf exists, anbtree_path_none is returned.
 */
anbtree_path anbtree_path_to_leaf(anbtree_ptr tree, uint64_t depth);

/**
 * Returns the path to the *last* leaf of depth <= `depth`
 * @discussion This function is essentially the same as
 * anbtree_path_to_leaf().
 */
anbtree_path anbtree_high_path_to_leaf(anbtree_ptr tree, uint64_t depth);

/**
 * Changes a node back into a leaf.  This requires that the nodes two
 * children were already leafs.
 */
void anbtree_free_node(anbtree_ptr tree, anbtree_path path);

/**
 * Creates a node with two leaf children.  This requires that the node was
 * previously a leaf.
 */
void anbtree_alloc_node(anbtree_ptr tree, anbtree_path path);

/**
 * Returns 1 if a node is allocated, 0 if not.
 */
uint8_t anbtree_is_allocated(anbtree_ptr tree, anbtree_path path);

/**
 * Gets the depth of a path.
 */
uint64_t anbtree_path_depth(anbtree_path path);

/**
 * Gets the index of a path with respect to its depth.
 */
uint64_t anbtree_path_local_index(anbtree_path path);

/**
 * Gets the left subnode path for a node's path.
 */
anbtree_path anbtree_path_left(anbtree_path path);

/**
 * Gets the right subnode path for a node's path.
 */
anbtree_path anbtree_path_right(anbtree_path path);

/**
 * Gets the parent of a node.
 */
anbtree_path anbtree_path_parent(anbtree_path path);

/**
 * Gets the sibling of a node.
 */
anbtree_path anbtree_path_sibling(anbtree_path path);

/**
 * Gets a path for a local index and depth.
 */
anbtree_path anbtree_path_from_info(uint64_t depth, uint64_t localIdx);

#endif