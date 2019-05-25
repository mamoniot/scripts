//By Monica Moniot
#ifdef __cplusplus
extern "C" {
#endif

#ifndef MAM_ALLOC__INCLUDE
#define MAM_ALLOC__INCLUDE
////////////////////////////////////////////////////////////////////////////////////////////////////
// An indev custom allocators library
// TODO: Build a fuzzer for the heap
// TODO: Full documentation
//
// options:
// MAM_ALLOC_STATIC
// MAM_ALLOC_DEBUG
// MAM_ALLOC_ASSERT
// MAM_ALLOC_SIZE_T
// MAM_ALLOC_BYTE_T
// MAM_ALLOC_ALIGNMENT
// MAM_ALLOC_COOKIE
//
// stack:
// mam_stack_init
// mam_stack_reset
// mam_stack_will_overflow
// mam_stack_pushi
// mam_stack_pushn
// mam_stack_push
// mam_stack_extend
// mam_stack_pop
// mam_stack_get_endi
// mam_stack_get_endn
// mam_stack_get_end
// mam_stack_get_nexti
// mam_stack_get_nextn
// mam_stack_get_next
//
// ring:
// mam_ring_init
// mam_ring_reset
// mam_ring_will_overflow
// mam_ring_pushi
// mam_ring_pushn
// mam_ring_push
// mam_ring_pop
// mam_ring_get_endi
// mam_ring_get_endn
// mam_ring_get_end
// mam_ring_get_nexti
// mam_ring_get_nextn
// mam_ring_get_next
//
// pool:
// mam_pool_init
// mam_pool_reset
// mam_pool_will_overflow
// mam_pool_alloci
// mam_pool_allocn
// mam_pool_alloc
// mam_pool_freei
// mam_pool_free
//
// heap:
// mam_heap_init
// mam_heap_reset
// mam_heap_will_overflow
// mam_heap_alloci
// mam_heap_allocn
// mam_heap_alloc
// mam_heap_freei
// mam_heap_free
// mam_heap_realloci
// mam_heap_reallocn
// mam_heap_realloc
//
// checker:
// mam_get_ptr
// mam_ptr_add
// mam_ptr_dist
// mam_checki
// mam_check
// mam_check_belowi
// mam_check_below
//
// mam_check_get_external_size
// mam_check_get_internal_size
// mam_check_get_external_item
// mam_check_get_internal_item
// mam_check_mark
// mam_check_unmark
//
//


#ifndef MAM_ALLOC_ASSERT
 #ifdef MAM_ALLOC_DEBUG
  #include <assert.h>
  #define MAM_ALLOC_ASSERT(is, msg) assert((is) && msg)
 #else
  #define MAM_ALLOC_ASSERT(is, msg) 0
 #endif
#endif

#ifndef MAM_ALLOC_SIZE_T
 #define MAM_ALLOC_SIZE_T int
#endif
#ifndef MAM_ALLOC_BYTE_T
 #define MAM_ALLOC_BYTE_T char
#endif
typedef MAM_ALLOC_SIZE_T mam_int;
typedef MAM_ALLOC_BYTE_T mam_byte;

#ifndef MAM_ALLOC_ALIGNMENT
 #define MAM_ALLOC_ALIGNMENT sizeof(void*)
#endif

#ifndef MAM_ALLOC_COOKIE
 #define MAM_ALLOC_COOKIE 0x44334433
#endif

#ifdef MAM_ALLOC_STATIC
 #define MAM_ALLOC__DECLR static
#else
 #define MAM_ALLOC__DECLR
#endif
#define MAM_ALLOC__DECLS static inline



typedef union MamStack {
	mam_byte mem[1];
	struct {
		mam_int mem_capacity;
		mam_int mem_size;
	};
} MamStack;

typedef union MamRing {
	mam_byte mem[1];
	struct {
		mam_int mem_capacity;
		mam_int mem_used;
		mam_int mem_bottom;
		mam_int mem_top;
		mam_int top_item;
	};
} MamRing;

typedef union MamPool {
	mam_byte mem[1];
	struct {
		mam_int mem_capacity;
		mam_int mem_size;
		mam_int first_unused;
		mam_int item_size;
	};
} MamPool;

typedef union MamHeap {
	mam_byte mem[1];
	struct {
		mam_int mem_capacity;
		mam_int mem_size;
		mam_int head_block;
		mam_int end_block;//points to the very last block of memory in the heap that is used
	};
} MamHeap;



#define mam_ptr_add(type, ptr, n) ((type*)((mam_byte*)(ptr) + (n)))
#define mam_ptr_dist(ptr0, ptr1) ((mam_int)((mam_byte*)(ptr1) - (mam_byte*)(ptr0)))

#ifdef MAM_ALLOC_DEBUG

#define mam__underflow_msg "mam_alloc: invalid buffer detected; either an invalid pointer was used, a buffer underflow occured, or an overflow of the adjacent buffer occured"
#define mam__overflow_msg "mam_alloc: buffer overflow detected"


/*******************************************************************************
If your debugger takes you here because of an assertion, look at the assertion
message for more information, and walk the stack back up to your own code.
This library provides the following memory check functions to do quick and
dirty memory intergrity tests while the library is in debug mode.
They catch the most common bugs such as buffer overflows or use-after-free,
and they never trigger an assert unless an actual memory bug occurred.
This works by placing memory cookies above and below every allocated buffer,
and checking their existance regularly.
The default cookie is 0x44334433 == 1144210483 == "3D3D"
********************************************************************************/
MAM_ALLOC__DECLS void mam_checki(void* allocator, mam_int item, mam_int item_size) {
	MAM_ALLOC_ASSERT(*((int*)((mam_byte*)allocator + item) - 1) == MAM_ALLOC_COOKIE, mam__underflow_msg);
	MAM_ALLOC_ASSERT(*(int*)((mam_byte*)allocator + item + item_size) == MAM_ALLOC_COOKIE, mam__overflow_msg);
}
MAM_ALLOC__DECLS void mam_check(void* ptr, mam_int ptr_size) {
	MAM_ALLOC_ASSERT(*((int*)ptr - 1) == MAM_ALLOC_COOKIE, mam__underflow_msg);
	MAM_ALLOC_ASSERT(*(int*)((mam_byte*)ptr + ptr_size) == MAM_ALLOC_COOKIE, mam__overflow_msg);
}
MAM_ALLOC__DECLS void mam_check_belowi(void* allocator, mam_int item) {
	MAM_ALLOC_ASSERT(*((int*)((mam_byte*)allocator + item) - 1) == MAM_ALLOC_COOKIE, mam__underflow_msg);
}
MAM_ALLOC__DECLS void mam_check_below(void* ptr) {
	MAM_ALLOC_ASSERT(*((int*)ptr - 1) == MAM_ALLOC_COOKIE, mam__underflow_msg);
}





#define mam_get_ptr(type, allocator, item) (mam_check_below(mam_ptr_add(mam_byte, allocator, item)), mam_ptr_add(type, allocator, item))

#define mam_check_get_external_size(size) (size - MAM_ALLOC_ALIGNMENT - sizeof(int))
#define mam_check_get_internal_size(size) (size + MAM_ALLOC_ALIGNMENT + 2*sizeof(int))
#define mam_check_get_external_item(item) (item + MAM_ALLOC_ALIGNMENT)
#define mam_check_get_internal_item(item) (item - MAM_ALLOC_ALIGNMENT)

MAM_ALLOC__DECLS void mam_check_mark(void* allocator, mam_int item, mam_int item_size) {
	//takes internal item and item_size
	*mam_ptr_add(int, allocator, item) = MAM_ALLOC_COOKIE;
	*(mam_ptr_add(int, allocator, item + item_size) - 1) = MAM_ALLOC_COOKIE;
}
MAM_ALLOC__DECLS void mam_check_unmark(void* allocator, mam_int item, mam_int item_size) {
	//takes internal item and item_size
	int* bot_cookie = mam_ptr_add(int, allocator, item);
	int* top_cookie = mam_ptr_add(int, allocator, item + item_size) - 1;
	MAM_ALLOC_ASSERT(*bot_cookie == MAM_ALLOC_COOKIE, mam__underflow_msg);
	MAM_ALLOC_ASSERT(*top_cookie == MAM_ALLOC_COOKIE, mam__overflow_msg);
	*bot_cookie = 0;
	*top_cookie = 0;
}

#else
#define mam_checki(allocator, item, item_size) 0
#define mam_check(ptr, ptr_size) 0
#define mam_check_below(ptr) 0
#define mam_get_ptr(type, allocator, item) mam_ptr_add(type, allocator, item)

#define mam_check_get_internal_size(size) size
#define mam_check_get_external_size(size) size
#define mam_check_get_external_item(item) item
#define mam_check_get_internal_item(item) item

#define mam_check_mark(allocator, item, item_size) 0
#define mam_check_unmark(allocator, item, item_size) 0
#endif

#define MAM__STACK_BASE mam_alloc_align(sizeof(MamStack) + sizeof(mam_int))

//////////////////////////////////////////////////////////
// Stack

MAM_ALLOC__DECLR MamStack* mam_stack_init(void* alloc_ptr, mam_int alloc_size);
MAM_ALLOC__DECLS void mam_stack_reset(MamStack* stack) {
	stack->mem_size = MAM__STACK_BASE;
}

MAM_ALLOC__DECLS int mam_stack_will_overflow(MamStack* stack, mam_int size) {
	return stack->mem_size + mam_alloc_align(mam_check_get_internal_size(size) + sizeof(mam_int)) > stack->mem_capacity;
}

MAM_ALLOC__DECLR mam_int mam_stack_pushi(MamStack* stack, mam_int size);
MAM_ALLOC__DECLS void* mam_stack_pushn(MamStack* stack, mam_int size) {
	return mam_ptr_add(void, stack, mam_stack_pushi(stack, size));
}
#define mam_stack_push(type, stack, size) ((type*)mam_stack_pushn(stack, sizeof(type)*(size)))


MAM_ALLOC__DECLR void mam_stack_extend(MamStack* stack, mam_int size);

MAM_ALLOC__DECLR void mam_stack_pop(MamStack* stack);


MAM_ALLOC__DECLR mam_int mam_stack_get_endi(MamStack* stack, mam_int* ret_size);
MAM_ALLOC__DECLS void* mam_stack_get_endn(MamStack* stack, mam_int* ret_size) {
	mam_int item = mam_stack_get_endi(stack, ret_size);
	return item ? mam_ptr_add(void, stack, item) : 0;
}
#define mam_stack_get_end(type, stack, ret_size) ((type*)mam_stack_get_endn(stack, ret_size))

MAM_ALLOC__DECLR mam_int mam_stack_get_nexti(MamStack* stack, mam_int item, mam_int* ret_size);
MAM_ALLOC__DECLS void* mam_stack_get_nextn(MamStack* stack, void* ptr, mam_int* ret_size) {
	mam_int item = mam_stack_get_nexti(stack, mam_ptr_dist(stack, ptr), ret_size);
	return item ? mam_ptr_add(void, stack, item) : 0;
}
#define mam_stack_get_next(type, stack, ptr, ret_size) ((type*)mam_stack_get_nextn(stack, ptr, ret_size))


//////////////////////////////////////////////////////////
// Ring

#define MAM__RING_BASE mam_alloc_align(sizeof(MamRing))
#define MAM__HIGH_BIT (((mam_int)1)<<(8*sizeof(mam_int) - 1))

MAM_ALLOC__DECLR MamRing* mam_ring_init(void* alloc_ptr, mam_int alloc_size);
MAM_ALLOC__DECLS void mam_ring_reset(MamRing* ring) {
	ring->mem_top = MAM__RING_BASE;
	ring->mem_bottom = ring->mem_top;
	ring->mem_used = 0;
}

MAM_ALLOC__DECLR int mam_ring_will_overflow(MamRing* ring, mam_int size);

MAM_ALLOC__DECLR mam_int mam_ring_pushi(MamRing* ring, mam_int size);
MAM_ALLOC__DECLS void* mam_ring_pushn(MamRing* ring, mam_int size) {
	return mam_ptr_add(void, ring, mam_ring_pushi(ring, size));
}
#define mam_ring_push(type, ring, size) ((type*)mam_ring_pushn(ring, sizeof(type)*(size)))

MAM_ALLOC__DECLR void mam_ring_pop(MamRing* ring);

MAM_ALLOC__DECLR mam_int mam_ring_get_endi(MamRing* ring, mam_int* ret_size);
MAM_ALLOC__DECLS void* mam_ring_get_endn(MamRing* ring, mam_int* ret_size) {
	mam_int item = mam_ring_get_endi(ring, ret_size);
	return item ? mam_ptr_add(void, ring, item) : 0;
}
#define mam_ring_get_end(type, ring, ret_size) ((type*)mam_ring_get_endn(ring, ret_size))

MAM_ALLOC__DECLR mam_int mam_ring_get_nexti(MamRing* ring, mam_int item, mam_int* ret_size);
MAM_ALLOC__DECLS void* mam_ring_get_nextn(MamRing* ring, void* ptr, mam_int* ret_size) {
	mam_int item = mam_ring_get_nexti(ring, mam_ptr_dist(ring, ptr), ret_size);
	return item ? mam_ptr_add(void, ring, item) : 0;
}
#define mam_ring_get_next(type, ring, ptr, ret_size) ((type*)mam_ring_get_nextn(stack, ptr, ret_size))


//////////////////////////////////////////////////////////
// Pool

MAM_ALLOC__DECLR MamPool* mam_pool_initn(void* alloc_ptr, mam_int alloc_size, mam_int item_size);

MAM_ALLOC__DECLS void mam_pool_reset(MamPool* pool) {
	pool->first_unused = 0;
	pool->mem_size = sizeof(MamPool);
}

MAM_ALLOC__DECLS int mam_pool_will_overflow(MamPool* pool) {
	return !pool->first_unused && pool->mem_size + pool->item_size > pool->mem_capacity;
}

MAM_ALLOC__DECLR mam_int mam_pool_alloci(MamPool* pool);
MAM_ALLOC__DECLS void* mam_pool_allocn(MamPool* pool) {
	return mam_ptr_add(void, pool, mam_pool_alloci(pool));
}
#define mam_pool_alloc(type, pool) ((type*)mam_pool_allocn(pool))

MAM_ALLOC__DECLR void mam_pool_freei(MamPool* pool, mam_int item);
MAM_ALLOC__DECLS void mam_pool_free(MamPool* pool, void* ptr) {
	mam_pool_freei(pool, ptr_dist(pool, ptr));
}

//////////////////////////////////////////////////////////
// Heap
typedef struct Mam__Block {
	mam_int pre;//points to the previous block in memory, is 0 for the first block
	mam_int size;//the current size of the memory in this block
	mam_int free_pre;//When this equals 0, the block is in use
	mam_int free_next;//points to next item in free list, this list is a circular linked list
} Mam__Block;
MAM_ALLOC__DECLS void mam__block_set_next(Mam__Block* block, mam_int cur_i, mam_int next_i) {
	block->size = next_i - cur_i;
}
MAM_ALLOC__DECLS mam_int mam__block_get_next(Mam__Block* block, mam_int cur_i) {
	return block->size + cur_i;
}

#define MAM_HEAP_ALIGNMENT mam_alloc_align(sizeof(Mam__Block))
MAM_ALLOC__DECLS mam_int mam__heap_align(mam_int size) {
	mam_int alignment = MAM_HEAP_ALIGNMENT;
	return alignment*((size + alignment - 1)/alignment + 1);
}

MAM_ALLOC__DECLR MamHeap* mam_heap_init(void* alloc_ptr, mam_int alloc_size);
MAM_ALLOC__DECLS void mam_heap_reset(MamHeap* heap) {
	heap->mem_size = mam_alloc_align(sizeof(MamHeap));
	heap->head_block = 0;
	heap->end_block = 0;
}

MAM_ALLOC__DECLS int mam_heap_will_overflow(MamHeap* heap, mam_int size) {
	return heap->mem_size + mam__heap_align(mam_check_get_internal_size(size)) > heap->mem_capacity;
}

MAM_ALLOC__DECLR mam_int mam_heap_alloci(MamHeap* heap, mam_int size);
MAM_ALLOC__DECLS void* mam_heap_allocn(MamHeap* heap, mam_int size) {
	return mam_ptr_add(void, heap, mam_heap_alloci(heap, size));
}
#define mam_heap_alloc(type, heap, size) ((type*)mam_heap_allocn(heap, sizeof(type)*(size)))

MAM_ALLOC__DECLR void mam_heap_freei(MamHeap* heap, mam_int item);
MAM_ALLOC__DECLS void mam_heap_free(MamHeap* heap, void* ptr) {
	mam_heap_freei(heap, mam_ptr_dist(heap, ptr));
}

#endif

#ifdef MAM_ALLOC_IMPLEMENTATION
#undef MAM_ALLOC_IMPLEMENTATION

MAM_ALLOC__DECLS mam_int mam_alloc_align(mam_int a) {
	//raises a to the nearest multiple of MAM_ALLOC_ALIGNMENT
	return MAM_ALLOC_ALIGNMENT*((a + (MAM_ALLOC_ALIGNMENT - 1))/MAM_ALLOC_ALIGNMENT);
}
MAM_ALLOC__DECLR MamStack* mam_stack_init(void* alloc_ptr, mam_int alloc_size) {
	MamStack* stack = (MamStack*)alloc_ptr;
	stack->mem_size = MAM__STACK_BASE;
	stack->mem_capacity = alloc_size;
	return stack;
}

MAM_ALLOC__DECLR mam_int mam_stack_pushi(MamStack* stack, mam_int size) {
	MAM_ALLOC_ASSERT(!mam_stack_will_overflow(stack, size), "mam_alloc: stack ran out of memory");
	size = mam_check_get_internal_size(size) + sizeof(mam_int);
	mam_int alloc_size = mam_alloc_align(size);

	mam_int item = stack->mem_size;
	//add previous size for pop
	*(mam_ptr_add(mam_int, stack, item + alloc_size) - 1) = size;
	stack->mem_size += alloc_size;

	mam_check_mark(stack, item, size - sizeof(mam_int));
	return mam_check_get_external_item(item);
}
MAM_ALLOC__DECLR void mam_stack_extend(MamStack* stack, mam_int size) {
	//TODO: fix error messages and check cookie
	MAM_ALLOC_ASSERT(!(stack->mem_size + size > stack->mem_capacity), "mam_alloc: stack ran out of memory");

	mam_int pop_size = *(mam_ptr_add(mam_int, stack, stack->mem_size) - 1);
	mam_int alloc_size = mam_alloc_align(pop_size);
	mam_int pop_item = stack->mem_size - alloc_size;

	MAM_ALLOC_ASSERT(stack->mem_size > MAM__STACK_BASE, "mam_alloc: attempt to extend stack when stack is empty");
	mam_check_unmark(stack, pop_item, pop_size - sizeof(mam_int));

	stack->mem_size -= alloc_size;

	mam_int new_size = pop_size + size;
	mam_int new_alloc_size = mam_alloc_align(new_size);
	//copy over previous size for pop
	*(mam_ptr_add(mam_int, stack, pop_item + new_alloc_size) - 1) = new_size;
	stack->mem_size += new_alloc_size;

	mam_check_mark(stack, pop_item, new_size - sizeof(mam_int));
}
MAM_ALLOC__DECLR void mam_stack_pop(MamStack* stack) {
	mam_int pop_size = *(mam_ptr_add(mam_int, stack, stack->mem_size) - 1);
	mam_int alloc_size = mam_alloc_align(pop_size);
	mam_int pop_item = stack->mem_size - alloc_size;

	MAM_ALLOC_ASSERT(stack->mem_size > MAM__STACK_BASE, "mam_alloc: attempt to pop stack when stack is empty");
	mam_check_unmark(stack, pop_item, pop_size - sizeof(mam_int));

	stack->mem_size -= alloc_size;
}

MAM_ALLOC__DECLR mam_int mam_stack_get_endi(MamStack* stack, mam_int* ret_size) {
	mam_int pop_size = *(mam_ptr_add(mam_int, stack, stack->mem_size) - 1);
	if(pop_size >= 0) {
		mam_int alloc_size = mam_alloc_align(pop_size);
		mam_int pop_item = stack->mem_size - alloc_size;
		if(ret_size) *ret_size = pop_size - sizeof(mam_int);
		return mam_check_get_external_item(pop_item);
	} else {
		return 0;
	}
}
MAM_ALLOC__DECLR mam_int mam_stack_get_nexti(MamStack* stack, mam_int item, mam_int* ret_size) {
	item = mam_check_get_internal_item(item);
	mam_int pop_size = *(mam_ptr_add(mam_int, stack, item) - 1);
	if(pop_size >= 0) {
		mam_int alloc_size = mam_alloc_align(pop_size);
		mam_int pop_item = stack->mem_size - alloc_size;
		if(ret_size) *ret_size = pop_size - sizeof(mam_int);
		return mam_check_get_external_item(pop_item);
	} else {
		return 0;
	}
}



MAM_ALLOC__DECLR MamRing* mam_ring_init(void* alloc_ptr, mam_int alloc_size) {
	MamRing* ring = (MamRing*)alloc_ptr;
	ring->mem_used = 0;
	ring->mem_top = MAM__RING_BASE;
	ring->mem_bottom = MAM__RING_BASE;
	ring->mem_capacity = alloc_size;
	return ring;
}
MAM_ALLOC__DECLR int mam_ring_will_overflow(MamRing* ring, mam_int size) {
	mam_int alloc_size = mam_alloc_align(mam_check_get_internal_size(size) + sizeof(mam_int));
	mam_int item = ring->mem_top;
	if(item + alloc_size > ring->mem_capacity) {
		mam_int padding_size = ring->mem_capacity - item;
		alloc_size += padding_size;
	}

	return ring->mem_used + alloc_size > ring->mem_capacity - sizeof(MamRing);
}
MAM_ALLOC__DECLR mam_int mam_ring_pushi(MamRing* ring, mam_int size) {
	MAM_ALLOC_ASSERT(!mam_ring_will_overflow(ring, size), "mam_alloc: ring ran out of memory");
	size = mam_check_get_internal_size(size) + sizeof(mam_int);
	mam_int alloc_size = mam_alloc_align(size);

	mam_int item = ring->mem_top;
	if(item + alloc_size > ring->mem_capacity) {
		//this allocation would trail off the end of the ring's memory, push it to the beginning
		//mark the previous allocation as the last
		*mam_ptr_add(mam_int, ring, ring->top_item) |= MAM__HIGH_BIT;
		mam_int padding_size = ring->mem_capacity - item;
		ring->mem_used += padding_size;

		item = MAM__RING_BASE;
	}
	*mam_ptr_add(mam_int, ring, item) = size;//store the size of this allocation before it
	ring->mem_used += alloc_size;
	ring->top_item = item;
	ring->mem_top = item + alloc_size;

	item += sizeof(mam_int);
	mam_check_mark(ring, item, size - sizeof(mam_int));
	return mam_check_get_external_item(item);
}
MAM_ALLOC__DECLR void mam_ring_pop(MamRing* ring) {
	MAM_ALLOC_ASSERT(ring->mem_used > 0, "mam_alloc: attempt to pop from empty ring");
	mam_int pop_size = *mam_ptr_add(mam_int, ring, ring->mem_bottom);
	mam_int alloc_size = mam_alloc_align(pop_size);
	mam_int item = ring->mem_bottom;
	if(pop_size & MAM__HIGH_BIT) {
		ring->mem_bottom = MAM__RING_BASE;
		pop_size &= ~MAM__HIGH_BIT;
	} else {
		ring->mem_bottom += alloc_size;
	}
	ring->mem_used -= alloc_size;
	mam_check_unmark(ring, item, pop_size - sizeof(mam_int));
}

MAM_ALLOC__DECLR mam_int mam_ring_get_endi(MamRing* ring, mam_int* ret_size) {
	if(ring->mem_used > 0) {
		if(ret_size) *ret_size = *mam_ptr_add(mam_int, ring, ring->mem_bottom) - sizeof(mam_int);
		return mam_check_get_external_item(ring->mem_bottom + sizeof(mam_int));
	} else {
		return 0;
	}
}
MAM_ALLOC__DECLR mam_int mam_ring_get_nexti(MamRing* ring, mam_int item, mam_int* ret_size) {
	item = mam_check_get_internal_item(item) - sizeof(mam_int);
	mam_int alloc_size = *mam_ptr_add(mam_int, ring, item);
	mam_int next = item + alloc_size;;
	if(next == ring->mem_capacity) {
		next = sizeof(MamRing);
	}
	if(next == ring->mem_top) {
		return 0;
	} else {
		if(ret_size) *ret_size = *mam_ptr_add(mam_int, ring, next) - sizeof(mam_int);
		return mam_check_get_external_item(next + sizeof(mam_int));
	}
}



MAM_ALLOC__DECLR MamPool* mam_pool_initn(void* alloc_ptr, mam_int alloc_size, mam_int item_size) {
	item_size = mam_check_get_internal_size(item_size);
	item_size = (item_size > sizeof(mam_int)) ? item_size : sizeof(mam_int);
	MamPool* pool = (MamPool*)alloc_ptr;
	pool->first_unused = 0;
	pool->mem_size = mam_alloc_align(sizeof(MamPool));
	pool->mem_capacity = alloc_size;
	#ifdef MAM_ALLOC_DEBUG
	pool->item_size = item_size;
	#else
	pool->item_size = mam_alloc_align(item_size);
	#endif
	return pool;
}

MAM_ALLOC__DECLR mam_int mam_pool_alloci(MamPool* pool) {
	MAM_ALLOC_ASSERT(mam_pool_will_overflow(pool), "mam_alloc: pool ran out of memory");
	mam_int item = pool->first_unused;
	if(item) {
		pool->first_unused = *mam_ptr_add(mam_int, pool, item);
	} else {
		item = pool->mem_size;
		#ifdef MAM_ALLOC_DEBUG
		pool->mem_size += mam_alloc_align(pool->item_size);
		#else
		pool->mem_size += pool->item_size;
		#endif
	}
	mam_check_mark(pool, item, pool->item_size);
	return mam_check_get_external_item(item);
}

MAM_ALLOC__DECLR void mam_pool_freei(MamPool* pool, mam_int item) {
	item = mam_check_get_internal_item(item);
	mam_check_unmark(pool, item, pool->item_size);

	*mam_ptr_add(mam_int, pool, item) = pool->first_unused;
	pool->first_unused = item;
}


MAM_ALLOC__DECLR MamHeap* mam_heap_init(void* alloc_ptr, mam_int alloc_size) {
	MamHeap* heap = (MamHeap*)alloc_ptr;
	heap->mem_size = mam_alloc_align(sizeof(MamHeap));
	heap->mem_capacity = alloc_size;
	heap->head_block = 0;
	heap->end_block = 0;
	return heap;
}

MAM_ALLOC__DECLR mam_int mam_heap_alloci(MamHeap* heap, mam_int size) {
	size = mam_check_get_internal_size(size);
	size = mam__heap_align(size);
	mam_int original_i = heap->head_block;
	if(original_i) {
		mam_int cur_i = original_i;
		do {
			Mam__Block* cur_block = mam_ptr_add(Mam__Block, heap, cur_i);
			mam_int next_i = cur_block->free_next;
			heap->head_block = next_i;
			if(cur_block->size == size) {//remove cur_block from free list
				mam_int pre_i = cur_block->free_pre;
				if(next_i == cur_i) {//all items would be removed from free list
					heap->head_block = 0;
				} else {
					heap->head_block = next_i;
					mam_ptr_add(Mam__Block, heap, pre_i)->free_next = next_i;
					mam_ptr_add(Mam__Block, heap, next_i)->free_pre = pre_i;
				}
				cur_block->free_pre = 0;

				mam_check_mark(heap, cur_i + MAM_HEAP_ALIGNMENT, size - MAM_HEAP_ALIGNMENT);
				return mam_check_get_external_item(cur_i + MAM_HEAP_ALIGNMENT);
			} else if(cur_block->size > size) {
				mam_int pre_i = cur_block->free_pre;

				mam_int fragment_i = cur_i + size;
				Mam__Block* fragment_block = mam_ptr_add(Mam__Block, heap, fragment_i);
				fragment_block->pre = cur_i;
				fragment_block->size = cur_block->size - size;
				cur_block->size = size;
				if(heap->end_block == cur_i) {
					heap->end_block = fragment_i;
				}

				if(next_i == cur_i) {//only one item in free list
					heap->head_block = fragment_i;
					fragment_block->free_pre = fragment_i;
					fragment_block->free_next = fragment_i;
				} else {
					fragment_block->free_pre = pre_i;
					fragment_block->free_next = next_i;
					mam_ptr_add(Mam__Block, heap, pre_i)->free_next = fragment_i;
					mam_ptr_add(Mam__Block, heap, next_i)->free_pre = fragment_i;
				}
				cur_block->free_pre = 0;

				mam_check_mark(heap, cur_i + MAM_HEAP_ALIGNMENT, size - MAM_HEAP_ALIGNMENT);
				return mam_check_get_external_item(cur_i + MAM_HEAP_ALIGNMENT);
			}
			cur_i = next_i;
		} while(original_i != cur_i);
	}
	//no free space, push the size
	MAM_ALLOC_ASSERT(!(heap->mem_size + size > heap->mem_capacity), "mam_alloc: heap has run out of memory");
	mam_int item = heap->mem_size;
	Mam__Block* block = mam_ptr_add(Mam__Block, heap, item);
	if(heap->end_block) {
		mam__block_set_next(mam_ptr_add(Mam__Block, heap, heap->end_block), heap->end_block, item);
		block->pre = heap->end_block;
	}
	block->size = size;
	block->free_pre = 0;
	heap->mem_size += size;
	heap->end_block = item;

	mam_check_mark(heap, item + MAM_HEAP_ALIGNMENT, size - MAM_HEAP_ALIGNMENT);
	return mam_check_get_external_item(item + MAM_HEAP_ALIGNMENT);
}

MAM_ALLOC__DECLR void mam_heap_freei(MamHeap* heap, mam_int item) {
	item = mam_check_get_internal_item(item);
	mam_int cur_i = item - MAM_HEAP_ALIGNMENT;
	Mam__Block* cur_block = mam_ptr_add(Mam__Block, heap, cur_i);
	mam_int pre_i = cur_block->pre;
	Mam__Block* pre_block = mam_ptr_add(Mam__Block, heap, pre_i);
	mam_int next_i = mam__block_get_next(cur_block, cur_i);
	Mam__Block* next_block = mam_ptr_add(Mam__Block, heap, next_i);

	MAM_ALLOC_ASSERT(!cur_block->free_pre, "mam_alloc: attempted to free freed memory");
	mam_check_unmark(heap, item, cur_block->size - MAM_HEAP_ALIGNMENT);

	mam_int head_i = heap->head_block;
	if(pre_i && pre_block->free_pre) {//is free, combine
		if(heap->end_block == cur_i) {//is last block in the whole heap, remove from heap
			heap->end_block = pre_block->pre;
			heap->mem_size -= cur_block->size + pre_block->size;
			//remove pre_block
			mam_int free_pre_i = pre_block->free_pre;
			mam_int free_next_i = pre_block->free_next;
			if(head_i == pre_i) {
				if(pre_i == free_next_i) {//empty free list
					heap->head_block = 0;
				} else {
					heap->head_block = free_next_i;
					mam_ptr_add(Mam__Block, heap, free_pre_i)->free_next = free_next_i;
					mam_ptr_add(Mam__Block, heap, free_next_i)->free_pre = free_pre_i;
				}
			} else {
				mam_ptr_add(Mam__Block, heap, free_pre_i)->free_next = free_next_i;
				mam_ptr_add(Mam__Block, heap, free_next_i)->free_pre = free_pre_i;
			}
		} else if(next_block->free_pre) {//remove next_block and cur_block
			mam_int free_pre_i = next_block->free_pre;
			mam_int free_next_i = next_block->free_next;
			if(head_i == next_i) {
				heap->head_block = pre_i;
			}
			mam_ptr_add(Mam__Block, heap, free_next_i)->free_pre = free_pre_i;
			mam_ptr_add(Mam__Block, heap, free_pre_i)->free_next = free_next_i;
			mam__block_set_next(pre_block, pre_i, mam__block_get_next(next_block, next_i));
			mam_ptr_add(Mam__Block, heap, mam__block_get_next(next_block, next_i))->pre = pre_i;
		} else {//remove cur_block
			mam__block_set_next(pre_block, pre_i, next_i);
			next_block->pre = pre_i;
		}
	} else if(heap->end_block == cur_i) {//cur_block is last block in heap, remove it
		heap->end_block = pre_i;
		heap->mem_size -= cur_block->size;
	} else if(next_block->free_pre) {//inherit next_block's free list info
		mam_int free_pre_i = next_block->free_pre;
		mam_int free_next_i = next_block->free_next;
		if(head_i == next_i) {
			heap->head_block = cur_i;
			if(free_pre_i == next_i) {//next_block is only member of free list
				cur_block->free_pre = cur_i;
				cur_block->free_next = cur_i;
			} else {
				cur_block->free_pre = free_pre_i;
				cur_block->free_next = free_next_i;
				mam_ptr_add(Mam__Block, heap, free_next_i)->free_pre = cur_i;
				mam_ptr_add(Mam__Block, heap, free_pre_i)->free_next = cur_i;
			}
		} else {
			cur_block->free_pre = free_pre_i;
			cur_block->free_next = free_next_i;
			mam_ptr_add(Mam__Block, heap, free_next_i)->free_pre = cur_i;
			mam_ptr_add(Mam__Block, heap, free_pre_i)->free_next = cur_i;
		}
		//remove next_block
		mam__block_set_next(cur_block, cur_i, mam__block_get_next(next_block, next_i));
		mam_ptr_add(Mam__Block, heap, mam__block_get_next(next_block, next_i))->pre = cur_i;
	} else if(head_i) {//add cur_block to the free list
		Mam__Block* head_block = mam_ptr_add(Mam__Block, heap, head_i);
		mam_int last_i = head_block->free_pre;
		mam_ptr_add(Mam__Block, heap, last_i)->free_next = cur_i;
		head_block->free_pre = cur_i;
		cur_block->free_next = head_i;
		cur_block->free_pre = last_i;
	} else {//list is empty
		heap->head_block = cur_i;
		cur_block->free_pre = cur_i;
		cur_block->free_next = cur_i;
	}
}

#endif

#ifdef __cplusplus
}
#endif


/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org>
*/
