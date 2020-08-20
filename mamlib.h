//By Monica Moniot
#ifndef MAMLIB__H_INCLUDE
#define MAMLIB__H_INCLUDE

#ifdef __cplusplus
	#ifdef MAMLIB_STATIC
		#define MAMLIB__DECLR static
	#else
		#define MAMLIB__DECLR extern "C"
	#endif
#else
	#ifdef MAMLIB_STATIC
		#define MAMLIB__DECLR static
	#else
		#define MAMLIB__DECLR extern
	#endif
#endif
#define MAMLIB__DECLS static inline

#define MAMLIB_CHECK_COOKIE 0x4433443344334433ll

#define MAMLIB_ALIGNMENT sizeof(void*)
#ifdef MAMLIB_NOSTD
	#define MAMLIB_MEMINT long long
	#define MAMLIB_BYTE char
#else
	#include "inttypes.h"
	#define MAMLIB_MEMINT size_t
	#define MAMLIB_BYTE int8_t
	#include "stdlib.h"
	#define MAMLIB_MALLOC malloc
	#define MAMLIB_REALLOC realloc
	#define MAMLIB_FREE free
	#include "string.h"
	#define MAMLIB_STRLEN strlen
	#define MAMLIB_MEMCPY memcpy
	#define MAMLIB_MEMCMP memcmp
	#include "assert.h"
	#define MAMLIB_ASSERT(b, m) assert(b && m)
	#include "stdio.h"
	#define MAMLIB_PUTCHAR putchar
	#include "stdarg.h"
#endif

#ifdef MAMLIB_STB_SPRINTF
	#ifdef MAMLIB_IMPLEMENTATION
		#define STB_SPRINTF_IMPLEMENTATION
	#endif
	#include "mam_sprintf.h"
#endif


typedef MAMLIB_MEMINT mam_int;
typedef MAMLIB_BYTE mam_byte;

//////////////////////////////////////////////////////////////////////////////
//
//                         Strings
//

typedef struct MamString {
	char* ptr;
	mam_int count;
} MamString;


MAMLIB__DECLS MamString mam_tostr(char* str) {
	MamString ret = {str};
	ret.count = MAMLIB_STRLEN(str);
	return ret;
}
MAMLIB__DECLS MamString mam_consttostr(const char* str) {
	MamString ret;
	ret.ptr = (char*)str;//lol
	ret.count = MAMLIB_STRLEN(str);
	return ret;
}
MAMLIB__DECLS int mam_streq(MamString str0, MamString str1) {
	return str0.count == str1.count && MAMLIB_MEMCMP(str0.ptr, str1.ptr, str0.count) == 0;
}
MAMLIB__DECLS int mam_strcmp(MamString str0, MamString str1) {
	return str0.count > str1.count ? 1 : (str0.count < str1.count ? -1 : MAMLIB_MEMCMP(str0.ptr, str1.ptr, str0.count));
}
MAMLIB__DECLS int mam_cstreq(MamString str0, const char* str1) {
	return mam_streq(str0, mam_consttostr(str1));
}
MAMLIB__DECLS int mam_cstrcmp(MamString str0, const char* str1) {
	return mam_strcmp(str0, mam_consttostr(str1));
}

MAMLIB__DECLR int mam_toint(MamString str, mam_int* ret_n);

#ifdef __cplusplus
MAMLIB__DECLS MamString mam_tostr(const char* str) {return mam_consttostr(str);}
#endif

#ifdef MAMLIB_IMPLEMENTATION
MAMLIB__DECLR int mam_toint(MamString str, mam_int* ret_n) {
	int is_pos = str.ptr[0] != '-';
	mam_int n = 0;
	mam_int i = 0;
	i += 1 - is_pos;
	for(; i < str.count; i += 1) {
		char ch = str.ptr[i];
		if('0' <= ch && ch <= '9') {
			n *= 10;
			n += ch - '0';
		} else {
			return 0;
		}
	}
	*ret_n = (2*is_pos - 1)*n;
	return 1;
}

#endif

//////////////////////////////////////////////////////////////////////////////
//
//                         Generalized Allocator API
//

