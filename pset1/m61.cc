#define M61_DISABLE 1
#include "m61.hh"
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cinttypes>
#include <cassert>

#include <map>
#include <iterator>

//  METADATA VARIABLES
static size_t glb_sz_metadata   = 8; // how many metadata bytes are prepended
static size_t meta_mdSz_byte    = 0; // where to find size of metadata
static size_t meta_totSz_byte   = 1; // where to find total size of allocation

//  GLOBAL DATA VARIABLES
static unsigned long long glb_cnt_active    = 0;
static unsigned long long glb_sz_active     = 0;

static unsigned long long glb_cnt_total     = 0;
static unsigned long long glb_sz_total      = 0;

static unsigned long long glb_cnt_fails     = 0;
static unsigned long long glb_sz_fails      = 0;

static uintptr_t glb_minHeap_addr;
static uintptr_t glb_maxHeap_addr;

//  ALLOCATION MAP
//  active allocations in the form of <start_address>, <last_address>
// std::map<std::uintptr_t*, std::uintptr_t*> glb_act_allocMap; 
std::map<void*, void*> glb_act_allocMap; 

//  freed allocations in the form of <start_address>, <last_address>
std::map<void*, void*> glb_freed_allocMap;

/// m61_malloc(sz, file, line)
///    Return a pointer to `sz` bytes of newly-allocated dynamic memory.
///    The memory is not initialized. If `sz == 0`, then m61_malloc must
///    return a unique, newly-allocated pointer value. The allocation
///    request was at location `file`:`line`.

void* m61_malloc(size_t sz, const char* file, long line) {
    (void) file, (void) line;   // avoid uninitialized variable warnings
    // Your code here.
    if  ((int)((unsigned)sz + 1 > sz)) {
        ++glb_cnt_active;
        ++glb_cnt_total;

        glb_sz_total += static_cast<unsigned long long>(sz);
        glb_sz_active += static_cast<unsigned long long>(sz);

        //  STORE METADATA AHEAD OF THE ALLOCATED MEMORY
        void* newPtr    =  base_malloc(sz + glb_sz_metadata); 
        char* metaPtr   = reinterpret_cast<char*>(newPtr);  

        *(metaPtr + meta_mdSz_byte)     = glb_sz_metadata; 
        *(metaPtr + meta_totSz_byte)    = sz; 

        // establish start and end of pointer location
        uintptr_t ptrStart  = (uintptr_t)metaPtr; 
        uintptr_t ptrEnd    = (uintptr_t)(metaPtr+sz+glb_sz_metadata); 

        //  UPDATE GLOBAL HEAP INFORMATION
        if (!glb_minHeap_addr || glb_minHeap_addr > (uintptr_t)metaPtr) {
            glb_minHeap_addr = ptrStart; 
        }

        if (!glb_maxHeap_addr || glb_maxHeap_addr < ptrEnd) {
            glb_maxHeap_addr = ptrEnd; 
        }
        
        //  UPDATE MAP OF ALLOCATED MEMORY
        glb_act_allocMap[newPtr] = reinterpret_cast<void*>(ptrEnd); 

        //UPDATE MAP OF CLEARED MEMORY
        std::map<void*, void*>::iterator itr_freed = glb_freed_allocMap.find(newPtr);
        
        if(itr_freed != glb_freed_allocMap.end()) {
            glb_freed_allocMap.erase(itr_freed); 
        }

        return reinterpret_cast<void*>((uintptr_t)metaPtr+glb_sz_metadata);

    } else {
        glb_cnt_fails++;
        glb_sz_fails+=sz;
        return nullptr;
    }
}


/// m61_free(ptr, file, line)
///    Free the memory space pointed to by `ptr`, which must have been
///    returned by a previous call to m61_malloc. If `ptr == NULL`,
///    does nothing. The free was called at location `file`:`line`.

