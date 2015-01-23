#pragma once
#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "Player.h"
#include "Tile.h"
//RakNet
#include "RakPeerInterface.h"
#include "MessageIdentifiers.h"
#include "BitStream.h"
#include "RakNetTypes.h"  // MessageID

#define SERVER_PORT 80

enum GameState{
	IN_MENU, IN_LOBBY, IN_GAME
};

enum GameMessages{
	ID_GAME_MESSAGE_1 = ID_USER_PACKET_ENUM + 1,
	ID_INIT_MESSAGE_1 = ID_USER_PACKET_ENUM + 2,
	ID_LOBBY_COUNT = ID_USER_PACKET_ENUM + 3,
	ID_REMOVE_CLIENT = ID_USER_PACKET_ENUM + 4,
	ID_CLIENT_POSITION = ID_USER_PACKET_ENUM + 5,
	ID_NEW_TILE = ID_USER_PACKET_ENUM + 6
};

class NetworkManager{

public:
	NetworkManager();
	~NetworkManager();

	std::string Connect(std::string _id, std::string _lobby);
	void Disconnect();
	bool IsConnected();

	void Update(float _delta);
	void SendChatMessage(std::string _message);
	void SendNewTile(glm::vec3 _pos, float _rotation);

	std::string GetPendingChatMessage();
	bool UpdateClients();
	bool UpdateTiles();
	std::vector<Player>* GetClients();
	std::vector<Tile*> GetTiles();
	int GetLobbyCount();

	std::string id, lobby;

	void UpdateServer(glm::vec3 _pos);

	std::string GetServerMessage();

	GameState state;
private:
	bool connected;
	bool update_clients, update_tiles;

	std::string server_message;

	RakNet::RakPeerInterface *peer;
	RakNet::Packet *packet;

	RakNet::SystemAddress serverAddress;

	int lobby_count;
	std::vector<Player> clients;

	std::vector<Tile*> tiles;

	std::vector<std::string> chat_log;
	std::vector<std::string> new_messages;

	float delay;
};

