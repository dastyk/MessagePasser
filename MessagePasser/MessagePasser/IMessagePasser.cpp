#include <IMessagePasser.h>
#include "MessagePasser.h"

DECLDIR IMessagePasser * CreateMessagePasser()
{
	return new MessagePasser;
}
