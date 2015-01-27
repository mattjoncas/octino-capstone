#include "NetworkManager.h"


NetworkManager::NetworkManager(){
	peer = RakNet::RakPeerInterface::GetInstance();

	//set up RakNet Server
	{
		RakNet::SocketDescriptor sd(SERVER_PORT, 0);
		peer->Startup(MAX_CLIENTS, &sd, 1);

		std::cout << "Server Running\n";
		peer->SetMaximumIncomingConnections(MAX_CLIENTS);
	}
}

NetworkManager::~NetworkManager(){
	RakNet::RakPeerInterface::DestroyInstance(peer);
}

bool NetworkManager::Update(){
	bool _recieved = false;

	packet = peer->Receive();
	if (packet){
		_recieved = true;

		switch (packet->data[0]){
		case ID_REMOTE_DISCONNECTION_NOTIFICATION:
			printf("Another client has disconnected.\n");
			break;
		case ID_REMOTE_CONNECTION_LOST:
			printf("Another client has lost the connection.\n");
			break;
		case ID_REMOTE_NEW_INCOMING_CONNECTION:
			printf("Another client has connected.\n");
			break;
		case ID_CONNECTION_REQUEST_ACCEPTED:
			printf("Our connection request has been accepted.\n");
			break;
		case ID_NEW_INCOMING_CONNECTION:
			printf("A connection is incoming.\n");
			break;
		case ID_NO_FREE_INCOMING_CONNECTIONS:
			printf("The server is full.\n");
			break;
		case ID_DISCONNECTION_NOTIFICATION:
		{
			if (FindClient(packet->systemAddress)){
				RakNet::BitStream bsOut;
				bsOut.Write((RakNet::MessageID)ID_REMOVE_CLIENT);
				std::string _id = FindClient(packet->systemAddress)->id;
				bsOut.Write(_id.c_str());
				std::cout << _id << " has disconnected.\n";
				Lobby _lobby = FindLobby(FindClient(packet->systemAddress)->lobby);
				for (int c = 0; c < _lobby.clients.size(); c++){
					if (_lobby.clients[c]->address != packet->systemAddress){
						SendPacket(&bsOut, _lobby.clients[c]);
					}
				}
				RemoveClient(packet->systemAddress);
			}
		}
			break;
		case ID_CONNECTION_LOST:
			printf("A client lost the connection.\n");
			break;
		case ID_GAME_MESSAGE_1:
		{
			RakNet::RakString rs;
			RakNet::BitStream bsIn(packet->data, packet->length, false);
			bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
			bsIn.Read(rs);
			chat_log.push_back(rs.C_String());
			printf("%s\n", rs.C_String());

			RakNet::BitStream bsOut;
			bsOut.Write((RakNet::MessageID)ID_GAME_MESSAGE_1);
			bsOut.Write(rs.C_String());

			//print message to all clients in lobby [excluding client who sent message]
			Lobby _lobby = FindLobby(FindClient(packet->systemAddress)->lobby);
			for (int c = 0; c < _lobby.clients.size(); c++){
				if (_lobby.clients[c]->address != packet->systemAddress){
					SendPacket(&bsOut, _lobby.clients[c]);
				}
			}
		}
			break;
		case ID_INIT_MESSAGE_1:
		{
			int _result = AddClient();
			if (_result < 2){
				//send confirmation
				RakNet::BitStream bsOut;
				bsOut.Write((RakNet::MessageID)ID_INIT_MESSAGE_1);
				bsOut.Write("success");
				SendPacket(&bsOut, packet->systemAddress);

				//update client lobby counts
				UpdateClientCounts(FindLobby(FindClient(packet->systemAddress)->lobby));
				//send tiles to newest client
				SendTiles(clients[clients.size() - 1]);
			}
			else{
				RakNet::BitStream bsOut;
				bsOut.Write((RakNet::MessageID)ID_INIT_MESSAGE_1);
				if (_result == 2){ bsOut.Write("id invalid"); }
				else if (_result == 3){ bsOut.Write("lobby full"); }
				else { bsOut.Write("problem connecting"); }
				SendPacket(&bsOut, packet->systemAddress);
			}
		}
			break;
		case ID_CLIENT_POSITION:
		{
			UpdateClientPosition(packet->systemAddress);
		}
			break;
		case ID_NEW_TILE:
		{
			RakNet::BitStream bsIn(packet->data, packet->length, false);
			bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
			
			float x, y, z, r;
			bsIn.Read(x);
			bsIn.Read(y);
			bsIn.Read(z);
			bsIn.Read(r);

			for (int i = 0; i < lobbies.size(); i++){
				if (lobbies[i].name == FindClient(packet->systemAddress)->lobby){
					lobbies[i].tiles.push_back(Tile(x, y, z, r, FindClient(packet->systemAddress)->id, 0));
					//send newest tile to all clients
					SendTile(lobbies[i]);
				}
			}

			std::cout << "Tile Count: " << FindLobby(FindClient(packet->systemAddress)->lobby).tiles.size() << "\n";
		}
			break;
		case ID_READY_UP:
		{
			printf("Client Ready.\n");
			FindClient(packet->systemAddress)->ReadyUp();
		}
			break;
		default:
			printf("Message with identifier %i has arrived.\n", packet->data[0]);
			break;
		}
	}
	for (int i = 0; i < lobbies.size(); i++){
		lobbies[i].Update();
		if (lobbies[i].ready && !lobbies[i].inGame){
			RakNet::BitStream bsOut;
			bsOut.Write((RakNet::MessageID)ID_START_GAME);
			//send message to lobby
			SendPacket(&bsOut, lobbies[i]);
			lobbies[i].StartGame();
		}
	}

	peer->DeallocatePacket(packet);

	return _recieved;
}

