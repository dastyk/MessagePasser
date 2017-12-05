#include <IMessagePasser.h>

#pragma comment(lib, "MessagePasserD.lib")

int main()
{
	auto mp = CreateMessagePasser();

	mp->Register("Test", { "Create" });

	mp->SendMessage("Create", PayLoad("Test", new int(1337), [](void*data) {
		delete (int*)data; 
	}));

	delete mp;
}