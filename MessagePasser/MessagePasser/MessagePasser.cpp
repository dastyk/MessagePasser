#include "MessagePasser.h"



MessagePasser::MessagePasser()
{
}


MessagePasser::~MessagePasser()
{
}

void MessagePasser::Register(Utilz::GUID name, const std::unordered_set<Utilz::GUID, Utilz::GUID::Hasher>& messages)
{
	targets[name].messages = messages;
}

void MessagePasser::Unregister(Utilz::GUID name)
{
	targets.erase(name);
}

void MessagePasser::SendMessage(Utilz::GUID to, Utilz::GUID message, PayLoad payload)
{
	if (auto const findTarget = targets.find(to); findTarget != targets.end())
	{
		findTarget->second.queue.push({ message,payload });
	}
}

void MessagePasser::SendMessage(Utilz::GUID message, PayLoad payload)
{
	for(auto& target : targets)
	{
		if (auto const findMessage = target.second.messages.find(message); findMessage != target.second.messages.end())
		{
			target.second.queue.push({ message,payload });
			break;
		}
	}
}

void MessagePasser::GetMessages(Utilz::GUID name, std::queue<Message>& queue)
{
	if (auto const findTarget = targets.find(name); findTarget != targets.end())
	{
		queue = std::move(findTarget->second.queue);
	}
}
