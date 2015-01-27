#include "Game.h"

mor::GameObject *cube;
mor::Camera *camera;
mor::Light *white_light;
std::vector<mor::GameObject*> tiles;

float t = 0.0f;
//materials
int blue, soft_green, gold, orange, chrome;
int tile_model;

bool inGame = false, inLobby = false, isReady = false;

//gui && networking 
gui::GUIManager gManager = gui::GUIManager();
int game_hud, menu, lobby;

NetworkManager nManager = NetworkManager();
glm::vec3 server_pos;

Game::~Game(){
	for (std::vector<mor::GameObject*>::iterator iter = objects.begin(); iter != objects.end(); ++iter){
		delete((*iter));
	}
	for (std::vector<mor::GameObject*>::iterator iter = tiles.begin(); iter != tiles.end(); ++iter){
		delete((*iter));
	}

	delete(camera);

	//delete(players);
}

void Game::Load(){
	srand(time(0));

	camera = new mor::Camera(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f), SCREEN_WIDTH, SCREEN_HEIGHT, false);

	white_light = new mor::Light(glm::vec4(0.0, 0.0, -20.0, 0.0), glm::vec4(0.05, 0.05, 0.05, 1.0), glm::vec4(1.0, 1.0, 1.0, 1.0), glm::vec4(1.0, 1.0, 1.0, 1.0), 200.0f, 1.0f);

	renderer.SetCamera(camera);
	renderer.AddLight(white_light);

	blue = renderer.LoadMaterial(glm::vec4(1.0, 1.0, 1.0, 1.0), glm::vec4(0.0, 0.83, 1.0, 1.0), glm::vec4(0.5, 0.5, 0.5, 1.0), 25.0f); //BLUE
	soft_green = renderer.LoadMaterial(glm::vec4(1.0, 1.0, 1.0, 1.0), glm::vec4(0.387, 0.528, 0.086, 1.0), glm::vec4(0.0, 0.0, 0.0, 1.0), 25.0f); //SOFT GREEN [no spec]
	gold = renderer.LoadMaterial(glm::vec4(1.0, 1.0, 1.0, 1.0), glm::vec4(0.31, 0.216, 0.095, 1.0), glm::vec4(1.0, 0.726, 0.181, 1.0), 1.0f); //GOLD
	orange = renderer.LoadMaterial(glm::vec4(1.0, 1.0, 1.0, 1.0), glm::vec4(1.0, 0.368, 0.029, 1.0), glm::vec4(0.638, 1.0, 0.2, 1.0), 50.0f);
	chrome = renderer.LoadMaterial(glm::vec4(1.0, 1.0, 1.0, 1.0), glm::vec4(0.05, 0.05, 0.05, 1.0), glm::vec4(0.5, 0.0, 0.0, 1.0), 1.0f);

	cube = new mor::GameObject();
	cube->model = renderer.LoadModel("cube");
	cube->Init(glm::vec3(0.0f, 0.0f, -50.0f), glm::vec3(glm::radians(0.0f), 0.0f, 0.0f), glm::vec3(70.0f, 50.0f, 1.0f));
	cube->material = soft_green;
	cube->bounding_shape = new AABox(cube->position, cube->scale.x, cube->scale.y, cube->scale.z);

	objects.push_back(cube);

	tile_model = renderer.LoadModel("tile");

	LoadGUI();

	players = nManager.GetClients();
}
void Game::LoadGUI(){
	menu = gManager.AddMenu();

	gManager.AddText(menu, "id_text", 400, 0, "Enter your ID", sf::Color::White, "TF2.ttf", 50);
	gManager.SetCentred("id_text", true);
	gManager.AddTextBox(menu, "id_text_box", 420, 80, 400, 70, "", sf::Color(9, 43, 106, 255), "TF2.ttf");
	gManager.SetCentred("id_text_box", true);
	gManager.AddText(menu, "lobby_text", 400, 150, "Enter Lobby to Create/Join", sf::Color::White, "TF2.ttf", 50);
	gManager.SetCentred("lobby_text", true);
	gManager.AddTextBox(menu, "lobby_text_box", 420, 80, 400, 230, "", sf::Color(9, 43, 106, 255), "TF2.ttf");
	gManager.SetCentred("lobby_text_box", true);
	gManager.AddButton(menu, "connect_button", 500, 75, 400, 500, "Connect", sf::Color(32, 69, 137, 255), sf::Color(9, 43, 106, 255), sf::Color(2, 35, 95, 255), "TF2.ttf");
	gManager.SetCentred("connect_button", true);

	gManager.AddText(menu, "menu_message", 400, 375, " ", sf::Color(202, 33, 33, 255), "TF2.ttf", 40);
	gManager.SetCentred("menu_message", true);

	lobby = gManager.AddMenu();

	gManager.AddButton(lobby, "ready_button", 500, 75, 400, 500, "Not Ready", sf::Color(32, 69, 137, 255), sf::Color(9, 43, 106, 255), sf::Color(2, 35, 95, 255), "TF2.ttf");
	gManager.SetCentred("ready_button", true);
	gManager.AddText(lobby, "client01", 400, (0 * 40) + 10, "", sf::Color::White, "TF2.ttf", 50);
	gManager.SetCentred("client01", true);
	gManager.AddText(lobby, "client02", 400, (1 * 40) + 10, "", sf::Color::White, "TF2.ttf", 50);
	gManager.SetCentred("client02", true);
	gManager.AddText(lobby, "client03", 400, (2 * 40) + 10, "", sf::Color::White, "TF2.ttf", 50);
	gManager.SetCentred("client03", true);
	gManager.AddText(lobby, "client04", 400, (3 * 40) + 10, "", sf::Color::White, "TF2.ttf", 50);
	gManager.SetCentred("client04", true);

	game_hud = gManager.AddMenu();

	gManager.AddText(game_hud, "lobby_count", 400, 0, "0", sf::Color::Green, "TF2.ttf", 50);
	gManager.SetCentred("lobby_count", true);
	gManager.AddText(game_hud, "client01", 5, (0 * 40) + 395, "", sf::Color::White, "TF2.ttf", 50);
	gManager.AddText(game_hud, "client02", 5, (1 * 40) + 395, "", sf::Color::White, "TF2.ttf", 50);
	gManager.AddText(game_hud, "client03", 5, (2 * 40) + 395, "", sf::Color::White, "TF2.ttf", 50);
	gManager.AddText(game_hud, "client04", 5, (3 * 40) + 395, "", sf::Color::White, "TF2.ttf", 50);

	gManager.AddTextBox(game_hud, "text_box", 300, 30, 0, 600 - 30, "type here.", sf::Color(9, 43, 106, 255), "TF2.ttf");
	gManager.BindMenu(menu);
}

