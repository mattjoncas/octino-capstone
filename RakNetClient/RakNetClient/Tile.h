#pragma once
#include "GameObject.h"

class Tile : public mor::GameObject{

public:
	Tile();
	Tile(glm::vec3 _position, glm::vec3 _rotation, int _value);
	~Tile();

	void Update(float _delta) override;

	void Drop();
	//adds the given tile if adjacent
	bool AddAdjacent(Tile *_tile);
	//check if a adjacent slot is filled
	bool CheckAdjacent(int slot_index);
	glm::vec3 GetAdjacentPosition(int slot_index);
	Tile* GetAdjacentTile(int _index);
	//get slot index based on requested operation [0 = +, 1 = -, 2 = *, 3 = /]
	int GetAdjacentIndex(int _operation);

	void SetTileRotation(glm::vec3 _rotation);

	inline int GetValue(){
		return value;
	}

	void RemoveAdjacentTiles();
private:
	int value;
	Tile *adjacent_tiles[8];
	//bool placed;
};

