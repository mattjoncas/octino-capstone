#include "Player.h"


Player::Player(){

}
Player::Player(std::string _id, glm::vec3 _position){
	id = _id;
	position = _position;
}

Player::~Player(){

}

void Player::SetPosition(glm::vec3 _position){
	position = _position;
}