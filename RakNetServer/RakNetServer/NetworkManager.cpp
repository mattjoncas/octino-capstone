#include "NetworkManager.h"


NetworkManager::NetworkManager(){
	peer = RakNet::RakPeerInterface::GetInstance();

	//set up RakNet Server
	{
		RakNet::SocketDescriptor sd(SERVER_PORT, 0);
		peer->Startup(MAX_CLIENTS, &sd, 1);


		char str[512];
		strcpy(str, peer->GetLocalIP(0));
		std::cout << "Server Running. Local IP: " << str << "\n";

		peer->SetMaximumIncomingConnections(MAX_CLIENTS);
	}

	srand(time(NULL));
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
			//send login result
			RakNet::BitStream bsOut;
			bsOut.Write((RakNet::MessageID)ID_INIT_MESSAGE_1);
			bsOut.Write(_result);
			SendPacket(&bsOut, packet->systemAddress);
			if (_result < 1){
				//update clients in lobby
				UpdateClients(FindLobby(FindClient(packet->systemAddress)->lobby));
				//send exsiting tiles to new client [not needed anymore*]
				SendTiles(clients[clients.size() - 1]);
			}
			else{
				printf("connection attempt failed.\n");
			}
		}
			break;
		case ID_CLIENT_DATA:
		{
			UpdateClient(packet->systemAddress);
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
			int v;
			bsIn.Read(v);

			for (int i = 0; i < lobbies.size(); i++){
				if (lobbies[i].name == FindClient(packet->systemAddress)->lobby){
					lobbies[i].AddTile(Tile(x, y, z, r, FindClient(packet->systemAddress)->id, v));
					//send newest tile to all clients
					SendTile(lobbies[i]);
					break;
				}
			}

			//std::cout << "Tile Count: " << FindLobby(FindClient(packet->systemAddress)->lobby).tiles.size() << "\n";
		}
			break;
		case ID_READY_UP:
		{
			printf("Client Ready.\n");
			FindClient(packet->systemAddress)->ReadyUp();
			UpdateClients(FindLobby(FindClient(packet->systemAddress)->lobby));
		}
			break;
		case ID_END_TURN:
		{
			for (int i = 0; i < lobbies.size(); i++){
				if (lobbies[i].name == FindClient(packet->systemAddress)->lobby){
					lobbies[i].IncrementTurn();
					UpdateClients(lobbies[i]);
					break;
				}
			}
		}
			break;
		case ID_RANDOM_LOBBY:
		{
			RakNet::BitStream bsOut;
			bsOut.Write((RakNet::MessageID)ID_RANDOM_LOBBY);
			bsOut.Write(GetRandomLobby().c_str());
			SendPacket(&bsOut, packet->systemAddress);
		}
			break;
		case ID_CREATE_NEW_ID:
		{
			RakNet::RakString new_id, new_pass;
			RakNet::BitStream bsIn(packet->data, packet->length, false);
			bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
			bsIn.Read(new_id);
			bsIn.Read(new_pass);

			RakNet::BitStream bsOut;
			bsOut.Write((RakNet::MessageID)ID_CREATE_NEW_ID);
			if (dManager.AddClient(new_id.C_String(), new_pass.C_String())){
				bsOut.Write(SUCCESSFUL);
			}
			else{
				bsOut.Write(DEFAULT_ERROR);
			}
			SendPacket(&bsOut, packet->systemAddress);
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
			UpdateClients(lobbies[i]);
		}
		if (lobbies[i].finished){
			RakNet::BitStream bsOut;
			bsOut.Write((RakNet::MessageID)ID_END_GAME);
			SendPacket(&bsOut, lobbies[i]);

			for (int c = 0; c < lobbies[i].clients.size(); c++){
				if (lobbies[i].clients[c]->hand_count == 0){
					dManager.AddWin(lobbies[i].clients[c]->id);
					break;
				}
			}

			lobbies[i].Reset();
			UpdateClients(lobbies[i]);
		}
	}

	peer->DeallocatePacket(packet);

	return _recieved;
}