void Game::Render(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	renderer.ShadowMapPass(tiles);
	renderer.Render(objects);

	renderer.Render(tiles);
}
void Game::Render(sf::RenderWindow *_window){
	Render();

	renderer.BindShader(-1);

	gManager.Render(_window);
}

void Game::Update(float _delta){
	if (inGame){
		renderer.Update(_delta);

		for (int i = 0; i < objects.size(); i++){
			objects[i]->Update(_delta);
		}

		white_light->SetPosition(glm::vec4(camera->pos, 1.0f));
	}
}
void Game::Update(float _delta, sf::RenderWindow *_window){
	Update(_delta);
	nManager.Update(_delta);
	std::string _message = nManager.GetServerMessage();
	if (_message != ""){
		gManager.SetText("menu_message", _message);
	}
	gManager.Update(_window, _delta);
	sf::Event event;
	switch (nManager.state){
	case 0:
		while (_window->pollEvent(event)){
			switch (event.type){
			case sf::Event::Closed:
				//try to disconnect
				nManager.Disconnect();
				isRunning = false;
				break;
			case sf::Event::TextEntered:
				if (event.text.unicode < 128 && event.text.unicode != 8 && event.text.unicode != 13){
					gManager.TextBoxInput(static_cast<char>(event.text.unicode));
				}
				else if (event.text.unicode == 8){
					gManager.TextBoxInput(NULL);
				}
				break;
			case sf::Event::MouseButtonReleased:
				if (gManager.ButtonClicked("connect_button")){
					//get message from nManager
					gManager.SetText("menu_message", nManager.Connect(gManager.GetText("id_text_box"), gManager.GetText("lobby_text_box")));
				}
				break;
			case sf::Event::KeyPressed:
				if (event.key.code == sf::Keyboard::Escape){
					nManager.Disconnect();
					isRunning = false;
				}
				if (event.key.code == sf::Keyboard::Return){
					gManager.SetText("menu_message",
						nManager.Connect(gManager.GetText("id_text_box"), gManager.GetText("lobby_text_box"))
						);
				}
				break;
			}
		}
		break;
	case 1:
		if (!inLobby){
			_window->setTitle(gManager.GetText("id_text_box") + " :: " + gManager.GetText("lobby_text_box"));
			gManager.BindMenu(lobby);
			inLobby = true;
		}
		while (_window->pollEvent(event)){
			switch (event.type){
			case sf::Event::Closed:
				//try to disconnect
				nManager.Disconnect();
				isRunning = false;
				break;
			case sf::Event::MouseButtonReleased:
				if (gManager.ButtonClicked("ready_button")){
					nManager.ReadyUp();
					if (!isReady){ 
						isReady = true;
						gManager.SetText("ready_button", "Ready");
					}
					else{
						isReady = false;
						gManager.SetText("ready_button", "Not Ready");
					}
				}
				break;
			case sf::Event::KeyPressed:
				if (event.key.code == sf::Keyboard::Escape){
					nManager.Disconnect();
					isRunning = false;
				}
				break;
			}
		}
		if (nManager.UpdateClients()){
			for (int i = 0; i < 4; i++){
				if (i < players->size()){
					gManager.SetText("client0" + std::to_string(i + 1), players->at(i).ID());
				}
				else{
					gManager.SetText("client0" + std::to_string(i + 1), "*empty*");
				}
			}
		}
		break;
	case 2:
		if (!inGame){
			inGame = true;
			gManager.BindMenu(game_hud);
		}
		while (_window->pollEvent(event)){
			switch (event.type){
			case sf::Event::Closed:
				//try to disconnect
				nManager.Disconnect();
				isRunning = false;
				break;
			case sf::Event::TextEntered:
				//if key pressed & key isn't BACKSPACE || ENTER
				if (event.text.unicode < 128 && event.text.unicode != 8 && event.text.unicode != 13){
					//text += static_cast<char>(event.text.unicode);
					gManager.TextBoxInput(static_cast<char>(event.text.unicode));
				}
				else if (event.text.unicode == 8){
					//text = text.substr(0, text.size() - 1);
					gManager.TextBoxInput(NULL);
				}
				break;
			case sf::Event::MouseButtonPressed:
				if (event.mouseButton.button == sf::Mouse::Left && !gManager.CursorOverGUI()){
					//spawn tile to server
					glm::vec3 h = Raycast(event.mouseButton.x, event.mouseButton.y);
					if (h != glm::vec3(-1, -1, -1)){
						h = glm::floor(h * (1.0f / 0.5f) + 0.5f) / (1.0f / 0.5f);
						nManager.SendNewTile(h, glm::radians(0.0f));
					}
				}
				break;
			case sf::Event::KeyPressed:
				SFInput(event.key.code);
				if (event.key.code == sf::Keyboard::Escape){
					nManager.Disconnect();
					isRunning = false;
				}
				if (event.key.code == sf::Keyboard::Return){
					if (gManager.GetText("text_box") != ""){
						nManager.SendChatMessage(gManager.GetText("text_box"));
						chat.push_back(gManager.GetText("text_box"));
						gManager.AddText(game_hud, "chat" + std::to_string(chat.size() - 1), 2, 20 * (chat.size() - 1), gManager.GetText("text_box"), sf::Color(202, 33, 33, 255), "TF2.ttf", 20);

						gManager.SetText("text_box", "");
						gManager.Unselect();
					}
				}
				break;
			}
		}
		std::string new_message = nManager.GetPendingChatMessage();
		if (new_message.size() > 0){
			chat.push_back(new_message);
			gManager.AddText(game_hud, "chat" + std::to_string(chat.size() - 1), 2, 20 * (chat.size() - 1), new_message, sf::Color(9, 43, 106, 255), "TF2.ttf", 20);
		}
		if (nManager.UpdateClients()){
			for (int i = 0; i < 4; i++){
				if (i < players->size()){
					gManager.SetText("client0" + std::to_string(i + 1), players->at(i).ID());
				}
				else{
					gManager.SetText("client0" + std::to_string(i + 1), "*empty*");
				}
			}
			gManager.SetText("lobby_count", std::to_string(nManager.GetLobbyCount()));
		}
		//update tiles if needed
		if (nManager.UpdateTiles()){
			AddTiles(nManager.GetTiles());
		}
		//update server when pos changes
		if (server_pos != GetPosition()){
			nManager.UpdateServer(GetPosition());
			server_pos = GetPosition();
		}
		break;
	}
}

