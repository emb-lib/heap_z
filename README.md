# heap_z memory manager
Lightweight and fast free memory manager suitable for embedded applications

## Usage
At first, memory pool must be declared, for example:

```C
#include <stdint.h>
#include <heap.h>

heap::pool<4096> HeapPool;
```

The second, declare heap object with `heap_guard`<sup>[1](#footnote1)</sup>:

```C++
heap::manager<heap_guard> heap::Manager(HeapPool);
```

That's all, heap functions (malloc(), free(), new , delete etc.) can be used in ordinary manner.

See [wiki page](https://github.com/emb-lib/heap_z/wiki) for additional information.

<hr>
<a name="footnote1"></a>[1] See [thread-safe guard description page for details](https://github.com/emb-lib/heap_z/wiki/thread-safe guard configuration)
