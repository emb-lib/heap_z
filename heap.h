//*-----------------------------------------------------------------------------
//*
//*     Heap Manager by Zltigo
//* 
//*     C++ design by Sergey A. Borshch
//*
//*     Description: Lightweight and fast free memory manager suitable 
//*                  for embedded applications
//* 
//*     The code is distributed under the MIT license terms:
//* 
//*     Permission is hereby granted, free of charge, to any person
//*     obtaining  a copy of this software and associated documentation
//*     files (the "Software"), to deal in the Software without restriction,
//*     including without limitation the rights to use, copy, modify, merge,
//*     publish, distribute, sublicense, and/or sell copies of the Software,
//*     and to permit persons to whom the Software is furnished to do so,
//*     subject to the following conditions:
//*
//*     The above copyright notice and this permission notice shall be included
//*     in all copies or substantial portions of the Software.
//*
//*     THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//*     EXPRESS  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//*     MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//*     IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//*     CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//*     TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH
//*     THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//*
//*-----------------------------------------------------------------------------
#ifndef HEAP_H__
#define HEAP_H__

//------------------------------------------------------------------------------
//  Terms
//  ~~~~~
//           
//    Chunk: aggregate data structure consists of pair MCB:ASA (see below). 
//           sizeof(Chunk) = sizeof(MCB) + sizeof(ASA)
// 
//    MCB:   Memory Control Block. Data structure for support of chunk 
//           management operations.
// 
//    ASA:   Allocated Storage Area. Part of chunk used as properly aligned
//           allocation item for application. Heap manager returns a pointer 
//           to ASA when allocation takes place.
// 
// 
// 
//  Heap Structure
//  ~~~~~~~~~~~~~~
//  start-+
//        |
//        |  +----------------------------------------------+
//        V  V                                              |
//   +--{MCB_0:ASA_0}<==>{MCB_1:ASA_1}<=...=>{MCB_N:ASA_N}--+
//   |     ^                             ^
//   +-----+                             |
//                                       |
//  freemem------------------------------+
//
//  mcb.next of the last MCB always points to the first MCB (circular pattern).
//  mcb.prev of the first MCB points to itself.
//  start points to first MCB
//  freemem points to first free MCB
//------------------------------------------------------------------------------


#include <stdint.h>
#include <stddef.h>
#include "heapcfg.h"


//------------------------------------------------------------------------------
template <typename guard>
class scope_guard
{
public:
    scope_guard(guard& g): gd(g) { gd.lock(); }
    ~scope_guard() { gd.unlock(); }
private:
    guard & gd;
};

//------------------------------------------------------------------------------
template <size_t size_bytes>
struct pool
{
    int Pool[size_bytes/sizeof(int)];
};

//------------------------------------------------------------------------------
template <typename guard>
class heap 
{
public:
    // Heap initialization
    template<size_t size_items>
    heap(int (& pool)[size_items]);

    template<size_t size_bytes>
    heap(pool<size_bytes> & pool_obj);

    heap(int * pool, int size_bytes);

    // Attach separate memory pool to the heap
    void add(void * pool, int size );

    // Allocate 'size' bytes of memory in heap pool and returns
    // the pointer to this memory. In case of lack of memory the
    // function returns NULL.
    void *malloc( size_t size );

    //--------------------------------------------------------------------------
    // Deallocates previously allocated memory that is pointed by 'ptr'. If the 
    // ponter 'ptr' contains address of memory that was not previously allocated 
    // or 'ptr' has  value '0' then nothing does happen (but there is a possibility 
    // to raise an exception)
    void free( void *ptr );

    //--------------------------------------------------------------------------
    // Info about count and sizes of free and allocated memory chunks
    //--------------------------------------------------------------------------
    struct summary
    {
        struct info
        {
            size_t Blocks;
            size_t Block_max_size;
            size_t Size;
        }
        Used, Free;

    };
    summary info();

private:
    // Scan through all free memory chunks to find out
    // the chunk which satisfy to required size
    static bool   const USE_FULL_SCAN = 1;
    static size_t const HEAP_ALIGN    = sizeof(int);

    // Memory Control Block (MCB)
    //--------------------------------------------------------------------------
    struct mcb
    {
        enum mark
        {
            FREE = 0,
            ALLOCATED,
        };
        struct type_size
        {
            size_t type:8;
            size_t size:24;
        };

        mcb *next;         // pointer to the next MCB                                             
                           // mcb.next of the last MCB always points to                           
                           // the first MCB                                                       
        mcb *prev;         // pointer to previous MCB                                             
                           // the first MCB always pounts to itself                               
                                                                                                  
