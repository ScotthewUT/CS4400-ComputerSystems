/*
 * PLEASE TEST WITH OTHER VERSION SUBMITTED
 *
 * mm_segregated_list - An attempt to organize allocator with an segregated free list.
 * 
 * When I failed to get my explicit list version to perform well, I reworked the structure
 * in attempt to both clean up unreadable code and implement a segregated list.
 *
 * Heap chunks are structured with a 12-byte prologue & 4-byte epilogue.
 * Blocks headers and footer are 4 bytes with size and allocation bit packed together.
 * The 8 bytes following the footer are dedicated to a 'prev' pointer.
 * The 'next' pointer directly follows 'prev'.
 * Minimum block size is 32 bytes to maintain alignment.
 * Six buckets are maintained to hold at the free list size classes.
 *
 * Scott Crowley (u1178178)
 * CS 4400 - Assignment 5 (ver 2)
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

/* Subtracts header & footer sizes from block to give payload size. */
#define GET_PAYLOAD_SIZE(blk_size) ((blk_size) - (HDR_SIZE + FTR_SIZE))

/* Takes a block pointer and returns a pointer to the previous free block. */
#define GET_PREV_FREE(ptr) (GET((char*)(ptr) + HDR_SIZE));

/* Takes a block pointer and returns a pointer to the next free block. */
#define GET_NEXT_FREE(ptr) (GET((char*)(ptr) + HDR_SIZE + sizeof(void*)))

/* Given a pointer to a header, get or set its value */
#define GET(p) (*(size_t *)(p))
#define PUT(p, val) (*(size_t *)(p) = (val))

/* Combine a size and alloc bit */
#define PACK(size, alloc) ((size) | (alloc))

/* Given a header pointer, get the alloc or size */
#define GET_ALLOC(p) (GET(p) & 0x1)
#define GET_SIZE(p) (GET(p) & ~0xF)

/* Helper methods */
void  add_free_block(void* block_p, size_t size);
void  alloc_block(void* block_p);
void* coalesce(void* block_p);
void* find_fit(size_t size);
void* more_heap_please(size_t size);
void* setup_heap(void* heap_p, size_t size);
void  split_block(void* block_p, size_t size);
void  unlink_free_block(void* block_p, int size_class);

/* Global constants */
#define ALIGNMENT 16        // 16-byte aligned
#define PRO_SIZE  12        // Prologue size (bytes)
#define EPI_SIZE   4        // Epilogue size (bytes)
#define HDR_SIZE   4        // Header size (bytes)
#define FTR_SIZE   4        // Footer size (bytes)
#define MIN_SIZE  32        // Minimum block size (bytes)
#define MAX_PAGE  48        // Maximum mem_map request (pages)

/* Global variables */
static size_t page_size;    // Size of the page in bytes
static size_t heap_size;    // Current total heap size
static void* root_32;       // Root of linked-list for size class 32 bytes & less
static void* root_128;      // Class 128  B
static void* root_512;      // Class 512  B
static void* root_2k;       // Class   2 KB
static void* root_8k;       // Class   8 KB
static void* root_big;      // Class  >8 KB


/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    page_size = mem_pagesize();
    heap_size = 0;
    root_32  = NULL;
    root_128 = NULL;
    root_512 = NULL;
    root_2k  = NULL;
    root_8k  = NULL;
    root_big = NULL;
    return 0;
}


/* 
 * mm_malloc - Allocate a block by using bytes from current_avail,
 *     grabbing a new page if necessary.
 */
void *mm_malloc(size_t size)

    void* block_p = find_fit(size);

    split_block(block_p, size);

    alloc_block(block_p);

    void* payload_p = (void*)((char*)block_p + HDR_SIZE);
    return payload_p;
}


/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void* ptr)
{
    //void* block_p = ptr;
    //coalesce(block_p);
}


/*
 * find_fit- Finds the 1st free block large enough to fill request.
 *      If none is found, requests more heap from system.
 */