typedef enum MamAllocMode {
	MAM_MODE_ALLOC,
	MAM_MODE_REALLOC,
	MAM_MODE_FREE,
	MAM_MODE_FREEALL,
} MamAllocMode;

typedef void* MamAllocator(MamAllocMode mode, mam_int alloc_size, void* old_ptr, void* allocator_data);


#define mam_gen_malloc(allocator, alloc_size, allocator_data) ((allocator)(MAM_MODE_ALLOC, alloc_size, 0, allocator_data))
#define mam_gen_realloc(allocator, alloc_size, old_ptr, allocator_data) ((allocator)(MAM_MODE_REALLOC, alloc_size, old_ptr, allocator_data))
#define mam_gen_free(allocator, old_ptr, allocator_data) ((allocator)(MAM_MODE_FREE, 0, old_ptr, allocator_data))
#define mam_gen_freeall(allocator, allocator_data) ((allocator)(MAM_MODE_FREEALL, 0, 0, allocator_data))



MAMLIB__DECLR void* mam__system_allocator(MamAllocMode mode, mam_int alloc_size, void* old_ptr, void* allocator_data);
#define mam_system_allocator mam_check_system_allocator


#ifdef MAMLIB_IMPLEMENTATION

MAMLIB__DECLR void* mam__system_allocator(MamAllocMode mode, mam_int alloc_size, void* old_ptr, void* allocator_data) {
	if(mode == MAM_MODE_ALLOC) {
		return MAMLIB_MALLOC(alloc_size);
	} else if(mode == MAM_MODE_REALLOC) {
		return MAMLIB_REALLOC(old_ptr, alloc_size);
	} else if(mode == MAM_MODE_FREE) {
		MAMLIB_FREE(old_ptr);
	}
	return 0;
}
#endif


#define malloc(size) mam_gen_malloc(mam_system_allocator, size, 0)
#define realloc(ptr, size) mam_gen_realloc(mam_system_allocator, size, ptr, 0)
#define free(ptr) mam_gen_free(mam_system_allocator, ptr, 0)

//////////////////////////////////////////////////////////////////////////////
//
//                         Memory checking
//

MAMLIB__DECLS mam_int mamlib_align(mam_int a) {
	//raises a to the nearest multiple of MAMLIB_ALIGNMENT
	return MAMLIB_ALIGNMENT*((a + (MAMLIB_ALIGNMENT - 1))/MAMLIB_ALIGNMENT);
}

#define mam_ptr_add(type, ptr, n) ((type*)((mam_byte*)(ptr) + (n)))


#ifdef MAMLIB_DEBUG
#define mam__underflow_msg "mamlib: invalid buffer detected; either an invalid pointer was used, a buffer underflow occured, or an overflow of an adjacent buffer occured"
#define mam__overflow_msg "mamlib: buffer overflow detected"

#define MAM_CHECK_BASE_SIZE (mamlib_align(2*sizeof(mam_int)))
#define MAM_CHECK_FULL_SIZE (MAM_CHECK_BASE_SIZE + sizeof(mam_int))

MAMLIB__DECLS void mam_check_mark(void* buffer, mam_int buffer_size) {
	*mam_ptr_add(mam_int, buffer, -(mam_int)sizeof(mam_int)) = MAMLIB_CHECK_COOKIE;
	*mam_ptr_add(mam_int, buffer, -(mam_int)2*sizeof(mam_int)) = buffer_size;
	*mam_ptr_add(mam_int, buffer, buffer_size) = MAMLIB_CHECK_COOKIE;
}
MAMLIB__DECLS void* mam_check_on_alloc(void* mem, mam_int buffer_size) {
	void* buffer = mam_ptr_add(void*, mem, MAM_CHECK_BASE_SIZE);
	mam_check_mark(buffer, buffer_size);
	return buffer;
}

