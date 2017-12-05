#include <IMessagePasser.h>
#include "MessagePasser.h"

DECLDIR IMessagePasser * CreateMessagePasser()
{
	return new MessagePasser;
}

DECLDIR void DestroyMessagePasser(IMessagePasser * mp)
{
	delete (MessagePasser*)mp;
}