void* find_fit(size_t size)
{   // TODO: Comments needed...
    void* ptr = NULL;

    switch (size) {
        case 1 ... 32:
            ptr = root_32;
	        while (ptr != NULL) {
		        int blk_size = GET_SIZE(ptr);
		        int pay_size = GET_PAYLOAD_SIZE(blk_size);
                if (pay_size >= size) {
                    unlink_free_block(ptr, 32);
                    return ptr;
                }
		        ptr = GET_NEXT_FREE(ptr);
	        }
        case 33 ... 128:
            ptr = root_128;
            while (ptr != NULL) {
		        int blk_size = GET_SIZE(ptr);
		        int pay_size = GET_PAYLOAD_SIZE(blk_size);
		        if (pay_size >= size) {
                    unlink_free_block(ptr, 128);
                    return ptr;
                }
		        ptr = GET_NEXT_FREE(ptr);
	        }
        case 129 ... 512:
            ptr = root_512;
            while (ptr != NULL) {
		        int blk_size = GET_SIZE(ptr);
		        int pay_size = GET_PAYLOAD_SIZE(blk_size);
		        if (pay_size >= size) {
                    unlink_free_block(ptr, 512);
                    return ptr;
                }
		        ptr = GET_NEXT_FREE(ptr);
	        }
        case 513 ... 2048:
            ptr = root_2k;
            while (ptr != NULL) {
		        int blk_size = GET_SIZE(ptr);
		        int pay_size = GET_PAYLOAD_SIZE(blk_size);
		        if (pay_size >= size) {
                    unlink_free_block(ptr , 2048);
                    return ptr;
                }
		        ptr = GET_NEXT_FREE(ptr);
	        }
        case 2049 ... 8192:
            ptr = root_8k;
            while (ptr != NULL) {
		        int blk_size = GET_SIZE(ptr);
		        int pay_size = GET_PAYLOAD_SIZE(blk_size);
		        if (pay_size >= size) {
                    unlink_free_block(ptr, 8192);
                    return ptr;
                }
		        ptr = GET_NEXT_FREE(ptr);
	        }
        default:
            ptr = root_big;
            while (ptr != NULL) {
		        int blk_size = GET_SIZE(ptr);
		        int pay_size = GET_PAYLOAD_SIZE(blk_size);
		        if (pay_size >= size) {
                    unlink_free_block(ptr, 9999);
                    return ptr;
                }
		        ptr = GET_NEXT_FREE(ptr);
	        }
            break;
    }
    return more_heap_please(size);
}


/*
 * TODO: Throws segfault. Not sure why...
 */
