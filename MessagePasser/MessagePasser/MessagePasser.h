#ifndef MESSAGE_PASSER_H_
#define MESSAGE_PASSER_H_


#include <vector>
#include <unordered_map>
#include <IMessagePasser.h>
#include "CircularFIFO.h"
#include <thread>
#include <mutex>

class MessagePasser : public IMessagePasser
{
public:
	MessagePasser();
	~MessagePasser();


	void Register(Utilz::GUID name, const std::unordered_set<Utilz::GUID, Utilz::GUID::Hasher>& messages)override;
	void Unregister(Utilz::GUID name)override;

	std::future<MessagePromiseType> SendMessage(Utilz::GUID to, Utilz::GUID from, Utilz::GUID message, PayLoad payload)override;
	std::future<MessagePromiseType> SendMessage(Utilz::GUID from, Utilz::GUID message, PayLoad payload)override;

	//void GetMessages(Utilz::GUID name, MessageQueue& queue)override;
	void ResolveMessages(Utilz::GUID name, const std::function<void(Message&)>& resolver)override;
	bool GetLogMessage(std::string& message) override;

	void Start() override;
	void Stop() override;
private:
	void Run();
	bool running;
	bool started;
	std::thread myThread;
	std::mutex defaultMessageLock; // Only used when someone unregistered tried to send a message.

	Utilz::CircularFiFo<std::string> log;

	struct Target
	{
		std::unordered_set<Utilz::GUID, Utilz::GUID::Hasher> messages;
		Utilz::CircularFiFo<Message> newMessages;
		Utilz::CircularFiFo<Message> deliveredMessages;
	};	
	struct TargetToAddStruct
	{
		Utilz::GUID name;
		std::unordered_set<Utilz::GUID, Utilz::GUID::Hasher> messages;
	};
	Utilz::CircularFiFo<TargetToAddStruct> targetsToAdd;
	Utilz::CircularFiFo<Utilz::GUID> targetsToRemove;
	std::mutex targetAddRemoveLock;
	std::unordered_map<Utilz::GUID, Target, Utilz::GUID::Hasher> targets;
};

#endif