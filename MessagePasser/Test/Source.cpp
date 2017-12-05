#include <IMessagePasser.h>

#pragma comment(lib, "MessagePasserD.lib")

int main()
{
	auto mp = CreateMessagePasser();

	mp->Register("Test", { "Create" });

	mp->SendMessage("Create", PayLoad("Test", 1337));


	MessageQueue messages;
	mp->GetMessages("Test", messages);

	while(messages.size())
	{
		auto top = messages.front();
		
		printf("%d", top.payload.Get<int*>());

		messages.pop();
	}

	delete mp;
}