/*******************************************************************************
If your debugger takes you here because of an assertion, look at the assertion
message for more information, and walk the stack back up to your own code.
This library provides the following memory check functions to do quick and
dirty memory intergrity tests while the library is in debug mode.
They catch the most common bugs such as buffer overflows or use-after-free.
This works by placing memory cookies above and below every allocated buffer,
and checking their existance regularly.
The default cookie is 0x4433443344334433 == 4914346605369574451 == "3D3D3D3D"
********************************************************************************/
MAMLIB__DECLS void mam_check(void* buffer) {
	MAMLIB_ASSERT(*(int*)((mam_byte*)buffer - sizeof(mam_int)) == MAMLIB_CHECK_COOKIE, mam__underflow_msg);
	mam_int buffer_size = *mam_ptr_add(mam_int, buffer, -(mam_int)2*sizeof(mam_int))
	MAMLIB_ASSERT(*(int*)((mam_byte*)buffer + buffer_size) == MAMLIB_CHECK_COOKIE, mam__overflow_msg);
}

MAMLIB__DECLS void mam_check_unmark(void* buffer) {
	MAMLIB_ASSERT(*(int*)((mam_byte*)buffer - sizeof(mam_int)) == MAMLIB_CHECK_COOKIE, mam__underflow_msg);
	mam_int buffer_size = *mam_ptr_add(mam_int, buffer, -(mam_int)2*sizeof(mam_int))
	MAMLIB_ASSERT(*(int*)((mam_byte*)buffer + buffer_size) == MAMLIB_CHECK_COOKIE, mam__overflow_msg);
	*mam_ptr_add(int, buffer, -(mam_int)sizeof(mam_int)) = 0;
	*mam_ptr_add(int, buffer, buffer_size) = 0;
}

MAMLIB__DECLS void* mam_check_allocation(MamAllocator* allocator, MamAllocMode mode, mam_int alloc_size, void* old_ptr, void* allocator_data) {
	if(mode == MAM_MODE_ALLOC) {
		if(alloc_size) {
			return mam_check_on_alloc(allocator(mode, alloc_size + MAM_CHECK_FULL_SIZE, old_ptr, allocator_data), alloc_size);
		}
	} else if(mode == MAM_MODE_REALLOC) {
		mam_check_unmark(old_ptr);
		return mam_check_on_alloc(allocator(mode, alloc_size + MAM_CHECK_FULL_SIZE, old_ptr, allocator_data), alloc_size);
	} else if(mode == MAM_MODE_FREE) {
		mam_check_unmark(old_ptr);
	}
	return allocator(mode, alloc_size, old_ptr, allocator_data);
}
#else
#define MAM_CHECK_BASE_SIZE 0
#define MAM_CHECK_MARK_SIZE 0

#define mam_check_mark(buffer, buffer_size) 0
#define mam_check_alloc(mem, buffer_size) ((void*)(mem))

#define mam_check(buffer, buffer_size) 0

#define mam_check_unmark(buffer, buffer_size) 0

MAMLIB__DECLS void* mam_check_allocation(MamAllocator* allocator, MamAllocMode mode, mam_int alloc_size, void* old_ptr, void* allocator_data) {
	return allocator(mode, alloc_size, old_ptr, allocator_data);
}
#endif


MAMLIB__DECLR void* mam_check_system_allocator(MamAllocMode mode, mam_int alloc_size, void* old_ptr, void* allocator_data);


#ifdef MAMLIB_IMPLEMENTATION

MAMLIB__DECLR void* mam_check_system_allocator(MamAllocMode mode, mam_int alloc_size, void* old_ptr, void* allocator_data) {
	return mam_check_allocation(&mam__system_allocator, mode, alloc_size, old_ptr, allocator_data);
}
#endif

//////////////////////////////////////////////////////////////////////////////
//
//                         Stack Allocator
//

typedef struct MamStack {
	mam_int size;
	mam_int highest_size;
	mam_int capacity;
} MamStack;


MAMLIB__DECLR MamStack* mam_stack_init(void* alloc_ptr, mam_int alloc_size);
MAMLIB__DECLR void* mam__stack_allocator(MamAllocMode mode, mam_int alloc_size, void* old_ptr, void* allocator_data);
MAMLIB__DECLR void* mam_stack_allocator(MamAllocMode mode, mam_int alloc_size, void* old_ptr, void* allocator_data);


