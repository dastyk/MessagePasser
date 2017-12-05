#ifndef _INTERFACE_MESSAGE_PASSER_H_
#define _INTERFACE_MESSAGE_PASSER_H_
#include "GUID.h"
#include <unordered_set>
#include <functional>
#include <queue>

class PayLoad
{
	friend struct Message;
	void* payload;
	std::function<void(void* data)> deleter;
	PayLoad()
	{

	}
public:
	template<class TYPE>
	PayLoad(TYPE payload) : payload(new TYPE(payload)), deleter([](void* obj) { 
		delete (TYPE*)obj;
	})
	{

	}
	template<class TYPE>
	PayLoad(TYPE* payload, const std::function<void(void* data)>& deleter) : payload(payload), deleter(deleter)
	{

	}
	PayLoad(PayLoad& other)
	{
		this->deleter = std::move(other.deleter);
		this->payload = other.payload;
	}
	PayLoad(PayLoad&& other)
	{
		this->deleter = std::move(other.deleter);
		this->payload = other.payload;
	}
	PayLoad& operator=(PayLoad&& other)
	{
		this->deleter = std::move(other.deleter);
		this->payload = other.payload;
		return *this;
	}
	PayLoad& operator=(PayLoad& other)
	{
		this->deleter = std::move(other.deleter);
		this->payload = other.payload;
		return *this;
	}
	~PayLoad()
	{
		if (deleter)
			deleter(payload);
	}
	template<class TYPE>
	const TYPE& Get()
	{
		return *(TYPE*)payload;
	}
};
struct Message
{
	Utilz::GUID to;
	Utilz::GUID from;
	Utilz::GUID message;
	PayLoad payload;
};
typedef std::unordered_set<Utilz::GUID, Utilz::GUID::Hasher> MessageSet;
typedef std::queue<Message> MessageQueue;
class IMessagePasser
{
public:

	virtual void Register(Utilz::GUID name, const std::unordered_set<Utilz::GUID, Utilz::GUID::Hasher>& messages) = 0;
	virtual void Unregister(Utilz::GUID name) = 0;

	virtual void SendMessage(Utilz::GUID to, Utilz::GUID from, Utilz::GUID message, PayLoad payload) = 0;
	virtual void SendMessage(Utilz::GUID from, Utilz::GUID message, PayLoad payload) = 0;

	virtual void GetMessages(Utilz::GUID name, MessageQueue& queue) = 0;

protected:
	IMessagePasser() {};
	virtual ~IMessagePasser() {};

};


#if defined DLL_EXPORT_MESSAGE_PASSER
#define DECLDIR __declspec(dllexport)
#else
#define DECLDIR __declspec(dllimport)
#endif

DECLDIR IMessagePasser* CreateMessagePasser();
DECLDIR void DestroyMessagePasser(IMessagePasser*);


#endif