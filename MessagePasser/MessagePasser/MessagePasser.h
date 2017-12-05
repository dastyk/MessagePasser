#ifndef MESSAGE_PASSER_H_
#define MESSAGE_PASSER_H_


#include <vector>

#include <unordered_map>

#include <IMessagePasser.h>


class MessagePasser : public IMessagePasser
{
public:
	MessagePasser();
	~MessagePasser();


	void Register(Utilz::GUID name, const std::unordered_set<Utilz::GUID, Utilz::GUID::Hasher>& messages)override;
	void Unregister(Utilz::GUID name)override;

	void SendMessage(Utilz::GUID to, Utilz::GUID message, PayLoad payload)override;
	void SendMessage(Utilz::GUID message, PayLoad payload)override;

	void GetMessages(Utilz::GUID name, MessageQueue& queue)override;

private:
	struct Target
	{
		std::unordered_set<Utilz::GUID, Utilz::GUID::Hasher> messages;
		std::queue<Message> queue;
	};
	std::unordered_map<Utilz::GUID, Target, Utilz::GUID::Hasher> targets;
};

#endif