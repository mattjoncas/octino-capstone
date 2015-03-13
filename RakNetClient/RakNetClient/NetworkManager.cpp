#include "NetworkManager.h"

NetworkManager::NetworkManager(){
	peer = RakNet::RakPeerInterface::GetInstance();

	//set up client
	RakNet::SocketDescriptor sd;
	peer->Startup(1, &sd, 1);

	connected = false;
	update_clients = true;
	update_tiles = true;

	//ping server [for science]
	//peer->Ping("255.255.255.255", SERVER_PORT, 0);
	peer->Ping("255.255.255.255", SERVER_PORT, 0);
	
	server_message = "";

	state = MAIN_MENU;
}

NetworkManager::~NetworkManager(){

	RakNet::RakPeerInterface::DestroyInstance(peer);
}
bool NetworkManager::IsConnected(){
	return connected;
}

std::string NetworkManager::Connect(){
	
	char str[512];
		
	strcpy(str, peer->GetLocalIP(0));
	std::cout << str << "\n";
		
	peer->Connect(str, SERVER_PORT, 0, 0);

	connected = true;

	return "Please enter id/lobby.";
}
void NetworkManager::Disconnect(){
	peer->CloseConnection(serverAddress, true, 0, HIGH_PRIORITY);

	connected = false;
	state = MAIN_MENU;
}
std::string NetworkManager::Join(std::string _id, std::string _pass, std::string _lobby){
	if (_id != "" && _lobby != ""){
		id = _id;
		lobby = _lobby;

		clients.clear();
		lobby_count = 0;

		RakNet::BitStream bsOut;
		bsOut.Write((RakNet::MessageID)ID_INIT_MESSAGE_1);
		bsOut.Write(id.c_str());
		bsOut.Write(_pass.c_str());
		bsOut.Write(lobby.c_str());
		peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, serverAddress, false);

		return "Connecting...";
	}
	else{
		return "id/lobby required.";
	}
}
void NetworkManager::UpdateServer(glm::vec3 _pos){
	RakNet::BitStream bsOut;
	bsOut.Write((RakNet::MessageID)ID_CLIENT_DATA);
	//send packet
	bsOut.Write(_pos.x);
	bsOut.Write(_pos.y);
	bsOut.Write(_pos.z);
	peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, serverAddress, false);
}

void NetworkManager::SendChatMessage(std::string _message){
	//write/send message
	RakNet::BitStream bsOut;
	bsOut.Write((RakNet::MessageID)ID_GAME_MESSAGE_1);
	
	//send packet
	std::string message = id + ": " + _message;
	bsOut.Write(message.c_str());
	peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, serverAddress, false);

	//add message to chat_log
	chat_log.push_back(message);
}
void NetworkManager::SendNewTile(glm::vec3 _pos, float _rotation, int _value){
	RakNet::BitStream bsOut;
	bsOut.Write((RakNet::MessageID)ID_NEW_TILE);
	bsOut.Write(_pos.x);
	bsOut.Write(_pos.y);
	bsOut.Write(_pos.z);
	bsOut.Write(_rotation);
	bsOut.Write(_value);
	peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, serverAddress, false);
}
void NetworkManager::EndTurn(){
	RakNet::BitStream bsOut;
	bsOut.Write((RakNet::MessageID)ID_END_TURN);
	peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, serverAddress, false);
}

std::string NetworkManager::GetPendingChatMessage(){
	if (new_messages.size() > 0){
		std::string _m = new_messages[0];
		new_messages.erase(new_messages.begin());
		chat_log.push_back(_m);
		return _m;
	}
	else{
		return "";
	}
}
bool NetworkManager::UpdateClients(){
	if (update_clients){ 
		update_clients = false;
		return true;
	}
	return false;
}
bool NetworkManager::UpdateTiles(){
	return update_tiles;
}
std::vector<Player>* NetworkManager::GetClients(){
	update_clients = false;
	return &clients;
}
std::vector<Tile*> NetworkManager::GetTiles(){
	update_tiles = false;
	return tiles;
}
int NetworkManager::GetLobbyCount(){
	return lobby_count;
}

std::string NetworkManager::GetServerMessage(){
	std::string _m = server_message;
	server_message = "";
	return _m;
}

void NetworkManager::ReadyUp(){
	RakNet::BitStream bsOut;
	bsOut.Write((RakNet::MessageID)ID_READY_UP);
	peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, serverAddress, false);
}
void NetworkManager::UpdateClientData(){
	RakNet::RakString rs;
	RakNet::BitStream bsIn(packet->data, packet->length, false);
	bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
	while (bsIn.GetNumberOfUnreadBits() > 0){
		bool new_client = true;
		bsIn.Read(rs);
		std::string _c = rs.C_String();
		int _ready, _turn;
		bsIn.Read(_ready);
		bsIn.Read(_turn);
		float x, y, z;
		bsIn.Read(x); bsIn.Read(y); bsIn.Read(z);
		for (int i = 0; i < clients.size(); i++){
			if (clients[i].ID() == _c){
				clients[i].SetReady(_ready);
				clients[i].SetTurn(_turn);
				clients[i].SetPosition(glm::vec3(x, y, z));
				if (clients[i].ID() == id){
					is_turn = clients[i].IsTurn();
				}
				new_client = false;
				break;
			}
		}
		if (new_client){
			std::cout << "new client added***\n";
			clients.push_back(Player(_c, glm::vec3(x, y, z)));
			clients[clients.size() - 1].SetReady(_ready);
		}
	}
	lobby_count = clients.size();
	update_clients = true;
}