MAMLIB__DECLS void* mam_stack_push(MamStack* stack, mam_int size) {
	return mam_gen_malloc(mam_stack_allocator, size, stack);
}
MAMLIB__DECLS void mam_stack_pop(MamStack* stack, void* ptr) {
	mam_gen_free(mam_stack_allocator, ptr, stack);
}
MAMLIB__DECLS void mam_stack_extend(MamStack* stack, void* ptr, mam_int size) {
	mam_gen_realloc(mam_stack_allocator, size, ptr, stack);
}
MAMLIB__DECLS void mam_stack_set_size(MamStack* stack, mam_int new_size) {
	stack->size = new_size;
	if(stack->highest_size < new_size) stack->highest_size = new_size;
	MAMLIB_ASSERT(new_size <= stack->capacity, "mamlib: stack is out of memory");
}
#define mam_stack_pusht(type, stack, size) ((type*)mam_stack_push(stack, sizeof(type)*(size)))


#ifdef MAMLIB_IMPLEMENTATION

MAMLIB__DECLR MamStack* mam_stack_init(void* alloc_ptr, mam_int alloc_size) {
	MamStack* stack = (MamStack*)alloc_ptr;
	stack->size = 0;
	stack->highest_size = 0;
	stack->capacity = alloc_size - sizeof(MamStack);
	return stack;
}

MAMLIB__DECLR void* mam__stack_allocator(MamAllocMode mode, mam_int alloc_size, void* old_ptr, void* allocator_data) {
	MamStack* stack = (MamStack*)allocator_data;
	if(mode == MAM_MODE_ALLOC) {
		alloc_size = mamlib_align(alloc_size);
		mam_int new_size = stack->size + alloc_size;

		void* mem = mam_ptr_add(void, stack, sizeof(MamStack) + stack->size);
		stack->size = new_size;
		if(stack->highest_size < new_size) stack->highest_size = new_size;
		if(new_size > stack->capacity) {
			MAMLIB_ASSERT(new_size > stack->capacity, "mamlib: stack is out of memory");
			mem = MAMLIB_MALLOC(alloc_size);
		}

		return mem;
	} else if(mode == MAM_MODE_REALLOC) {
		mam_int new_size = old_ptr - mam_ptr_add(mam_byte, stack, sizeof(MamStack));
		MAMLIB_ASSERT(new_size > 0, "mamlib: attempt to pop stack when stack is empty");

		alloc_size = mamlib_align(alloc_size);
		new_size = new_size + alloc_size;

		void* mem = mam_ptr_add(void, stack, sizeof(MamStack) + stack->size);
		stack->size = new_size;
		if(stack->highest_size < new_size) stack->highest_size = new_size;
		if(new_size > stack->capacity) {
			MAMLIB_ASSERT(new_size > stack->capacity, "mamlib: stack is out of memory");
			mem = MAMLIB_MALLOC(alloc_size);
		}
		return mem;
	} else if(mode == MAM_MODE_FREE) {
		mam_int new_size = old_ptr - mam_ptr_add(mam_byte, stack, sizeof(MamStack));

		stack->size = new_size;
		MAMLIB_ASSERT(new_size > 0, "mamlib: attempt to pop stack when stack is empty");
	} else if(mode == MAM_MODE_FREEALL) {
		stack->size = 0;
	}
	return 0;
}
MAMLIB__DECLR void* mam_stack_allocator(MamAllocMode mode, mam_int alloc_size, void* old_ptr, void* allocator_data) {
	return mam_check_allocation(&mam__stack_allocator, mode, alloc_size, old_ptr, allocator_data);
}
#endif

//////////////////////////////////////////////////////////////////////////////
//
//                         Generalized Logging API
//

typedef void MamLogger(MamString str, void* logger_data);

MAMLIB__DECLR void mam__system_logger(MamString str, void* logger_data);
#define mam_system_logger mam__system_logger

#ifdef MAMLIB_IMPLEMENTATION
MAMLIB__DECLR void mam_system_logger(MamString str, void* logger_data) {
	for(mam_int i = 0; i < str.count; i += 1) MAMLIB_PUTCHAR(str.ptr[i]);
}
#endif


