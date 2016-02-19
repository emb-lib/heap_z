# heap_z
Lightweight and fast free memory manager suitable for embedded applications

## Usage
At first, memory pool must be declared, for example:

```C
__attribute__((section(".heap")))
uint32_t HeapPool[ 4096/sizeof(uint32_t) ];
```
Memory pool located in the section '.heap'.

The second, declare heap object:

```C++
heap Heap(HeapPool, sizeof(HeapPool));                
```
or

That's all, heap functions can be used in usual manner.
