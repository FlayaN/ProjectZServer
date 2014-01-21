#include <SDL.h>
#include <SDL_net.h>
#include <iostream>
#include <vector>
#include <cstring>

#include "Server.h"

int main(int argc, char** argv)
{
	SDL_Init(SDL_INIT_EVERYTHING);
	SDLNet_Init();
	//int curId = 0;
	//int playerNum = 0;
	SDL_Event event;
	IPaddress ip;
	SDLNet_ResolveHost(&ip, NULL, 1234);
	std::vector<data> socketV;
	char tmp[1400];
	bool running = true;
	SDLNet_SocketSet sockets = SDLNet_AllocSocketSet(10);
	SDL_Window* win = SDL_CreateWindow("ProjectZ Server",  SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_SWSURFACE);
	SDL_HideWindow(win);
	TCPsocket server = SDLNet_TCP_Open(&ip);
	
	Server* s = new Server();
	while (running)
	{
		while( SDL_PollEvent(&event)) 
		{
			if(event.type == SDL_QUIT || event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
				running = false;
		}
		TCPsocket tmpSocket = SDLNet_TCP_Accept(server);
		if(tmpSocket)
		{
			s->join(tmpSocket, sockets, socketV);
		}
		//Check for incoming data
		while(SDLNet_CheckSockets(sockets, 0) > 0)
		{
			for(int i = 0; i < socketV.size(); i++)
			{
				if(SDLNet_SocketReady(socketV[i].socket))
				{
					socketV[i].timeout = SDL_GetTicks();
					SDLNet_TCP_Recv(socketV[i].socket, tmp, 1400);

					int num, id;
					sscanf(tmp, "%d %d", &num, &id);

					/*int num = tmp[0] - '0';
					int j = 1;

					while(tmp[j] >= '0' && tmp[j] <= '9')
					{
						num *= 10;
						num += tmp[j] - '0';
						j++;

					}*/

					if(num != 1)
						std::cout << "Num: " << num << " id: " << id << " i: " << i << std::endl;

					s->process(num, id, i, tmp, socketV, sockets);
				}
			}
		}

		//disconnect - timeout
		for(int j = 0; j < socketV.size(); j++)
		{
			//std::cout << "TMP: " << SDL_GetTicks() - socketV[j].timeout << std::endl;
			if(SDL_GetTicks() - socketV[j].timeout > 10000)
			{
				std::cout << "PlayerLeave(timeout): " << std::endl;
				sprintf(tmp, "2 %d \n", socketV[j].id);
				s->sendToAll(tmp, socketV);
				s->deleteSocket(j, socketV, sockets);
			}
		}
		//SDL_Delay(1);
	}

	for(int i = 0; i < socketV.size(); i++)
	{
		SDLNet_TCP_Close(socketV[i].socket);
	}

	SDLNet_FreeSocketSet(sockets);
	SDLNet_TCP_Close(server);
	SDLNet_Quit();
	SDL_Quit();
	return 0;
}