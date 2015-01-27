#pragma once
#include <string>
#include "RakPeerInterface.h"

class Client{
public:
	Client();
	~Client();

	RakNet::SystemAddress address;
	std::string id;
	std::string lobby;

	float x, y, z;

	Client(RakNet::SystemAddress _address, std::string _id, std::string _lobby);

	bool ready;

	void ReadyUp();
};

