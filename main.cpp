#include <SDL.h>
#include <SDL_net.h>
#include <iostream>
#include <vector>
#include <cstring>

struct data
{
	TCPsocket socket;
	Uint32 timeout;
	int id;
	data(TCPsocket sock, Uint32 t, int i) : socket(sock), timeout(t), id(i) {}
};

int main(int argc, char** argv)
{
	SDL_Init(SDL_INIT_EVERYTHING);
	SDLNet_Init();
	int curId = 0;
	int playerNum = 0;
	SDL_Event event;
	IPaddress ip;
	SDLNet_ResolveHost(&ip, NULL, 1234);
	std::vector<data> socketV;
	char tmp[1400];
	bool running = true;
	SDLNet_SocketSet sockets = SDLNet_AllocSocketSet(10);
	SDL_Window* win = SDL_CreateWindow("ProjectZ Server",  SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_SWSURFACE);
	TCPsocket server = SDLNet_TCP_Open(&ip);

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
			if(playerNum < 10)
			{
				SDLNet_TCP_AddSocket(sockets, tmpSocket);
				socketV.push_back(data(tmpSocket, SDL_GetTicks(), curId));
				playerNum++;
				sprintf(tmp, "0 %d \n", curId);
				curId++;

				std::cout << "New Connection: " << curId << std::endl;
			}
			else
			{
				sprintf(tmp, "3 \n");
			}
			SDLNet_TCP_Send(tmpSocket, tmp, strlen(tmp)+1);
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
					int num = tmp[0] - '0';
					int j = 1;

					while(tmp[j] >= '0' && tmp[j] <= '9')
					{
						num *= 10;
						num += tmp[j] - '0';
						j++;

					}

					if(num == 1) //Data
					{
						//std::cout << "Packet nr 1 Received(data)" << std::endl;
						for(int k = 0; k < socketV.size(); k++)
						{
							if(k == i)
								continue;
							SDLNet_TCP_Send(socketV[k].socket, tmp, strlen(tmp)+1);
						}
					}else if(num == 2) //Disconnect
					{
						std::cout << "Packet nr 2 Received(Disconnect)" << std::endl;
						for(int k = 0; k < socketV.size(); k++)
						{
							if(k == i)
								continue;
							SDLNet_TCP_Send(socketV[k].socket, tmp, strlen(tmp)+1);
						}
						SDLNet_TCP_DelSocket(sockets, socketV[i].socket);
						SDLNet_TCP_Close(socketV[i].socket);
						socketV.erase(socketV.begin()+i);
						playerNum--;
					}else if(num = 3) //Shot, not used
					{
						std::cout << "Packet nr 3 Received" << std::endl;
						int tmpvar;
						sscanf(tmp, "3 %d", &tmpvar);
						for(int k = 0; k < socketV.size(); k++)
						{
							if(socketV[k].id = tmpvar)
							{
								SDLNet_TCP_Send(socketV[k].socket, tmp, strlen(tmp)+1);
								break;
							}

						}
					}
				}
			}
		}


		//disconnect - timeout
		for(int j = 0; j < socketV.size(); j++)
		{
			if(SDL_GetTicks() - socketV[j].timeout>5000)
			{
				sprintf(tmp, "2 %d\n", socketV[j].id);
				for(int k = 0; k < socketV.size(); k++)
				{
					SDLNet_TCP_Send(socketV[k].socket, tmp, strlen(tmp)+1);
				}
				SDLNet_TCP_DelSocket(sockets, socketV[j].socket);
				SDLNet_TCP_Close(socketV[j].socket);
				socketV.erase(socketV.begin()+j);
				playerNum--;
			}
		}
		SDL_Delay(1);
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