#include <stdio.h>
#include <string>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <cstdlib>

#include <enet/enet.h>
#include <curl/curl.h>

ENetEvent event;
ENetHost* server;
ENetPacket* packet;
int curId = 0;

char buffer[1400];

void sendToAllExceptId(int id)
{
	for(int i = 0; i < server->peerCount; i++)
	{
		if(server->peers[i].data != nullptr)
		{
			if(id != *(int*)server->peers[i].data)
			{
				packet = enet_packet_create(event.packet->data, event.packet->dataLength, 0);
				enet_peer_send(&server->peers[i], 0, packet);
				enet_host_flush(server);
			}
		}
	}
}

void sendToAll()
{
	packet = enet_packet_create(event.packet->data, event.packet->dataLength, 0);
	enet_host_broadcast(server, 0, packet);
	enet_host_flush(server);
}

size_t writeToString(void *ptr, size_t size, size_t count, void *stream)
{
	((std::string*)stream)->append((char*)ptr, 0, size * count);
	return size * count;
}

int  main(int argc, char ** argv)
{
	char* result;
	CURL *curl;
	CURLcode res;
	std::string response;
	curl = curl_easy_init();
	
	if(curl)
	{
		curl_easy_setopt(curl, CURLOPT_URL, "http://www.icanhazip.com/");
		
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeToString);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
		
		res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);
	}

	std::cout << "Ip is: " << response << std::endl;

	int i;
	
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

					event.peer->data = new int(curId);

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

					//if(type != 1)
					//	printf ("A packet of length %u containing %s was received from %d on channel %u.\n", event.packet->dataLength, event.packet->data, *(int*)event.peer->data, event.channelID);

					switch (type)
					{
						case 1: //Data
						{
							sendToAllExceptId(id);
							break;
						}
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
					printf ("%d disconected.\n", *(int*)event.peer->data);

					for(int i = 0; i < server->peerCount; i++)
					{
						sprintf(buffer, "2 %d", *(int*)event.peer->data);
						packet = enet_packet_create(buffer, strlen(buffer)+1, ENET_PACKET_FLAG_RELIABLE);
						enet_peer_send(&server->peers[i], 0, packet);
						enet_host_flush(server);
					}
					event.peer->data = NULL;

					break;
				}
			}
		}
	}
	enet_host_destroy(server);
	enet_deinitialize();
}