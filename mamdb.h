

#ifndef MAMDB__H_INCLUDE
#define MAMDB__H_INCLUDE


#include "inttypes.h"

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef uint8_t  byte;
typedef uint32_t uint;
typedef  int8_t   int8;
typedef uint8_t  uint8;
typedef  int16_t  int16;
typedef uint16_t uint16;
typedef  int32_t  int32;
typedef uint32_t uint32;
typedef  int64_t  int64;
typedef uint64_t uint64;


//#define MAX_UINT32 0xffffffffu
//#define MAX_UINT64 0xffffffffffffffffull
//#define MIN_INT32 0x80000000
//#define MAX_INT32 0x7fffffff
//#define MIN_INT64 0x8000000000000000ll
//#define MAX_INT64 0x7fffffffffffffffll

#define mamdb_cast(type, value) ((type)(value))
#define mamdb_ptr_add(type, ptr, n) ((type*)((byte*)(ptr) + (n)))
#define mamdb_ptr_sub(ptr0, ptr1) ((inta)((byte*)(ptr0) - (byte*)(ptr1)))

#define mamdb_memcpy memcpy
//#define memzro(ptr, size) memset(ptr, 0, size)
//#define memzero(ptr, size) memset(ptr, 0, sizeof(*ptr)*(size))
//#define memcopy(ptr0, ptr1, size) memcpy(ptr0, ptr1, sizeof(*ptr0)*(size))
//#define from_cstr(str) str, strlen(str)
//#define swapt(type, v0, v1) do {type mam__t = *(v0); *(v0) = *(v1); *(v1) = mam__t} while(0);
//#define malloct(type, size) ((type*)malloc(sizeof(type)*(size)))
//#define realloct(type, ptr, size) ((type*)realloc(ptr, sizeof(type)*(size)))

#ifndef alloca
	#ifdef _MSC_VER
		#include <malloc.h>
		#ifndef alloca
			#define alloca(size) _alloca(size)
		#endif
	#else
		#include <alloca.h>
	#endif
#endif
#define allocat(type, size) ((type*)alloca(sizeof(type)*(size)))


typedef uint64_t BlockId;

#define MAMDB_BLOCK_SIZE (4<<10)

struct BTree {
	BlockId root;
	uint32 root_height;
};

//bool mamdb_getblock(BlockId id, void* ret_block_mem);
//bool mamdb_newblock(BlockId* ret_id);
//bool mamdb_delblock(BlockId id);
//bool mamdb_setblock(BlockId id, void* block_mem);

bool mamdbc_getblock(BTree* btree, uint32 height, BlockId id, byte** ret_block_ptr);
bool mamdbc_newblock(BTree* btree, uint32 height, BlockId* ret_id, byte** ret_block_ptr);
bool mamdbc_delblock(BTree* btree, uint32 height, BlockId id);
bool mamdbc_repblock(BTree* btree, uint32 height, BlockId id, byte** ret_block_ptr);
bool mamdbc_freeallblocks(BTree* btree, uint32 height, BlockId id);
bool mamdbc_flush(BTree* btree);


bool mamdb_get(BTree* btree, uint64 key, uint64* ret_val) {
	void* node_data = alloca(BLOCK_SIZE);
	BlockId node_id = btree->root;
	while(true) {
		mamdb_getblock(node_id, node_data);

		bool is_leaf = (uint64*)node_data;
		uint64 next_leaf = (uint64*)node_data;
		uint64* keys = (uint64*)node_data;
		uint32 kv_size = (uint32*)node_data;
		uint64* vals = (uint64*)node_data;

		uint32 start = 0;
		uint32 end = kv_size;
		uint32 closest_key_i;
		bool is_exact = false;
		while(true) {
			uint32 mid = (start + end)/2;
			uint64 cur_key = keys[mid];
			if(key < cur_key) {
				end = mid;
			} else if(key > cur_key) {
				start = mid + 1;
			} else {
				closest_key_i = mid;
				is_exact = true;
				break;
			}
			if(start == end) {
				closest_key_i = start;
				break;
			}
		}

		if(is_leaf) {
			if(is_exact) {
				*ret_val = vals[closest_key_i];
				return 1;
			} else {
				return 0;
			}
		} else {
			node_id = (BlockId)vals[closest_key_i];
		}
	}
}

#define mamdb_mempush(dest, src, size) mamdb_memcpy(dest, src, size); dest += size
#define mamdb_mempush_single(type, dest, src) *(type*)dest = src; dest += sizeof(type)

