#ifndef _ALLOC_H
#define _ALLOC_H
namespace Mystl
{
#if 1
#include<new>
#endif
#include<iostream>

#define Throw_Error std::cout<<"out of memory"<<std::endl;exit(1);

template<int i>
class _first_class_alloc
{
	static int trytimes;
	static void (* hander)();
	static void *_first_class_alloc_malloc(size_t t)
	{
		void * res = NULL;
		int trylocal = trytimes;
		while( trylocal>0 )
		{
			--trylocal;
			if(!hander)
			{
				Throw_Error;
			}
			hander();
			res = malloc(t);
			if(res) return res;
		}
		Throw_Error;
	}
	static void *_first_class_alloc_realloc(void *p,size_t t)
	{
		void * res = NULL;
		int trylocal = trytimes;
		while( trylocal>0 )
		{
			--trylocal;
			if(!hander)
			{
				Throw_Error;
			}
			hander();
			res = realloc(p,t);
			if(res) return res;
		}
		Throw_Error;
	}
public:

	static void *allocate(size_t t)
	{
		void *res = malloc(t);
		if(NULL == res)res = _first_class_alloc_malloc(t);
		return res;
	}
	static void deallocate(void *p)
	{
		free(p);
	}
	static void *reallocate(void *p,size_t sz)
	{
		void* rp=realloc(p,sz);
		if(NULL == rp)rp = _first_class_alloc_realloc(p,sz);
		return rp;
	}
	static void (*set_malloc(void (*f)()))()
	{
		void (*r)() = hander;
		hander = f;
		return r;
	}
};
//Initialize
template<int i>
void (*_first_class_alloc<i>::hander)() = NULL;

template<int i>
int _first_class_alloc<i>::trytimes = 10;
typedef _first_class_alloc<0> malloc_alloc;

enum{BLOCKSIZE = 8,MAX_BYTES = 128,LISTSIZE = MAX_BYTES / BLOCKSIZE};
union obj
	{
		union obj * free_list;
		char clinet_data[1];
	};

template<int i>
class _second_class_alloc
{
private:
	static size_t ROUND(size_t n)
	{
		return (n + BLOCKSIZE - 1) & ~(BLOCKSIZE - 1);
	}
	static obj * free_list[LISTSIZE];
	static size_t FREE_LIST_INDEX(size_t bytes)
	{
		return ((bytes + BLOCKSIZE - 1) / (BLOCKSIZE) - 1);
	}
	static void *refill(size_t n);
	static char *chunk_alloc(size_t size, int &nobjs);

	static char *start;
	static char *end;
	static size_t heap_size;
public:
	static void * allocate(size_t n);
	static void deallocate(void *p, size_t n);
	static void * reallocate(void *p, size_t old_size, size_t new_size);
};
template<int i>
char *_second_class_alloc<i>::start = 0;
template<int i>
char *_second_class_alloc<i>::end = 0;
template<int i>
size_t _second_class_alloc<i>::heap_size = 0;


template<int i>
obj *_second_class_alloc<i>::free_list[LISTSIZE] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,};

template<int i>
void *_second_class_alloc<i>::allocate(size_t n)
{
	if(n > MAX_BYTES)
	{
		return malloc_alloc::allocate(n);
	}
	obj ** my_free_list;
	obj *res;
	my_free_list = free_list + FREE_LIST_INDEX(n);
	res = *my_free_list;
	if(res == 0)
	{
		void *r = refill(ROUND(n));
		
		return r;
	}
	*my_free_list = res->free_list;
	return res;

}
template<int i>
void _second_class_alloc<i>::deallocate(void *p, size_t n)
{
	if(n > MAX_BYTES)
	{
		malloc_alloc::deallocate(p);
		return;
	}
	obj *q = (obj*)p;
	obj ** my_free_list;
	my_free_list = free_list + FREE_LIST_INDEX(n);
	q->free_list = *my_free_list;
	*my_free_list = q;
	return;
}
template<int i>
void *_second_class_alloc<i>::refill(size_t n)
{
	int nodes = 20;
	char *chunk = chunk_alloc(n,nodes);
	obj ** my_free_list;
	obj * res;
	obj * cur,* next;
	if(1 == nodes)return chunk;
	my_free_list = free_list + FREE_LIST_INDEX(n);

	res = (obj*)chunk;
	next = (obj*)(chunk+n);
	*my_free_list = next;
	for(int k = 0;;++k)
	{
		cur = next;
		next = (obj*)(((char*)next) +n);
		if(k == nodes - 1)
		{
			cur->free_list = 0;
			break;
		}
		else cur->free_list = next;
	}
	return res;
}
template<int i>
char *_second_class_alloc<i>::chunk_alloc(size_t size, int &nobjs)
{
	char *res;
	size_t total = size * nobjs;
	size_t bytes_left = end - start;
	if(bytes_left >= total)
	{
		res = start;
		start += total;
		return res;
	}
	else if(bytes_left >= size)
	{
		nobjs = bytes_left / size;
		total = size * nobjs;
		res = start;
		start += total;
		return res;
	}
	else
	{
		size_t need = 2 * total + ROUND(heap_size >> 4);
		if(bytes_left > 0)
		{
			obj ** my_free_list = free_list + FREE_LIST_INDEX(bytes_left);
			((obj*)start)->free_list = *my_free_list;
			*my_free_list = (obj*)start;
		}
		start = (char*)malloc(need);
		if(start == 0)
		{
			obj **my_free_list,*p;
			for(int k = size; k <= MAX_BYTES; k += BLOCKSIZE)
			{
				my_free_list = free_list + FREE_LIST_INDEX(k);
				p = *(my_free_list);
				if( p != 0)
				{
					*my_free_list = p->free_list;
					start = (char*) p;
					end = start + k;
					return chunk_alloc(size,nobjs);

				}
			}
			end = 0;
			Throw_Error;
		}
		end = start + need;
		heap_size += need;
		return chunk_alloc(size,nobjs);
	}
}

typedef _second_class_alloc<0> alloc;
}

#endif
