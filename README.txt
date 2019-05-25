basic.h:
a collections of simple macros

mam_alloc: basic allocators
	ring: ring buffer
	stack: variable length allocator without memory reuse
	pool: fixed length allocator with memory reuse
	slab: variable length allocator with memory reuse
