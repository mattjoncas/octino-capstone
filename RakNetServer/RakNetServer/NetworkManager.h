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
	ID_GAME_MESSAGE_1 = ID_USER_PACKET_ENUM + 1, //chat message
	ID_INIT_MESSAGE_1 = ID_USER_PACKET_ENUM + 2, //message from a new potential client trying to join
	ID_LOBBY_COUNT = ID_USER_PACKET_ENUM + 3, //
	ID_REMOVE_CLIENT = ID_USER_PACKET_ENUM + 4, //client to be removed
	ID_CLIENT_DATA = ID_USER_PACKET_ENUM + 5, //client info to be sent to other clients
	ID_NEW_TILE = ID_USER_PACKET_ENUM + 6, //new tile from a client
	ID_START_GAME = ID_USER_PACKET_ENUM + 7, //tell clients to start game
	ID_READY_UP = ID_USER_PACKET_ENUM + 8, //clients can toggle if they are ready
	ID_END_TURN = ID_USER_PACKET_ENUM + 9 //
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

	void UpdateClients(Lobby _lobby);
	void UpdateClient(RakNet::SystemAddress _address);

	void SendPacket(RakNet::BitStream *_data, RakNet::SystemAddress _address);
	void SendPacket(RakNet::BitStream *_data, Client *_client);
	void SendPacket(RakNet::BitStream *_data, Lobby _lobby);
	void SendTiles(Lobby _lobby);
	void SendTiles(Client *_client);
	void SendTile(Lobby _lobby);

	std::vector<std::string> chat_log;
};

