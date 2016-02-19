# heap_z
Lightweight and fast free memory manager suitable for embedded applications

## Usage
At first, memory pool must be declared, for example:

```C
#include <stdint.h>

uint32_t HeapPool[4096/sizeof(uint32_t)];
```
Another example:

```C
__attribute__((section(".heap")))
uint32_t HeapPool[ 4096/sizeof(uint32_t) ];
```
Memory pool located in the section '.heap' <sup>[1](#footnote1)</sup>.


The second, declare heap object:

```C++
heap Heap(HeapPool, sizeof(HeapPool));
```
Or if memory pool object declared in the same scope (i.e. type and size of the object are known), there is more short alternativ:e
```C++
heap Heap(HeapPool);
```


That's all, heap functions (malloc(), free(), new , delete etc.) can be used in ordinary manner.

<hr>
<a name="footnote1"></a>[1] Memory pool can be placed anywhere in memory, not in separate section only.
