#ifndef _INTERFACE_MESSAGE_PASSER_H_
#define _INTERFACE_MESSAGE_PASSER_H_
#include "GUID.h"
#include <unordered_set>
#include <functional>
#include <queue>
#include <future>
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
	explicit PayLoad(TYPE payload) : payload(new TYPE(payload)), deleter([](void* obj) { 
		delete (TYPE*)obj;
	})
	{

	}
	template<>
	explicit PayLoad(std::nullptr_t np) : payload(np), deleter()
	{

	}
	template<class TYPE>
	explicit PayLoad(TYPE* payload, const std::function<void(void* data)>& deleter) : payload(payload), deleter(deleter)
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
	void operator=(PayLoad&& other)
	{
		this->deleter = std::move(other.deleter);
		this->payload = other.payload;
	}
	void operator=(PayLoad& other)
	{
		this->deleter = std::move(other.deleter);
		this->payload = other.payload;
	}
	~PayLoad()
	{
		if (deleter)
			deleter(payload);
	}
	template<class TYPE>
	const TYPE& Get()const
	{
		return *(TYPE*)payload;
	}
};
typedef bool MessagePromiseType;
struct Message
{
	Utilz::GUID to;
	Utilz::GUID from;
	Utilz::GUID message;
	std::promise<MessagePromiseType> promise;
	PayLoad payload;
};
typedef std::unordered_set<Utilz::GUID, Utilz::GUID::Hasher> MessageSet;
typedef std::queue<Message> MessageQueue;
class IMessagePasser
{
public:

	virtual void Register(Utilz::GUID name, const std::unordered_set<Utilz::GUID, Utilz::GUID::Hasher>& messages) = 0;
	virtual void Unregister(Utilz::GUID name) = 0;

	virtual std::future<MessagePromiseType> SendMessage(Utilz::GUID to, Utilz::GUID from, Utilz::GUID message, PayLoad payload = PayLoad(nullptr)) = 0;
	virtual std::future<MessagePromiseType> SendMessage(Utilz::GUID from, Utilz::GUID message, PayLoad payload = PayLoad(nullptr)) = 0;

	//virtual void GetMessages(Utilz::GUID name, MessageQueue& queue) = 0;
	virtual void ResolveMessages(Utilz::GUID name, const std::function<void(Message&)>& resolver) = 0;
	virtual bool GetLogMessage(std::string& message) = 0;

	virtual void Start() = 0;
	virtual void Stop() = 0;

protected:
	IMessagePasser() {};
	virtual ~IMessagePasser() {};

};


#if defined DLL_EXPORT_MESSAGE_PASSER
#define DECLDIR __declspec(dllexport)
#else
#define DECLDIR __declspec(dllimport)
#endif
#define DECLDIR
DECLDIR IMessagePasser* CreateMessagePasser();
DECLDIR void DestroyMessagePasser(IMessagePasser*);


#endif