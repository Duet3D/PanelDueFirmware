/*
 * ListHelpers.hpp
 *
 *  Created on: 17 Feb 2021
 *      Author: manuel
 */

#ifndef SRC_OBJECTMODEL_LISTHELPERS_HPP_
#define SRC_OBJECTMODEL_LISTHELPERS_HPP_

#include <cstdint>
#include <General/inplace_function.h>

template<typename L, typename T>
T* GetOrCreate(L& list, const size_t index, const bool create)
{
	const size_t count = list.Size();
	for (size_t i = 0; i < count; ++i)
	{
		if (list[i]->index == index)
		{
			return list[i];
		}
	}

	if (create && !list.Full())
	{
		T* elem = new T;
		elem->Reset();
		elem->index = index;
		list.Add(elem);
		list.Sort([] (T* e1, T* e2) { return e1->index > e2->index; });
		return elem;
	}

	return nullptr;
}

template<typename L, typename T>
T* Find(L& list, stdext::inplace_function<bool(T*)> filter)
{
	const size_t count = list.Size();
	for (size_t i = 0; i < count; ++i)
	{
		if (filter(list[i]))
		{
			return list[i];
		}
	}
	return nullptr;
}

template<typename L, typename T>
void Iterate(L& list, stdext::inplace_function<void(T*)> func, const size_t startAt)
{
	const size_t count = list.Size();
	for (size_t i = 0; i < count; ++i)
	{
		if (list[i]->index >= startAt)
		{
			func(list[i]);
		}
	}
}

template<typename L, typename T>
bool IterateWhile(L& list, stdext::inplace_function<bool(T*)> func, const size_t startAt)
{
	const size_t count = list.Size();
	for (size_t i = 0; i < count; ++i)
	{
		if (list[i]->index >= startAt)
		{
			if(!func(list[i]))
			{
				return false;
			}
		}
	}
	return true;
}

template<typename L, typename T>
size_t Remove(L& list, const size_t index, const bool allFollowing)
{
	// Nothing to do on an empty list or
	// if the last element is already smaller than what we look for
	if (list.IsEmpty() || list[list.Size()-1]->index < index)
	{
		return 0;
	}

	size_t removed = 0;
	for (size_t i = list.Size(); i != 0;)
	{
		--i;
		T* elem = list[i];
		if (elem->index == index || (allFollowing && elem->index > index))
		{
			list.Erase(i);
			delete elem;
			++removed;
			if (!allFollowing)
			{
				break;
			}
		}
	}
	return removed;
}

#endif /* SRC_OBJECTMODEL_LISTHELPERS_HPP_ */