        type_size ts;      // ASA size (bytes)
                           // ASA that is controlled by MCB is loacated 
                           // directly after the MCB          

        // split current memory chunk. Returns the pointer to new MCB
        mcb * split(size_t size, mcb * start);

        // join current memory chunk with the next
        void merge_with_next(mcb * start);

        void * pool() { return this + 1; }
    };

    void init(mcb * pstart, size_t size_bytes);
    //--------------------------------------------------------------------------
    // Heap descriptors 
    //--------------------------------------------------------------------------
    mcb *start;            // heap begin pointer (points to the first MCB) 
                           
    mcb *freemem;          // pointer to the first free MCB      
                           
    guard Guard;           // thread-safe support 
                           
};

//------------------------------------------------------------------------------
template<typename guard>
template<size_t size_items>
heap<guard>::heap(int (& pool)[size_items])
    : start((mcb *)pool)
    , freemem((mcb *)pool)
    , Guard()
{
    init(start, sizeof(pool));
}

//------------------------------------------------------------------------------
template<typename guard>
heap<guard>::heap(int * pool, int size_bytes)
    : start((mcb *)pool)
    , freemem((mcb *)pool)
    , Guard()
{
    init(start, size_bytes);
}

//------------------------------------------------------------------------------
template<typename guard>
template<size_t size_bytes>
heap<guard>::heap(pool<size_bytes> & pool_obj)
    : start((mcb *)pool_obj.Pool)
    , freemem((mcb *)pool_obj.Pool)
    , Mutex()
{
    init(start, sizeof(pool_obj));
}

//------------------------------------------------------------------------------
template<typename guard>
void heap<guard>::init(mcb * pstart, size_t size_bytes)
{
    // Circular pattern 
    pstart->next = pstart;

    // Pointer to previous MCB points to itself
    pstart->prev = pstart;

    // ASA size
    pstart->ts.size = size_bytes - sizeof(mcb);
    
    // Set memory chunk free
    pstart->ts.type = mcb::FREE;

    // After initialization, heap is one free memory chunk with 
    // ASA size = sizeof(heap) - sizeof(MCB)
}

//------------------------------------------------------------------------------
template<typename guard>
typename heap<guard>::summary  heap<guard>::info()
{
    summary Result =
    {
        { 0, 0, 0 },
        { 0, 0, 0 }
    };

    scope_guard<guard> ScopeGuard(Guard);   // protect the following code from asyncronous access
    mcb *pBlock = freemem;
    do
    {
        typename summary::info * pInfo = pBlock->ts.type == mcb::FREE ? &Result.Free : &Result.Used;
        ++pInfo->Blocks;
        pInfo->Size += pBlock->ts.size;
        if(pInfo->Block_max_size < pBlock->ts.size)
            pInfo->Block_max_size = pBlock->ts.size;
        pBlock = pBlock->next;
    }
    while(pBlock != start);
    return Result;
}
//------------------------------------------------------------------------------
template<typename guard>
void heap<guard>::mcb::merge_with_next(mcb * start)
{
    // Check Next MCB
    mcb* other = next;
    // Join current and next chunks
    ts.size = ts.size + other->ts.size;
    other = next = other->next;
    // After joining chunks, if the next chunk is not the last 
    // then set the chunk's mcb.prev to current chunk
    if( other != start )
        other->prev = this;

}
//------------------------------------------------------------------------------
template<typename guard>
void heap<guard>::free(void *pool )
{
    // All pointer values should be checked to hit in RAM, otherwise an exception can occur
    
    // Check pointer alignment
    if( !pool || ((uintptr_t)pool & (HEAP_ALIGN - 1)))
        return;

    mcb *xptr;
    mcb *tptr = (mcb *)pool - 1;

    scope_guard<guard> ScopeGuard(Guard);    // protect the following code from asyncronous access
    
    // Crosscheck for valid values
    xptr = tptr->prev;
    if( (xptr != tptr && xptr->next != tptr) || pool < start )
        return;

    // Valid pointer present ------------------------------------------------
    tptr->ts.type = mcb::FREE;          // Mark as "free"
    // Check Next MCB
    xptr = tptr->next;
    
    // If the next chunk is free and the chunk is not the first
    // in the heap
    if( xptr->ts.type == mcb::FREE && xptr != start )
    {
        // Join current (tptr) and next (xptr) chunks
        tptr->merge_with_next(start);
    }
    // Check previous MCB
    xptr = tptr->prev;
    // If previous chunk is free and current chunk is not
    // first in the heap...
    if( xptr->ts.type == mcb::FREE && tptr != start )
    {
        // Join current (tptr) and previous (xptr) chunks
        xptr->merge_with_next(start);
        tptr = xptr;            // tprt always point to freed chunk
    }
    // Set heap->freem for more efficient search
    if( tptr < freemem )        // Is freed chunk located berore the fisrt one that was considered free?
        freemem = tptr;         // Update free chunk pointer
}
//------------------------------------------------------------------------------
template<typename guard>
void heap<guard>::add(void * pool, int size )
{
    mcb *xptr = (mcb *)pool;
    mcb *tptr = freemem;
    
    // Init MCB in new chunk
    xptr->next = tptr;
    xptr->prev = xptr;                   // the first mcb in pool always points to itself
    xptr->ts.size = size - sizeof(mcb);
    xptr->ts.type = mcb::FREE;
    // Reinit Primary MCB
    tptr->next = xptr;
}
//------------------------------------------------------------------------------
template<typename guard>
typename heap<guard>::mcb * heap<guard>::mcb::split(size_t size, heap<guard>::mcb * start)
{
    uintptr_t new_mcb_addr = (uintptr_t)this + size;
    mcb *new_mcb = (mcb *)new_mcb_addr;
    new_mcb->next = next;
    new_mcb->prev = this;
    new_mcb->ts.size = ( ts.size - size );
    new_mcb->ts.type = FREE;

    // Reinit current MCB
    next = new_mcb;
    ts.size = size;
    ts.type = ALLOCATED;  // Mark block as used

    // If the next MCB is not last then mcb.prev of the following MCB
    // must point to allocated (xptf) MCB
    if( new_mcb->next != start )
        ( new_mcb->next )->prev = new_mcb;
    return new_mcb;
}

