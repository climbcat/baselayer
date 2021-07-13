#ifndef __MEMORY_H__
#define __MEMORY_H__


#include <cstdlib>
#include <assert.h>
#include <iostream>

#include "types.h"


struct MemoryBlock {
  MemoryBlock* next;
  MemoryBlock* prev;
  size_t total_size;
  bool used = false; // TODO: experiment with bitflags here
};


void ListInsert(MemoryBlock* item, MemoryBlock* after) {
  item->next = after->next;
  after->next->prev = item;
  item->prev = after;
  after->next = item;
}

void ListRemove(MemoryBlock* item) {
  item->next->prev = item->prev;
  item->prev->next = item->next;
  item->prev = NULL;
  item->next = NULL;
}

void ListInit(MemoryBlock* first) {
  first->next = first;
  first->prev = first;
}

void ListPrintSizes(MemoryBlock* from) {
  MemoryBlock* itm = from->next;
  u64 i = 0;
  while (itm != from) {
    std::cout << "element #" << i << " size=" <<  itm->total_size << std::endl;
    itm = itm->next;
    ++i;
  }
}


class GeneralPurposeAllocator {
public:
  MemoryBlock* blocks;
  size_t max_size;
  size_t min_block_size;
  size_t num_blocks;

  unsigned long blocks_merged = 0;
  unsigned long load = 0;

  GeneralPurposeAllocator(size_t max_size) {
    this->max_size = max_size;
    this->min_block_size = 2 * sizeof(MemoryBlock);

    this->blocks = (MemoryBlock*) calloc(max_size, 1);

    this->blocks->total_size = this->max_size;
    this->blocks->used = false;
    ListInit(this->blocks);
    this->num_blocks = 1;
  }
  ~GeneralPurposeAllocator() {
    std::free(this->blocks);
  }

  void* Alloc(size_t size) {
    assert(size <= this->max_size - this->load * sizeof(MemoryBlock));

    void* memloc = NULL;
    const size_t alloc_size = size + sizeof(MemoryBlock);

    MemoryBlock* at = this->blocks;
    u32 idx = 0;
    size_t free_size;
    while (idx < this->num_blocks) {

      if (at->used == false) {
        free_size = at->total_size;
        if (free_size >= alloc_size) {

          if (free_size > alloc_size + this->min_block_size) {
            MemoryBlock* remainder = (MemoryBlock*) ((u8*) at + alloc_size); // TODO: truncate somehow to align with a MemoryBlock array
            remainder->total_size = free_size - alloc_size;
            remainder->used = false;
            ListInsert(remainder, at);
            this->num_blocks++;
          }

          memloc = at;
          at->total_size = alloc_size; // TODO: fix, this is not always correct
          at->used = true;

          this->load += at->total_size;
          return (u8*) at + sizeof(MemoryBlock);
        }
      }

      at = at->next;
      ++idx;
    }

    // TODO: out of memory case / impl. eviction strategy
    // for now we crash
    assert(1==0);
  }

  bool _TryMergeAdjacentBlocks(MemoryBlock* a, MemoryBlock* b) {
    if (a->used != false && b->used != false)
        return false;

    MemoryBlock* tmp;
    if (b->next == a) { tmp = a; a = b; b = tmp; }
    else if (a->next != b) return false;

    // a sits right before b
    a->total_size += b->total_size;
    ListRemove(b);
    this->num_blocks--;
    this->blocks_merged++;
    return true;
  }

  bool Free(void* addr) {
    // guards
    size_t relative_location = (u8*) addr - (u8*) this->blocks;
    assert(relative_location >= 0);
    assert(relative_location < this->max_size);

    MemoryBlock* at = (MemoryBlock*) (((u8*) addr) - sizeof(MemoryBlock));
    if (at->used == false)
      return false;

    assert(this->load > at->total_size);

    this->load -= at->total_size;
    at->used = false;

    this->_TryMergeAdjacentBlocks(at->prev, at);

    return true;
  }
};


class PoolAllocator {
public:
  void* root;
  MemoryBlock* blocks;
  size_t block_size;
  size_t pool_size;
  size_t min_block_size;

  unsigned long load = 0;
  PoolAllocator(size_t block_size, size_t pool_size) {
    this->min_block_size = sizeof(MemoryBlock);
    assert(block_size >= this->min_block_size);

    this->block_size = block_size;
    this->pool_size = pool_size;

    this->blocks = (MemoryBlock*) calloc(block_size * pool_size, 1);
    this->blocks->used = false;
    this->blocks->total_size = this->block_size;
    ListInit(this->blocks);

    // record for destructor
    this->root = this->blocks;

    // build the free list
    MemoryBlock* prev = this->blocks;
    MemoryBlock* next;
    for (int i = 1; i < this->pool_size; i++) {
      next = this->blocks + i;
      next->total_size = this->block_size;
      next->used = false;

      ListInsert(next, prev);
      prev = next;
    }
  }
  ~PoolAllocator() {
    free(this->root);
  }
  void* Alloc() {
    if (this->blocks == NULL) {
      assert(this->load == this->pool_size);
      return NULL;
    }

    this->load++;
    MemoryBlock* toalloc = this->blocks;
    if (this->blocks->next != this->blocks) {
      this->blocks = toalloc->next;
      ListRemove(toalloc);
    }
    else {
      // list is now full
      assert(this->load == this->pool_size);
      this->blocks = NULL;
    }

    // clear header bytes
    memset(toalloc, sizeof(MemoryBlock), 0);
    return toalloc;
  }
  bool Free(void* addr) {
    size_t relative_location = (u8*) addr - (u8*) this->root;
    assert(relative_location >= 0); // location lower bound
    assert(relative_location < this->pool_size * this->block_size); // location upper bound
    assert(relative_location % this->block_size == 0); // alignment

    if (this-load == 0)
      return false;

    MemoryBlock* addrblock = (MemoryBlock*) addr;
    addrblock->total_size = this->block_size;
    addrblock->used = false;

    if (this->blocks != NULL) {
      ListInsert(this->blocks, addrblock);
    }
    else {
      // first free element after full
      assert(this->load == this->pool_size);
      this->blocks = (MemoryBlock*) addr;
    }

    this->load--;
    return true;
  }
};


class StackAllocator {
public:
  void* root;
  size_t total_size;
  size_t used;
  StackAllocator(size_t total_size) {
    this->total_size = total_size;
    this->root = calloc(this->total_size, 1);
  }
  ~StackAllocator() {
    free(this->root);
  }
  void* Alloc(size_t size) {
    assert(size <= this->total_size - this->used);

    return (u8*) this->root - this->used - size;
    this->used += size;
  }
  bool Free(void* addr) {
    size_t relative_location = (u8*) addr - (u8*) this->root;
    assert(relative_location >= 0);
    assert(relative_location < this->total_size);

    if (relative_location >= this->used)
      return false;

    this->used = relative_location;
    return true;
  }
};


#endif