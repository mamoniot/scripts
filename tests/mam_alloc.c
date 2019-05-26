//By Monica Moniot
#define MAM_ALLOC_IMPLEMENTATION
#define MAM_ALLOC_DEBUG
#define MAM_ALLOC_ALIGNMENT 8
#include "mam_alloc.h"
#include "stdlib.h"
#include "stdio.h"
#include "assert.h"


int is_aligned(int a) {
	return a%MAM_ALLOC_ALIGNMENT == 0;
}

void stack_fuzzer() {
	MamStack* stack = mam_stack_init(malloc(1000), 1000);
	int total_allocs = 0;
	int total_alloc_mem = 0;
	int ten_mem_index = 0;

	srand(12);
	for(int j = 0; j < 100000; j += 1) {
		int scenario = rand()%100;

		if(scenario < 15) {
			//test mam_stack_push
			if(!mam_stack_will_overflow(stack, 10*sizeof(int))) {
				ten_mem_index = mam_stack_pushi(stack, 10*sizeof(int));
				int* ten_mem = mam_get_ptr(int, stack, ten_mem_index);
				for(int i = 0; i < 10; i += 1) {
					ten_mem[i] = 100;
				}
				assert(is_aligned(ten_mem_index));

				total_alloc_mem += 10*sizeof(int);
				total_allocs += 1;
			}
		} else if(scenario < 30) {
			//test mam_stack_extend
			if(!mam_stack_will_overflow(stack, 12)) {
				char* text = mam_stack_push(char, stack, 0);

				const char* hello = "hello";
				mam_stack_extend(stack, 5);
				memcpy(text, hello, 5);

				mam_stack_extend(stack, 1);
				text[5] = ' ';

				const char* world = "world";
				mam_stack_extend(stack, 6);
				memcpy(&text[6], world, 6);

				assert(strcmp(text, "hello world") == 0);

				total_allocs += 1;
				total_alloc_mem += 12;
			}
		} else if(scenario < 50) {
			//test mam_stack_will_overflow
			int mem_left = stack->mem_capacity - stack->mem_size;
			assert(mem_left >= 0);
			assert(!mam_stack_will_overflow(stack, mem_left - 4*MAM_ALLOC_ALIGNMENT));
		} else if(scenario < 60) {
			//test memory layout independence
			MamStack* new_stack = (MamStack*)malloc(1000);
			memcpy(new_stack, stack, stack->mem_size);//one might imagine copying the stack to a file instead
			memset(stack, 'x', stack->mem_capacity);
			free(stack);
			stack = new_stack;
		} else if(scenario < 75) {
			//test push
			int buffer_size = rand()%200 + 1;
			if(mam_stack_will_overflow(stack, buffer_size)) {
				assert(stack->mem_size + buffer_size + 4*MAM_ALLOC_ALIGNMENT >= stack->mem_capacity);
			} else {
				char* buffer = mam_stack_push(char, stack, buffer_size);
				mam_check(buffer, buffer_size);
				buffer[0] = 'a';
				buffer[buffer_size - 1] = 'b';
				mam_check(buffer, buffer_size);

				total_allocs += 1;
				total_alloc_mem += buffer_size;
			}
		} else if(scenario < 95) {
			//test mam_stack_pop
			if(total_allocs > 0) {
				int pre_size = stack->mem_size;
				int pop_size;
				int pop_i = mam_stack_get_endi(stack, &pop_size);
				mam_stack_pop(stack);

				assert(stack->mem_size < pre_size);

				total_allocs -= 1;
				total_alloc_mem -= pop_size;
				if(ten_mem_index == pop_i) {
					assert(pop_size == 10*sizeof(int));
					ten_mem_index = 0;
				}
			}
		} else {
			//test mam_stack_reset, and also empty out the stack
			mam_stack_reset(stack);
			total_allocs = 0;
			total_alloc_mem = 0;
			ten_mem_index = 0;
		}
		//check to make sure the entire stack is still correct
		assert(total_alloc_mem <= stack->mem_size);

		int cur_total_allocs = 0;
		int cur_total_alloc_mem = 0;
		int buffer_size = 0;
		for(char* buffer = mam_stack_get_end(char, stack, &buffer_size); buffer; buffer = mam_stack_get_next(char, stack, buffer, &buffer_size)) {
			mam_check(buffer, buffer_size);
			assert(is_aligned(mam_ptr_dist(stack, buffer)));

			cur_total_allocs += 1;
			cur_total_alloc_mem += buffer_size;
		}

		assert(cur_total_allocs == total_allocs);
		assert(cur_total_alloc_mem == total_alloc_mem);

		if(ten_mem_index) {
			int* ten_mem = mam_get_ptr(int, stack, ten_mem_index);
			for(int i = 0; i < 10; i += 1) {
				assert(ten_mem[i] == 100);
			}
		}
	}
	free(stack);
}

