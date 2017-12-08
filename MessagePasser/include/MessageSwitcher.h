#ifndef _MESSAGE_SWTITCHER_H_
#define _MESSAGE_SWTITCHER_H_
#include <tuple>
#include "GUID.h"




#include <utility>
#include <cstdlib>

template<class T>
constexpr void swap(T& l, T& r)
{
	T tmp = std::move(l);
	l = std::move(r);
	r = std::move(tmp);
}

template <typename T, size_t N>
struct array
{
	constexpr T& operator[](size_t i)
	{
		return arr[i];
	}

	constexpr const T& operator[](size_t i) const
	{
		return arr[i];
	}

	constexpr const T* begin() const
	{
		return arr;
	}
	constexpr const T* end() const
	{
		return arr + N;
	}

	T arr[N];
};

template <typename T, size_t N>
constexpr void sort_impl(array<T, N> &array, size_t left, size_t right)
{
	if (left < right)
	{
		size_t m = left;

		for (size_t i = left + 1; i<right; i++)
			if (array[i]<array[left])
				swap(array[++m], array[i]);

		swap(array[left], array[m]);

		sort_impl(array, left, m);
		sort_impl(array, m + 1, right);
	}
}

template <typename T, size_t N>
constexpr array<T, N> sort(array<T, N> array)
{
	auto sorted = array;
	sort_impl(sorted, 0, N);
	return sorted;
}
//
//constexpr array<int, 11> unsorted{ 5,7,3,4,1,8,2,9,0,6,10 }; // odd number of elements
//constexpr auto sorted = sort(unsorted);

template <size_t Low, size_t High, class TYPE, class ARRAY>
size_t searchBinary(TYPE key, const ARRAY& arr)
{
	constexpr int Mid = (Low + High) / 2;
	if (arr[Mid] == key)
	{
		return Mid;
	}
	else if (arr[Mid] > key)
	{
		return searchBinary<Low, Mid - 1, TYPE, ARRAY>(key, arr);
	}
	else
	{
		return searchBinary<Mid + 1, High, TYPE, ARRAY>(key, arr);
	}
}


template<Utilz::HashValue... MSGS>
class MessageSwitcher
{
	static constexpr auto messages = sort(constexpr array<Utilz::HashValue, sizeof...(MSGS)>{ MSGS... });


	inline size_t findMessage(Utilz::HashValue message)
	{
		return searchBinary<0, sizeof...(MSGS)>(message, messages);
	}

public:
	template<class... F>
	MessageSwitcher(const F&... funcs) 
	{
		Utilz::HashValue b = "Create"_hash;
		auto msg = findMessage(b);
		switch (b)
		{
		case 0:
		{
			break;
		}
		case messages[0]:
		{
			int i = 0;
			break;
		}
		default:
			break;
		}
	}
};
#endif