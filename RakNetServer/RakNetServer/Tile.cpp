#include "Tile.h"


Tile::Tile(){

}
Tile::Tile(float _x, float _y, float _z, float _rotation, std::string _id, int _value){
	x = _x;
	y = _y;
	z = _z;
	rotation = _rotation;
	player_id = _id;
	value = _value;
}

Tile::~Tile(){

}
