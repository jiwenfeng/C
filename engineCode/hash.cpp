//$Id: hash.cpp 65945 2008-12-15 06:26:49Z tony $
//$Author: shavingha $
//$Id/

#include <math.h>
#include "hash.hpp"

//hash from dhxy2 engine.

#define mod(k,n) (k)%(n)

////////////////////////////////int_hash///////////////////////////////////
//Compute a raised to the power m modulo n
static unsigned int raisemod(unsigned int a, unsigned int m, unsigned int n)
{
	unsigned int z=1;
	int  i=16;
	for (i=16; i>=0; i--)
	{
		z = mod(z*z,n);
		if (m & (1<<i)) z = mod(z*a,n);
	}
	return(z);
}

//Test if the number n is prime - return TRUE if it is
static bool is_prime(unsigned int n)
{
	static unsigned int first[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 0};

	int i;
	for (i=0; first[i] != 0; i++) {//try obvious sizes first
		if ((n % first[i]) == 0) {
			return false;
		}
	}

	int half = (int)sqrt(n);
	for (i=29; i <= half; i++) {
		if (n % i == 0) {
			return false;
		}
	}
	return true;
}

//Find the next prime larger than n
static unsigned int next_prime(unsigned int n)
{
	while(!is_prime(++n)) {
		;
	}
	return(n);
}

//Find a prime t such that t*2+1 is also prime
unsigned int find_double_prime(unsigned int n)
{
	unsigned int k = n/2;
	while (1) {
		k = next_prime(k);

		if (is_prime(k*2+1)) return(k*2+1);
	}
	return 0; //not found
}

//HASH算法
#define GOLDEN 0.6180339887

//Simple hash functions
unsigned int divide_hash(unsigned int key, size_t size)
{
	return(mod(key, size));
}

unsigned int mult_hash(unsigned int key, size_t size)
{
	register double val = key * GOLDEN;
	//printf("val %f\n", val);
	return (unsigned int)((val - (int)val) * size);
}

unsigned int double_mult(unsigned int key, size_t size, unsigned int i)
{
	//Variation based on knuth - try double multiplication
	return mod((mult_hash(key, size) + i * (1+mult_hash(key, size))) , size);
}

unsigned int double_div(unsigned int key, size_t size, unsigned int i)
{
	//From page 522 of Knuth - try h2 relatively prime to h1
	return(mod((divide_hash(key, size) + i * (1+(mod(key , (size-2))))) , size));
}

unsigned int new_hash(unsigned int key, size_t size, unsigned int i)
{
	unsigned int d = divide_hash(key, size);
	if (d <= 1) d=2;
	if (d >= size-1) d=3;
	return(mod((d + raisemod(d, i, size)) , size));
}

////////////////////////////////////////string hash////////////////////////////////////////////
static int T[] =
{
    1, 87, 49, 12, 176, 178, 102, 166, 121, 193, 6, 84, 249, 230, 44, 163,
    14, 197, 213, 181, 161, 85, 218, 80, 64, 239, 24, 226, 236, 142, 38, 200,
    110, 177, 104, 103, 141, 253, 255, 50, 77, 101, 81, 18, 45, 96, 31, 222,
    25, 107, 190, 70, 86, 237, 240, 34, 72, 242, 20, 214, 244, 227, 149, 235,
    97, 234, 57, 22, 60, 250, 82, 175, 208, 5, 127, 199, 111, 62, 135, 248,
  174, 169, 211, 58, 66, 154, 106, 195, 245, 171, 17, 187, 182, 179, 0, 243,
 132, 56, 148, 75, 128, 133, 158, 100, 130, 126, 91, 13, 153, 246, 216, 219,
    119, 68, 223, 78, 83, 88, 201, 99, 122, 11, 92, 32, 136, 114, 52, 10,
    138, 30, 48, 183, 156, 35, 61, 26, 143, 74, 251, 94, 129, 162, 63, 152,
    170, 7, 115, 167, 241, 206, 3, 150, 55, 59, 151, 220, 90, 53, 23, 131,
    125, 173, 15, 238, 79, 95, 89, 16, 105, 137, 225, 224, 217, 160, 37, 123,
    118, 73, 2, 157, 46, 116, 9, 145, 134, 228, 207, 212, 202, 215, 69, 229,
    27, 188, 67, 124, 168, 252, 42, 4, 29, 108, 21, 247, 19, 205, 39, 203,
 233, 40, 186, 147, 198, 192, 155, 33, 164, 191, 98, 204, 165, 180, 117, 76,
    140, 36, 210, 172, 41, 54, 159, 8, 185, 232, 113, 196, 231, 47, 146, 120,
    51, 65, 28, 144, 254, 221, 93, 189, 194, 139, 112, 43, 71, 109, 184, 209,
};

inline int hashstr(const char * s, int max)
{
	register unsigned int h;
	register unsigned char *p;

	h = (unsigned char) *s;
	if (!h) return 0;

	register int oh = T[(unsigned char) *s];
	for (p = (unsigned char *) s + 1; *p && p <= (unsigned char *) s + max; p++) {
		h = T[h ^ *p];
		oh = T[oh ^ *p];
	}
	return (int)(h | (oh << 8));
}


/*
 * whashstr is faster than hashstr, but requires an even power-of-2 table size
 * Taken from Amylaars driver.
 */
