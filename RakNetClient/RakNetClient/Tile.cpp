#include "Tile.h"


Tile::Tile(){

}
Tile::Tile(glm::vec3 _position, glm::vec3 _rotation, int _value){
	Init(_position, _rotation, glm::vec3(1.0f));

	value = _value;

	for (int i = 0; i < 8; i++){
		adjacent_tiles[i] = nullptr;
	}
	/*
	for (int i = 0; i < 4; i++){
		valid_sum[i] = nullptr;
	}*/

	for (int i = 0; i < 4; i++){
		solved[i] = false;
	}
}

Tile::~Tile(){
	
}

void Tile::Update(float _delta){
	if (position.z <= -49.5f && acceleration != glm::vec3(0.0f)){
		position.z = -49.5f;
		acceleration = glm::vec3(0.0f);
		velocity = glm::vec3(0.0f);
		
		GetChild(0)->acceleration.z = -9.8f;
		GetChild(0)->velocity.z = 7.5f;
		number_bounced = true;
	}
	GameObject::Update(_delta);
	if (number_bounced){
		if (GetChild(0)->position.z < 0.1f){
			GetChild(0)->position = glm::vec3(0.0f, 0.0f, 0.1f);
			GetChild(0)->velocity = glm::vec3(0.0f);
			GetChild(0)->acceleration = glm::vec3(0.0f);
			number_bounced = false;
		}
	}
}
void Tile::SetTileRotation(glm::vec3 _rotation){
	//set tile gameobject rotation

	//adjust child[0](num model) to offset the rotation, and stay up proper
}
void Tile::Drop(){
	position.z += 25.0f;
	acceleration.z = -9.8f;
	velocity.z = -25.0f;
}
bool Tile::AddAdjacent(Tile *_tile){
	for (int a = 0; a < 8; a++){
		if (glm::vec2(GetAdjacentPosition(a)) == glm::vec2(_tile->position) && !adjacent_tiles[a]){
			adjacent_tiles[a] = _tile;
			
			//update new adjacent tile
			int ta = a + 4;
			if (ta > 7){
				ta -= 8;
			}
			/*
			if (a == 0) { valid_sum[0] = new bool; _tile->valid_sum[2] = valid_sum[0]; }
			else if (a == 2) { valid_sum[1] = new bool; _tile->valid_sum[3] = valid_sum[1]; }
			else if (a == 4) { valid_sum[2] = new bool; _tile->valid_sum[4] = valid_sum[2]; }
			else if (a == 6) { valid_sum[3] = new bool; _tile->valid_sum[1] = valid_sum[3]; }*/

			_tile->adjacent_tiles[ta] = this;
			
			return true;
		}
	}
	return false;
}
bool Tile::CheckAdjacent(int slot_index){
	if (slot_index >= 0 && slot_index <= 8){
		int tu = slot_index + 1, td = slot_index - 1;
		if (tu > 7){ tu = 0; };
		if (td < 0){ td = 7; };

		if (adjacent_tiles[slot_index] || adjacent_tiles[tu] || adjacent_tiles[td]){
			return true;
		}
	}
	return false;
}
glm::vec3 Tile::GetAdjacentPosition(int slot_index){
	switch (slot_index){
	case 0:
		return position + glm::vec3(0.0f, 2.0f, 0.0f);
	case 1:
		return position + glm::vec3(1.5f, 1.5f, 0.0f);
	case 2:
		return position + glm::vec3(2.0f, 0.0f, 0.0f);
	case 3:
		return position + glm::vec3(1.5f, -1.5f, 0.0f);
	case 4:
		return position + glm::vec3(0.0f, -2.0f, 0.0f);
	case 5:
		return position + glm::vec3(-1.5f, -1.5f, 0.0f);
	case 6:
		return position + glm::vec3(-2.0f, 0.0f, 0.0f);
	case 7:
		return position + glm::vec3(-1.5f, 1.5f, 0.0f);
	}
}
Tile* Tile::GetAdjacentTile(int _index){
	return adjacent_tiles[_index];
}

int Tile::GetAdjacentIndex(int _operation){
	if (_operation == 0){
		if (glm::degrees(rotation.z) == 0.0f){ return 7; }
		else if (glm::degrees(rotation.z) == 90.0f){ return 5; }
		else if (glm::degrees(rotation.z) == 180.0f){ return 3; }
		else { return 1; }
	}
	else if (_operation == 1){
		if (glm::degrees(rotation.z) == 0.0f){ return 1; }
		else if (glm::degrees(rotation.z) == 90.0f){ return 7; }
		else if (glm::degrees(rotation.z) == 180.0f){ return 5; }
		else { return 3; }
	}
	else if (_operation == 2){
		if (glm::degrees(rotation.z) == 0.0f){ return 5; }
		else if (glm::degrees(rotation.z) == 90.0f){ return 3; }
		else if (glm::degrees(rotation.z) == 180.0f){ return 1; }
		else { return 7; }
	}
	else{
		if (glm::degrees(rotation.z) == 0.0f){ return 3; }
		else if (glm::degrees(rotation.z) == 90.0f){ return 1; }
		else if (glm::degrees(rotation.z) == 180.0f){ return 7; }
		else { return 5; }
	}
}

void Tile::RemoveAdjacentTiles(){
	for (int a = 0; a < 8; a++){
		if (adjacent_tiles[a]){
			int ta = a + 4;
			if (ta > 7){
				ta -= 8;
			}
			adjacent_tiles[a]->adjacent_tiles[ta] = nullptr;
			adjacent_tiles[a] = nullptr;
		}
	}
}