int NetworkManager::AddClient(){
	RakNet::RakString client_id, client_pass, lobby_name;
	RakNet::BitStream bsIn(packet->data, packet->length, false);
	bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
	//get id / password
	bsIn.Read(client_id);
	bsIn.Read(client_pass);
	//get lobby name
	bsIn.Read(lobby_name);

	for (int i = 0; i < clients.size(); i++){
		if (clients[i]->id == client_id.C_String()){
			return INVALID_ID;
		}
	}
	if (!dManager.CheckID(client_id.C_String())){
		return INVALID_ID;
	}
	else{
		if (!dManager.Login(client_id.C_String(), client_pass.C_String())){
			std::cout << client_id.C_String() << ", " << client_pass.C_String() << "\n";
			return INVALID_PASSWORD;
		}
	}

	Client *newClient = new Client(packet->systemAddress, client_id.C_String(), lobby_name.C_String());

	for (int i = 0; i < lobbies.size(); i++){
		if (lobbies[i].name == lobby_name.C_String()){
			if (lobbies[i].inGame){
				std::cout << "Cannot join " << lobbies[i].name << ", in game.\n";
				delete(newClient);
				return LOBBY_INGAME;
			}
			if (!lobbies[i].IsFull()){
				lobbies[i].clients.push_back(newClient);
				clients.push_back(newClient);
				std::cout << client_id << " joined '" << lobbies[i].name << "'\n";
				return SUCCESSFUL;
			}
			else{
				std::cout << lobbies[i].name << " is full.\n";
				delete(newClient);
				return LOBBY_FULL;
			}
		}
	}
	//lobby needs to be created
	lobbies.push_back(Lobby());
	lobbies[lobbies.size() - 1].name = lobby_name.C_String();
	lobbies[lobbies.size() - 1].clients.push_back(newClient);
	clients.push_back(newClient);
	std::cout << client_id << " created '" << lobbies[lobbies.size() - 1].name << "'\n";
	return SUCCESSFUL;
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
					if ((*iter)->is_turn){
						lobbies[i].IncrementTurn();
					}
					lobbies[i].clients.erase(iter);
					//if lobby is empty remove it
					if (lobbies[i].clients.size() < 1){
						lobbies[i].clients.clear();
						lobbies.erase(lobbies.begin() + i);
						_update = false;
					}
					std::cout << "**lobbie count: " << lobbies.size() << "\n";
					break;
				}
			}
			//update client lobby counts
			if (_update){
				UpdateClients(lobbies[i]);
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
}
Lobby NetworkManager::FindLobby(std::string _name){
	for (int i = 0; i < lobbies.size(); i++){
		if (_name == lobbies[i].name){
			return lobbies[i];
		}
	}
}
void NetworkManager::UpdateClients(Lobby _lobby){
	RakNet::BitStream bsOut;
	bsOut.Write((RakNet::MessageID)ID_LOBBY_COUNT);
	//get client names
	for (int c = 0; c < _lobby.clients.size(); c++){
		bsOut.Write(_lobby.clients[c]->id.c_str());
		bsOut.Write(_lobby.clients[c]->ready);
		bsOut.Write(_lobby.clients[c]->is_turn);
		bsOut.Write(_lobby.clients[c]->x);
		bsOut.Write(_lobby.clients[c]->y);
		bsOut.Write(_lobby.clients[c]->z);
	}
	SendPacket(&bsOut, _lobby);
}
void NetworkManager::UpdateClient(RakNet::SystemAddress _address){
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
	int h;
	bsIn.Read(h);
	FindClient(_address)->hand_count = h;
	UpdateClients(FindLobby(FindClient(_address)->lobby));
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
		bsOut.Write(_lobby.tiles[c].rotation);
		bsOut.Write(_lobby.tiles[c].value);
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
		bsOut.Write(_lobby.tiles[c].rotation);
		bsOut.Write(_lobby.tiles[c].value);
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
	bsOut.Write(_lobby.tiles[_lobby.tiles.size() - 1].value);
	SendPacket(&bsOut, _lobby);
}

std::string NetworkManager::GetRandomLobby(){
	bool open_lobby = false;
	for (int i = 0; i < lobbies.size(); i++){
		if (!lobbies[i].IsFull() && !lobbies[i].inGame){
			open_lobby = true;
			break;
		}
	}
	if (open_lobby){
		std::string lobby_name = "";
		do {
			lobby_name = lobbies[rand() % lobbies.size()].name;
			
		} while (lobby_name == "");
		std::cout << "lobby open, '" << lobby_name << "' returned.\n";
		return lobby_name;
	}
	else{
		printf("No lobbies open.\n");
		return "";
	}
}