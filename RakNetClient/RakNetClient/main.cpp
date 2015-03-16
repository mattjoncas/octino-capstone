#include "NetworkManager.h"
#include <gl/glew.h>
#include <SFML/OpenGL.hpp>
#include "Game.h"

#define SCREEN_WIDTH 1500
#define SCREEN_HEIGHT 800

sf::ContextSettings settings(24, 8, 8, 2, 0);
sf::RenderWindow window(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "Prototype", sf::Style::Close, settings);

bool running = true;

int main(){
	window.setVerticalSyncEnabled(true);

	Game game;
	game.SetWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
	game.Load();

	//glClearColor(0.883f, 0.734f, 0.48f, 1.0);
	//glClearColor(0.05f, 0.05f, 0.05f, 1.0);
	//glClearColor(0.1f, 0.1f, 0.1f, 1.0);
	glClearColor(0.204f, 0.251f, 0.137f, 1.0);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	
	sf::Clock c = sf::Clock();
	while (running){
		
		//error checking
		if (glGetError()){
			std::cout << "error\n";
		}
		game.Update(c.getElapsedTime().asSeconds(), &window);
		
		running = game.IsRunning();

		c.restart();
		//render
		game.Render(&window);
		
		window.display();
	}

	window.close();
	
	return 0;
}