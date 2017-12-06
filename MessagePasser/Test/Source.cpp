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
				break;
			}
			case "Create"_hash:
			{
				auto count = msg.payload.Get<int>();
				for(int i =0 ; i< count; i++)
					mp->SendMessage("Prod", "Print", PayLoad("Message"));
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
struct MoreComplex
{
	int asd;
	void* dit;
	std::vector<int> vec;
};

int main()
{
	auto mp = CreateMessagePasser();

	mp->Register("Main", { "Print" });
	mp->Register("Prod", { "Stop, Create" });

	mp->SendMessage("Main", "Main", "Print", PayLoad("Starting"));

	auto prod = std::thread(Prod, mp);
	mp->SendMessage("Prod", "Main", "Create", PayLoad(5));

	MessageQueue messages;
	while(true)
	{
		StartProfile;
		mp->GetMessages("Main", messages);
		while (messages.size())
		{
			auto& msg = messages.front();

			switch (msg.message.id)
			{
			case "Print"_hash:
			{
				auto temp = msg.payload.Get<const char*>();
				printf("%s\n", temp);
				break;
			}
			default:
				break;
			}


			messages.pop();
		}
		mp->SendMessage("Prod", "Main", "Create", PayLoad(1));
		StopProfile;
	}
	


	


	////mp->SendMessage("Prod", "Stop", PayLoad("Main"));
	////mp->SendMessage("Main", "Print", PayLoad("Main", std::string("Stopping")));

	//MessageQueue messages;
	//int count = 0;
	//while (true)
	//{
	//	mp->GetMessages("Main", messages);
	//	while (messages.size())
	//	{
	//		auto& msg = messages.front();
	//		auto hash = "Print"_hash;
	//		switch (msg.message.id)
	//		{
	//		case "Print"_hash:
	//		{
	//			count++;
	//			auto temp = msg.payload.Get<std::string>();
	//			//printf("%s\n", temp.c_str());
	//			if (count > 100)
	//				mp->SendMessage("Prod", "Stop", PayLoad("Main"));
	//			break;
	//		}
	//		default:
	//			break;
	//		}
	//		messages.pop();
	//	}
	//}
	prod.join();

	DestroyMessagePasser(mp);
}