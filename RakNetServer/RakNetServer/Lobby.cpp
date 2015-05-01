#include "Lobby.h"


Lobby::Lobby(){
	inGame = false;
	ready = false;
	finished = false;
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
	if (inGame){
		for (int i = 0; i < clients.size(); i++){
			if (clients[i]->hand_count == 0){
				finished = true;
			}
		}
	}
}
void Lobby::StartGame(){
	//init for game
	tiles.clear();

	inGame = true;

	clients[0]->is_turn = 1;
}
void Lobby::Reset(){
	inGame = false;
	ready = false;

	for (int i = 0; i < clients.size(); i++){
		clients[i]->is_turn = 0;
		clients[i]->ready = 0;
	}
	finished = false;
}

void Lobby::IncrementTurn(){
	for (int i = 0; i < clients.size(); i++){
		if (clients[i]->is_turn){
			clients[i]->is_turn = false;
			if (i < clients.size() - 1){
				clients[i + 1]->is_turn = true;
			}
			else{
				clients[0]->is_turn = true;
			}
			break;
		}
	}
}

void Lobby::AddTile(Tile _new_tile){
	tiles.push_back(_new_tile);
}

bool Lobby::IsFull(){
	if (clients.size() >= 4){
		return true;
	}
	return false;
}