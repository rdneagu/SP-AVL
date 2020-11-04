/*
 * Author: Robert Neagu 
 * ID: 2318538N
 * Systems Programming - Coursework 1a
 *
 * This is my own work except the tree iterator which is based on: https://gist.github.com/Knifa/1318375
 * 
 * This AVL C implementation is based on the Java version from Moodle
 */

#include "date.h"
#include "tldlist.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/**
 * Allocates the heap memory required for a TLD node and assigns the tld to it
 *
 * @returns TLDNode *
 */
TLDNode *createTLDNode(TLDList *tldList, char *tld, TLDNode *parent);

/**
 * Frees the memory that was allocated to the node
 */
void destroyTLDNode(TLDNode *node);

/**
 * Sets the current balance of the node by getting the height difference
 * of the left and right nodes
 */
void setBalance(TLDNode *node);

/**
 * Returns the current height of the node
 * 
 * @returns int
 */
int height(TLDNode *node);

/**
 * Updates the height of a node by getting the maximum height
 * between the left and right child nodes and adding 1
 */
void reheight(TLDNode *node);

/**
 * Performs a tree rebalance by rotating nodes depending on specific cases
 */
void rebalance(TLDList *tld, TLDNode *node);

/**
 * Performs a left rotation on a node
 *
 * @returns TLDNode *
 */
TLDNode *rotateLeft(TLDNode *node);

/**
 * Performs a right rotation on a node
 *
 * @returns TLDNode *
 */
TLDNode *rotateRight(TLDNode *node);

/**
 * Performs a left then right rotation on a node
 *
 * @returns TLDNode *
 */
TLDNode *rotateLeftThenRight(TLDNode *node);

/**
 * Performs a right then left rotation on a node
 *
 * @returns TLDNode *
 */
TLDNode *rotateRightThenLeft(TLDNode *node);

/**
 * Recursively adds all the nodes to an iterator sorted inorder
 */
void iteratorAdd(TLDIterator *iter, TLDNode *root, int *index);

/**
 * Structs
 */
struct tldnode {
  TLDNode *parent;
  TLDNode *left;
  TLDNode *right;
  int height;
  int balance;
  char *tld;
  long count;  
};

struct tldlist {
  TLDNode *root;
  Date *begin;
  Date *end;
  long count;
  long size;
};

struct tlditerator {
  TLDNode **next;
  int index;
  long length;
};

/**
 * TLD List
 */
TLDList *tldlist_create(Date *begin, Date *end) {
  TLDList *tldList = (TLDList *) malloc(sizeof(TLDList));
  if (tldList != NULL) {
    tldList->root = NULL;
    tldList->begin = date_duplicate(begin);
    tldList->end = date_duplicate(end);
    tldList->count = 0;
    tldList->size = 0;
  }
  return tldList;
}

void tldlist_destroy(TLDList *tld) {
  TLDIterator *iter = tldlist_iter_create(tld);
  TLDNode *node;
  // Loop through the list and destroy all the nodes
  while ((node = tldlist_iter_next(iter))) {
    destroyTLDNode(node);
  }
  // Finally, destroy the iterator and free the memory allocated for the TLD list
  tldlist_iter_destroy(iter);
  date_destroy(tld->begin);
  date_destroy(tld->end);
  free(tld);
}

int tldlist_add(TLDList *tld, char *hostname, Date *d) {
  // Make sure the date of the TLD about to be inserted is between the begin and end range
  if (date_compare(tld->begin, d) > 0 || date_compare(tld->end, d) < 0) {
    return 0;
  }

  // Parse the TLD from the hostname
  // Allocate memory with calloc to make sure there is a null character at the end
  char *tldFound = strrchr(hostname, '.') + 1;
  char *tldString = (char *) calloc(strlen(tldFound) + 1, sizeof(char));
  strcpy(tldString, tldFound);

  // If the tree already has a root
  if (tld->root != NULL) {
    TLDNode *currentNode = tld->root;
    while (currentNode != NULL) {
      // Compare node's tld with the parsed tld
      int direction = strcmp(currentNode->tld, tldString);

      // If node->tld equals the tld from hostname, increment tld count and break the loop
      if (direction == 0) {
        free(tldString); // Free up the memory allocated for the string as we don't need it
        currentNode->count++;
        break;
      }

      TLDNode *parentNode = currentNode;

      // If node->tld is higher than the tld from hostname, move to the left node
      // If node->tld is lower than the tld from hostname, move to the right node 
      currentNode = (direction > 0) ? currentNode->left : currentNode->right;

      // If the node is not occupied, create the node at the proper position then rebalance the tree
      if (currentNode == NULL) {
        if (direction > 0) {
          parentNode->left = createTLDNode(tld, tldString, parentNode);
        } else if (direction < 0) {
          parentNode->right = createTLDNode(tld, tldString, parentNode);
        }
        rebalance(tld, parentNode);
      }
    }
  } else {
    // Set the tree root to the newly created node
    tld->root = createTLDNode(tld, tldString, NULL);
  }
  // Increment the node count
  tld->count++;
  return 1;
}