void Game::Input(){
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::U)){
		renderer.SetDebug(true);
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::I)){
		renderer.SetDebug(false);
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)){
		camera->pos.y += 0.1f;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)){
		camera->pos.y -= 0.1f;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)){
		camera->pos.x -= 0.1f;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)){
		camera->pos.x += 0.1f;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)){
		camera->pos.z -= 0.1f;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)){
		camera->pos.z += 0.1f;
	}
}
void Game::SFInput(sf::Keyboard::Key _key){
	if (_key == sf::Keyboard::U){
		renderer.SetDebug(true);
	}
	if (_key == sf::Keyboard::I){
		renderer.SetDebug(false);
	}
	if (_key == sf::Keyboard::W){
		camera->pos.y += 0.2f;
	}
	if (_key == sf::Keyboard::S){
		camera->pos.y -= 0.2f;
	}
	if (_key == sf::Keyboard::A){
		camera->pos.x -= 0.2f;
	}
	if (_key == sf::Keyboard::D){
		camera->pos.x += 0.2f;
	}
	if (_key == sf::Keyboard::Up){
		camera->pos.z -= 0.2f;
	}
	if (_key == sf::Keyboard::Down){
		camera->pos.z += 0.2f;
	}
}

glm::vec3 Game::GetPosition(){
	return camera->pos;
}

