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

//----------------------------------------------------------------------------
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
// 
// {MCB_0:ASA_0}{MCB_1:ASA_1}...{MCB_N:ASA_N}
// 
//  mcb.next of the last MCB always points to the first MCB (circular pattern).
//  mcb.prev of the first MCB points to itself.
//----------------------------------------------------------------------------


#include <stdint.h>
#include <stddef.h>
#include "heapcfg.h"


//------------------------------------------------------------------------------
template <typename mutex>
class guard
{
public:
    guard(mutex& m): mx(m) { mx.lock(); }
    ~guard() { mx.unlock(); }
private:
    mutex & mx;
};

//------------------------------------------------------------------------------
template <typename locker>
class heap 
{
public:
    // Heap initialization
    template<size_t size_items>
    heap(uint32_t (& pool)[size_items]);

    heap(uint32_t * pool, int size_bytes);

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
    static size_t const HEAP_ALIGN    = sizeof(uint32_t);

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
                           // mcb.next of the last MCB always pounts to                           
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
                           
    locker Mutex;         // thread-safe support 
                           
};

template<typename locker>
template<size_t size_items>
heap<locker>::heap(uint32_t (& pool)[size_items])
    : start((mcb *)pool)
    , freemem((mcb *)pool)
    , Mutex()
{
    init(start, sizeof(pool));
}

//------------------------------------------------------------------------------
// Heap initialization
//------------------------------------------------------------------------------
template<typename locker>
heap<locker>::heap(uint32_t * pool, int size_bytes)
    : start((mcb *)pool)
    , freemem((mcb *)pool)
    , Mutex()
{
    init(start, size_bytes);
}

template<typename locker>
void heap<locker>::init(mcb * pstart, size_t size_bytes)
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
extern heap<TLocker> Heap;
//------------------------------------------------------------------------------
#endif  // HEAP_H__