int NetworkManager::AddClient(){
	RakNet::RakString client_id, lobby_name;
	RakNet::BitStream bsIn(packet->data, packet->length, false);
	bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
	//get id
	bsIn.Read(client_id);
	//get lobby name
	bsIn.Read(lobby_name);

	for (int i = 0; i < clients.size(); i++){
		if (clients[i]->id == client_id.C_String()){
			printf("connection attempt failed.\n");
			return 2;
		}
	}

	Client *newClient = new Client(packet->systemAddress, client_id.C_String(), lobby_name.C_String());

	for (int i = 0; i < lobbies.size(); i++){
		if (lobbies[i].name == lobby_name.C_String()){
			if (lobbies[i].clients.size() < 4){
				lobbies[i].clients.push_back(newClient);
				clients.push_back(newClient);
				std::cout << client_id << " joined '" << lobbies[i].name << "'\n";
				return 1;
			}
			else{
				std::cout << lobbies[i].name << " is full.\n";
				delete(newClient);
				return 3;
			}
		}
	}
	//lobby needs to be created
	lobbies.push_back(Lobby());
	lobbies[lobbies.size() - 1].name = lobby_name.C_String();
	lobbies[lobbies.size() - 1].clients.push_back(newClient);
	clients.push_back(newClient);
	std::cout << client_id << " created '" << lobbies[lobbies.size() - 1].name << "'\n";
	return 0;
}
void NetworkManager::RemoveClient(RakNet::SystemAddress _address){
	if (FindClient(packet->systemAddress) == NULL){
		return;
	}
	std::string _lobby = FindClient(packet->systemAddress)->lobby;
	for (int i = 0; i < lobbies.size(); i++){
		if (lobbies[i].name == _lobby){
			bool _update = true;
			//remove client from lobby first
			for (std::vector<Client*>::iterator iter = lobbies[i].clients.begin(); iter != lobbies[i].clients.end(); ++iter){
				if ((*iter)->address == FindClient(packet->systemAddress)->address){
					lobbies[i].clients.erase(iter);
					//if lobby is empty remove it
					if (lobbies[i].clients.size() < 1){
						lobbies.erase(lobbies.begin() + i);
						_update = false;
					}
					std::cout << "**lobbie count: " << lobbies.size() << "\n";
					break;
				}
			}
			//update client lobby counts
			if (_update){
				UpdateClientCounts(lobbies[i]);
			}
		}
	}
	//then remove client from main client list
	for (std::vector<Client*>::iterator iter = clients.begin(); iter != clients.end(); ++iter){
		if ((*iter)->address == FindClient(packet->systemAddress)->address){
			std::string client_id = (*iter)->id;
			delete((*iter));
			clients.erase(iter);
			std::cout << client_id << " removed, Clients left: " << clients.size() << "\n";
			break;
		}
	}
}
Client* NetworkManager::FindClient(RakNet::SystemAddress _address){
	for (int i = 0; i < clients.size(); i++){
		if (_address == clients[i]->address){
			return clients[i];
		}
	}
	return NULL;
}Lobby NetworkManager::FindLobby(std::string _name){
	for (int i = 0; i < lobbies.size(); i++){
		if (_name == lobbies[i].name){
			return lobbies[i];
		}
	}
}
void NetworkManager::UpdateClientCounts(Lobby _lobby){
	RakNet::BitStream bsOut;
	bsOut.Write((RakNet::MessageID)ID_LOBBY_COUNT);
	//get client names
	for (int c = 0; c < _lobby.clients.size(); c++){
		bsOut.Write(_lobby.clients[c]->id.c_str());
		bsOut.Write(_lobby.clients[c]->x);
		bsOut.Write(_lobby.clients[c]->y);
		bsOut.Write(_lobby.clients[c]->z);
	}
	SendPacket(&bsOut, _lobby);
}
void NetworkManager::UpdateClientPosition(RakNet::SystemAddress _address){
	RakNet::RakString rs;
	RakNet::BitStream bsIn(packet->data, packet->length, false);
	bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
	//x
	float n;
	bsIn.Read(n);
	FindClient(_address)->x = n;
	//y
	bsIn.Read(n);
	FindClient(_address)->y = n;
	//z
	bsIn.Read(n);
	FindClient(_address)->z = n;
	UpdateClientCounts(FindLobby(FindClient(_address)->lobby));
}

