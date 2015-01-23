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

private:
	std::string id;
	glm::vec3 position;
};