glm::vec3 Game::Raycast(float mouseX, float mouseY){
	glm::vec2 m = glm::vec2(mouseX / SCREEN_WIDTH, mouseY / SCREEN_HEIGHT);

	glm::vec3 mts = camera->pos;
	//this contains camera props the are currently private [need to fix this vv]
	float hNear = 2 * tan(glm::radians(45.0f) / 2) * 0.1f;
	float wNear = hNear * SCREEN_WIDTH / SCREEN_HEIGHT;

	mts.x -= wNear / 2;
	mts.y += hNear / 2;
	mts.x += wNear * m.x;
	mts.y -= hNear * m.y;
	mts.z -= 0.1f;

	Ray r = Ray(camera->pos, glm::normalize(mts - camera->pos), 0.0f);
	r.Intersects(Plane(glm::vec3(0.0f, 0.0f, 1.0f), -49.0f));

	if (tiles.size() > 0){
		float dist = 100.0f;
		glm::vec3 final = glm::vec3(-1);
		glm::vec3 p;
		int final_tile, final_adjacent;
		for (int i = 0; i < tiles.size(); i++){
			Tile *t = dynamic_cast<Tile*>(tiles[i]);
			for (int a = 0; a < 8; a++){
				if (!t->adjacent_tiles[a].filled){
					p = t->adjacent_tiles[a].pos;
					if (glm::distance(r.Hit(), p) < dist && glm::distance(r.Hit(), p) < 1.0f){
						dist = glm::distance(r.Hit(), p);
						final = p;
						final_tile = i; final_adjacent = a;
					}
				}
			}
		}
		return final;
	}
	return r.Hit();
}

void Game::AddTiles(std::vector<Tile*> _tiles){
	for (int i = 0; i < _tiles.size(); i++){
		tiles.push_back(_tiles[i]);
		tiles[tiles.size() - 1]->model = tile_model;
		tiles[tiles.size() - 1]->bounding_shape = new AABox(tiles[tiles.size() - 1]->position, 2.0f, 2.0f, 0.5f);
		if (tiles.size() > 1){
			Tile *t = dynamic_cast<Tile*>(tiles[tiles.size() - 1]);
			//NOT WORKING **VVV**
			for (int i = 0; i < tiles.size() - 1; i++){
				printf("x: %f, y: %f, z: %f\n", tiles[i]->position.x, tiles[i]->position.y, tiles[i]->position.z);
				printf("\n");
				for (int a = 0; a < 8; a++){
					printf("x: %f, y: %f, z: %f\n", t->adjacent_tiles[a].pos.x, t->adjacent_tiles[a].pos.y, t->adjacent_tiles[a].pos.z);
					if (t->adjacent_tiles[a].pos == tiles[i]->position){
						t->adjacent_tiles[a].filled = true;

						int u = a + 1, d = a - 1;
						if (u > 7){ u = 0; };
						if (d < 0){ d = 7; };

						t->adjacent_tiles[u].filled = true;
						t->adjacent_tiles[d].filled = true;
						
						//update surrounding tiles
						Tile *_tile = dynamic_cast<Tile*>(tiles[i]);
						int ta = a + 4;
						if (ta > 7){
							ta -= 8;
						}
						_tile->adjacent_tiles[ta].filled = true;

						int tu = ta + 1, td = ta - 1;
						if (tu > 7){ tu = 0; };
						if (td < 0){ td = 7; };

						_tile->adjacent_tiles[tu].filled = true;
						_tile->adjacent_tiles[td].filled = true;
					}
				}
			}
			printf("***\n");
			for (int a = 0; a < 8; a++){
				if (t->adjacent_tiles[a].filled){
					printf("filled\n");
				}
				else{
					printf("not filled\n");
				}
			}
		}
	}
}