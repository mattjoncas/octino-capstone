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

	int score;

	std::vector<std::string> chat;

	std::vector<Player> *players;

	void LoadGUI();
public:
	~Game();
	virtual void Load() override;
	virtual void Render() override;
	virtual void Update(float _delta) override;

	void SFInput(sf::Keyboard::Key _key);
	
	glm::vec3 GetPosition();
	glm::vec3 Raycast(float mouseX, float mouseY);

	void AddTiles(std::vector<Tile*> _tiles);

	void Update(float _delta, sf::RenderWindow *_window);
	void Render(sf::RenderWindow *_window);
};

