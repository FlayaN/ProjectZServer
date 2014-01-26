#include <stdio.h>
#include <string>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <regex>

#include <SDL.h>
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

std::string doWebRequest(std::string url)
{
	CURL* curl_handle = NULL;
	std::string response;

	/* initializing curl and setting the url */
	curl_handle = curl_easy_init();
	curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl_handle, CURLOPT_HTTPGET, 1);

	/* follow locations specified by the response header */
	curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1);

	/* setting a callback function to return the data */
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, writeToString);

	/* passing the pointer to the response as the callback parameter */
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &response);

	/* perform the request */
	curl_easy_perform(curl_handle);

	/* cleaning all curl stuff */
	curl_easy_cleanup(curl_handle);

	return response;
}

int  main(int argc, char ** argv)
{
	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_Event ev;
	SDL_Window* win = SDL_CreateWindow("ProjectZ Server",  SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_SWSURFACE);

	std::string ip = doWebRequest("http://icanhazip.com");
	ip = std::regex_replace(ip,std::regex("\\s+"), "");
	std::string name = "Hannes";
	std::string description = "Hannes Server";

	//std::regex space("[[:space:]]");
	description = std::regex_replace(description, std::regex("[[:space:]]"), "%20");

	std::cout << "Ip: " << ip << std::endl;
	std::cout << "Response: " << doWebRequest("http://hannesf.com/ProjectZ/add.php?ip=" + ip + "&name=" + name + "&description=" + description) << std::endl;

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

	bool running = true;
	bool shouldQuit = false;
	
	while(running)
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
					
					std::cout << "Sending : " << buffer << std::endl;
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

		while( SDL_PollEvent(&ev))
		{
			if(event.type == SDL_QUIT || ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_ESCAPE)
				shouldQuit = true;
        }

		if(shouldQuit)
		{
			std::cout << "Started shutting down" << std::endl;
			if(doWebRequest("http://hannesf.com/ProjectZ/remove.php?ip=" + ip) == "success")
			running = false;
		}
	}
	enet_host_destroy(server);
	enet_deinitialize();
}