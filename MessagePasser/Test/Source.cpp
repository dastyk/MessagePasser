#include <IMessagePasser.h>
#include <thread>
#include <string>
#include <Profiler.h>
#include <MessageSwitcher.h>
#ifdef _DEBUG
#pragma comment(lib, "MessagePasserD.lib")
#else
#pragma comment(lib, "MessagePasser.lib")
#endif
void Prod(IMessagePasser* mp) 
{
	bool running = true;

	MessageSwitcher<
		"Stop"_hash,
		"Create"_hash
	> ms
	(
		[&running, mp](CallbackParamerer msg) {
		running = false;
		mp->SendMessage("Prod", "Print", PayLoad("Stopping"));
		msg.promise.set_value(true);
	},
		[](CallbackParamerer msg) {
		auto count = msg.payload.Get<int>();
		for (int i = 0; i< count; i++)
			printf("%d\n", i);
		msg.promise.set_value(true);
	}
	);

	auto asd = ms.GetMessageSet();
	mp->Register("Prod", asd);

	
	MessageQueue messages;
	
	while (running)
	{
		StartProfile;		
		mp->GetMessages("Prod", messages);
		ms.ResolveAllMessage(messages);
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

	auto prod = std::thread(Prod, mp);
	auto future = mp->SendMessage("Prod", "Main", "Create", PayLoad(5));
	mp->Start();
	MessageQueue messages;
	bool running = true;
	size_t count = 0;
	while(count++ < 4000)
	{
		StartProfile;
		if (future.get())
		{
			printf("Done\n");
			
			future = mp->SendMessage("Prod", "Main", "Create", PayLoad(5));
			//running = false;
		}

		StopProfile;
	}
	future = mp->SendMessage("Prod", "Main", "Stop");
	prod.join();

	DestroyMessagePasser(mp);
}