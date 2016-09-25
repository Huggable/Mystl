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

	static void* allocate(size_t t)
	{
		void *res = malloc(t);
		if(NULL == res)res = _first_class_alloc_malloc(t);
		return res;
	}
	static void deallocate(void *p)
	{
		free(p);
	}
	static void* reallocate(void *p,size_t sz)
	{
		void* rp=realloc(p,sz);
		if(NULL == rp)rp = _first_class_alloc_realloc(p,sz);
		return rp;
	}
	static void (*set_malloc(void (*f)()))()
	{
		void (* r)() = hander;
		hander = f;
		return r;
	}
};
//Initialize
template<int i>
void (*_first_class_alloc<i>::hander)() = NULL;

template<int i>
int _first_class_alloc<i>::trytimes = 10;
typedef _first_class_alloc<0> alloc;
}

#endif