//////////////////////////////////////////////////////////////////////////////
//
//                         Fast String Builder
//

#define MAM_BUILDER_BUFFER_CAPACITY (1024 - sizeof(void*) - sizeof(mam_int))
typedef struct MamBuilderBuffer {
	struct MamBuilderBuffer* next;
	mam_int size;
	mam_byte mem[MAM_BUILDER_BUFFER_CAPACITY];
} MamBuilderBuffer;

typedef struct MamBuilder {
	mam_int size;
	mam_int failed;
	MamAllocator* allocator;
	void* allocator_data;
	MamBuilderBuffer head;
	MamBuilderBuffer* tail;
} MamBuilder;

MAMLIB__DECLR void mam_builder_init(MamBuilder* builder);
MAMLIB__DECLR void mam_builder_delete(MamBuilder* builder);
MAMLIB__DECLR void mam_builder_push_char(MamBuilder* builder, char ch);
MAMLIB__DECLR void mam_builder_push_mem(MamBuilder* builder, const void* mem, mam_int mem_size);
MAMLIB__DECLS void mam_builder_push_str(MamBuilder* builder, MamString str) {
	mam_builder_push_mem(builder, str.ptr, str.count);
}
MAMLIB__DECLS void mam_builder_push_cstr(MamBuilder* builder, const char* str) {
	mam_builder_push_str(builder, mam_consttostr(str));
}
MAMLIB__DECLR void mam_builder_push_uint(MamBuilder* builder, mam_int i);
MAMLIB__DECLR void mam_builder_push_int(MamBuilder* builder, mam_int i);
MAMLIB__DECLR void mam_builder_push_hex_(MamBuilder* builder, mam_int i);
MAMLIB__DECLR void mam_builder_push_hex(MamBuilder* builder, mam_int i);

#ifdef MAMLIB_STB_SPRINTF
MAMLIB__DECLR void mam_builder_pushf(MamBuilder* builder, const char* fmt, ...);
#endif


MAMLIB__DECLR void mam_builder_log(MamBuilder* builder, MamLogger* logger, void* logger_data);
MAMLIB__DECLS void mam_builder_print(MamBuilder* builder) {
	mam_builder_log(builder, &mam_system_logger, 0);
}


MAMLIB__DECLS MamString mam_builder_copy(MamBuilder* builder) {return mam_builder_tostr(builder, &mam_system_allocator, 0);}
MAMLIB__DECLR MamString mam_builder_copytogen(MamBuilder* builder, MamAllocator* allocator, void* allocator_data);
MAMLIB__DECLR void mam_builder_copytomem(MamBuilder* builder, char* mem, mam_int mem_size);


#ifdef MAMLIB_IMPLEMENTATION
MAMLIB__DECLR void mam_builder_init(MamBuilder* builder) {
	builder->head.next = 0;
	builder->head.size = 0;
	builder->tail = &builder->head;
	builder->allocator = &mam_system_allocator;
	builder->allocator_data = 0;
	builder->failed = 0;
}
MAMLIB__DECLR void mam_builder_delete(MamBuilder* builder) {
	MamBuilderBuffer* head = &builder->head;
	while(head) {
		mam_gen_free(builder->allocator, head, builder->allocator_data);
		head = head->next;
	}
}

