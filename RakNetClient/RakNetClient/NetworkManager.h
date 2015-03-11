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
	ID_RANDOM_LOBBY = ID_USER_PACKET_ENUM + 10 //get a random lobby from the server
};

class NetworkManager{

public:
	enum GameState{
		MAIN_MENU, NETWORK_MENU, IN_LOBBY, IN_GAME
	};

	NetworkManager();
	~NetworkManager();

	std::string Connect();
	void Disconnect();
	std::string Join(std::string _id, std::string _lobby);
	bool IsConnected();

	void Update(float _delta);
	void SendChatMessage(std::string _message);
	void SendNewTile(glm::vec3 _pos, float _rotation, int _value);
	void EndTurn();

	std::string GetPendingChatMessage();
	bool UpdateClients();
	bool UpdateTiles();
	std::vector<Player>* GetClients();
	std::vector<Tile*> GetTiles();
	int GetLobbyCount();

	std::string GetRandomLobby();

	std::string id, lobby;
	bool is_turn;

	void UpdateServer(glm::vec3 _pos);

	std::string GetServerMessage();

	GameState state;

	void ReadyUp();
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

	void UpdateClientData();
};

