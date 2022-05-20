/*
 * PLEASE TEST WITH THIS VERSION.
 *
 * mm_explicit_list - An attempt to organize allocator with an explicit free list.
 * 
 * Heap chunks are structured with a 11-byte prologue & 1-byte epilogue.
 * Blocks headers are 5 bytes, a 1-byte status char followed by 4-byte size.
 * The 8 bytes following the footer are dedicated to a 'prev' pointer.
 * The 'next' pointer directly follows 'prev'.
 * Block footers are 5 bytes as well, but with 4-byte size followed by 1-byte status.
 * Minimum block size is 32 bytes to maintain alignment.
 *
 * Scott Crowley (u1178178)
 * CS 4400 - Assignment 5
 * 16 April 2020
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/* Rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~(ALIGNMENT-1))

/* Rounds up to the nearest multiple of mem_pagesize() */
#define PAGE_ALIGN(size) (((size) + (page_size - 1)) & ~(page_size - 1))

/* Gives the maximum allowed request for mem_map (1MB assuming 4KB page sizes). */
#define MAX_REQ_SIZE() (MAX_PAGE * page_size)

/* Given a mm_malloc request size, gives the mininimum request size for mem_map. */
#define MIN_REQ_SIZE(size) (PAGE_ALIGN(size + PRO_SIZE + HDR_SIZE + FTR_SIZE + EPI_SIZE))

/* Takes a block pointer and returns the size of the block. */
#define GET_BLOCK_SIZE(ptr) (*((int*)((char*)(ptr) + 1)))

/* Subtracts header & footer sizes from block to give payload size. */
#define GET_PAYLOAD_SIZE(block) ((block) - (HDR_SIZE + FTR_SIZE))

/* Takes a block pointer and returns a pointer to the previous free block. */
#define GET_PREV_FREE(ptr) (*((size_t*)((char*)(ptr) + HDR_SIZE)));

/* Takes a block pointer and returns a pointer to the next free block. */
#define GET_NEXT_FREE(ptr) (*((size_t*)((char*)(ptr) + HDR_SIZE + sizeof(void*))))

/* Helper methods */
void* find_fit(size_t size);
void* more_heap_please(size_t size);
void* setup_heap(void* heap_p, size_t size);
void  split_block(void* block_p, size_t size);
void  alloc_block(void* block_p);
void* coalesce(void* block_p);


/* Global constants */
#define ALIGNMENT 16            // 16-byte aligned
#define PRO_SIZE  11            // Prologue size (bytes)
#define EPI_SIZE   1            // Epilogue size (bytes)
#define HDR_SIZE   5            // Header size (bytes)
#define FTR_SIZE   5            // Footer size (bytes)
#define MIN_SIZE  32            // Minimum block size (bytes)
#define MAX_PAGE  48            // Maximum mem_map request (pages)

/* Global variables */
static size_t page_size;        // Size of the page in bytes
static size_t heap_size;        // Current total heap size
static void* free_list_root;    // Points at the 1st node in the free list


/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    page_size = mem_pagesize();
    heap_size = 0;
    free_list_root = NULL;
    return 0;
}


/* 
 * mm_malloc - Allocate a block by using bytes from current_avail,
 *     grabbing a new page if necessary.
 */
void *mm_malloc(size_t size)
{   
    void* block_p = find_fit(size);

    split_block(block_p, size);
    alloc_block(block_p);

    void* payload_p = (void*)((char*)block_p + HDR_SIZE);
    return payload_p;
}


/*
 * mm_free - Simply calls the coalesce method for now...
 */
void mm_free(void* ptr)
{
    void* block_p = ptr;
    coalesce(block_p);
}


/*
 * find_fit- Finds the 1st free block large enough to fill request.
 *      If none is found, requests more heap from system.
 */
void* find_fit(size_t size)
{
    void* ptr = free_list_root;
    while (ptr != NULL) {
        unsigned int blk_size = GET_BLOCK_SIZE(ptr);
        unsigned int pay_size = GET_PAYLOAD_SIZE(blk_size);
        if (pay_size >= size)
            return ptr;
        ptr = GET_NEXT_FREE(ptr);
    }
    return more_heap_please(size);
}


