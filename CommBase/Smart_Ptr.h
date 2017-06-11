/*
 * Smart_Ptr.h
 *
 *  Created on: Jun 8, 2017
 *      Author: root
 */

#ifndef SMART_PTR_H_
#define SMART_PTR_H_

#include "./Memory/MemAllocator.h"
#include "Ref_Object.h"

namespace CommBaseOut
{

/*
 * 智能指针
 *非线程安全，如果在多线程传递的变量，禁止使用这个智能指针
 */
template<class T> class Smart_Ptr
{
public:
	Smart_Ptr():m_ptr(0),m_ref(0)
	{
	}

	Smart_Ptr(Smart_Ptr const &ptr)
	{
		if(ptr.Get() == 0)
		{
			m_ref = 0;
			m_ptr = 0;
		}
		else
		{
			m_ptr = ptr.Get();
			m_ref = ptr.GetRef();

			(*m_ref)++;
		}
	}

	Smart_Ptr(T *ptr):m_ptr(ptr),m_ref(0)
	{
		if(m_ptr)
		{
			m_ref = NEW_BASE(int, 1);
			*m_ref = 1;
		}
	}

	template<class Y>
	Smart_Ptr(Smart_Ptr<Y> const &ptr)
	{
		if(ptr.Get() == 0)
		{
			m_ref = 0;
			m_ptr = 0;
		}
		else
		{
			m_ptr = static_cast<T *>(ptr.Get());
			m_ref = ptr.GetRef();

			(*m_ref)++;
		}
	}

	~Smart_Ptr()
	{
		if(m_ref)
		{
			(*m_ref)--;

			if(*m_ref <= 0)
			{
				DELETE_BASE(m_ref, eMemoryArray);
				m_ref = 0;

				if(m_ptr)
				{
					delete m_ptr;
					m_ptr = 0;
				}
			}
		}
	}

	Smart_Ptr& operator = (T* ptr)
	{
		if(ptr == m_ptr)
		{
			return *this;
		}

		if(m_ref)
		{
			(*m_ref)--;

			if(*m_ref <= 0)
			{
				DELETE_BASE(m_ref, eMemoryArray);
				m_ref = 0;

				if(m_ptr)
				{
					delete m_ptr;
					m_ptr = 0;
				}
			}
		}

		if(ptr == 0)
		{
			m_ref = 0;
			m_ptr = 0;
		}
		else
		{
			m_ref = NEW_BASE(int, 1);//NEW int(1);
			*m_ref = 1;
			m_ptr = ptr;
		}

		return *this;
	}

	Smart_Ptr& operator = (Smart_Ptr const &ptr)
	{
		if(m_ptr == ptr.Get())
		{
			return *this;
		}

		if(m_ref)
		{
			(*m_ref)--;

			if(*m_ref <= 0)
			{
				DELETE_BASE(m_ref, eMemoryArray);
				m_ref = 0;

				if(m_ptr)
				{
					delete m_ptr;
					m_ptr = 0;
				}
			}
		}

		if(ptr.Get() == 0)
		{
			m_ref = 0;
			m_ptr = 0;
		}
		else
		{
			m_ref = ptr.GetRef();
			m_ptr = ptr.Get();

			(*m_ref)++;
		}

		return *this;
	}

	template<class Y>
	Smart_Ptr& operator = (Smart_Ptr<Y> const &ptr)
	{
		if(m_ptr == ptr.Get())
		{
			return *this;
		}

		if(m_ref)
		{
			(*m_ref)--;

			if(*m_ref <= 0)
			{
				DELETE_BASE(m_ref, eMemoryArray);
				m_ref = 0;

				if(m_ptr)
				{
					delete m_ptr;
					m_ptr = 0;
				}
			}
		}

		if(ptr.Get() == 0)
		{
			m_ref = 0;
			m_ptr = 0;
		}
		else
		{
			m_ref = ptr.GetRef();
			m_ptr = static_cast<T *>(ptr.Get());

			(*m_ref)++;
		}

		return *this;
	}

	T& operator * () const
	{
		return *m_ptr;
	}

	T* operator -> () const
	{
		return m_ptr;
	}

	bool operator == (T* ptr)
	{
		return m_ptr == ptr;
	}

	bool operator == (Smart_Ptr const &ptr)
	{
		return m_ptr == ptr.m_ptr;
	}

	template<class Y>
	bool operator == (Smart_Ptr<Y> &ptr)
	{
		return m_ptr == ptr.Get();
	}

	operator bool () const
	{
		return m_ptr != 0;
	}

	bool operator! () const
	{
		return m_ptr == 0;
	}

	T* Get() const
	{
		return m_ptr;
	}

	int *GetRef() const
	{
		return m_ref;
	}

private :
	T* m_ptr;
	int* m_ref;
};


/*
 * 智能指针
 * 多线程安全智能，使用的对象有限，必须是继承自Ref_Object的对象
 */
template<class T>
class Safe_Smart_Ptr
{
public:
	typedef T element_type;
public:

	Safe_Smart_Ptr() : t_(NULL)
	{
	}

	Safe_Smart_Ptr(T * p) : t_(p)
	{
		if(t_)
			t_->addref();
	}

	Safe_Smart_Ptr(Safe_Smart_Ptr const & r) : t_(r.t_)
	{
		if(t_)
			t_->addref();
	}

	template<class Y>
	Safe_Smart_Ptr(Safe_Smart_Ptr<Y> const & r) : t_(r.t_)
	{
		if(t_)
			t_->addref();
	}

	Safe_Smart_Ptr & operator=(Safe_Smart_Ptr const & r)
	{
		if(t_)
			t_->release();

		t_ = r.t_;

		if(t_)
			t_->addref();

		return *this;
	}