bool mamdb_set(BTree* btree, uint64 key, uint64 value) {
	//TODO: handle empty tree
	//root is always the highest node, leafs are always at height 0
	uint32 max_size = (MAMDB_BLOCK_SIZE - 16)/16;
	//ASSERT((MAMDB_BLOCK_SIZE - 16)/16 < MAX_UINT32 && (MAMDB_BLOCK_SIZE - 16)/16 > 2)

	uint32 root_height = btree->root_height;
	byte** nodes = (byte**)alloca(root_height*sizeof(byte*));
	BlockId* node_ids = (BlockId*)alloca(root_height*sizeof(BlockId));
	uint64* key_path = (uint64*)alloca(root_height*sizeof(uint64));
	uint32 cur_height = root_height;

	node_ids[cur_height] = btree->root;
	while(true) {
		uint64 node_id = node_ids[cur_height];
		mamdbc_getblock(btree, cur_height, node_id, &nodes[cur_height]);
		byte* node_data = nodes[cur_height];

		// | MAMDB_BLOCK_SIZE                                                |
		// | 8       | 8*kv_size | 8*kv_size | 8                         | - |
		// | kv_size | keys, ... | vals, ... | val[kv_size] or next_leaf | - |
		uint32 kv_size = *mamdb_ptr_add(uint32, node_data, 0);
		uint64* keys = mamdb_ptr_add(uint64, node_data, 8);
		uint64* vals = mamdb_ptr_add(uint64, node_data, 8 + 8*kv_size);
		bool is_leaf = cur_height == 0;


		uint32 start = 0;
		uint32 end = kv_size;
		uint32 closest_key_i;
		bool is_exact = false;
		while(true) {
			uint32 mid = (start + end)/2;
			uint64 cur_key = keys[mid];
			if(key < cur_key) {
				end = mid;
			} else if(key > cur_key) {
				start = mid + 1;
			} else {
				closest_key_i = mid;
				is_exact = true;
				break;
			}
			if(start == end) {
				closest_key_i = start;
				break;
			}
		}
		key_path[cur_height] = closest_key_i;
		//ASSERT(key < keys[closest_key_i])

		if(is_leaf) {
			if(is_exact) {
				vals[closest_key_i] = value;
				return 1;
			} else {
				break;
			}
		} else {
			cur_height -= 1;
			node_ids[cur_height] = (BlockId)vals[closest_key_i];
		}
	}

	//insert a key and value
	uint64 ins_key = key;
	uint64 ins_val = value;
	uint64 ins_i = key_path[cur_height];
	while(true)  {
		//ASSERT(ins_key_i <= ins_i && ins_key_i >= ins_i - 1)
		uint64 node_id = node_ids[cur_height];
		byte* node_data = nodes[cur_height];

		uint64 size = *mamdb_ptr_add(uint64, node_data, 0);
		uint64* keys = mamdb_ptr_add(uint64, node_data, 8);
		uint64* vals = mamdb_ptr_add(uint64, node_data, 8 + 8*size);
		bool is_leaf = cur_height == 0;

		byte* dest;
		if(size < max_size) {
			byte* rep_data;
			mamdbc_repblock(btree, cur_height, node_id, &rep_data);
			//insert into block
			// | node_data                       |
			// | 8    | 8*size    | 8*(size + 1) |
			// | size | keys, ... | vals, ...    |
			//------>
			// | rep_data                                                                                       |
			// | 8        | 8*ins_i   | 8       | 8*(size - ins_i) | 8*(ins_i + 1) | 8       | 8*(size - ins_i) |
			// | size + 1 | keys, ... | ins_key | keys[ins_i], ..  | vals, ...     | ins_val | vals[ins_i], ... |
			//====>
			dest = rep_data;
			mamdb_mempush_single(uint64, dest, size + 1);
			mamdb_mempush(dest, keys, 8*ins_i);
			mamdb_mempush_single(uint64, dest, ins_key);
			mamdb_mempush(dest, &keys[ins_i], 8*(size - ins_i));
			mamdb_mempush(dest, vals, 8*(ins_i + 1));
			mamdb_mempush_single(uint64, dest, ins_val);
			mamdb_mempush(dest, &keys[ins_i], 8*(size - ins_i));

			return 1;
		} else {
			//split
			BlockId new_id;
			byte* new_data,* rep_data;
			mamdbc_newblock(btree, cur_height, &new_id, &new_data);
			mamdbc_repblock(btree, cur_height, node_id, &rep_data);
			/*
			if(ins_i <= (size + 1)/2 - 1) {
				if(is_leaf) {
					rep_data = | (size + 1)/2 | keys, ...ins_i | ins_key | keys[ins_i], ...(size + 1)/2 - 1 | new_id | vals[1], ...ins_i + 1 | ins_val | vals[ins_i + 1], ...(size + 1)/2 - 1 |
					new_data = | size/2 + 1 | keys[(size + 1)/2 - 1], ...size | vals[0] | vals[(size + 1)/2 - 1], ...size |
					ins_key = keys[(size + 1)/2 - 1]
				} else {
					rep_data = | (size + 1)/2 | keys, ...ins_i | ins_key | keys[ins_i], ...(size + 1)/2 - 1 | vals, ...ins_i + 1 | ins_val | vals[ins_i + 1], ...(size + 1)/2 |
					new_data = | size/2 | keys[(size + 1)/2], ...size | vals[(size + 1)/2], ...size |
					ins_key = keys[(size + 1)/2 - 1]
				}
			} else {
				if(is_leaf) {
					rep_data = | (size + 1)/2 | keys, ...(size + 1)/2 | new_id | vals, ...(size + 1)/2 |
					new_data = | size/2 + 1 | keys[(size + 1)/2], ...ins_i | ins_key | keys[ins_i], ...size | vals[0] | vals[(size + 1)/2], ...ins_i + 1 | ins_val | vals[ins_i + 1], ...size |
					ins_key = ins_i == (size + 1)/2 ? ins_key : keys[(size + 1)/2]
				} else {
					rep_data = | (size + 1)/2 | keys, ...(size + 1)/2 | vals, ...(size + 1)/2 + 1 |
					new_data = | size/2 | keys[(size + 1)/2 + 1], ...ins_i | ins_key | keys[ins_i], ...size | vals[(size + 1)/2 + 1], ...ins_i + 1 | ins_val | vals[ins_i + 1], ...size | vals[size] |
					ins_key = ins_i == (size + 1)/2 ? ins_key : keys[(size + 1)/2]
				}
			}

			====>

			if(ins_i <= (size + 1)/2 - 1) {
				rep_data = | (size + 1)/2 | keys, ...ins_i | ins_key | keys[ins_i], ...(size + 1)/2 - 1 | vals, ...ins_val_i | ins_val | vals[ins_val_i], ...(size + 1)/2 - 1 | is_leaf ? new_id : vals[(size + 1)/2 - 1] |
				new_data = | size/2 + is_leaf | keys[(size + 1)/2 - is_leaf], ...size | vals[(size + 1)/2 - is_leaf], ...size + 1 |
				ins_key = keys[(size + 1)/2 - 1]
			} else {
				rep_data = | (size + 1)/2 | keys, ...(size + 1)/2 | vals, ...(size + 1)/2 | is_leaf ? new_id : vals[(size + 1)/2] |
				new_data = | size/2 + is_leaf | keys[(size + 1)/2 + 1 - is_leaf], ...ins_i | ins_key | keys[ins_i], ...size | vals[(size + 1)/2 + 1 - is_leaf], ...ins_val_i | ins_val | vals[ins_val_i], ...size | vals[size] |
				ins_key = ins_i == (size + 1)/2 ? ins_key : keys[(size + 1)/2]
			}

			====>
			*/
			if(ins_key_i <= (size + 1)/2 - 1) {//here
				dest = rep_data;
				mamdb_mempush_single(uint64, dest, (size + 1)/2);
				mamdb_mempush(dest, keys, 8*ins_key_i);
				mamdb_mempush_single(uint64, dest, ins_key);
				mamdb_mempush(dest, &keys[ins_key_i], 8*((size + 1)/2 - 1 - ins_key_i));
				mamdb_mempush(dest, vals, 8*ins_val_i);
				mamdb_mempush_single(uint64, dest, ins_val);
				mamdb_mempush(dest, &vals[ins_val_i], 8*((size + 1)/2 - 1 - ins_val_i));
				mamdb_mempush_single(uint64, dest, is_leaf ? new_id : vals[(size + 1)/2 - 1]);

				dest = new_data;
				mamdb_mempush_single(uint64, dest, size/2 + is_leaf);
				mamdb_mempush(dest, &keys[(size + 1)/2 - is_leaf], 8*(size - ((size + 1)/2 - is_leaf)));
				mamdb_mempush(dest, &vals[(size + 1)/2 - is_leaf], 8*(size + 1 - ((size + 1)/2 - is_leaf)));

				ins_key = keys[(size + 1)/2 - 1];
			} else {
				dest = rep_data;
				mamdb_mempush_single(uint64, dest, (size + 1)/2);
				mamdb_mempush(dest, keys, 8*((size + 1)/2));
				mamdb_mempush(dest, vals, 8*((size + 1)/2));
				mamdb_mempush_single(uint64, dest, is_leaf ? new_id : vals[(size + 1)/2]);

				dest = new_data;
				mamdb_mempush_single(uint64, dest, size/2 + is_leaf);
				if(ins_key_i - ((size + 1)/2 + 1 - is_leaf) >= 0) {
					mamdb_mempush(dest, &keys[(size + 1)/2 + 1 - is_leaf], 8*(ins_key_i - ((size + 1)/2 + 1 - is_leaf)));
					mamdb_mempush_single(uint64, dest, ins_key);
				}
				mamdb_mempush(dest, &keys[ins_key_i], 8*(size - ins_key_i));
				//ASSERT(ins_val_i - ((size + 1)/2 + 1 - is_leaf) >= 0);
				mamdb_mempush(dest, &vals[(size + 1)/2 + 1 - is_leaf], 8*(ins_val_i - ((size + 1)/2 + 1 - is_leaf)));
				mamdb_mempush_single(uint64, dest, ins_val);
				mamdb_mempush(dest, &vals[ins_val_i], 8*(size - ins_val_i));
				mamdb_mempush_single(uint64, dest, vals[size]);

				ins_key = (ins_key_i == (size + 1)/2) ? ins_key : keys[(size + 1)/2];
			}
			cur_height += 1;
			ins_val = new_id;
			ins_val_i = key_path[cur_height] + 1;
			ins_key_i = ins_val_i - 1;
		}
	}
}

#endif