long tldlist_count(TLDList *tld) {
  return (tld != NULL) ? tld->count : 0;
}

/**
 * TLD Iterator
 */
TLDIterator *tldlist_iter_create(TLDList *tld) {
  if (tld == NULL) {
    return NULL;
  }

  TLDIterator *iter = (TLDIterator *) malloc(sizeof(TLDIterator));
  if (iter != NULL) {
    iter->index = 0;
    iter->length = tld->size;
    iter->next = (TLDNode **) malloc(sizeof(TLDNode *) * iter->length);
    if (iter->next == NULL) {
      free(iter);
      return NULL;
    }
    // Initialize an identifier for the location in memory where the next node pointer will be added
    // Call the recursive function to add all the nodes' pointers to the iterator and pass the identifier by reference
    int index = 0;
    iteratorAdd(iter, tld->root, &index);
  }
  return iter;
}

void iteratorAdd(TLDIterator *iter, TLDNode *root, int *index) {
  if (root != NULL) {
    iteratorAdd(iter, root->left, index);
    *(iter->next + (*index)++) = root; // Push the node pointer into the list and increment the identifier
    iteratorAdd(iter, root->right, index);
  }
}

TLDNode *tldlist_iter_next(TLDIterator *iter) {
  if (iter->index >= iter->length)
    return NULL;
  
  return *(iter->next + iter->index++); // Return the next node pointer in list and increment the iterator identifier
}

void tldlist_iter_destroy(TLDIterator *iter) {
  free(iter->next);
  free(iter);
}

/**
 * TLD Node
 */
TLDNode *createTLDNode(TLDList *tldList, char *tld, TLDNode *parent) {
  TLDNode *node = (TLDNode *) malloc(sizeof(TLDNode));
  if (node != NULL) {
    node->parent = parent;
    node->left = NULL;
    node->right = NULL;
    node->tld = tld;
    node->height = 0;
    node->balance = 0;
    node->count = 1;
    tldList->size++;
  }
  return node;
}

char *tldnode_tldname(TLDNode *node) {
  return (node != NULL) ? node->tld : "";
}

long tldnode_count(TLDNode *node) {
  return (node != NULL) ? node->count : 0;
}

void destroyTLDNode(TLDNode *node) {
  free(node->tld);
  free(node);
}

/**
 * Tree balancing
 */
void setBalance(TLDNode *node) {
  reheight(node);
  node->balance = height(node->right) - height(node->left);
}

int height(TLDNode *node) {
  return (node != NULL) ? node->height : -1;
}

void reheight(TLDNode *node) {
  if (node != NULL) {
    int leftHeight = height(node->left);
    int rightHeight = height(node->right);
    int maxHeight = (leftHeight > rightHeight) ? leftHeight : rightHeight;
    node->height = 1 + maxHeight;
  }
}

void rebalance(TLDList *tld, TLDNode *node) {
  setBalance(node);

  if (node->balance == -2) {
    if (height(node->left->left) >= height(node->left->right)) {
      node = rotateRight(node);
    } else {
      node = rotateLeftThenRight(node);
    }
  } else if (node->balance == 2) {
    if (height(node->right->right) >= height(node->right->left)) {
      node = rotateLeft(node);
    } else {
      node = rotateRightThenLeft(node);
    }
  }

  if (node->parent != NULL) {
    rebalance(tld, node->parent);
  } else {
    tld->root = node;
  }
}

TLDNode *rotateLeft(TLDNode *node) {
  TLDNode *right = node->right;
  right->parent = node->parent;

  node->right = right->left;
  if (node->right != NULL) {
    node->right->parent = node;
  }

  right->left = node;
  node->parent = right;

  if (right->parent != NULL) {
    if (right->parent->right == node) {
      right->parent->right = right;
    } else {
      right->parent->left = right;
    }
  }

  setBalance(node);
  setBalance(right);

  return right;
}
 
TLDNode *rotateRight(TLDNode *node) {
  TLDNode *left = node->left;
  left->parent = node->parent;

  node->left = left->right;

  if (node->left != NULL) {
    node->left->parent = node;
  }

  left->right = node;
  node->parent = left;

  if (left->parent != NULL) {
    if (left->parent->right == node) {
      left->parent->right = left;
    } else {
      left->parent->left = left;
    }
  }

  setBalance(node);
  setBalance(left);

  return left;
}
 
TLDNode *rotateLeftThenRight(TLDNode *node) {
  node->left = rotateLeft(node->left);
  return rotateRight(node);
}
 
TLDNode *rotateRightThenLeft(TLDNode *node) {
  node->right = rotateRight(node->right);
  return rotateLeft(node);
}