#include "MessagePasser.h"
#include <Profiler.h>


MessagePasser::MessagePasser()
{
	targets["MessagePasser"].messages = { "Stop" };

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

std::future<MessagePromiseType> MessagePasser::SendMessage(Utilz::GUID to, Utilz::GUID from, Utilz::GUID message, PayLoad payload)
{
	StartProfile;
	std::promise<MessagePromiseType> promise;
	auto future = promise.get_future();
	if (auto const findTarget = targets.find(from); findTarget != targets.end())
	{	
		findTarget->second.newMessages.push({ to, from, message, std::move(promise), payload });	
	}
	else
	{
		std::lock_guard<std::mutex> lg(defaultMessageLock);
		targets["MessagePasser"].newMessages.push({ to, from, message, std::move(promise), payload });
	}

	ProfileReturnConst(future);
}

std::future<MessagePromiseType> MessagePasser::SendMessage(Utilz::GUID from, Utilz::GUID message, PayLoad payload)
{
	StartProfile;
	std::promise<MessagePromiseType> promise;
	auto future = promise.get_future();
	if (auto const findTarget = targets.find(from); findTarget != targets.end())
	{
		findTarget->second.newMessages.push({ "Unspecified", from, message, std::move(promise), payload });
	}
	else
	{
		std::lock_guard<std::mutex> lg(defaultMessageLock);
		targets["MessagePasser"].newMessages.push({ "Unspecified", from, message, std::move(promise), payload });
	}

	ProfileReturnConst(future);
}

void MessagePasser::GetMessages(Utilz::GUID name, MessageQueue& queue)
{
	StartProfile;
	if (auto const findTarget = targets.find(name); findTarget != targets.end())
	{
		std::lock_guard<std::mutex> lock(findTarget->second.queueLock);
		queue = std::move(findTarget->second.queue);
	}
	StopProfile;
}

bool MessagePasser::GetLogMessage(std::string & message)
{
	if (!log.wasEmpty())
	{
		message = std::move(log.top());
		log.pop();
		return true;
	}
	return false;
}

void MessagePasser::Run()
{
	while (running)
	{
		StartProfile;
		for (auto& from : targets)
		{
			if (!from.second.newMessages.wasEmpty())
			{
				
				while (!from.second.newMessages.wasEmpty())
				{
					auto& top = from.second.newMessages.top();
					if (top.to == "Unspecified")
					{
						for (auto& to : targets)
						{
							if (auto const findMSG = to.second.messages.find(top.message); findMSG != to.second.messages.end())
							{
								std::lock_guard<std::mutex> lock(to.second.queueLock);
								to.second.queue.push(std::move(top));
								from.second.newMessages.pop();
								continue;
							}
						}
						log.push("No target had message " + std::to_string(top.message.id));
					}
					else if (auto const to = targets.find(top.to); to != targets.end())
					{
						std::lock_guard<std::mutex> lock(to->second.queueLock);
						to->second.queue.push(std::move(top));
						from.second.newMessages.pop();
					}	
					else
					{
						log.push("Could not find target " + std::to_string(top.to.id) + ", Message: " + std::to_string(top.message.id));
					}
					
				}
			}
		}
		StopProfile;
	}
}
