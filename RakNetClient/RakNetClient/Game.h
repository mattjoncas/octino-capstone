#pragma once
#include "NetworkManager.h"
#include "Scene.h"
#include "GUIManager.h"

#include "Player.h"
#include "Tile.h"
#include <memory>

#include <SFML/Audio.hpp>

class Game : public mor::Scene{

private:
	gui::GUIManager gManager = gui::GUIManager::GetInstance(true);
	NetworkManager nManager = NetworkManager();

	//virtual void Load() override;
	virtual void Input() override;

	std::vector<std::string> chat;
	std::vector<Player> *players;

	std::vector<int> hand;
	void FillHand(int hand_size);
	void EmptyHand();
	void PickUpTile();

	void LoadGUI();

	void SendTiles();
	void UpdateTileCursor(int mouse_x, int mouse_y);
	bool AddTempTile(glm::vec3 _pos, glm::vec3 _rot, int _value);
	void DeleteTempTiles();
	void AddTiles(std::vector<Tile*> _tiles);

	bool TileValidPlacement(glm::vec3 tile_pos);
	
	bool CheckTiles();
	bool TilePass(Tile *i_tile, Tile *a_tile, int previous_index, std::vector<int> _equation, std::vector<Tile*> _tiles);
	void AdjustCamera();
	void RemoveTile();
	void ClearTiles();

	void AddChatMessage(std::string _message, bool incoming_message);
	void ClearChat();

	void GeneratePuzzle();
	
	void BindMainMenu();
public:
	~Game();
	virtual void Load() override;
	virtual void Render() override;
	virtual void Update(float _delta) override;
	
	glm::vec3 GetPosition();
	glm::vec3 Raycast(float mouseX, float mouseY);

	void Update(float _delta, sf::RenderWindow *_window);
	void Render(sf::RenderWindow *_window);

	void LoadAudio();
};

