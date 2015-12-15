To run the network compile the solution and run the executable given. Type "ne help" to recieve a list of commands to use with the engine. In order to use 
"ne client help" or "ne server help" the corresponding server or client must be created.

In order to run a server type "ne server create", in order to create a client type "ne client create", to then connect the client to the server type 
"ne client connect" AFTER the client has been created. If there is no server up the client will still "connect to a server" only to be kicked off 
from a keep alive counter. 

Currently I keep track of RTT, it uses IP to distinguish between users(but I give them an ID to print out to keep it cleaner), I have a keep alive on both the server and the client, and an ACK system. There is also a simulator to run tests such as dropped packets and slower packets that normal among other things. 

In order to change port/IP it must be changed in the "Input.txt" in the solutions directory.