/*
 * more_heap_please - Calls memlib.mem_map to request more memory.
 */
void* more_heap_please(size_t size)
{
    size_t call_size = heap_size;           // Double heap size.
    size_t  min_size = MIN_REQ_SIZE(size);  // Min heap to fit user request.
    size_t  max_size = MAX_REQ_SIZE();      // Most memory to ask mem_map for.

    if (call_size < min_size * 2)
        call_size = min_size * 2;
    if (call_size > max_size)
        call_size = max_size;

    void* heap_p = mem_map(call_size);
    heap_size = heap_size + call_size;
    return setup_heap(heap_p, call_size);
}


/*
 * setup_heap - Initializes new heap with prologue, epilogue, and free block.
 */
void* setup_heap(void* heap_p, size_t size)
{
    char* pro_status_p = (char*)heap_p + PRO_SIZE - 1;
    *pro_status_p = 'P';

    char* epilogue_p = (char*)heap_p + size - EPI_SIZE;
    *epilogue_p = 'E';

    void* block_p = (char*)heap_p + PRO_SIZE;
    size_t* prev  = (char*)block_p + HDR_SIZE;
    *prev = NULL;

    size_t* next = (size_t*)((char*)prev + sizeof(size_t*));
    *next = free_list_root;
    free_list_root = block_p;

    int* hdr_size_p = (int*)((char*)block_p + 1);
    *hdr_size_p = size - (PRO_SIZE + EPI_SIZE);

    return block_p;
}


/*
 * split_block - If there's space enough to split a new free block after
 *      filling request, does so.
 */
void split_block(void* block_p, size_t size)
{
    int new_size;
    int min_payload = MIN_SIZE - (HDR_SIZE + FTR_SIZE);
    if (size <= min_payload)
        new_size = MIN_SIZE;
    else
        new_size = ALIGN(size + HDR_SIZE + FTR_SIZE);

    int blk_size = GET_BLOCK_SIZE(block_p);
    if (blk_size - new_size < MIN_SIZE) {           // Not enough space for a split block.
        int* ptr = (int*)((char*)block_p + 1);      // Points at block's size in header.
        *ptr = blk_size;                            // Writes in block size.
        ptr = (int*)((char*)ptr + blk_size - (FTR_SIZE + 1));   // Points at block's size in footer.
        *ptr = blk_size;                                        // Writes in block size.
        return;
    }
    int rem_size = blk_size - new_size;             // Size of the 2nd block.
    
    int* ptr = (int*)((char*)block_p + 1);          // Points at 1st block's size in header.
    *ptr = new_size;                                // Writes in 1st block's size.

    ptr = (int*)((char*)ptr + new_size - (FTR_SIZE + 1));       // Points at 1st block's size in footer.
    *ptr = new_size;                                            // Writes in 1st block's size.

    void* block2_p = (int*)((char*)ptr + FTR_SIZE); // Points at 2nd block.
    *((char*)ptr) = 'F';                            // Sets 2nd block to 'F' for free.

    ptr = (int*)((char*)ptr + FTR_SIZE + 1);        // Points at 2nd block's size in header.
    *ptr = rem_size;                                // Writes in 2nd block's size.

    ptr = (int*)((char*)block2_p + HDR_SIZE);       // Points at 2nd block's prev.
    *((size_t*)ptr) = block_p;                      // Sets 2nd block's prev to 1st block addr.

    ptr = (int*)((char*)ptr + sizeof(void*));       // Points at 2nd block's next.
    *((size_t*)ptr) = GET_NEXT_FREE(block_p);       // Copies 1st block's next into 2nd block's next.

    ptr = (int*)((char*)block_p + HDR_SIZE + sizeof(void*));    // Points at 1st block's next.
    *((size_t*)ptr) = block2_p;                                 // Copies 2nd block's addr into 1st block's next.

    ptr = (int*)((char*)block2_p + rem_size - FTR_SIZE);        // Points at 2nd block's size in footer.
    *ptr = rem_size;                                            // Writes in 2nd block's size.

    ptr = (int*)((char*)ptr + sizeof(int));         // Points at 2nd block's status in footer.
    *((char*)ptr) = 'F';                            // Sets 2nd block to 'F' for free.
}