void ring_fuzzer() {
	MamRing* ring = mam_ring_init(malloc(1000), 1000);
	int total_allocs = 0;
	int total_alloc_mem = 0;
	int ten_mem_index = 0;

	srand(12);
	for(int j = 0; j < 100000; j += 1) {
		int scenario = rand()%100;

		if(scenario < 15) {
			//test mam_ring_push
			if(!mam_ring_will_overflow(ring, 10*sizeof(int))) {
				ten_mem_index = mam_ring_pushi(ring, 10*sizeof(int));
				int* ten_mem = mam_get_ptr(int, ring, ten_mem_index);
				for(int i = 0; i < 10; i += 1) {
					ten_mem[i] = 100;
				}
				assert(is_aligned(ten_mem_index));

				total_alloc_mem += 10*sizeof(int);
				total_allocs += 1;
			}
		} else if(scenario < 35) {
			//test mam_ring_will_overflow
			int mem_left = ring->mem_capacity - (ring->mem_used + sizeof(MamRing));
			assert(mem_left >= 0);
			assert(!mam_ring_will_overflow(ring, mem_left/2 - 4*MAM_ALLOC_ALIGNMENT));
		} else if(scenario < 50) {
			//test memory layout independence
			MamRing* new_ring = (MamRing*)malloc(1000);
			memcpy(new_ring, ring, ring->mem_capacity);//one might imagine copying the ring to a file instead
			memset(ring, 'x', ring->mem_capacity);
			free(ring);
			ring = new_ring;
		} else if(scenario < 72) {
			//test push
			int buffer_size = rand()%200 + 1;
			if(mam_ring_will_overflow(ring, buffer_size)) {
				assert(ring->mem_used + sizeof(MamRing) + 2*buffer_size + 4*MAM_ALLOC_ALIGNMENT >= ring->mem_capacity);
			} else {
				char* buffer = mam_ring_push(char, ring, buffer_size);
				mam_check(buffer, buffer_size);
				buffer[0] = 'a';
				buffer[buffer_size - 1] = 'b';
				mam_check(buffer, buffer_size);

				total_allocs += 1;
				total_alloc_mem += buffer_size;
			}
		} else if(scenario < 97) {
			//test mam_ring_pop
			if(total_allocs > 0) {
				int pre_size = ring->mem_used;
				int pop_size;
				int pop_i = mam_ring_get_endi(ring, &pop_size);
				mam_ring_pop(ring);

				assert(ring->mem_used < pre_size);

				total_allocs -= 1;
				total_alloc_mem -= pop_size;
				if(ten_mem_index == pop_i) {
					assert(pop_size == 10*sizeof(int));
					ten_mem_index = 0;
				}
			}
		} else {
			//test mam_ring_reset, and also empty out the ring
			mam_ring_reset(ring);
			total_allocs = 0;
			total_alloc_mem = 0;
			ten_mem_index = 0;
		}
		//check to make sure the entire ring is still correct
		assert(total_alloc_mem <= ring->mem_used);

		int cur_total_allocs = 0;
		int cur_total_alloc_mem = 0;
		int buffer_size = 0;
		for(char* buffer = mam_ring_get_end(char, ring, &buffer_size); buffer; buffer = mam_ring_get_next(char, ring, buffer, &buffer_size)) {
			mam_check(buffer, buffer_size);
			assert(is_aligned(mam_ptr_dist(ring, buffer)));

			cur_total_allocs += 1;
			cur_total_alloc_mem += buffer_size;
		}

		assert(cur_total_allocs == total_allocs);
		assert(cur_total_alloc_mem == total_alloc_mem);

		if(ten_mem_index) {
			int* ten_mem = mam_get_ptr(int, ring, ten_mem_index);
			for(int i = 0; i < 10; i += 1) {
				assert(ten_mem[i] == 100);
			}
		}
	}
	free(ring);
}

int main() {
	stack_fuzzer();
	ring_fuzzer();


	printf("all tests passed");
	return 0;
}
