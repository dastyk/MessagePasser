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
		[mp](CallbackParamerer msg) {
		auto count = msg.payload.Get<int>();
		int sum = 0;
		for (int i = 0; i < count; i++)
			sum += i + 1;
		mp->SendMessage("Main", "Prod", "acc", PayLoad(sum));
		msg.promise.set_value(true);
	}
	);

	auto asd = ms.GetMessageSet();
	mp->Register("Prod", asd);

	while (running)
	{
		StartProfile;		
		mp->ResolveMessages("Prod", [ms](Message& msg) {
			ms.ResolveMessage(msg);
		});
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
	bool acc = true;
	auto mp = CreateMessagePasser();
	auto ms = MessageSwitcher<"acc"_hash>([&acc](CallbackParamerer msg) {
		acc = true;
	});

	mp->Register("Main", ms.GetMessageSet());

	auto prod = std::thread(Prod, mp);
	mp->Start();
	bool running = true;
	size_t count = 0;

	while(count < 10000000)
	{
		StartProfile;
		mp->ResolveMessages("Main", [ms](Message& msg) {
			ms.ResolveMessage(msg);
		});
		if (acc)
		{
			count++;
			acc = false;

			mp->SendMessage("Prod", "Main", "Create", PayLoad(5));

		}
		StopProfile;
	}
	mp->SendMessage("Prod", "Main", "Stop");
	prod.join();

	DestroyMessagePasser(mp);
}