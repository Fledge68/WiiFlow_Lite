// A simple smart pointer class i made a long time ago, quickly adpated to the multiple alloc functions
// Not thread-safe (on copy & on destruction)

#ifndef __SMARTPTR_HPP
#define __SMARTPTR_HPP

#include <algorithm>
#include <malloc.h>

#include "mem2.hpp"
#include "loader/utils.h"
#include "music/gui_sound.h"

template <class T> class SmartPtr
{
public:
	enum SrcAlloc { SRCALL_NEW, SRCALL_MEM1, SRCALL_MEM2 };
	T &operator*(void) const { return *m_p; }
	T *operator->(void) const { return m_p; }
	bool operator!(void) const { return m_p == NULL; }
	T *get(void) const { return m_p; }
	virtual void release(void)
	{
		if (m_p != NULL && m_refcount != NULL && --*m_refcount == 0)
		{
			switch(m_srcAlloc)
			{
				case SRCALL_NEW:
					delete m_p;
					break;
				default:
					free(m_p);
					break;
			}
			delete m_refcount;
		}

		m_p = NULL;
		m_refcount = NULL;
	}
	SmartPtr<T> &operator=(const SmartPtr<T> &sp)
	{
		SmartPtr<T> temp(sp);
		_swap(temp);
		return *this;
	}
	explicit SmartPtr<T>(T *p = NULL, SrcAlloc t = SRCALL_NEW) : m_p(p), m_refcount(p != NULL ? new int(1) : NULL), m_srcAlloc(t) { }
	SmartPtr<T>(const SmartPtr<T> &sp) : m_p(sp.m_p), m_refcount(sp.m_refcount), m_srcAlloc(sp.m_srcAlloc)
	{
		if (m_refcount != NULL)
			++*m_refcount;
	}
	virtual ~SmartPtr<T>(void) { release(); }
protected:
	T *m_p;
	int *m_refcount;
	SrcAlloc m_srcAlloc;
protected:
	void _swap(SmartPtr<T> &sp) throw()
	{
		std::swap(m_p, sp.m_p);
		std::swap(m_refcount, sp.m_refcount);
		std::swap(m_srcAlloc, sp.m_srcAlloc);
	}
};

typedef SmartPtr<unsigned char> SmartBuf;
typedef SmartPtr<GuiSound> SmartGuiSound;

SmartBuf smartMemAlign32(unsigned int size);
SmartBuf smartMem2Alloc(unsigned int size);
SmartBuf smartAnyAlloc(unsigned int size);

SmartBuf smartMem1Alloc(unsigned int size);
#endif // !defined(__SMARTPTR_HPP)