/*
 * alloc_block - Marks the block allocated and removes it from free list.
 */
void alloc_block(void* block_p)
{
    *((char*)block_p) = 'A';
    char* status_p = (char*)block_p + GET_BLOCK_SIZE(block_p) - 1;
    *status_p = 'A';

    void* prev = GET_PREV_FREE(block_p);
    void* next = GET_NEXT_FREE(block_p);

    if (prev != NULL) {
        void* ptr = (char*)prev + HDR_SIZE + sizeof(void*);
        *((size_t*)ptr) = next;
    }
    else
        free_list_root = next;

    if (next != NULL) {
        void* ptr = (char*)next + HDR_SIZE;
        *((size_t*)ptr) = prev;
    }
}


/*
 * Attempts to coalesce recently freed blocks.
 * More debugging here is necessary...
 */
void* coalesce(void* block_p)
{   // TODO - Clean up the Comments
    void* coblk_p;
    unsigned int coa_size;
    unsigned int blk_size = GET_BLOCK_SIZE(block_p);

    void* ptr = (char*)block_p - 1; // Points at preceding block's footer status
    char stat = *((char*)ptr);      // Gets status of preceding block

    

    if (stat == 'F') {
        ptr = (void*)((char*)block_p - FTR_SIZE);   // Points at preceding block's size in footer
        coa_size = *((int*)ptr);                     // Gets size of preceding block
        coblk_p = (void*)((char*)block_p - coa_size);// Points at preceding block
        ptr = (void*)((char*)coblk_p + 1);           // Points at preceding block's size in header
        *((int*)ptr) = coa_size + blk_size;          // Rewrites with combined size
        ptr = (void*)((char*)block_p + blk_size - FTR_SIZE); // Points at this block's size in footer
        blk_size = coa_size + blk_size;              // Combines block sizes
        *((int*)ptr) = blk_size;                     // Rewrites with combined size
        block_p = coblk_p;                           // Moves this block's pointer to preceding block
        
    }
    coblk_p = (void*)((char*)block_p + blk_size);        // Points at successor
    stat = *((char*)ptr);                                // Gets status of successor

    if (stat == 'F') {
        ptr = (void*)((char*)coblk_p + 1);             // Points at successor's size in header
        coa_size = *((int*)ptr);                     // Gets size of successor
        ptr = (void*)((char*)coblk_p + coa_size - FTR_SIZE); // Points at successor's size in footer
        blk_size = coa_size + blk_size;              // Combines block sizes
        *((int*)ptr) = blk_size;                     // Rewrites with combined size
        *((int*)block_p) = blk_size;                 // Rewrites this block's header with combined size

        size_t* prev = (size_t*)((char*)block_p + HDR_SIZE);                // Points at this block's prev
        size_t* next = (size_t*)((char*)prev + sizeof(void*)); // Points at this block's next

        *next = free_list_root; // This block's next now points at former root
        *prev = NULL;           // This is the new root
        free_list_root = block_p; // This is the new root

        prev = GET_PREV_FREE(coblk_p);    // Points at successor's previous block
        next = GET_NEXT_FREE(coblk_p);    // Points at successor's next block

        ptr = (size_t*)((char*)prev + HDR_SIZE + sizeof(void*)); // Points at successor's previous block's next
        *((size_t*)ptr) = next; // Rewrites successor's previous block's next with successor's next block

        ptr = (size_t*)((char*)next + HDR_SIZE); // Points at successor's next block's prev
        *((size_t*)ptr) = prev; // Rewrites successor's next block's prev with successor's previous block
    }

    ptr = (char*)block_p - 1;  // Points at preceding block's footer status
    stat = *((char*)ptr);      // Gets status of preceding block
    if (stat == 'P') {
        ptr = (void*)((char*)block_p + blk_size);            // Points at successor's status
        stat = *((char*)ptr);                                // Gets status of successor
        if (stat == 'E') {
            free_list_root = GET_NEXT_FREE(block_p);
            ptr = (void*)((char*)block_p - EPI_SIZE);
            size_t size = EPI_SIZE + blk_size + PRO_SIZE;
            mem_unmap(ptr, size);
        }
    }
    return block_p;
}
