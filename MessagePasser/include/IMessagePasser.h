#ifndef _INTERFACE_MESSAGE_PASSER_H_
#define _INTERFACE_MESSAGE_PASSER_H_
#include "GUID.h"
#include <unordered_set>
#include <functional>
#include <queue>

class PayLoad
{
	Utilz::GUID from;
	void* payload;
	std::function<void(void* data)> deleter;

public:
	template<class TYPE>
	PayLoad(Utilz::GUID from, TYPE payload) : from(from), payload(new TYPE(payload)), deleter([](void*data) { delete (TYPE*)data; })
	{

	}
	template<class TYPE>
	PayLoad(Utilz::GUID from, TYPE* payload, const std::function<void(void* data)>& deleter) : from(from), payload(payload), deleter(deleter)
	{

	}
	PayLoad(PayLoad& other)
	{
		this->deleter = std::move(other.deleter);
		this->from = other.from;
		this->payload = other.payload;
	}
	PayLoad(PayLoad&& other)
	{
		this->deleter = std::move(other.deleter);
		this->from = other.from;
		this->payload = other.payload;
	}
	PayLoad& operator=(PayLoad&& other)
	{
		this->deleter = std::move(other.deleter);
		this->from = other.from;
		this->payload = other.payload;
		return *this;
	}
	PayLoad& operator=(PayLoad& other)
	{
		this->deleter = std::move(other.deleter);
		this->from = other.from;
		this->payload = other.payload;
		return *this;
	}
	~PayLoad()
	{
		if (deleter)
			deleter(payload);
	}
	template<class Type>
	const Type& Get()
	{
		return *(Type*)payload;
	}
};
struct Message
{
	Utilz::GUID message;
	PayLoad payload;
};
typedef std::unordered_set<Utilz::GUID, Utilz::GUID::Hasher> MessageSet;
typedef std::queue<Message> MessageQueue;
class IMessagePasser
{
public:
	virtual ~IMessagePasser() {};

	virtual void Register(Utilz::GUID name, const std::unordered_set<Utilz::GUID, Utilz::GUID::Hasher>& messages) = 0;
	virtual void Unregister(Utilz::GUID name) = 0;

	virtual void SendMessage(Utilz::GUID to, Utilz::GUID message, PayLoad payload) = 0;
	virtual void SendMessage(Utilz::GUID message, PayLoad payload) = 0;

	virtual void GetMessages(Utilz::GUID name, MessageQueue& queue) = 0;

protected:
	IMessagePasser() {};
};


#if defined DLL_EXPORT_MESSAGE_PASSER
#define DECLDIR __declspec(dllexport)
#else
#define DECLDIR __declspec(dllimport)
#endif

DECLDIR IMessagePasser* CreateMessagePasser();


#endif