# heap_z memory manager
Lightweight and fast free memory manager suitable for embedded applications

## Usage
At first, memory pool must be declared, for example:

```C
#include <stdint.h>

uint32_t HeapPool[4096/sizeof(uint32_t)];
```

The second, declare heap object<sup>[1](#footnote1)</sup>:

```C++
heap<heap_guard> Heap(HeapPool, sizeof(HeapPool));
```
Or if memory pool object declared in the same scope (i.e. type and size of the object are known), there is more short alternative.:
```C++
heap<heap_guard> Heap(HeapPool);
```

That's all, heap functions (malloc(), free(), new , delete etc.) can be used in ordinary manner.

See [wiki page](https://github.com/emb-lib/heap_z/wiki) for additional information.

<hr>
<a name="footnote1"></a>[1] See [thread-safe guard description page for details](https://github.com/emb-lib/heap_z/wiki/thread-safe guard configuration)
