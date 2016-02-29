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

#include <stdlib.h>
#include <stdio.h>
#include <new>
#include "heap.h"

using namespace heap;

//------------------------------------------------------------------------------
extern std::nothrow_t const std::nothrow = {};

void * operator new(size_t size, std::nothrow_t const &)
{
    return Manager.malloc(size);
}

void * operator new(size_t size)
{
    return Manager.malloc(size);
}

void operator delete(void * ptr)         // delete allocated storage
{
    Manager.free(ptr);
}

extern "C" void * malloc(size_t size)
{
    return Manager.malloc(size);
}

extern "C" void free(void * ptr)
{
    Manager.free(ptr);
}
//------------------------------------------------------------------------------


