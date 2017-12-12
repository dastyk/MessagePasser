# MessagePasser

A simple message passer. 
The message passer can handle any number of threads and is completely lock free. 
Each message consists of a target, source, message, and payload.
The payload is a kind of unique pointer which will transfer ownership of the payload when copying or moving.
The payload will only destroy the payload when the owner falls out of scope.
The user can provide a function which the payload will use when destroying the payload.
