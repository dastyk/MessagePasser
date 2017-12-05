#include "MessagePasser.h"



MessagePasser::MessagePasser()
{
	running = true;
	myThread = std::thread(&MessagePasser::Run, this);
}


MessagePasser::~MessagePasser()
{
	running = false;
	if (myThread.joinable())
	{
		myThread.join();
	}
}

void MessagePasser::Register(Utilz::GUID name, const std::unordered_set<Utilz::GUID, Utilz::GUID::Hasher>& messages)
{
	targets[name].messages = messages;
}

void MessagePasser::Unregister(Utilz::GUID name)
{
	targets.erase(name);
}

void MessagePasser::SendMessage(Utilz::GUID to, Utilz::GUID from, Utilz::GUID message, PayLoad payload)
{
	if (auto const findTarget = targets.find(from); findTarget != targets.end())
	{
		findTarget->second.newMessages.push({to, from, message,payload });
	}
}

void MessagePasser::SendMessage(Utilz::GUID from, Utilz::GUID message, PayLoad payload)
{
	if (auto const findTarget = targets.find(from); findTarget != targets.end())
	{
		findTarget->second.newMessages.push({ "Unspecified", from, message,payload });
	}
}

void MessagePasser::GetMessages(Utilz::GUID name, MessageQueue& queue)
{
	if (auto const findTarget = targets.find(name); findTarget != targets.end())
	{
		std::lock_guard<std::mutex> lock(findTarget->second.queueLock);
		queue = std::move(findTarget->second.queue);
	}
}

void MessagePasser::Run()
{
	while (running)
	{
		for (auto& from : targets)
		{
			if (!from.second.newMessages.wasEmpty())
			{
				
				while (!from.second.newMessages.wasEmpty())
				{
					auto top = from.second.newMessages.top();
					if (top.to == "Unspecified")
					{
						for (auto& to : targets)
						{
							if (auto const findMSG = to.second.messages.find(top.message); findMSG != to.second.messages.end())
							{
								std::lock_guard<std::mutex> lock(to.second.queueLock);
								to.second.queue.push(std::move(top));
								break;
							}
						}
					}
					else if (auto const to = targets.find(top.to); to != targets.end())
					{
						std::lock_guard<std::mutex> lock(to->second.queueLock);
						to->second.queue.push(std::move(top));
					}				
					from.second.newMessages.pop();
				}
			}
		}
	}
}
