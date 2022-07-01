

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
	BlockId root_id;
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
bool mamdbc_freeallblocks(BTree* btree);
bool mamdbc_flush(BTree* btree);


bool mamdb_get(BTree* btree, uint64 key, uint64* ret_val) {
	void* node_data = alloca(BLOCK_SIZE);
	BlockId node_id = btree->root;
	while(true) {
		mamdb_getblock(node_id, node_data);

		bool is_leaf = (uint64*)node_data;
		uint64 next_leaf = (uint64*)node_data;
		uint64* keys = (uint64*)node_data;
		uint32 size = (uint32*)node_data;
		uint64* vals = (uint64*)node_data;

		uint32 start = 0;
		uint32 end = size;
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

	node_ids[cur_height] = btree->root_id;
	while(true) {
		uint64 node_id = node_ids[cur_height];
		mamdbc_getblock(btree, cur_height, node_id, &nodes[cur_height]);
		byte* node_data = nodes[cur_height];

		// | MAMDB_BLOCK_SIZE                                           |
		// | 8    | 8*size    | 8                    | 8*size       | - |
		// | size | keys, ... | vals[0] or next_leaf | vals[1], ... | - |
		uint32 size = *mamdb_ptr_add(uint32, node_data, 0);
		uint64* keys = mamdb_ptr_add(uint64, node_data, 8);
		uint64* vals = mamdb_ptr_add(uint64, node_data, 8 + 8*size);
		bool is_leaf = cur_height == 0;


		uint32 start = 0;
		uint32 end = size;
		uint32 least_gt_key_i;
		while(true) {
			uint32 mid = (start + end)/2;
			uint64 cur_key = keys[mid];
			if(key < cur_key) {
				end = mid;
			} else {
				start = mid + 1;
			}
			if(start == end) {
				least_gt_key_i = start;
				break;
			}
		}
		//ASSERT(key < keys[least_gt_key_i])

		if(is_leaf) {
			if(least_gt_key_i > 0 && keys[least_gt_key_i - 1] == key) {
				//found exact matching key
				byte* rep_data;
				mamdbc_repblock(btree, cur_height, node_id, &rep_data);
				mamdb_memcpy(rep_data, node_data, MAMDB_BLOCK_SIZE);
				uint64* rep_vals = mamdb_ptr_add(uint64, rep_data, 8 + 8*size);
				rep_vals[least_gt_key_i] = value;//write to right of keys[least_gt_key_i - 1]

				mamdbc_freeallblocks(btree);
				return 1;
			} else {
				key_path[cur_height] = least_gt_key_i;//insert before key[least_gt_key_i]
				break;
			}
		} else {
			key_path[cur_height] = least_gt_key_i;
			node_ids[cur_height] = (BlockId)vals[least_gt_key_i];//go left of keys[least_gt_key_i]
			cur_height -= 1;
		}
	}

	//insert a key and value
	uint64 ins_key = key;
	uint64 ins_val = value;
	uint64 ins_i = key_path[cur_height];
	while(true)  {
		//ASSERT(ins_i <= ins_i && ins_i >= ins_i - 1)
		uint64 node_id = node_ids[cur_height];
		byte* node_data = nodes[cur_height];

		uint64 size = *mamdb_ptr_add(uint64, node_data, 0);
		uint64* keys = mamdb_ptr_add(uint64, node_data, 8);
		uint64* vals = mamdb_ptr_add(uint64, node_data, 8 + 8*size);
		bool is_leaf = cur_height == 0;

		byte* dest;
		#define mamdb_destpush(src, start, end) mamdb_memcpy(dest, (src) + (start), 8*((end) - (start))); dest += 8*((end) - (start))
		#define mamdb_destpush_uint64(src) *(uint64*)dest = src; dest += 8

		if(size < max_size) {
			byte* rep_data;
			mamdbc_repblock(btree, cur_height, node_id, &rep_data);
			//insert into block
			// | node_data                       |
			// | 8    | 8*size    | 8*(size + 1) |
			// | size | keys, ... | vals, ...    |
			//------>
			// | rep_data                                                                                               |
			// | 8        | 8*ins_i   | 8       | 8*(size - ins_i) | 8*(ins_i + 1) | 8       | 8*(size + 1 - ins_i - 1) |
			// | size + 1 | keys, ... | ins_key | keys[ins_i], ..  | vals, ...     | ins_val | vals[ins_i], ...         |
			//====>
			dest = rep_data;
			mamdb_destpush_uint64(size + 1);
			mamdb_destpush(keys, 0, ins_i);
			mamdb_destpush_uint64(ins_key);
			mamdb_destpush(keys, ins_i, size);
			mamdb_destpush(vals, 0, ins_i + 1);
			mamdb_destpush_uint64(ins_val);
			mamdb_destpush(vals, ins_i + 1, size + 1);

			mamdbc_freeallblocks(btree);
			return 1;
		} else {
			//split
			BlockId new_id;
			byte* new_data,* rep_data;
			mamdbc_newblock(btree, cur_height, &new_id, &new_data);
			mamdbc_repblock(btree, cur_height, node_id, &rep_data);
			/*
			node_data = | size | keys, ...size | vals[0] | vals[1], ...size + 1 |
			----->
			if(ins_i <= (size + 1)/2 - 1) {
				if(is_leaf) {
					rep_data = | (size + 1)/2 | keys, ...ins_i | ins_key | keys[ins_i], ...(size + 1)/2 - 1 | new_id | vals[1], ...ins_i + 1 | ins_val | vals[ins_i + 1], ...(size + 1)/2 |
					new_data = | size/2 + 1 | keys[(size + 1)/2 - 1], ...size | vals[0] | vals[(size + 1)/2], ...size + 1 |
					ins_key = keys[(size + 1)/2 - 1]
				} else {
					rep_data = | (size + 1)/2 | keys, ...ins_i | ins_key | keys[ins_i], ...(size + 1)/2 - 1 | vals[0] | vals[1], ...ins_i + 1 | ins_val | vals[ins_i + 1], ...(size + 1)/2 |
					new_data = | size/2 | keys[(size + 1)/2], ...size | vals[(size + 1)/2] | vals[(size + 1)/2 + 1], ...size + 1 |
					ins_key = keys[(size + 1)/2 - 1]
				}
			} else {
				if(is_leaf) {
					rep_data = | (size + 1)/2 | keys, ...(size + 1)/2 | new_id | vals[1], ...(size + 1)/2 + 1 |
					new_data = | size/2 + 1 | keys[(size + 1)/2], ...ins_i | ins_key | keys[ins_i], ...size | vals[0] | vals[(size + 1)/2 + 1], ...ins_i + 1 | ins_val | vals[ins_i + 1], ...size + 1 |
					ins_key = ins_i == (size + 1)/2 ? ins_key : keys[(size + 1)/2]
				} else {
					rep_data = | (size + 1)/2 | keys, ...(size + 1)/2 | vals[0] | vals[1], ...(size + 1)/2 + 1 |
					new_data = | size/2 | keys[(size + 1)/2 + 1], ...ins_i | ins_key | keys[ins_i], ...size | vals[(size + 1)/2 + 1] | vals[(size + 1)/2 + 2], ...ins_i + 1 | ins_val | vals[ins_i + 1], ...size + 1 |
					or | size/2 | keys[(size + 1)/2], ...size | ins_val | vals[(size + 1)/2 + 1], ...size + 1 | if ins_i == (size + 1)/2
					ins_key = ins_i == (size + 1)/2 ? ins_key : keys[(size + 1)/2]
				}
			}

			====>

			if(ins_i <= (size + 1)/2 - 1) {
				rep_data = | (size + 1)/2 | keys, ...ins_i | ins_key | keys[ins_i], ...(size + 1)/2 - 1 | is_leaf?new_id:vals[0] | vals[1], ...ins_i + 1 | ins_val | vals[ins_i + 1], ...(size + 1)/2 |
				new_data = | size/2 + is_leaf | keys[(size + 1)/2 - is_leaf], ...size | is_leaf?vals[0]:vals[(size + 1)/2] | vals[(size + 1)/2 + 1 - is_leaf], ...size + 1 |
				ins_key = keys[(size + 1)/2 - 1]
			} else {
				rep_data = | (size + 1)/2 | keys, ...(size + 1)/2 | is_leaf?new_id:vals[0] | vals[1], ...(size + 1)/2 + 1 |
				new_data = | size/2 + is_leaf | keys[(size + 1)/2 + 1 - is_leaf], ...ins_i | ins_key | keys[ins_i], ...size | is_leaf?vals[0]:vals[(size + 1)/2 + 1] | vals[(size + 1)/2 + 2 - is_leaf], ...ins_i + 1 | ins_val | vals[ins_i + 1], ...size + 1 |
				or | size/2 | keys[(size + 1)/2], ...size | ins_val | vals[(size + 1)/2 + 1], ...size + 1 | if ins_i == (size + 1)/2 && !is_leaf
				ins_key = ins_i == (size + 1)/2 ? ins_key : keys[(size + 1)/2]
			}

			====>
			*/
			if(ins_i <= (size + 1)/2 - 1) {//here
				//rep_data = | (size + 1)/2 | keys, ...ins_i | ins_key | keys[ins_i], ...(size + 1)/2 - 1 | is_leaf?new_id:vals[0] | vals[1], ...ins_i + 1 | ins_val | vals[ins_i + 1], ...(size + 1)/2 |
				dest = rep_data;
				mamdb_destpush_uint64((size + 1)/2);
				mamdb_destpush(keys, 0, ins_i);
				mamdb_destpush_uint64(ins_key);
				mamdb_destpush(keys, ins_i, (size + 1)/2 - 1);
				mamdb_destpush_uint64(is_leaf?new_id:vals[0]);
				mamdb_destpush(vals, 1, ins_i + 1);
				mamdb_destpush_uint64(ins_val);
				mamdb_destpush(vals, ins_i + 1, (size + 1)/2);

				//new_data = | size/2 + is_leaf | keys[(size + 1)/2 - is_leaf], ...size | is_leaf?vals[0]:vals[(size + 1)/2] | vals[(size + 1)/2 + 1 - is_leaf], ...size + 1 |
				dest = new_data;
				mamdb_destpush_uint64(size/2 + is_leaf);
				mamdb_destpush(keys, (size + 1)/2 - is_leaf, size);
				mamdb_destpush_uint64(is_leaf?vals[0]:vals[(size + 1)/2]);
				mamdb_destpush(vals, (size + 1)/2 + 1 - is_leaf, size + 1);

				ins_key = keys[(size + 1)/2 - 1];
			} else {
				//rep_data = | (size + 1)/2 | keys, ...(size + 1)/2 | is_leaf?new_id:vals[0] | vals[1], ...(size + 1)/2 + 1 |
				dest = rep_data;
				mamdb_destpush_uint64((size + 1)/2);
				mamdb_destpush(keys, 0, (size + 1)/2);
				mamdb_destpush_uint64(is_leaf?new_id:vals[0]);
				mamdb_destpush(vals, 1, (size + 1)/2 + 1);

				//new_data = | size/2 + is_leaf | keys[(size + 1)/2 + 1 - is_leaf], ...ins_i | ins_key | keys[ins_i], ...size | is_leaf?vals[0]:vals[(size + 1)/2 + 1] | vals[(size + 1)/2 + 2 - is_leaf], ...ins_i + 1 | ins_val | vals[ins_i + 1], ...size + 1 |
				//or | size/2 | keys[(size + 1)/2], ...size | ins_val | vals[(size + 1)/2 + 1], ...size + 1 | if ins_i == (size + 1)/2 && !is_leaf
				dest = new_data;
				mamdb_destpush_uint64(size/2 + is_leaf);
				if((ins_i == (size + 1)/2) && !is_leaf) {
					mamdb_destpush(keys, (size + 1)/2, size);
				} else {
					mamdb_destpush(keys, (size + 1)/2 + 1 - is_leaf, ins_i);
					mamdb_destpush_uint64(ins_key);
					mamdb_destpush(keys, ins_i, size);
					mamdb_destpush_uint64(is_leaf?vals[0]:vals[(size + 1)/2 + 1]);
					mamdb_destpush(vals, (size + 1)/2 + 2 - is_leaf, ins_i + 1);
				}
				mamdb_destpush_uint64(ins_val);
				mamdb_destpush(vals, ins_i + 1, size + 1);

				ins_key = (ins_i == (size + 1)/2) ? ins_key : keys[(size + 1)/2];
			}
			ins_val = new_id;
			if(cur_height == root_height) {
				//the root has just split
				BlockId new_root_id;
				byte* new_root_data;
				mamdbc_newblock(btree, cur_height + 1, &new_root_id, &new_root_data);

				dest = new_root_data;
				mamdb_destpush_uint64(1);
				mamdb_destpush_uint64(ins_key);
				mamdb_destpush_uint64(btree->root_id);
				mamdb_destpush_uint64(ins_val);

				btree->root_id = new_root_id;
				btree->root_height += 1;

				mamdbc_freeallblocks(btree);
				return 1;
			} else {
				cur_height += 1;
				ins_i = key_path[cur_height];
			}
		}
	}
}

#endif
