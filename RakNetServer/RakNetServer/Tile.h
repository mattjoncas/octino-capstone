#pragma once
#include <string>

class Tile{
public:
	Tile();
	~Tile();

	std::string player_id;
	int value;

	float x, y, z, rotation;

	Tile(float _x, float _y, float _z, float _rotation, std::string _id, int _value);
};