void unlink_free_block(void* block_p, int size_class)
{
    void* ptr;
    void* root;
    void* prev = GET_PREV_FREE(block_p);
    void* next = GET_NEXT_FREE(block_p);

    switch (size_class) {
        case   32:
            root = root_32;
            break;
        case  128:
            root = root_128;
            break;
        case  512:
            root = root_512;
            break;
        case 2048:
            root = root_2k;
            break;
        case 8192:
            root = root_8k;
            break;
        default:
            root = root_big;
            break;
    }

    if (prev != NULL) {
        // Points at previous block's next and rewrites it with this block's next.
        ptr = (void*)((char*)prev + HDR_SIZE + sizeof(void*));
        *((size_t*)ptr) = next;
    }
    else
        root = next;

    if (next != NULL) {
        // Points at next blocks' prev and rewrites it with this block's prev.
        ptr = (void*)((char*)next + HDR_SIZE);
        *((size_t*)ptr) = prev;
    }

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
{   // Points at prologue's footer and sets its value.
    void* ptr = (void*)((char*)heap_p + PRO_SIZE - FTR_SIZE);
    PUT(ptr, PACK(PRO_SIZE, 1));

    // Points at epilogue's header and sets its value.
    ptr = (void*)((char*)heap_p + size - HDR_SIZE);
    PUT(ptr, PACK(EPI_SIZE, 1));

    // Gets the block pointer.
    void* block_p = ((char*)heap_p + PRO_SIZE);

    // Computes block's size and sets its header value.
    int blk_size = size - (PRO_SIZE + EPI_SIZE);
    PUT(block_p, PACK(blk_size, 1));

    // Points at block's prev and nulls it.
    ptr = (void*)((char*)block_p + HDR_SIZE);
    *((size_t*)ptr) = NULL;

    // Points at block's next and nulls it.
    ptr = (void*)((char*)block_p + HDR_SIZE + sizeof(void*));
    *((size_t*)ptr) = NULL;

    return block_p;
}


/*
 * split_block - If there's space enough to split a new free block after
 *      filling request, does so.
 */
void split_block(void* block_p, size_t size)
{   
    void* ptr;
    int new_size;
    int blk_size = GET_SIZE(block_p);

    int min_payload = MIN_SIZE - (HDR_SIZE + FTR_SIZE);
    if (size <= min_payload)
        new_size = MIN_SIZE;
    else
        new_size = ALIGN(size + HDR_SIZE + FTR_SIZE);

    // Not enough size to split of a new block, return this full block.
    if (blk_size - new_size < MIN_SIZE)
        return;

    // Size of the 2nd block.
    int rem_size = blk_size - new_size;
    
    // Sets reserved block's header value.
    PUT(block_p, PACK(new_size, 1));

    // Points at reserved block's footer and sets its value.
    ptr = (void*)((char*)block_p + new_size - FTR_SIZE);
    PUT(ptr, PACK(new_size, 1));

    // Points at split block's header and sets its value.
    void* split_block_p = (int*)((char*)block_p + new_size);
    PUT(split_block_p, PACK(rem_size, 0));

    // Points at split block's footer and sets its value.
    ptr = (void*)((char*)split_block_p + rem_size - FTR_SIZE);
    PUT(ptr, PACK(rem_size, 0));

    add_free_block(split_block_p, rem_size);
}


void add_free_block(void* block_p, size_t size)
{   // Points at block's prev and nulls it.
    size_t* prev = (void*)((char*)block_p + HDR_SIZE);
    *prev = NULL;

    // Points at block's next.
    size_t* next = (void*)((char*)prev + sizeof(void*));

    // Pick the appropriate size class root and splice in the free block.
    switch (size) {
        case 1 ... 32:
            *next = (size_t*)root_32;
            root_32 = block_p;
            break;
        case 33 ... 128:
            *next = (size_t*)root_128;
            root_128 = block_p;
            break;
        case 129 ... 512:
            *next = (size_t*)root_512;
            root_512 = block_p;
            break;
        case 513 ... 2048:
            *next = (size_t*)root_2k;
            root_2k = block_p;
            break;
        case 2049 ... 8192:
            *next = (size_t*)root_8k;
            root_8k = block_p;
            break;
        default:
            *next = (size_t*)root_big;
            root_big = block_p;
            break;
    }
}


/*
 * alloc_block - Marks the block allocated.
 */
void alloc_block(void* block_p)
{
    void* ptr;
    int blk_size = GET_SIZE(block_p);

    // Set the block's header value.
    PUT(block_p, PACK(blk_size, 1));

    // Points at block's footer and sets its value.
    ptr = (void*)((char*)block_p + blk_size - FTR_SIZE);
    PUT(ptr, PACK(blk_size, 1));
}


/*
 * TODO: Needs to be reworked for the new structure...
 */
//void* coalesce(void* block_p)
//{   // TODO - Clean up the Comments
//    void* coblk_p;
//    unsigned int coa_size;
//    unsigned int blk_size = GET_BLOCK_SIZE(block_p);
//
//    void* ptr = (char*)block_p - 1; // Points at preceding block's footer status
//    char stat = *((char*)ptr);      // Gets status of preceding block
//
//    
//
//    if (stat == 'F') {
//        ptr = (void*)((char*)block_p - FTR_SIZE);   // Points at preceding block's size in footer
//        coa_size = *((int*)ptr);                     // Gets size of preceding block
//        coblk_p = (void*)((char*)block_p - coa_size);// Points at preceding block
//        ptr = (void*)((char*)coblk_p + 1);           // Points at preceding block's size in header
//        *((int*)ptr) = coa_size + blk_size;          // Rewrites with combined size
//        ptr = (void*)((char*)block_p + blk_size - FTR_SIZE); // Points at this block's size in footer
//        blk_size = coa_size + blk_size;              // Combines block sizes
//        *((int*)ptr) = blk_size;                     // Rewrites with combined size
//        block_p = coblk_p;                           // Moves this block's pointer to preceding block
//        
//    }
//    coblk_p = (void*)((char*)block_p + blk_size);        // Points at successor
//    stat = *((char*)ptr);                                // Gets status of successor
//
//    if (stat == 'F') {
//        ptr = (void*)((char*)coblk_p + 1);             // Points at successor's size in header
//        coa_size = *((int*)ptr);                     // Gets size of successor
//        ptr = (void*)((char*)coblk_p + coa_size - FTR_SIZE); // Points at successor's size in footer
//        blk_size = coa_size + blk_size;              // Combines block sizes
//        *((int*)ptr) = blk_size;                     // Rewrites with combined size
//        *((int*)block_p) = blk_size;                 // Rewrites this block's header with combined size
//
//        size_t* prev = (size_t*)((char*)block_p + HDR_SIZE);                // Points at this block's prev
//        size_t* next = (size_t*)((char*)prev + sizeof(void*)); // Points at this block's next
//
//        *next = free_list_root; // This block's next now points at former root
//        *prev = NULL;           // This is the new root
//        free_list_root = block_p; // This is the new root
//
//        prev = GET_PREV_FREE(coblk_p);    // Points at successor's previous block
//        next = GET_NEXT_FREE(coblk_p);    // Points at successor's next block
//
//        ptr = (size_t*)((char*)prev + HDR_SIZE + sizeof(void*)); // Points at successor's previous block's next
//        *((size_t*)ptr) = next; // Rewrites successor's previous block's next with successor's next block
//
//        ptr = (size_t*)((char*)next + HDR_SIZE); // Points at successor's next block's prev
//        *((size_t*)ptr) = prev; // Rewrites successor's next block's prev with successor's previous block
//    }
//
//    ptr = (char*)block_p - 1;  // Points at preceding block's footer status
//    stat = *((char*)ptr);      // Gets status of preceding block
//    if (stat == 'P') {
//        ptr = (void*)((char*)block_p + blk_size);            // Points at successor's status
//        stat = *((char*)ptr);                                // Gets status of successor
//        if (stat == 'E') {
//            free_list_root = GET_NEXT_FREE(block_p);
//            ptr = (void*)((char*)block_p - EPI_SIZE);
//            size_t size = EPI_SIZE + blk_size + PRO_SIZE;
//            mem_unmap(ptr, size);
//        }
//    }
//
//    return block_p;
//}
