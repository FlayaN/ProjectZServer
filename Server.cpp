#include "Server.h"

Server::Server(void)
{
	curId = 0;
	playerNum = 0;
}

void Server::join(TCPsocket socket, SDLNet_SocketSet& sockets, std::vector<data>& socketV)
{
	if(playerNum < 10)
	{
		SDLNet_TCP_AddSocket(sockets, socket);
		socketV.push_back(data(socket, SDL_GetTicks(), curId));
		playerNum++;
		sprintf(buffer, "0 %d \n", curId);
		curId++;

		std::cout << "New Connection: " << curId << std::endl;
	}
	else
	{
		sprintf(buffer, "2 \n");
	}
	SDLNet_TCP_Send(socket, buffer, strlen(buffer)+1);
}

void Server::process(int type, int id, char buff[1400], std::vector<data>& socketV, SDLNet_SocketSet& sockets)
{
	switch(type)
	{
	case 1: //Data
		sendToAllExceptId(id, buff, socketV);
		break;
	case 2: //Disconnect
		sendToAllExceptId(id, buff, socketV);
		deleteSocket(id, socketV, sockets);
		break;
	default:
		std::cout << "Unknown packet id received" << std::endl;
		break;
	}
}

void Server::sendToAllExceptId(int id, char buff[1400], std::vector<data>& socketV)
{
	for(int k = 0; k < socketV.size(); k++)
	{
		if(k == id)
			continue;
		SDLNet_TCP_Send(socketV[k].socket, buff, strlen(buff)+1);
	}
}

void Server::sendToAll(char buff[1400], std::vector<data> socketV)
{
	for(auto s : socketV)
	{
		SDLNet_TCP_Send(s.socket, buff, strlen(buff)+1);
	}
}

void Server::deleteSocket(int i, std::vector<data>& socketV, SDLNet_SocketSet& sockets)
{
	SDLNet_TCP_DelSocket(sockets, socketV[i].socket);
	SDLNet_TCP_Close(socketV[i].socket);
	socketV.erase(socketV.begin()+i);
	playerNum--;
}