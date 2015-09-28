//
// $Id: lmemory.cpp 73655 2009-03-06 09:41:56Z tony $
//

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "lmemory.hpp"

#define FHEADER_SIZE (sizeof(free_list_t))
#define FREELIST_T( m ) ((free_list_t*)(m))

size_t AllSize[] = {12, 20, 24, 27, 28, 32, 36, 48, 56, 112, 224, 448};
#define NBUCKETS	(int)(sizeof(AllSize)/sizeof(size_t))
char * SizeMap;

void _init_n_map()
{
	SizeMap = (char*)malloc(AllSize[NBUCKETS-1]);
	for(int i = 0; i < NBUCKETS; i++)
	{
		size_t j = (i == 0)? 0: AllSize[i-1]; 
		for(; j < AllSize[i]; j++)
		{
			SizeMap[j] = i;
		}
	}
}

int _get_n(size_t size)
{
	if(size > AllSize[NBUCKETS -1])
		return -1;
	return SizeMap[size - 1];
}

buckets_t* ff_buckets_t_init( buckets_t* ud )
{
	buckets_t* ret;

	if( ud == 0 )
	{
		ret = (buckets_t*)malloc( sizeof(buckets_t) );
		if( ret == 0 )
			return 0;
		ret->first_ = (free_list_t *) malloc(sizeof(free_list_t) * NBUCKETS);
		ret->fsize_ = (size_t *) malloc(sizeof(size_t) * NBUCKETS);
	}
	else
	{
		ret = (buckets_t*)ud;
	}

	bzero( ret->first_, sizeof(size_t) * NBUCKETS);
	memcpy(ret->fsize_, AllSize, sizeof(size_t) * NBUCKETS);

	ret->nblocks_ = 0;
	ret->big_msize = 0;
	_init_n_map();
	return ret;
}

void ff_create_memory( buckets_t* ud, free_list_t* fl )
{
	assert( ud && fl );
	register void* mb;
	register void* ptr;
	register void* h;
	register size_t n;
	register size_t size;

	mb = (void*)malloc( BLOCKSIZE );
	if( mb == 0 )
		return;

	ud->nblocks_++;

	assert( fl - ud->first_ >=0 && fl - ud->first_ < NBUCKETS );
	size = ( ud->fsize_[ fl - ud->first_ ]);
	n =  BLOCKSIZE / size;
	ptr = (char*)mb;

	while( n > 0 )
	{
		h = (char*)ptr + size;
		FREELIST_T(ptr)->next_ = (free_list*)h;
		ptr = h;
		n--;
	}
	FREELIST_T((char*)ptr - size)->next_ = fl->next_;
	fl->next_ = (free_list_t*)((char*)mb);	
}

void* ff_malloc( buckets_t* ud, size_t nsize )
{
	assert( ud );
	register int n;
	register free_list_t* h;

	n = _get_n(nsize);

	if( n >= 0 && n < NBUCKETS )
	{
		if( ud->first_[n].next_ == 0 )
		{
			ff_create_memory( ud, ud->first_+n );
		}
		if( ud->first_[n].next_ == 0 )
			return 0;
		
		h = ud->first_[n].next_;
		ud->first_[n].next_ = h->next_;
		h->next_ = 0;
		return h; 
	}
	else
	{
		ud->big_msize += nsize;
		return malloc(nsize);
	}
}

void ff_free( buckets_t* ud, void* ptr , size_t osize)
{
	assert( ud );
	register int n = _get_n(osize);

	if( n >= 0 && n < NBUCKETS )
	{
		free_list_t* h = FREELIST_T( ptr );
		h->next_ = ud->first_[n].next_;
		ud->first_[n].next_ = h;
	}
	else
	{
		ud->big_msize -= osize;
		free(ptr);
	}

	return;
}


void* ff_realloc( void* bt, void* ptr, size_t osize, size_t nsize )
{
	assert( bt );	
	register buckets_t* ud;
	register void* nb;

	if( osize == nsize )
		return ptr;

	ud = (buckets_t*)bt;
	if (nsize == 0)
	{
		ff_free(ud, ptr, osize);
		return NULL;
	}

	if( ptr == 0 )
		return ff_malloc( ud, nsize );

	nb = ff_malloc( ud, nsize );

	if(nb)
		memcpy( nb, ptr, ( (osize < nsize) ? osize : nsize ) );

	ff_free(ud, ptr, osize);

	return nb;
}

//只能取出在first_管理下的内存块数量
//非规则大小的内存块的大小是单独维护
int ff_bucket_count( buckets_t* ud ) {
	assert (ud);
	return ud->nblocks_ ;
}
