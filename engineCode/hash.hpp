//$Author: shavingha $
//$Id: hash.hpp 65945 2008-12-15 06:26:49Z tony $
#include "global.hpp"

#ifndef __HASH_H_
#define __HASH_H_

#define TNUMBER 0
#define TSTRING 1

//hash number.
unsigned int find_double_prime(unsigned int n);
unsigned int divide_hash(unsigned int key, size_t size);
unsigned int mult_hash(unsigned  int key, size_t size);
unsigned int double_mult(unsigned int key, size_t size, unsigned int i);
unsigned int double_div(unsigned int key, size_t size, unsigned int i);
unsigned int new_hash(unsigned int key, size_t size, unsigned int i);
#define IHASH(key, size) divide_hash(key, size)

//hash str. max是指最大hash长度,对太长的字符串不完全hash.
//size是指hash表的大小。
int hashstr(const char * key, int max);
int whashstr(const char * key, int max);
int lhashstr(const char * str, int l); //注意l的参数不是最大hash长度，而是str的长度。
#define SHASH_MAXLEN 32
#define SHASH(s) whashstr(s, SHASH_MAXLEN) //***全部hash函数都用同一个配置***

typedef struct hlink_ hlink;
struct hlink_
{
	int key;
	size_t klen;
	//int hash; //记录冗余信息。
	void * val; //value
	unsigned char type;
	hlink * next;
};

typedef struct 
{
	#ifdef DEBUG
	size_t free_size;
	size_t node_size;//所有分配出来的节点数目。
	#endif
	size_t slot_size; //nodes数组的大小;
	hlink * freelist;//空闲节点列表。
	size_t len; //元素个数.
	hlink * nodes[0];//zero array.必须在结构体末尾
}hash_table;



hash_table * init_hash_table(int size);
void free_hash_table(hash_table * h);

bool _insert_node(hash_table * h,  int key, size_t klen, unsigned char type, void * val, bool update);
bool _delete_node(hash_table * h,  int key, size_t klen, unsigned char type);
bool _find_node(hash_table * h, int key, size_t klen, unsigned char type, void * * val);

#define hash_len(ht) (ht->len)

//整数hash处理
#define insert_inode(h, key, val) _insert_node(h, key, 0, TNUMBER, val, false)
#define update_inode(h, key, val) _insert_node(h, key, 0, TNUMBER, val, true)
#define delete_inode(h, key) _delete_node(h, key, 0, TNUMBER)
#define find_inode(h, key, val) _find_node(h, key, 0, TNUMBER, val)

//字符串hash处理。
#define insert_snode(h, key, klen, val) _insert_node(h, (int)key, klen, TSTRING, val, false)
#define update_snode(h, key, klen, val) _insert_node(h, (int)key, klen, TSTRING, val, true)
#define delete_snode(h, key, klen, val) _delete_node(h, (int)key, klen, TSTRING)
#define find_snode(h, key, klen, val) _find_node(h, (int)key, klen, TSTRING, val)

#endif //__HASH_H_