inline int whashstr(const char * s, int max)
{
	register unsigned char oh, h;
	register unsigned char *p;
	register int i;

	if (!*s) return 0;
	p = (unsigned char *) s;
	oh = T[*p];
	h = (*(p++) + 1) & 0xff;
	for (i = max - 1; *p && --i >= 0; ) {
		oh = T[oh ^ *p];
		h = T[h ^ *(p++)];
	}

	return (oh << 8) + h;
}


//from lua
inline int lhashstr(const char * str, int l)
{
	unsigned int h = (unsigned int)l;  /* seed */
	size_t step = (l>>5)+1;  /* if string is too long, don't hash all its chars */
	size_t l1;
	for (l1=l; l1>=step; l1-=step) { /* compute hash */
		h = h ^ ((h<<5)+(h>>2)+(unsigned char)str[l1-1]);
	}
	return h;
}


//////////////////////////hash_table////////////////////////////////////////////////
hash_table * init_hash_table(int size)
{
	unsigned int prime = find_double_prime(size);
	size_t mem_size = sizeof(hash_table) + sizeof(hlink *) * prime;
	hash_table * h = (hash_table *)malloc(mem_size);
	bzero(h, mem_size);
	h->freelist = NULL;
	h->slot_size = prime;
	h->len = 0;
	return h;
}

void free_hash_table(hash_table * h)
{
	unsigned int i;
	for (i=0; i<h->slot_size; i++) {
		hlink * node = h->nodes[i];
		while (node) {
			hlink * next = node->next;
			free(node);
			node = next;
		}
	}

	free(h);
}

//放入空闲列表。
static inline void free_node(hash_table * h, hlink * node)
{
	node->next = h->freelist;
	h->freelist = node;
	#ifdef DEBUG
	h->free_size++;
	#endif
}

//分配节点。
static inline hlink * malloc_node(hash_table * h)
{
	hlink * node;
	//先找空闲节点。
	if (h->freelist) {
		node = h->freelist;
		h->freelist = node->next;
		node->next = NULL;
		#ifdef DEBUG
		h->free_size--;
		#endif
	}
	else {
		node = (hlink *)malloc(sizeof(hlink));
		#ifdef DEBUG
		h->node_size++;
		#endif
	}
	return node;
}

inline bool eq(hlink * node1, int key, size_t klen, unsigned char type)
{
	if (node1->type != type)
		return false;

	if (node1->type == TNUMBER) {
		return node1->key == key;
	}
	else { //STRING
		return node1->klen == klen && 
			!strncmp((const char *)node1->key, (const char *)key, node1->klen);
	}
}

inline bool eq_node(hlink * node1, hlink * node2)
{
	return eq(node1, node2->key, node2->klen, node2->type);
}


//klen需要传入的原因是不希望在内部再进行类似的计算。type标识key是NUNBER or STRING
//update 参数表示当遇到冲突时是放弃还是直接更新。
bool _insert_node(hash_table * h,  int key, size_t klen, unsigned char type, void * val, bool update)
{
	int hash;
	if (type == TNUMBER) {
		hash = IHASH(key, h->slot_size);
	}
	else { 
		hash = SHASH((const char *)key) % h->slot_size;
	}
	hlink * node = malloc_node(h);
	node->key = key;
	node->klen = klen;
	node->type = type;
	node->val = val;
	node->next = NULL;
	
	if (!h->nodes[hash]) {
		h->nodes[hash] = node;
		h->len++;
		return true;
	}

	hlink * tmp = h->nodes[hash];
	while(tmp) {
		if (eq_node(tmp, node)) {
			if (!update) {
				free_node(h, node);
				return false;
			} else {
				void * old_val = tmp->val;
				tmp->val = node->val;
				tmp->type = node->type;
				return old_val;
			}
		}
		else {
			tmp = tmp->next;
		}
	}

	node->next = h->nodes[hash];
	h->nodes[hash]=node;
	h->len++;

	return true;
}

//删除的时候不free node，而是放入空闲列表。
bool _delete_node(hash_table * h,  int key, size_t klen, unsigned char type)
{
	int hash;
	if (type == TNUMBER) {
		hash = IHASH(key, h->slot_size);
	}
	else { 
		hash = SHASH((const char *)key) % h->slot_size;
	}

	assert(type==TNUMBER || type==TSTRING);
	hlink * node = h->nodes[hash];
	hlink * prev = NULL; //node的前一个节点。
	while(node) {
		//相同key的节点只可能有一个。
		if (eq(node, key, klen, type)) {
			if (prev) {
				prev->next = node->next;
			}
			else {
				h->nodes[hash] = node->next;
			}
			free_node(h, node);
			h->len--;
			return true;
		}
		else {
			prev = node;
			node = node->next;
		}
	}

	return false;
}

bool _find_node(hash_table * h, int key, size_t klen, unsigned char type, void * *val)
{
	int hash;
	if (type == TNUMBER) {
		hash = IHASH(key, h->slot_size);
	}
	else {
		hash = SHASH((const char *)key) % h->slot_size;
	}

	hlink * node = h->nodes[hash];
	while(node) {
		if (eq(node, key, klen, type)) {
			*val = node->val;
			return true;
		}
		node = node->next;
	}
	return false;
}