void NetworkManager::SendPacket(RakNet::BitStream *_data, RakNet::SystemAddress _address){
	peer->Send(_data, HIGH_PRIORITY, RELIABLE_ORDERED, 0, _address, false);
}
void NetworkManager::SendPacket(RakNet::BitStream *_data, Client *_client){
	peer->Send(_data, HIGH_PRIORITY, RELIABLE_ORDERED, 0, _client->address, false);
}
void NetworkManager::SendPacket(RakNet::BitStream *_data, Lobby _lobby){
	//send to each client
	for (int c = 0; c < _lobby.clients.size(); c++){
		peer->Send(_data, HIGH_PRIORITY, RELIABLE_ORDERED, 0, _lobby.clients[c]->address, false);
	}
}

void NetworkManager::SendTiles(Lobby _lobby){
	RakNet::BitStream bsOut;
	bsOut.Write((RakNet::MessageID)ID_NEW_TILE);
	//get client names
	for (int c = 0; c < _lobby.tiles.size(); c++){
		bsOut.Write(_lobby.tiles[c].x);
		bsOut.Write(_lobby.tiles[c].y);
		bsOut.Write(_lobby.tiles[c].z);
	}
	SendPacket(&bsOut, _lobby);
}
void NetworkManager::SendTiles(Client *_client){
	RakNet::BitStream bsOut;
	bsOut.Write((RakNet::MessageID)ID_NEW_TILE);

	Lobby _lobby = FindLobby(_client->lobby);

	//get client names
	for (int c = 0; c < _lobby.tiles.size(); c++){
		bsOut.Write(_lobby.tiles[c].x);
		bsOut.Write(_lobby.tiles[c].y);
		bsOut.Write(_lobby.tiles[c].z);
	}
	SendPacket(&bsOut, _client);
}
void NetworkManager::SendTile(Lobby _lobby){
	RakNet::BitStream bsOut;
	bsOut.Write((RakNet::MessageID)ID_NEW_TILE);
	bsOut.Write(_lobby.tiles[_lobby.tiles.size() - 1].x);
	bsOut.Write(_lobby.tiles[_lobby.tiles.size() - 1].y);
	bsOut.Write(_lobby.tiles[_lobby.tiles.size() - 1].z);
	bsOut.Write(_lobby.tiles[_lobby.tiles.size() - 1].rotation);
	SendPacket(&bsOut, _lobby);
}