void NetworkManager::Update(float _delta){
	packet = peer->Receive();
	switch (state){
	case MAIN_MENU:
		//NetworkManager doesn't need to do anything when at main_menu

		break;
	case NETWORK_MENU:
		if (packet){
			switch (packet->data[0]){
			case ID_CONNECTION_REQUEST_ACCEPTED:
				printf("Connection request accepted.\n");
				serverAddress = packet->systemAddress;
				break;
			case ID_INIT_MESSAGE_1:
			{
				int login_result;
				RakNet::BitStream bsIn(packet->data, packet->length, false);
				bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
				bsIn.Read(login_result);
				if (login_result == SUCCESSFUL){
					state = IN_LOBBY;
					printf("finally connected\n");
					server_message = "Connection Successful.";
				}
				else if (login_result == INVALID_PASSWORD){
					printf("invalid password.\n");
					server_message = "Invalid Password.";
				}
				else if (login_result == INVALID_ID){
					printf("invalid id.\n");
					server_message = "Invalid ID.";
				}
				else if (login_result == LOBBY_FULL){
					printf("lobby full.\n");
					server_message = "Lobby Full.";
				}
				else if (login_result == LOBBY_INGAME){
					printf("lobby is in game.\n");
					server_message = "Lobby in Game.";
				}
				else{
					printf("problem connecting\n");
					server_message = "Problem Connecting.";
				}
			}
				break;
			default:
				printf("Message with identifier %i has arrived.\n", packet->data[0]);
				break;
			}
		}
		break;
	case IN_LOBBY:
		if (packet){
			switch (packet->data[0]){
			case ID_START_GAME:
				state = IN_GAME;
				break;
			case ID_LOBBY_COUNT:
			{
				UpdateClientData();
			}
				break;
			case ID_REMOVE_CLIENT:
			{
				RakNet::RakString rs;
				RakNet::BitStream bsIn(packet->data, packet->length, false);
				bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
				bsIn.Read(rs);
				for (std::vector<Player>::iterator iter = clients.begin(); iter != clients.end(); ++iter){
					if (iter->ID() == rs.C_String()){
						clients.erase(iter);
						break;
					}
				}
				lobby_count = clients.size();
				update_clients = true;
			}
				break;
			default:
				printf("Message with identifier %i has arrived.\n", packet->data[0]);
				break;
			}
		}
		break;
	case IN_GAME:
		if (packet){
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
				connected = true;
				serverAddress = packet->systemAddress;
				break;
			case ID_NEW_INCOMING_CONNECTION:
				printf("A connection is incoming.\n");
				break;
			case ID_NO_FREE_INCOMING_CONNECTIONS:
				printf("The server is full.\n");
				break;
			case ID_DISCONNECTION_NOTIFICATION:
				printf("We have been disconnected.\n");
				connected = false;
				break;
			case ID_CONNECTION_LOST:
				printf("Connection lost.\n");
				break;
			case ID_GAME_MESSAGE_1:
			{
				RakNet::RakString rs;
				RakNet::BitStream bsIn(packet->data, packet->length, false);
				bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
				bsIn.Read(rs);
				//add new message to list of messages to be handled
				new_messages.push_back(rs.C_String());
			}
				break;
			case ID_LOBBY_COUNT:
			{
				UpdateClientData();
			}
				break;
			case ID_REMOVE_CLIENT:
			{
				RakNet::RakString rs;
				RakNet::BitStream bsIn(packet->data, packet->length, false);
				bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
				bsIn.Read(rs);
				for (std::vector<Player>::iterator iter = clients.begin(); iter != clients.end(); ++iter){
					if (iter->ID() == rs.C_String()){
						clients.erase(iter);
						break;
					}
				}
				lobby_count = clients.size();
				update_clients = true;
			}
				break;
			case ID_NEW_TILE:
			{
				//grab tile objects from server
				RakNet::BitStream bsIn(packet->data, packet->length, false);
				bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
				tiles.clear();
				while (bsIn.GetNumberOfUnreadBits() > 0){
					float x, y, z, r;
					bsIn.Read(x);
					bsIn.Read(y);
					bsIn.Read(z);
					bsIn.Read(r);
					int v;
					bsIn.Read(v);
					tiles.push_back(new Tile(glm::vec3(x, y, z), glm::vec3(0.0f, 0.0f, r), v));
				}
				update_tiles = true;
			}
				break;
			default:
				printf("Message with identifier %i has arrived.\n", packet->data[0]);
				break;
			}
		}
		break;
	}
	peer->DeallocatePacket(packet);
}

std::string NetworkManager::GetRandomLobby(){
	RakNet::BitStream bsOut;
	bsOut.Write((RakNet::MessageID)ID_RANDOM_LOBBY);
	peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, serverAddress, false);

	//bool waiting = true;
	while (true){
		packet = peer->Receive();
		if (packet){
			switch (packet->data[0]){
			case ID_RANDOM_LOBBY:
				RakNet::RakString rs;
				RakNet::BitStream bsIn(packet->data, packet->length, false);
				bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
				bsIn.Read(rs);
				peer->DeallocatePacket(packet);
				return rs.C_String();
			}
		}

		peer->DeallocatePacket(packet);
	}
}