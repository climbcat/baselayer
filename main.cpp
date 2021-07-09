#include <cstdint>
#include <cstdlib>
#include <cstring>

#include <iostream>
#include <assert.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

struct MemBlockHeader {
  size_t user_size;
  size_t total_size;
  void* memblock;
};

struct Entity {
  char* name;
  char* body;
};

struct ListHeader {
  ListHeader* next = NULL;
  ListHeader* prev = NULL;
  size_t total_size = 0;
};

void list_insert(ListHeader* item, ListHeader* after) {
  item->next = after->next;
  after->next->prev = item;
  item->prev = after;
  after->next = item;
}
void list_remove(ListHeader* item) {
  item->next->prev = item->prev;
  item->prev->next = item->next;
}
void list_init(ListHeader* first) {
  first->next = first;
  first->prev = first;
}

void list_print_sizes(ListHeader* from) {
  ListHeader* itm = from->next;
  u64 i = 0;
  while (itm != from) {
    std::cout << "element #" << i << " size=" <<  itm->total_size << std::endl;
    itm = itm->next;
    ++i;
  }
}

class GeneralPurposeAllocator {
public:
  void* rootblock;
  ListHeader* free_list_root;
  ListHeader* used_list_root;
  size_t max_size;
  size_t list_size;
  size_t memblock_minsize;

  GeneralPurposeAllocator(size_t max_size) {
    this->max_size = max_size;
    this->memblock_minsize = sizeof(ListHeader);

    this->free_list_root = (ListHeader*) calloc(max_size + 2 * sizeof(ListHeader), 1);
    list_init(this->free_list_root);
    this->used_list_root = this->free_list_root + sizeof(ListHeader);
    list_init(this->used_list_root);
    this->rootblock = this->free_list_root + 2 * sizeof(ListHeader);

    ListHeader* first = (ListHeader*) this->rootblock;
    first->total_size = this->max_size;
    list_insert((ListHeader*) first, this->free_list_root);
    this->list_size = 2;
  }
  ~GeneralPurposeAllocator() {
    free(rootblock);
  }

  void* alloc(size_t size) {
    void* memloc = NULL;
    size_t block_size = size + sizeof(MemBlockHeader);

    ListHeader* at = this->free_list_root;
    u32 idx = 0;
    size_t freeblock_size;
    while (idx < this->list_size) {
      freeblock_size = at->total_size;
      if (freeblock_size >= block_size) {

        if (freeblock_size > block_size + this->memblock_minsize) {
          ListHeader* remains_free = (ListHeader*) ((u8*) at + block_size);
          remains_free->total_size = freeblock_size - block_size;
          list_insert(remains_free, at);
          this->list_size++;
        }

        memloc = (MemBlockHeader*) at;
        list_remove(at);
        this->list_size--;
        memset(at, sizeof(ListHeader), 0);

        break;
      }

      at = at->next;
      ++idx;
    }

    assert(memloc != NULL);

    MemBlockHeader* memblock_header = (MemBlockHeader*) memloc;
    memblock_header->user_size = size;
    memblock_header->total_size = size + sizeof(MemBlockHeader);
    memblock_header->memblock = (u8*) memloc + sizeof(MemBlockHeader);
    return memblock_header->memblock;
  }

  void insert_free_block(size_t size, ListHeader* block, ListHeader* after) {
    list_insert(block, after);
    this->list_size++;
    block->total_size = size;
  }
  size_t get_memblock_total_size_and_memset(void* addr) {
    MemBlockHeader* memblock_header = (MemBlockHeader*) (u8*) addr - sizeof(MemBlockHeader);
    size_t total_size = memblock_header->total_size;
    memset(addr, sizeof(MemBlockHeader), 0);
    return total_size;
  }

  void free(void* addr) {
    size_t rel_loc = (u8*) addr - (u8*) this->rootblock;
    assert(rel_loc >= 0);
    assert(rel_loc < this->max_size);

    // free block header
    size_t size = this->get_memblock_total_size_and_memset(addr);
    ListHeader* freed = (ListHeader*) addr;
    freed->total_size = size;

    // where to insert freed
    ListHeader* insert_after = this->free_list_root;
    ListHeader* at = this->free_list_root;
    u32 idx = 0;
    while (idx < this->list_size) {
      size_t diff = ((u8*) at - (u8*) this->rootblock) - rel_loc;

      if (diff > 0) {
        size = diff;
        insert_after = at->prev;
        break;
      }

      at = at->next;
      ++idx;
    }
    list_insert((ListHeader*) freed, insert_after);
    this->list_size++;

    // TODO: merge with prev and / or next block
  }
};


void test(GeneralPurposeAllocator* alloc) {

  const char* s01 = "en spændende test string";
  const char* s02 = "entity test string body indhold...";

  Entity my_entity;
  my_entity.name = (char*) alloc->alloc(strlen(s01));


  my_entity.body = (char*) alloc->alloc(strlen(s02));

  list_print_sizes(alloc->free_list_root);
  
  strcpy(my_entity.name, s01);
  strcpy(my_entity.body, s02);
}

int main (int argc, char **argv) {
  GeneralPurposeAllocator alloc(10000);


  test(&alloc);

  return 0;
}
