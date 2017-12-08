#ifndef _MESSAGE_SWTITCHER_H_
#define _MESSAGE_SWTITCHER_H_
#include <tuple>
#include "GUID.h"
#include <type_traits>
#include <optional>
#include <array>
#include <unordered_set>
#include <utility>
#include <cstdlib>
#include <IMessagePasser.h>
#include <Profiler.h>
namespace Sorter
{



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
		constexpr const size_t size() const
		{
			return N;
		}
		T arr[N];
	};

	template <typename T, size_t N>
	constexpr void sort_impl(array<T, N> &array, size_t left, size_t right)
	{
		if (left < right)
		{
			size_t m = left;

			for (size_t i = left + 1; i < right; i++)
				if (array[i] < array[left])
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
	inline typename std::enable_if < High - Low < 64, std::optional<std::reference_wrapper<const TYPE>>>::type
		searchBinary(TYPE key, const ARRAY& arr)
	{
		for (size_t i = Low; i < High; i++)
			if (arr[i] == key)
				return arr[i];
		return std::nullopt;
	}

	template <size_t Low, size_t High, class TYPE, class ARRAY>
	typename std::enable_if < Low < High && High - Low >= 64, std::optional<std::reference_wrapper<const TYPE>>>::type
		searchBinary(TYPE key, const ARRAY& arr)
	{
		constexpr size_t Mid = Low + ((High - Low) >> 1);
		if (arr[Mid] == key)
		{
			return arr[Mid];
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
}
using CallbackParamerer = Message&;
template<Utilz::HashValue... MSGS>
class MessageSwitcher
{
	static constexpr size_t NUM_MESSAGES = sizeof...(MSGS);
	using CallbackType = std::function<void(CallbackParamerer)>;
	using CallbackArray = const std::array<CallbackType, NUM_MESSAGES>;
	struct MSGPair
	{
		Utilz::HashValue message;
		size_t index;
		constexpr operator Utilz::HashValue()const { return message; }
		operator size_t()const = delete;
		constexpr bool operator<(const Utilz::HashValue other)const { return message < other; }
		constexpr bool operator==(const Utilz::HashValue other)const { return message == other; }
	};
	template<typename M, typename I>
	static constexpr MSGPair CreatePair(M m, I i)
	{
		return { m, i };
	}

	template<size_t... I>
	static constexpr auto CreateMessageArray(std::index_sequence<I...>)
	{
		return Sorter::sort(constexpr Sorter::array<MSGPair, NUM_MESSAGES>{ CreatePair(MSGS, I)... });
	};
	static constexpr auto messages = CreateMessageArray(std::make_index_sequence<NUM_MESSAGES>{});
	CallbackArray callbacks;
	inline auto findMessage(Utilz::HashValue message)const
	{
		return Sorter::searchBinary<0, NUM_MESSAGES>(CreatePair(message, 0u), messages);
	}

public:
	template<class... F>
	constexpr MessageSwitcher(const F&... funcs) : callbacks({ funcs... })
	{
		static_assert(NUM_MESSAGES == sizeof...(F), "All messages must have an implementation");	
	}

	constexpr std::unordered_set<Utilz::GUID, Utilz::GUID::Hasher> GetMessageSet()const
	{
		return { MSGS ... };
	}

	void ResolveMessage(CallbackParamerer msg)const
	{
		StartProfile;
		if (const auto find = findMessage(msg.message); find.has_value())
		{
			callbacks[find->get().index](msg);
		}
		StopProfile;
	}

	void ResolveAllMessages(MessageQueue& queue)const
	{
		StartProfile;
		while (queue.size())
		{
			Message msg = std::move(queue.front());
			ResolveMessage(msg);
			queue.pop();
		}
		StopProfile;
	}
};
#endif