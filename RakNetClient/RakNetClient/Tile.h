#pragma once
#include "GameObject.h"

class Tile : public mor::GameObject{

	struct Adjacent{
		Adjacent(){}
		Adjacent(glm::vec3 _pos){
			pos = _pos;
			filled = false;
		}
		glm::vec3 pos;
		bool filled;
	};

public:
	Tile();
	Tile(glm::vec3 _position, glm::vec3 _rotation);
	~Tile();

	void Update(float _delta) override;

	void Drop();

	Adjacent adjacent_tiles[8];
private:
	//bool placed;
};

