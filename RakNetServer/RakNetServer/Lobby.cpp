#include "Lobby.h"


Lobby::Lobby(){
	inGame = false;
	ready = false;
}

Lobby::~Lobby(){

}

void Lobby::Update(){
	if (!ready){
		ready = true;
		for (int i = 0; i < clients.size(); i++){
			if (!clients[i]->ready){
				ready = false;
				break;
			}
		}
	}
}
void Lobby::StartGame(){
	//init for game
	tiles.clear();

	inGame = true;
}