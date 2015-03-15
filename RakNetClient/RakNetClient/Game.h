#pragma once
#include "NetworkManager.h"
#include "WorldManager.h"
#include "GUIManager.h"

#include "Player.h"
#include "Tile.h"
#include <memory>

class Game : public mor::WorldManager{

private:
	//Game();

	//virtual void Load() override;
	virtual void Input() override;

	std::vector<std::string> chat;

	std::vector<Player> *players;

	std::vector<int> hand;
	void FillHand();

	void LoadGUI();

	void SendTiles();
	bool AddTempTile(glm::vec3 _pos, glm::vec3 _rot, int _value);
	void DeleteTempTiles();
	void AddTiles(std::vector<Tile*> _tiles);

	bool TileValidPlacement(glm::vec3 tile_pos);
	//logic test
	bool CheckTiles();
	bool TilePass(Tile *i_tile, Tile *a_tile, int previous_index, std::vector<int> _equation);
	void AdjustCamera();

	void AddChatMessage(std::string _message, bool incoming_message);

	void GeneratePuzzle();
public:
	~Game();
	virtual void Load() override;
	virtual void Render() override;
	virtual void Update(float _delta) override;

	void SFInput(sf::Keyboard::Key _key);
	
	glm::vec3 GetPosition();
	glm::vec3 Raycast(float mouseX, float mouseY);

	void Update(float _delta, sf::RenderWindow *_window);
	void Render(sf::RenderWindow *_window);
};