//------------------------------------------------------------------------------
template<typename guard>
void * heap<guard>::malloc( size_t size )
{
    // add mcb size and round up to HEAP_ALIGN
    size = (size + sizeof(mcb) + ( HEAP_ALIGN - 1 )) & ~( HEAP_ALIGN - 1 );

    mcb *xptr;
    if(USE_FULL_SCAN)
        xptr = 0;

    void *Allocated;
    size_t free_cnt = 0;

    scope_guard<guard> Guard(Guard);                                  // protect the following code from asyncronous access
    mcb *tptr = freemem;                                              // Scan begins from the first free MCB
    for(;;)
    {
        if( tptr->ts.type == mcb::FREE )
        {
            if( !USE_FULL_SCAN )
                ++free_cnt;
            if( tptr->ts.size >= size                                 // Current free ASA size is equal to required size or
                 && tptr->ts.size <= size + sizeof(mcb) + HEAP_ALIGN) // current free ASA size is greater then required size
                                                                      // and the rest (after splitting) of current chunk
                                                                      // is large enough to allocate MCB + one allocation unit.
            {
                tptr->ts.type = mcb::ALLOCATED;                       // Allocate the chunk
                Allocated = tptr->pool();
                if( USE_FULL_SCAN )
                    ++free_cnt;
                break;
            }
            else
            {
                if( USE_FULL_SCAN )
                {
                    if( xptr == NULL )
                    {
                        if( tptr->ts.size >= size)                    // Is memory chunk large enough to allocate MCB and 
                            xptr = tptr;                              // required ammount of memory as ASA?
                        ++free_cnt;
                    }
                }
                else if( tptr->ts.size >= size )                      // Is memory chunk large enough to allocate MCB and   
                {                                                     // required ammount of memory as ASA?                 
                    // Create new free MCB in parent's MCB tail
                    xptr = tptr->split(size, start);
                    Allocated = tptr->pool();
                    break;
                }
            }
        }

        tptr = tptr->next;                                            // Get ptr to next MCB
        if( tptr == start )                                           // End of heap?
        {
            if( USE_FULL_SCAN && xptr != 0 )
            {
                tptr = xptr;
                // Create new free MCB in parent's MCB tail
                xptr = tptr->split(size, start);
                Allocated = tptr->pool();
                break;
            }
            else
            {
                Allocated = 0;                                        // No Memory
                break;
            }
        }
    }

    if( ( free_cnt == 1 )&&( Allocated ) )          // Is the first free chunk has been allocated?
        freemem = tptr->next;                       // Set 'first free chunk pointer' to the MCB of the next chunk
                                                    // because either the chunk is free or, at least, it is closer to
                                                    // the next free chunk
    return Allocated;
}
//------------------------------------------------------------------------------

extern heap<heap_guard> Heap;
//------------------------------------------------------------------------------
#endif  // HEAP_H__

