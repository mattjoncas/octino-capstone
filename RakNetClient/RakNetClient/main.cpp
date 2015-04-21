#include "NetworkManager.h"
#include <gl/glew.h>
#include <SFML/OpenGL.hpp>
#include "Game.h"
#include "SceneManager.h"

#define SCREEN_WIDTH 1600
#define SCREEN_HEIGHT 900
//sf::VideoMode::GetDesktopMode();

sf::ContextSettings settings(24, 8, 8, 2, 0);
sf::RenderWindow window(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "Prototype", sf::Style::Fullscreen, settings);

int main(){
	window.setVerticalSyncEnabled(true);

	mor::SceneManager sManager = mor::SceneManager::GetInstance();

	Game *game = new Game();
	game->SetWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
	game->Load();

	sManager.AddScene(game);
	sManager.BindScene(game);
	
	glClearColor(0.204f, 0.251f, 0.137f, 1.0);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	
	sf::Clock c = sf::Clock();
	while (sManager.IsRunning()){
		
		//error checking
		if (glGetError()){
			std::cout << "error\n";
		}
		dynamic_cast<Game*>(sManager.C_Scene())->Update(c.getElapsedTime().asSeconds(), &window);
		//game->Update(c.getElapsedTime().asSeconds(), &window);
		sManager.Update();

		c.restart();
		dynamic_cast<Game*>(sManager.C_Scene())->Render(&window);
		//game->Render(&window);
		
		window.display();
	}

	window.close();
	
	return 0;
}