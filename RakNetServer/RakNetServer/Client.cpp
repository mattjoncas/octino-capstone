#include "Client.h"


Client::Client(){

}
Client::Client(RakNet::SystemAddress _address, std::string _id, std::string _lobby){
	address = _address;
	id = _id;
	lobby = _lobby;
	x = 0;
	y = 0;
	z = 0;

	ready = 0;
	is_turn = 0;

	hand_count = 0;
}

Client::~Client(){

}

void Client::ReadyUp(){
	if (!ready){
		ready = true;
	}
	else{
		ready = false;
	}
}
