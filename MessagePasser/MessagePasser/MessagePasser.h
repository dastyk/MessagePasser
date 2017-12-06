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

	void SendMessage(Utilz::GUID to, Utilz::GUID from, Utilz::GUID message, PayLoad payload)override;
	void SendMessage(Utilz::GUID from, Utilz::GUID message, PayLoad payload)override;

	void GetMessages(Utilz::GUID name, MessageQueue& queue)override;
	bool GetLogMessage(std::string& message) override;
private:
	void Run();
	bool running;
	std::thread myThread;
	Utilz::CircularFiFo<std::string> log;
	struct Target
	{
		Utilz::CircularFiFo<Message> newMessages;
		std::unordered_set<Utilz::GUID, Utilz::GUID::Hasher> messages;
		std::mutex queueLock;
		std::queue<Message> queue;
	};
	std::unordered_map<Utilz::GUID, Target, Utilz::GUID::Hasher> targets;
};

#endif