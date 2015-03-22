#pragma once
#include <stdio.h>
#include <time.h>
#include <iostream>
#include <string>
#include <vector>

#include "Lobby.h"
//RakNet
#include "RakPeerInterface.h"
#include "MessageIdentifiers.h"
#include "BitStream.h"
#include "RakNetTypes.h"  // MessageID

#include "DatabaseManager.h"

#define MAX_CLIENTS 15
#define SERVER_PORT 3265

enum GameMessages{
	ID_GAME_MESSAGE_1 = ID_USER_PACKET_ENUM + 1, //chat message
	ID_INIT_MESSAGE_1 = ID_USER_PACKET_ENUM + 2, //message from a new potential client trying to join
	ID_LOBBY_COUNT = ID_USER_PACKET_ENUM + 3, //
	ID_REMOVE_CLIENT = ID_USER_PACKET_ENUM + 4, //client to be removed
	ID_CLIENT_DATA = ID_USER_PACKET_ENUM + 5, //client info to be sent to other clients
	ID_NEW_TILE = ID_USER_PACKET_ENUM + 6, //new tile from a client
	ID_START_GAME = ID_USER_PACKET_ENUM + 7, //tell clients to start game
	ID_READY_UP = ID_USER_PACKET_ENUM + 8, //clients can toggle if they are ready
	ID_END_TURN = ID_USER_PACKET_ENUM + 9, //client can tell the server when their turn is finished
	ID_RANDOM_LOBBY = ID_USER_PACKET_ENUM + 10, //get a random lobby from the server
	ID_END_GAME = ID_USER_PACKET_ENUM + 11, //end game
	ID_CREATE_NEW_ID = ID_USER_PACKET_ENUM + 12 //client is creating a new id
};
enum LoginMessages{
	SUCCESSFUL, INVALID_ID, INVALID_PASSWORD, LOBBY_FULL, LOBBY_INGAME, DEFAULT_ERROR
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

	DatabaseManager dManager;

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

	std::string GetRandomLobby();

	std::vector<std::string> chat_log;
};

