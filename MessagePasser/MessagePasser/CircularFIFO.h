#ifndef _UTILZ_CIRCULAR_FIFO_H_
#define _UTILZ_CIRCULAR_FIFO_H_
#include <atomic>
#include <assert.h>

namespace Utilz
{
	template<typename Element, size_t size = 256>
	class CircularFiFo
	{
	public:
		enum { Capacity = size + 1 };
		CircularFiFo() : tail(0), head(0) {};
		~CircularFiFo() {};

		inline bool WasFull() const
		{
			auto index = tail.load();
			auto next_tail = (index + 1) % Capacity;
			return (next_tail == head.load());
		}

		inline bool wasEmpty() const
		{
			return (head.load() == tail.load());
		}

		//Consumer only
		inline const Element& top()
		{
			const auto current_head = head.load();
			//if (current_head == tail.load())
			//	return false;   // empty queue

			return buffer[current_head];
			//return true;
		}

		//Consumer only
		inline bool pop()
		{
			const auto current_head = head.load();
			if (current_head == tail.load())
				return false;   // empty queue

			head.store((current_head + 1) % Capacity);
			return true;
		}

		// Producer only
		inline bool push(const Element& item)
		{
			auto current_tail = tail.load();
			auto current_head = head.load();
			auto next_tail = (current_tail + 1) % Capacity;
			if (next_tail != head.load())
			{
				buffer[current_tail] = item;
				tail.store(next_tail);
				return true;
			}
			assert(next_tail == current_head);
			return false;  // full queue
		}

	private:
		Element buffer[Capacity];
		std::atomic<size_t> head;
		std::atomic<size_t> tail;
	};
}	//namespace UTilz

#endif  // _UTILZ_CIRCULAR_FIFO_H_