/*
 * Vector.hpp
 *
 * Created: 09/11/2014 09:50:41
 *  Author: David
 */ 


#ifndef VECTOR_H_
#define VECTOR_H_

#include <cstddef>		// for size_t
#include "ecv.h"
#undef array
#undef result

// Bounded vector class
template<class T, size_t N> class Vector
{
public:
	Vector() : filled(0) { }
		
	bool full() const { return filled == N; }

	constexpr size_t capacity() const { return N; }

	size_t size() const { return filled; }

	bool isEmpty() const { return filled == 0; }

	const T& operator[](size_t index) const pre(index < N) { return storage[index]; }

	T& operator[](size_t index) pre(index < N) { return storage[index]; }

	bool add(const T& x);

	bool add(const T* _ecv_array p, size_t n);
	
	void erase(size_t pos, size_t count = 1);

	void truncate(size_t pos) pre(pos <= filled);

	void clear() { filled = 0; }

	const T* _ecv_array c_ptr() { return storage; }

	void sort(bool (*sortfunc)(T, T));

	bool replace(T oldVal, T newVal);

protected:
	T storage[N];
	size_t filled;	
};

template<class T, size_t N> bool Vector<T, N>::add(const T& x)
{
	if (filled < N)
	{
		storage[filled++] = x;
		return true;
	}
	return false;
}

template<class T, size_t N> bool Vector<T, N>::add(const T* _ecv_array p, size_t n)
{
	while (n != 0)
	{
		if (filled == N)
		{
			return false;
		}
		storage[filled++] = *p++;
		--n;
	}
	return true;
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

template<class T, size_t N> void Vector<T, N>::truncate(size_t pos)
{
	if (pos < filled)
	{
		filled = pos;
	}
}

template<class T, size_t N> bool Vector<T, N>::replace(T oldVal, T newVal)
{
	for (size_t i = 0; i < filled; ++i)
	{
		if (storage[i] == oldVal)
		{
			storage[i] = newVal;
			return true;
		}
	}
	return false;
}

#endif /* VECTOR_H_ */
