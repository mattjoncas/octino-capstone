#pragma once
#include <string>
#include <glm\glm.hpp>

class Player{

public:
	Player();
	Player(std::string _id, glm::vec3 _position);
	~Player();

	inline std::string ID(){
		return id;
	}
	inline glm::vec3 Position(){
		return position;
	}
	void SetPosition(glm::vec3 _position);

	inline void SetReady(bool _isReady){
		ready = _isReady;
	}
	inline void SetReady(int _isReady){
		if (_isReady == 0){
			ready = false;
		}
		else{
			ready = true;
		}
	}
	inline bool IsReady(){
		return ready;
	}

	inline void SetTurn(bool _is_turn){
		is_turn = _is_turn;
	}
	inline void SetTurn(int _is_turn){
		if (_is_turn == 0){
			is_turn = false;
		}
		else{
			is_turn = true;
		}
	}
	inline bool IsTurn(){
		return is_turn;
	}
private:
	std::string id;
	glm::vec3 position;

	bool ready, is_turn;
};

