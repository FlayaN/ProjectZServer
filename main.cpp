#include <stdio.h>
#include <string>
#include <stdlib.h>
#include <iostream>
#include <enet/enet.h>


ENetEvent event;
ENetHost* server;
ENetPacket* packet;

char buffer[1400];

void sendToAllExceptId(int id)
{
	for(int i = 0; i < server->peerCount; i++)
	{
		if(id != std::atoi((char*)event.peer->data))
		{
			packet = enet_packet_create(event.packet->data, event.packet->dataLength, 0);
			enet_peer_send(&server->peers[i], 0, packet);
			enet_host_flush(server);
		}
	}
}

void sendToAll()
{
	for(int i = 0; i < server->peerCount; i++)
	{
		packet = enet_packet_create(event.packet->data, event.packet->dataLength, 0);
		enet_peer_send(&server->peers[i], 0, packet);
		enet_host_flush(server);
	}
}

int  main(int argc, char ** argv)
{
	int i;
	int curId = 0;
	
	if(enet_initialize() != 0)
	{
		printf("Could not initialize enet.");
		return 0;
	}

	ENetAddress address;
	address.host = ENET_HOST_ANY;
	address.port = 1234;

	server = enet_host_create(&address, 10, 2, 0, 0);

	if(server == NULL)
	{
		printf("Could not start server.\n");
		return 0;
	}

	
	while(true) 
	{
		while(enet_host_service(server, &event, 1000) > 0)
		{
			switch(event.type)
			{
				case ENET_EVENT_TYPE_CONNECT:
				{
					printf ("A new client connected from %x:%u.\n", event.peer->address.host, event.peer->address.port);
					
					char idBuffer[1400];
					sprintf(idBuffer, "%d", curId);

					event.peer->data = idBuffer;

					sprintf(buffer, "0 %d", curId);
					
					packet = enet_packet_create(buffer, strlen(buffer)+1, ENET_PACKET_FLAG_RELIABLE);
					
					enet_peer_send(event.peer, 0, packet);
					enet_host_flush(server);
					curId++;

					break;
				}

				case ENET_EVENT_TYPE_RECEIVE:
				{
					int type, id;
					sscanf((char*)event.packet->data, "%d %d", &type, &id);

					if(type != 1)
						printf ("A packet of length %u containing %s was received from %s on channel %u.\n", event.packet -> dataLength, event.packet -> data, event.peer -> data, event.channelID);

					switch (type)
					{
						case 3: //Message
						{
							sendToAll();
							break;
						}
					}

					enet_packet_destroy (event.packet);

					break;
				}

				case ENET_EVENT_TYPE_DISCONNECT:
				{
					printf ("%s disconected.\n", event.peer->data);
					event.peer->data = NULL;
					break;
				}
			}
		}
	}
	enet_host_destroy(server);
	enet_deinitialize();
}