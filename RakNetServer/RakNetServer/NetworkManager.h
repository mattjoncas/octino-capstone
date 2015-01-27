#pragma once
#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>

#include "Lobby.h"
//RakNet
#include "RakPeerInterface.h"
#include "MessageIdentifiers.h"
#include "BitStream.h"
#include "RakNetTypes.h"  // MessageID

#define MAX_CLIENTS 15
#define SERVER_PORT 80

enum GameMessages{
	ID_GAME_MESSAGE_1 = ID_USER_PACKET_ENUM + 1,
	ID_INIT_MESSAGE_1 = ID_USER_PACKET_ENUM + 2,
	ID_LOBBY_COUNT = ID_USER_PACKET_ENUM + 3,
	ID_REMOVE_CLIENT = ID_USER_PACKET_ENUM + 4,
	ID_CLIENT_POSITION = ID_USER_PACKET_ENUM + 5,
	ID_NEW_TILE = ID_USER_PACKET_ENUM + 6,
	ID_START_GAME = ID_USER_PACKET_ENUM + 7,
	ID_READY_UP = ID_USER_PACKET_ENUM + 8
};

class NetworkManager
{
public:
	NetworkManager();
	~NetworkManager();

	bool Update();
private:
	RakNet::RakPeerInterface *peer;
	RakNet::Packet *packet;

	std::vector<Client*> clients;
	std::vector<Lobby> lobbies;

	int AddClient();
	void RemoveClient(RakNet::SystemAddress _address);
	Client* FindClient(RakNet::SystemAddress _address);
	Lobby FindLobby(std::string _name);

	void UpdateClientCounts(Lobby _lobby);
	void UpdateClientPosition(RakNet::SystemAddress _address);

	void SendPacket(RakNet::BitStream *_data, RakNet::SystemAddress _address);
	void SendPacket(RakNet::BitStream *_data, Client *_client);
	void SendPacket(RakNet::BitStream *_data, Lobby _lobby);
	void SendTiles(Lobby _lobby);
	void SendTiles(Client *_client);
	void SendTile(Lobby _lobby);

	std::vector<std::string> chat_log;
};

