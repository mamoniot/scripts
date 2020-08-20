//By Monica Moniot
#ifndef MAMLIB__H_INCLUDE
#define MAMLIB__H_INCLUDE

#ifdef __cplusplus
 #ifdef MAMLIB_STATIC
  #define MAMLIB__DECLR extern "C" static
 #else
  #define MAMLIB__DECLR extern "C"
 #endif
#else
 #ifdef MAMLIB_STATIC
  #define MAMLIB__DECLR static
 #else
  #define MAMLIB__DECLR
 #endif
#endif

#define MAMLIB__DECLS static inline

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
#undef MAMLIB_IMPLEMENTATION

MAMLIB__DECLR int mam_toint(MamString str, mam_int* ret_n) {
	int is_neg = str.ptr[0] == '-';
	mam_int n = 0;
	mam_int i = 0;
	i += is_neg;
	for(;i < str.count; i += 1) {
		char ch = str.ptr[i];
		if('0' <= ch && ch <= '9') {
			n *= 10;
			n += ch - '0';
		} else {
			return 0;
		}
	}
	*ret_n = (2*is_neg)*n;
	return 1;
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
	MamBuilderBuffer head;
	MamBuilderBuffer* tail;
	MamAllocator* allocator;
	void* allocator_data;
	mam_int size;
	mam_int failed;
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

MAMLIB__DECLR void mam_builder_push_charf(MamBuilder* builder, const char* origin, char source);
MAMLIB__DECLR void mam_builder_push_strf(MamBuilder* builder, const char* origin, MamString source);
MAMLIB__DECLS void mam_builder_push_cstrf(MamBuilder* builder, const char* origin, const char* source) {
	mam_builder_push_strf(builder, origin, mam_consttostr(source));
}
MAMLIB__DECLR void mam_builder_push_intf(MamBuilder* builder, const char* origin, mam_int source);
MAMLIB__DECLR void mam_builder_push_hexf(MamBuilder* builder, const char* origin, mam_int source);


MAMLIB__DECLR void mam_builder_log(MamBuilder* builder, MamLogger* logger, void* logger_data);
MAMLIB__DECLS void mam_builder_print(MamBuilder* builder) {
	mam_builder_print(builder, &mam_system_logger, 0);
}


MAMLIB__DECLR MamString mam_builder_tomem(MamBuilder* builder, MamAllocator* allocator, void* allocator_data);
MAMLIB__DECLS MamString mam_builder_tostr(MamBuilder* builder) {return mam_builder_tostr(builder, &mam_system_allocator, 0);}


#ifdef MAMLIB_IMPLEMENTATION
#undef MAMLIB_IMPLEMENTATION
MAMLIB__DECLR void mam_builder_init(MamBuilder* builder) {
	builder->head.next = 0;
	builder->head.size = 0;
	builder->tail = &builder->head;
	builder->allocator = &mam_system_allocator;
	builder->allocator_data = 0;
	builder->failed = 0;
}

MAMLIB__DECLR void mam_builder_push_char(MamBuilder* builder, char ch) {
	MamBuilderBuffer* tail = builder->tail;
	if(tail->size >= MAM_BUILDER_BUFFER_CAPACITY) {
		MamBuilderBuffer* new_tail = (MamBuilderBuffer*)builder->allocator(MAM_MODE_ALLOC, sizeof(MamBuilderBuffer), 0, 0, builder->allocator_data);
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

			MamBuilderBuffer* new_tail = (MamBuilderBuffer*)builder->allocator(MAM_MODE_ALLOC, sizeof(MamBuilderBuffer), 0, 0, builder->allocator_data);
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

MAMLIB__DECLR void mam_builder_push_charf(MamBuilder* builder, const char* origin, char source) {
	for(; *origin; origin += 1) {
		if(*origin == '%') {
			mam_builder_push_char(builder, source);
		} else {
			mam_builder_push_char(builder, *origin);
		}
	}
}
MAMLIB__DECLR void mam_builder_push_strf(MamBuilder* builder, const char* origin, MamString source) {
	for(; *origin; origin += 1) {
		if(*origin == '%') {
			mam_builder_push_str(builder, source);
		} else {
			mam_builder_push_char(builder, *origin);
		}
	}
}
MAMLIB__DECLR void mam_builder_push_intf(MamBuilder* builder, const char* origin, mam_int source) {
	for(; *origin; origin += 1) {
		if(*origin == '%') {
			mam_builder_push_int(builder, source);
		} else {
			mam_builder_push_char(builder, *origin);
		}
	}
}
MAMLIB__DECLR void mam_builder_push_hexf(MamBuilder* builder, const char* origin, mam_int source) {
	for(; *origin; origin += 1) {
		if(*origin == '%') {
			mam_builder_push_hex(builder, source);
		} else {
			mam_builder_push_char(builder, *origin);
		}
	}
}


MAMLIB__DECLR void mam_builder_print(MamBuilder* builder, MamLogger* logger, void* logger_data) {
	MamBuilderBuffer* head = &builder->head;
	while(head) {
		MamString str = {(char*)head->mem, head->size};
		logger(MAM_MODE_DEFAULT, str, logger_data);
		head = head->next;
	}
}


MAMLIB__DECLR MamString mam_builder_to_string(MamBuilder* builder, MamAllocator* allocator, void* allocator_data) {
	MamString str = {(char*)allocator(MAM_MODE_ALLOC, builder->size, 0, 0, allocator_data), builder->size};
	if(!str.ptr) return str;
	MamBuilderBuffer* head = &builder->head;
	while(head) {
		MAMLIB_MEMCPY(str.ptr, head->mem, head->size);
		head = head->next;
	}
	return str;
}

#endif
