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
	peer->Ping("255.255.255.255", SERVER_PORT, 0);
	
	server_message = "";

	state = IN_MENU;
}

NetworkManager::~NetworkManager(){

	RakNet::RakPeerInterface::DestroyInstance(peer);
}
bool NetworkManager::IsConnected(){
	return connected;
}

std::string NetworkManager::Connect(std::string _id, std::string _lobby){
	if (_id != "" && _lobby != ""){
		id = _id;
		lobby = _lobby;

		char str[512];
		
		strcpy(str, peer->GetLocalIP(0));
		std::cout << peer->GetLocalIP(0) << "\n";
		
		peer->Connect(str, SERVER_PORT, 0, 0);

		return "Connecting...";
	}
	else{
		return "Please enter id/lobby.";
	}
}
void NetworkManager::Disconnect(){
	peer->CloseConnection(serverAddress, true, 0, HIGH_PRIORITY);
}
void NetworkManager::UpdateServer(glm::vec3 _pos){
	RakNet::BitStream bsOut;
	bsOut.Write((RakNet::MessageID)ID_CLIENT_POSITION);
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
void NetworkManager::SendNewTile(glm::vec3 _pos, float _rotation){
	RakNet::BitStream bsOut;
	bsOut.Write((RakNet::MessageID)ID_NEW_TILE);
	bsOut.Write(_pos.x);
	bsOut.Write(_pos.y);
	bsOut.Write(_pos.z);
	bsOut.Write(_rotation);
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
	return update_clients;
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

void NetworkManager::Update(float _delta){
	packet = peer->Receive();
	switch (state){
	case IN_MENU:
		if (packet){
			switch (packet->data[0]){
			case ID_CONNECTION_REQUEST_ACCEPTED:
				printf("Connection request accepted, attempting to connect.\n");
				serverAddress = packet->systemAddress;
				{
					RakNet::BitStream bsOut;
					bsOut.Write((RakNet::MessageID)ID_INIT_MESSAGE_1);
					bsOut.Write(id.c_str());
					bsOut.Write(lobby.c_str());
					peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, serverAddress, false);
				}
				break;
			case ID_INIT_MESSAGE_1:
			{
				RakNet::RakString rs;
				RakNet::BitStream bsIn(packet->data, packet->length, false);
				bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
				bsIn.Read(rs);
				if (strcmp(rs.C_String(), "success") == 0){
					state = IN_LOBBY;
					delay = 1.0f;
					printf("finally connected\n");
					server_message = "Connection Successful.";
				}
				else if (strcmp(rs.C_String(), "id invalid") == 0){
					printf("invalid id.\n");
					server_message = "Invalid ID.";
					Disconnect();
				}
				else{
					printf("problem connecting\n");
					server_message = "Problem Connecting.";
					Disconnect();
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
		delay -= _delta;
		if (delay < 0){
			connected = true;
			state = IN_GAME;
			delay = 1.0f;
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
				RakNet::RakString rs;
				RakNet::BitStream bsIn(packet->data, packet->length, false);
				bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
				while (bsIn.GetNumberOfUnreadBits() > 0){
					bool new_client = true;
					bsIn.Read(rs);
					std::string _c = rs.C_String();
					for (int i = 0; i < clients.size(); i++){
						if (clients[i].ID() == _c){
							float x, y, z;
							bsIn.Read(x); bsIn.Read(y); bsIn.Read(z);
							clients[i].SetPosition(glm::vec3(x, y, z));
							new_client = false;
							break;
						}
					}
					if (new_client){
						float x, y, z;
						bsIn.Read(x); bsIn.Read(y); bsIn.Read(z);
						clients.push_back(Player(_c, glm::vec3(x, y, z)));
					}
				}
				lobby_count = clients.size();
				update_clients = true;
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
					tiles.push_back(new Tile(glm::vec3(x, y, z), glm::vec3(0.0f, 0.0f, r)));
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