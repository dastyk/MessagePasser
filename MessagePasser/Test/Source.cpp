#include <IMessagePasser.h>
#include <thread>
#include <string>
#include <Profiler.h>

#pragma comment(lib, "MessagePasserD.lib")

void Prod(IMessagePasser* mp) 
{
	bool running = true;

	while (running)
	{
		StartProfile;
		MessageQueue messages;
		mp->GetMessages("Prod", messages);
		while (messages.size())
		{
			auto& msg = messages.front();
			switch (msg.message.id)
			{
			case "Stop"_hash:
			{
				running = false;
				mp->SendMessage("Prod", "Print", PayLoad("Stopping"));
				msg.promise.set_value(true);
				break;
			}
			case "Create"_hash:
			{
				auto count = msg.payload.Get<int>();
				for(int i =0 ; i< count; i++)
					printf("%d\n", i);
				msg.promise.set_value(true);
				break;
			}
			default:
				break;
			}
			messages.pop();
		}
		StopProfile;
	}
}
#include <chrono>
using namespace std::chrono_literals;
struct MoreComplex
{
	int asd;
	void* dit;
	std::vector<int> vec;
};

int main()
{
	auto mp = CreateMessagePasser();

//	mp->Register("Main", { "Print" });
	mp->Register("Prod", { "Stop, Create" });

//	mp->SendMessage("Main", "Main", "Print", PayLoad("Starting"));

	auto prod = std::thread(Prod, mp);
	auto future = mp->SendMessage("Prod", "Main", "Create", PayLoad(5));

	MessageQueue messages;
	while(true)
	{
		StartProfile;
		/*mp->GetMessages("Main", messages);
		while (messages.size())
		{
			auto& msg = messages.front();

			switch (msg.message.id)
			{
			case "Print"_hash:
			{
				auto temp = msg.payload.Get<const char*>();
				printf("%s\n", temp);
				msg.promise.set_value(true);
				break;
			}
			default:
				break;
			}


			messages.pop();
		}
		mp->SendMessage("Prod", "Main", "Create", PayLoad(1));*/

		if (future.get())
		{
			printf("Done\n");
			future = mp->SendMessage("Prod", "Main", "Create", PayLoad(5));
		}

		StopProfile;
	}
	
	prod.join();

	DestroyMessagePasser(mp);
}