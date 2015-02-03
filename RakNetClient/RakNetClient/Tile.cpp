#include "Tile.h"


Tile::Tile(){

}
Tile::Tile(glm::vec3 _position, glm::vec3 _rotation){
	Init(_position, _rotation, glm::vec3(1.0f));

	adjacent_tiles[0] = Adjacent(_position + glm::vec3(0.0f, 2.0f, 0.0f));
	adjacent_tiles[1] = Adjacent(_position + glm::vec3(1.5f, 1.5f, 0.0f));
	adjacent_tiles[2] = Adjacent(_position + glm::vec3(2.0f, 0.0f, 0.0f));
	adjacent_tiles[3] = Adjacent(_position + glm::vec3(1.5f, -1.5f, 0.0f));
	adjacent_tiles[4] = Adjacent(_position + glm::vec3(0.0f, -2.0f, 0.0f));
	adjacent_tiles[5] = Adjacent(_position + glm::vec3(-1.5f, -1.5f, 0.0f));
	adjacent_tiles[6] = Adjacent(_position + glm::vec3(-2.0f, 0.0f, 0.0f));
	adjacent_tiles[7] = Adjacent(_position + glm::vec3(-1.5f, 1.5f, 0.0f));
}

Tile::~Tile(){

}

void Tile::Update(float _delta){
	if (position.z <= -49.0f && acceleration != glm::vec3(0.0f)){
		position.z == -49.0f;
		acceleration = glm::vec3(0.0f);
		velocity = glm::vec3(0.0f);
	}
	
	GameObject::Update(_delta);
}
void Tile::Drop(){
	position.z += 25.0f;
	acceleration.z = -9.8f;
	velocity.z = -25.0f;
}