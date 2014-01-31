#ifdef __APPLE__
    #include <SDL2/SDL.h>
#else
    #include <SDL.h>
#endif

#include <stdio.h>
#include <string>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <regex>

#include <enet/enet.h>
#include <curl/curl.h>
#include "lib/rapidjson/document.h"
#include "lib/rapidjson/filestream.h"

#include "Utility.h"

ENetEvent event;
ENetHost* server;
ENetPacket* packet;
int curId = 0;

char buffer[1400];

void sendToAllExceptId(int id, void* data, size_t length)
{
	for(int i = 0; i < server->peerCount; i++)
	{
		if(server->peers[i].data != nullptr)
		{
			if(id != *(int*)server->peers[i].data)
			{
				packet = enet_packet_create(data, length, 0);
				enet_peer_send(&server->peers[i], 0, packet);
				enet_host_flush(server);
			}
		}
	}
}

void sendToAll(void* data, size_t length)
{
	packet = enet_packet_create(data, length, 0);
	enet_host_broadcast(server, 0, packet);
	enet_host_flush(server);
}

void sendToSender(ENetPeer* peer, void* data, size_t length)
{
	packet = enet_packet_create(data, length, 0);
	enet_peer_send(peer, 0, packet);
	enet_host_flush(server);
}

void sendToId(int id, void* data, size_t length)
{
	for(int i = 0; i < server->peerCount; i++)
	{
		if(server->peers[i].data != nullptr)
		{
			if(id == *(int*)server->peers[i].data)
			{
				packet = enet_packet_create(data, length, 0);
				enet_peer_send(&server->peers[i], 0, packet);
				enet_host_flush(server);
			}
		}
	}
}

int main(int argc, char ** argv)
{
	std::string settingsServerPath = Utility::getBasePath() + "assets/config/settings/server.json";
	rapidjson::Document doc;
	FILE* pFile1 = fopen(settingsServerPath.c_str(), "rb");
	rapidjson::FileStream fs1(pFile1);
	doc.ParseStream<0>(fs1);

	int maxPlayers = doc["maxPlayers"].GetInt();
	std::string name = doc["name"].GetString();
	std::string description = doc["description"].GetString();


	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_Event ev;
	SDL_Window* win = SDL_CreateWindow("ProjectZ Server",  SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_SWSURFACE);

	std::string ip = Utility::doWebRequest("http://icanhazip.com");
	ip = std::regex_replace(ip,std::regex("\\s+"), "");
	

	description = std::regex_replace(description, std::regex("[[:space:]]"), "%20");

	std::cout << "Ip: " << ip << std::endl;
	std::cout << "Response: " << Utility::doWebRequest("http://hannesf.com/ProjectZ/add.php?ip=" + ip + "&name=" + name + "&description=" + description) << std::endl;

	
	int playerCount = 0;
	
	if(enet_initialize() != 0)
	{
		printf("Could not initialize enet.");
		return 0;
	}

	ENetAddress address;
	address.host = ENET_HOST_ANY;
	address.port = 1234;

	server = enet_host_create(&address, maxPlayers, 2, 0, 0);

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
					break;
				}

				case ENET_EVENT_TYPE_RECEIVE:
				{
					int type, id;
					sscanf((char*)event.packet->data, "%d %d", &type, &id);

					switch (type)
					{
						case 0: //Join
						{
							std::cout << curId << " joined" << std::endl;
							event.peer->data = new int(curId);

							sprintf(buffer, "0 %d", curId);
							sendToSender(event.peer, buffer, sizeof(buffer)+1);
							curId++;
							playerCount++;

							break;
						}
						case 1: //Disconnect
						{
							std::cout << *(int*)event.peer->data << " disconected" << std::endl;
							sprintf(buffer, "1 %d", *(int*)event.peer->data);
							sendToAll(buffer, sizeof(buffer)+1);
							event.peer->data = NULL;
							playerCount--;
							break;
						}
						case 2: //Ping
						{
							sendToSender(event.peer, event.packet->data, event.packet->dataLength);
							break;
						}
						case 3: //PlayerCount
						{
							sprintf(buffer, "3 %d %d", playerCount, maxPlayers);
							sendToSender(event.peer, buffer, sizeof(buffer)+1);
							break;
						}
						case 4: //Data
						{
							sendToAllExceptId(id, event.packet->data, event.packet->dataLength);
							break;
						}
						case 5: //Message
						{
							sendToAll(event.packet->data, event.packet->dataLength);
							break;
						}
						default:
						{
							std::cout << "Unknown packet id" << std::endl;
						}
					}

					//enet_packet_destroy (event.packet);

					break;
				}

				case ENET_EVENT_TYPE_DISCONNECT:
				{
					break;
				}
                case ENET_EVENT_TYPE_NONE:
                {
                    std::cout << "Unkown packet" << std::endl;
                    break;
                }
			}
		}

		while( SDL_PollEvent(&ev))
		{
			if(ev.type == SDL_QUIT || (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_ESCAPE))
				shouldQuit = true;
        }

		if(shouldQuit)
		{
			std::cout << "Started shutting down" << std::endl;
			if(Utility::doWebRequest("http://hannesf.com/ProjectZ/remove.php?ip=" + ip) == "success")
				running = false;
		}
	}
	enet_host_destroy(server);
	enet_deinitialize();
}