void m61_free(void* ptr, const char* file, long line) {
    (void) file, (void) line;   // avoid uninitialized variable warnings
    
    // Your code here.
    bool isNullPtr      = (ptr==nullptr);
    void* originalPtr   = ptr;
    
    if (!isNullPtr){
        ptr = reinterpret_cast<void*>((uintptr_t)ptr - glb_sz_metadata);
    }

    //  BULID ITERATOR VARIABLES FROM MAPS OF ALLOCATED AND FREED POINTERS
    std::map<void*, void*>::iterator itr_allocated  = glb_act_allocMap.find(ptr);
    std::map<void*, void*>::iterator itr_freed      = glb_freed_allocMap.find(ptr);
    std::map<void*, void*>::iterator itr_loop;

    //  COMPUTE INFORMATION ABOUT THE REQUEST
    bool isWithinHeap   = (glb_minHeap_addr <= (uintptr_t)ptr || glb_maxHeap_addr >= (uintptr_t)ptr);
    bool isAllocated    = (itr_allocated != glb_act_allocMap.end());
    bool isFreed        = (itr_freed != glb_freed_allocMap.end());
    bool isWithinAlloc  = false;

    // CHECK WHETHER A FREE WOULD BE INSIDE OF AN ALLOCATED REGION
    if (!isNullPtr){
        for (itr_loop = glb_act_allocMap.begin(); itr_loop!=glb_act_allocMap.end(); itr_loop++){ // for each allocation
            if (reinterpret_cast<uintptr_t>(ptr) > reinterpret_cast<uintptr_t>(itr_loop->first) // check if greater than lowest
                && reinterpret_cast<uintptr_t>(ptr) < reinterpret_cast<uintptr_t>(itr_loop->second) ) { // && check if lower that highest

                isWithinAlloc = true; // if so, then you're trying to take allocated memory at the wrong place
            }
        }
    }

    if (isFreed){
        printf("MEMORY BUG???: invalid free of pointer %p, double free\n", originalPtr );
        return;
    }
    
    if (!isNullPtr && !isAllocated && !isWithinAlloc){
        printf("MEMORY BUG???: invalid free of pointer %p, not in heap\n", originalPtr);
        return;
    } 

     if (!isNullPtr && isWithinHeap && !isAllocated && !isFreed) {
        printf("MEMORY BUG: %s:%li: invalid free of pointer %p, not allocated\n", file, line, originalPtr);
        return;
    }


    if (isWithinHeap || isNullPtr) {
        if(!isNullPtr){
            //  EXTRACT SIZE META DATA FROM POINTER
            char*   metaPtr         = reinterpret_cast<char*>(ptr);         
            size_t  metaAllocSz     = static_cast<size_t>(*(metaPtr+meta_totSz_byte));    

            //  UPDATE GLOBAL DATA VALUES
            glb_sz_active -= static_cast<unsigned long>(metaAllocSz);   
            --glb_cnt_active;

            //  UPDATE MAP OF ALLOCATED LOCATIONS
            glb_freed_allocMap[ptr] = reinterpret_cast<void*>((uintptr_t)ptr + meta_totSz_byte);    
            glb_act_allocMap.erase(itr_allocated);  

            base_free(ptr);
        }    
    }
}


/// m61_calloc(nmemb, sz, file, line)
///    Return a pointer to newly-allocated dynamic memory big enough to
///    hold an array of `nmemb` elements of `sz` bytes each. If `sz == 0`,
///    then must return a unique, newly-allocated pointer value. Returned
///    memory should be initialized to zero. The allocation request was at
///    location `file`:`line`.

void* m61_calloc(size_t nmemb, size_t sz, const char* file, long line) {
    // Your code here (to fix test014).

    if ((int)((unsigned)nmemb + 1 < nmemb )) {
        ++glb_cnt_fails;
        return nullptr;
    }


    void* ptr = m61_malloc(nmemb * sz, file, line);
    if (ptr) {
        memset(ptr, 0, nmemb * sz);
    }
    return ptr;
}


/// m61_get_statistics(stats)
///    Store the current memory statistics in `*stats`.

void m61_get_statistics(m61_statistics* stats) {
    // Stub: set all statistics to enormous numbers
    memset(stats, 0, sizeof(m61_statistics));
    // Your code here.
    stats->nactive      = glb_cnt_active;
    stats->active_size  = glb_sz_active;

    stats->ntotal       = glb_cnt_total;
    stats->total_size   = glb_sz_total;

    stats->nfail        = glb_cnt_fails;
    stats->fail_size    = glb_sz_fails;

    stats->heap_min     = glb_minHeap_addr;
    stats->heap_max     = glb_maxHeap_addr;
}


/// m61_print_statistics()
///    Print the current memory statistics.

void m61_print_statistics() {
    m61_statistics stats;
    m61_get_statistics(&stats);


    printf("alloc count: active %10llu   total %10llu   fail %10llu\n",
           stats.nactive, stats.ntotal, stats.nfail);
    printf("alloc size:  active %10llu   total %10llu   fail %10llu\n",
           stats.active_size, stats.total_size, stats.fail_size);
}


/// m61_print_leak_report()
///    Print a report of all currently-active allocated blocks of dynamic
///    memory.

void m61_print_leak_report() {
    // Your code here.
}


/// m61_print_heavy_hitter_report()
///    Print a report of heavily-used allocation locations.

void m61_print_heavy_hitter_report() {
    // Your heavy-hitters code here
}