MAMLIB__DECLR void mam_builder_push_char(MamBuilder* builder, char ch) {
	MamBuilderBuffer* tail = builder->tail;
	if(tail->size >= MAM_BUILDER_BUFFER_CAPACITY) {
		MamBuilderBuffer* new_tail = (MamBuilderBuffer*)mam_gen_malloc(builder->allocator, sizeof(MamBuilderBuffer), builder->allocator_data);
		if(!new_tail) {builder->failed = 1; return;}
		new_tail->size = 0;
		new_tail->next = 0;
		tail->next = new_tail;
		tail = new_tail;
	}
	tail->mem[tail->size] = ch;
	tail->size += 1;
	builder->size += 1;
}
MAMLIB__DECLR void mam_builder_push_mem(MamBuilder* builder, const void* mem, mam_int mem_size) {
	MamBuilderBuffer* tail = builder->tail;
	while(1) {
		mam_int size_left = MAM_BUILDER_BUFFER_CAPACITY - tail->size;
		if(mem_size > size_left) {
			MAMLIB_MEMCPY(&tail->mem[tail->size], mem, size_left);
			tail->size += size_left;
			builder->size += size_left;
			mem_size -= size_left;
			mem = mam_ptr_add(void, mem, size_left);

			MamBuilderBuffer* new_tail = (MamBuilderBuffer*)mam_gen_malloc(builder->allocator, sizeof(MamBuilderBuffer), builder->allocator_data);
			if(!new_tail) {builder->failed = 1; return;}
			new_tail->size = 0;
			new_tail->next = 0;
			tail->next = new_tail;
			tail = new_tail;
		} else {
			MAMLIB_MEMCPY(&tail->mem[tail->size], mem, mem_size);
			tail->size += mem_size;
			return;
		}
	}
}

MAMLIB__DECLR void mam_builder_push_uint(MamBuilder* builder, mam_int i) {
	if(i > 10) mam_builder_push_uint(builder, i/10);
	mam_builder_push_char(builder, i%10 + '0');
}
MAMLIB__DECLR void mam_builder_push_int(MamBuilder* builder, mam_int i) {
	if(i < 0) {
		mam_builder_push_char(builder, '-');
		mam_builder_push_uint(builder, -i);
	} else {
		mam_builder_push_uint(builder, i);
	}
}
MAMLIB__DECLR void mam_builder_push_hex_(MamBuilder* builder, mam_int i) {
	if(i > 16) mam_builder_push_uint(builder, i/16);
	mam_builder_push_char(builder, i%16 + '0');
}
MAMLIB__DECLR void mam_builder_push_hex(MamBuilder* builder, mam_int i) {
	mam_builder_push_char(builder, '0');
	mam_builder_push_char(builder, 'x');
	mam_builder_push_hex_(builder, i);
}

#ifdef MAMLIB_STB_SPRINTF
MAMLIB__DECLR char* mam__builder_pushf_cb(char const* buffer, void* user, int buffer_size) {
	mam_builder_push_mem((MamBuilder*)user, buffer, buffer_size);
}
MAMLIB__DECLR int mam_builder_pushf(MamBuilder* builder, const char* fmt, ...) {
	char buffer[STB_SPRINTF_MIN];
	int ret;
	va_list va;
	va_start(va, fmt);
	ret = stbsp_vsprintfcb(&mam__builder_pushf_cb, builder, 0, buffer, fmt, va);
	va_end(va);
	return ret;
}
#endif

MAMLIB__DECLR void mam_builder_log(MamBuilder* builder, MamLogger* logger, void* logger_data) {
	MamBuilderBuffer* head = &builder->head;
	while(head) {
		MamString str = {(char*)head->mem, head->size};
		logger(MAM_MODE_DEFAULT, str, logger_data);
		head = head->next;
	}
}

MAMLIB__DECLR void mam_builder_copytomem(MamBuilder* builder, char* mem, mam_int mem_size) {
	MamBuilderBuffer* head = &builder->head;
	while(head) {
		if(mem_size < head->size) {
			MAMLIB_MEMCPY(mem, head->mem, mem_size);
			break;
		} else {
			MAMLIB_MEMCPY(mem, head->mem, head->size);
			mem += head->size;
			mem_size -= head->size;
			head = head->next;
		}
	}
}
MAMLIB__DECLR MamString mam_builder_copytogen(MamBuilder* builder, MamAllocator* allocator, void* allocator_data) {
	MamString str = {(char*)mam_gen_malloc(allocator, builder->size, allocator_data), builder->size};
	if(!str.ptr) return str;
	MamBuilderBuffer* head = &builder->head;
	while(head) {
		MAMLIB_MEMCPY(str.ptr, head->mem, head->size);
		head = head->next;
	}
	return str;
}

#endif

#ifdef MAMLIB_IMPLEMENTATION
#undef MAMLIB_IMPLEMENTATION
#endif
