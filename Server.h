#include <SDL.h>
#include <SDL_net.h>
#include <iostream>
#include <vector>
#include <cstring>

#ifndef SERVER_H
#define SERVER_H

struct data
{
	TCPsocket socket;
	Uint32 timeout;
	int id;
	data(TCPsocket sock, Uint32 t, int i) : socket(sock), timeout(t), id(i) {}
};

class Server
{
public:
	Server(void);
	~Server(void);
	void recv(int type, int id, char buffer[1400]);
	void join(TCPsocket, SDLNet_SocketSet&, std::vector<data>&);
	void process(int, int, int, char[1400], std::vector<data>&, SDLNet_SocketSet&);

	void deleteSocket(int, std::vector<data>&, SDLNet_SocketSet&);

	void sendToAllExceptId(int, char[1400], std::vector<data>&);
	void sendToAll(char[1400], std::vector<data>);
private:

	char buffer[1400];
	int curId;
	int playerNum;
};
#endif