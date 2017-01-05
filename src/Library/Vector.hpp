/*
 * Vector.hpp
 *
 * Created: 09/11/2014 09:50:41
 *  Author: David
 */ 


#ifndef VECTOR_H_
#define VECTOR_H_

#include "ecv.h"
#include <cstddef>		// for size_t
#include <cstdarg>
#include <cstring>
#undef printf
#undef scanf
void printf();			// to keep gcc happy when we include cstdio
void scanf();			// to keep gcc happy when we include cstdio
#include <cstdio>

// Bounded vector class
template<class T, size_t N> class Vector
{
public:
	Vector() : filled(0) { }
		
	bool full() const { return filled == N; }

	size_t capacity() const { return N; }

	size_t size() const { return filled; }

	bool isEmpty() const { return filled == 0; }

	const T& operator[](size_t index) const pre(index < N) { return storage[index]; }

	T& operator[](size_t index) pre(index < N) { return storage[index]; }

	void add(const T& x) pre(filled < N) { storage[filled++] = x; }

	void add(const T* array p, size_t n) pre(filled + n <= N);
	
	void erase(size_t pos, size_t count = 1);

	void clear() { filled = 0; }

	const T* array c_ptr() { return storage; }

	void sort(bool (*sortfunc)(T, T));

protected:
	T storage[N];
	size_t filled;	
};

template<class T, size_t N> void Vector<T, N>::add(const T* array p, size_t n)
{
	while (n != 0 && filled != N)
	{
		storage[filled++] = *p++;
		--n;
	}
}

template<class T, size_t N> void Vector<T, N>::sort(bool (*sortfunc)(T, T))
{
	for (size_t i = 1; i < filled; ++i)
	{
		for (size_t j = 0; j < i; ++j)
		{
			if ((*sortfunc)(storage[j], storage[i]))
			{
				T temp = storage[i];
				// Insert element i just before element j
				for (size_t k = i; k > j; --k)
				{
					storage[k] = storage[k - 1];
				}
				storage[j] = temp;
			}
		}
	}
}

template<class T, size_t N> void Vector<T, N>::erase(size_t pos, size_t count)
{
	while (pos + count < filled)
	{
		storage[pos] = storage[pos + count];
		++pos;
	}
	if (pos < filled)
	{
		filled = pos;
	}
}

// String class. This is like the vector class except that we always keep a null terminator so that we can call c_str() on it.
template<size_t N> class String : public Vector<char, N + 1>
{
public:
	String() : Vector<char, N + 1>()
	{
		this->clear();
	}
	
	String(const char* array s) : Vector<char, N + 1>()
	{
		this->clear();
		this->copy(s);
	}

	// Redefine 'full' so as to make room for a null terminator
	bool full() const { return this->filled == N; }
		
	// Redefine 'add' to add a null terminator
	void add(char x) pre(this->filled < N)
	{
		this->storage[this->filled++] = x; 
		this->storage[this->filled] = '\0';
	}
	
	// Redefine 'erase' to preserve the null terminator
	void erase(size_t pos, size_t count = 1)
	{
		static_cast<Vector<char, N + 1>*>(this)->erase(pos, count);
		this->storage[this->filled] = '\0';
	}
		
	const char* array c_str() const { return this->storage; }
		
	void clear()
	{
		this->filled = 0;
		this->storage[0] = '\0'; 
	}
	
	void catFrom(const char* s)
	{
		while (*s != '\0' && this->filled < N)
		{
			this->storage[this->filled++] = *s++;
		}
		this->storage[this->filled] = '\0';
	}
	
	void copy(const char* s)
	{
		this->clear();
		this->catFrom(s);
	}
	
	template<size_t M> void copy(String<M> s)
	{
		copy(s.c_str());
	}
	
	int printf(const char *fmt, ...);
	
	int catf(const char *fmt, ...);
	
	// Compare with a C string. If the C string is too long but the part of it we could accommodate matches, return true.
	bool similar(const char* s) const
	{
		return strncmp(s, this->storage, N) == 0;
	}
	
	// Compare with a C string
	bool equals(const char* s) const
	{
		return strcmp(s, this->storage) == 0;
	}

	bool equalsIgnoreCase(const char* s) const
	{
		return strcasecmp(s, this->storage) == 0;
	}
};

template<size_t N> int String<N>::printf(const char *fmt, ...)
{
	va_list vargs;
	va_start(vargs, fmt);
	int ret = vsnprintf(this->storage, N + 1, fmt, vargs);
	va_end(vargs);

	if (ret < 0)
	{
		this->filled = 0;
		this->storage[0] = 0;
	}
	else if (ret >= 0 && (size_t)ret < N)
	{
		this->filled = ret;
	}
	else
	{
		this->filled = N;
	}
	return ret;
}

template<size_t N> int String<N>::catf(const char *fmt, ...)
{
	va_list vargs;
	va_start(vargs, fmt);
	int ret = vsnprintf(this->storage + this->filled, N + 1 - this->filled, fmt, vargs);
	va_end(vargs);
	
	if (ret < 0)
	{
		this->storage[this->filled] = 0;		// in case snprintf messed it up
	}
	else if (this->filled + ret < N)
	{
		this->filled += ret;
	}
	else
	{
		this->filled = N;
	}
	return ret;
}

#endif /* VECTOR_H_ */