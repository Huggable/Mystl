#ifndef _CONSTRUCT_H
#define _CONSTRUCT_H

#include <new.h>
namespace Mystl
{
	template <class T1, class T2>
	inline void construct(T1* p, const T2& value) {
		new (p) T1(value);
	}
	template <class T1>
	inline void construct(T1* p) {
		new (p) T1();
	}
	template<class T1>
	inline void destroy(T1 *p){
		p->~T1();
	}

	//Unfinish
	template<class ForwardIterator>
	inline void destroy(ForwardIterator start,ForwardIterator end)
	{
		for(;start!=end;++start)destroy(&*start);
	}
}
#endif