	template<class Y>
	Safe_Smart_Ptr & operator=(Safe_Smart_Ptr<Y> const & r)
	{
		if(t_)
			t_->release();

		t_ = r.t_;

		if(t_)
			t_->addref();

		return *this;
	}

	template<class Y> Safe_Smart_Ptr & operator=(T * r)
	{
		if(t_)
			t_->release();

		t_ = r;

		if(t_)
			r->addref();
	}

	T & operator*() const // never throws
	{
		return *t_;
	}

	T * operator->() const // never throws
	{
		return t_;
	}

	T * Get() const// never throws
	{
		return t_;
	}

	operator bool () const
	{
		return t_ != 0;
	}

	bool operator! () const
	{
		return t_ == 0;
	}

	void swap(Safe_Smart_Ptr & rhs)
	{
		T * tmp = t_;
		t_ = rhs.t_;
		rhs.t_ = tmp;
	}

public:
	~Safe_Smart_Ptr(void)
	{
		if(t_ != 0)
		{
			if(t_->release())
				t_ = 0;
		}
	}

public:
	T* t_;
};

/*
template<class T>
class Smart_Ptr_Mem
{
public :
	Smart_Ptr_Mem():m_ptr(0),m_ref(0)
	{
	}

	Smart_Ptr_Mem(Smart_Ptr_Mem const &ptr)
	{
		if(ptr.Get() == 0)
		{
			m_ref = 0;
			m_ptr = 0;
		}
		else
		{
			m_ptr = ptr.Get();
			m_ref = ptr.GetRef();

			(*m_ref)++;
		}
	}

	Smart_Ptr_Mem(T *ptr):m_ptr(ptr),m_ref(0)
	{
		if(m_ptr)
		{
			m_ref = (int *)ALLOCATE_MEMORY(sizeof(int));
			*m_ref = 1;
		}
	}

	template<class Y>
	Smart_Ptr_Mem(Smart_Ptr_Mem<Y> const &ptr)
	{
		if(ptr.Get() == 0)
		{
			m_ref = 0;
			m_ptr = 0;
		}
		else
		{
			m_ptr = static_cast<T *>(ptr.Get());
			m_ref = ptr.GetRef();

			(*m_ref)++;
		}
	}

	~Smart_Ptr_Mem()
	{
		if(m_ref)
		{
			(*m_ref)--;

			if(*m_ref <= 0)
			{
				DELETE_MP_OBJECT(m_ref);
				m_ref = 0;

				if(m_ptr)
				{
					delete m_ptr;
					m_ptr = 0;
				}
			}
		}
	}

	Smart_Ptr_Mem& operator = (T* ptr)
	{
		if(ptr == m_ptr)
		{
			return *this;
		}

		if(m_ref)
		{
			(*m_ref)--;

			if(*m_ref <= 0)
			{
				DELETE_MP_OBJECT(m_ref);
				m_ref = 0;

				if(m_ptr)
				{
					delete m_ptr;
					m_ptr = 0;
				}
			}
		}

		if(ptr == 0)
		{
			m_ref = 0;
			m_ptr = 0;
		}
		else
		{
			m_ref = (int *)ALLOCATE_MEMORY(sizeof(int));
			*m_ref = 1;
			m_ptr = ptr;
		}

		return *this;
	}
	Smart_Ptr_Mem& operator = (Smart_Ptr_Mem const &ptr)
	{
		if(m_ptr == ptr.Get())
		{
			return *this;
		}

		if(m_ref)
		{
			(*m_ref)--;

			if(*m_ref <= 0)
			{
				DELETE_MP_OBJECT(m_ref);
				m_ref = 0;

				if(m_ptr)
				{
					delete m_ptr;
					m_ptr = 0;
				}
			}
		}

		if(ptr.Get() == 0)
		{
			m_ref = 0;
			m_ptr = 0;
		}
		else
		{
			m_ref = ptr.GetRef();
			m_ptr = ptr.Get();

			(*m_ref)++;
		}

		return *this;
	}

	template<class Y>
	Smart_Ptr_Mem& operator = (Smart_Ptr_Mem<Y> const &ptr)
	{
		if(m_ptr == ptr.Get())
		{
			return *this;
		}

		if(m_ref)
		{
			(*m_ref)--;

			if(*m_ref <= 0)
			{
				DELETE_MP_OBJECT(m_ref);
				m_ref = 0;

				if(m_ptr)
				{
					delete m_ptr;
					m_ptr = 0;
				}
			}
		}

		if(ptr.Get() == 0)
		{
			m_ref = 0;
			m_ptr = 0;
		}
		else
		{
			m_ref = ptr.GetRef();
			m_ptr = ptr.Get();

			(*m_ref)++;
		}

		return *this;
	}

	T& operator * () const
	{
		return *m_ptr;
	}

	T* operator -> () const
	{
		return m_ptr;
	}

	bool operator == (T* ptr)
	{
		return m_ptr == ptr;
	}
	bool operator == (Smart_Ptr_Mem const &ptr)
	{
		return m_ptr == ptr.m_ptr;
	}

	template<class Y>
	bool operator == (Smart_Ptr_Mem<Y> &ptr)
	{
		return m_ptr == ptr.Get();
	}

	operator bool () const
	{
		return m_ptr != 0;
	}

	bool operator! () const
	{
		return m_ptr == 0;
	}

	T* Get() const
	{
		return m_ptr;
	}

	int *GetRef() const
	{
		return m_ref;
	}

private :
	T* m_ptr;
	int* m_ref;
};
*/

}


#endif /* SMART_PTR_H_ */
