#pragma once
#include <iostream>
#include <string>
#include <vector>

#include "Client.h"
#include "Tile.h"

class Lobby{
public:
	Lobby();
	~Lobby();

	std::string name;
	std::vector<Client*> clients;

	std::vector<Tile> tiles;

	bool inGame, ready, finished;
	bool IsFull();

	void Update();
	void StartGame();
	void Reset();

	void IncrementTurn();

	void AddTile(Tile _new_tile);
};

