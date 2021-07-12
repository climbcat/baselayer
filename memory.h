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

struct Entity {
  char* name;
  char* body;
};

void list_insert(MemoryBlock* item, MemoryBlock* after) {
  item->next = after->next;
  after->next->prev = item;
  item->prev = after;
  after->next = item;
}

void list_remove(MemoryBlock* item) {
  item->next->prev = item->prev;
  item->prev->next = item->next;
}

void list_init(MemoryBlock* first) {
  first->next = first;
  first->prev = first;
}

void list_print_sizes(MemoryBlock* from) {
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
    list_init(this->blocks);
    this->num_blocks = 1;
  }
  ~GeneralPurposeAllocator() {
    std::free(this->blocks);
  }

  void* alloc(size_t size) {
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
            list_insert(remainder, at);
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

  bool try_merge_adjacent_blocks(MemoryBlock* a, MemoryBlock* b) {
    if (a->used != false && b->used != false)
        return false;

    MemoryBlock* tmp;
    if (b->next == a) { tmp = a; a = b; b = tmp; }
    else if (a->next != b) return false;

    // a sits right before b
    a->total_size += b->total_size;
    list_remove(b);
    this->num_blocks--;
    this->blocks_merged++;
    return true;
  }

  bool free(void* addr) {
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

    this->try_merge_adjacent_blocks(at->prev, at);

    return true;
  }
};

#endif