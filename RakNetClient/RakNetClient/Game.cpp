#include "Game.h"

mor::GameObject *cube;
mor::Camera *camera;
mor::Light *white_light;
std::vector<mor::GameObject*> tiles;
mor::GameObject *tile_cursor;
mor::GameObject *menu_tile, *tile_num;

float t = 0.0f;
float tile_rotation = 0.0f;
int selected_tile, tiles_placed;
//materials
int blue, soft_green, gold, orange, chrome;
int tile_model, texture_shader;

bool inGame = false, inLobby = false, isReady = false;

//gui && networking 
gui::GUIManager gManager = gui::GUIManager();
int game_hud, main_menu, network_menu, lobby;

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
	delete(tile_cursor);
	delete(menu_tile);
}

void Game::Load(){
	srand(time(NULL));

	camera = new mor::Camera(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f), renderer.ScreenWidth(), renderer.ScreenHeight(), false);
	white_light = new mor::Light(glm::vec4(0.0, 0.0, 20.0, 0.0), glm::vec4(0.05, 0.05, 0.05, 1.0), glm::vec4(1.0, 1.0, 1.0, 1.0), glm::vec4(1.0, 1.0, 1.0, 1.0), 300.0f, 1.0f);
	white_light->SetPosition(glm::vec4(00.0f, 0.0f, 40.0f, 0.0f));

	renderer.SetCamera(camera);
	renderer.AddLight(white_light);

	blue = renderer.LoadMaterial(glm::vec4(1.0, 1.0, 1.0, 1.0), glm::vec4(0.0, 0.83, 1.0, 1.0), glm::vec4(0.5, 0.5, 0.5, 1.0), 25.0f); //BLUE
	soft_green = renderer.LoadMaterial(glm::vec4(1.0, 1.0, 1.0, 1.0), glm::vec4(0.387, 0.528, 0.086, 1.0), glm::vec4(0.0, 0.0, 0.0, 1.0), 25.0f); //SOFT GREEN [no spec]
	gold = renderer.LoadMaterial(glm::vec4(1.0, 1.0, 1.0, 1.0), glm::vec4(0.31, 0.216, 0.095, 1.0), glm::vec4(1.0, 0.726, 0.181, 1.0), 1.0f); //GOLD
	orange = renderer.LoadMaterial(glm::vec4(1.0, 1.0, 1.0, 1.0), glm::vec4(1.0, 0.368, 0.029, 1.0), glm::vec4(0.638, 1.0, 0.2, 1.0), 50.0f);
	chrome = renderer.LoadMaterial(glm::vec4(1.0, 1.0, 1.0, 1.0), glm::vec4(0.05, 0.05, 0.05, 1.0), glm::vec4(0.5, 0.0, 0.0, 1.0), 1.0f);

	cube = new mor::GameObject(glm::vec3(0.0f, 0.0f, -50.0f), glm::vec3(glm::radians(0.0f)), glm::vec3(70.0f, 50.0f, 1.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f));
	cube->Init(renderer.LoadModel("cube"), NULL, NULL, soft_green);
	cube->bounding_shape = new AABox(cube->position, cube->scale.x, cube->scale.y, cube->scale.z);

	objects.push_back(cube);

	tile_model = renderer.LoadModel("tile");
	texture_shader = renderer.LoadShader("texture_vert.glsl", "texture_frag.glsl");
	tile_cursor = new mor::GameObject(glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f));
	tile_cursor->model = tile_model;
	tile_cursor->shader = renderer.LoadShader("tile_cursor_vert.glsl", "tile_cursor_frag.glsl");
	tile_cursor->SetActive(false);
	//add cursor number
	mor::GameObject *cursor_num = new mor::GameObject(glm::vec3(0.0f, 0.0f, 0.1f), glm::vec3(glm::radians(-90.0f), 0.0f, 0.0f), glm::vec3(1.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f));
	tile_cursor->AddChild(cursor_num);

	menu_tile = new mor::GameObject(glm::vec3(2.0f, 0.0f, -1.2f), glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f, 5.0f, 0.0f));
	//menu_tile->Init(tile_model, texture_shader, renderer.LoadTexture("tile09.png"), NULL); <-WHY IS THIS BROKEN!!?
	menu_tile->model = tile_model;
	menu_tile->material = renderer.LoadMaterial(glm::vec4(1.0), glm::vec4(0.8, 0.8, 0.8, 1.0), glm::vec4(0.25, 0.25, 0.25, 1.0), 100.0f);

	tile_num = new mor::GameObject(glm::vec3(0.0f, 0.0f, 0.1f), glm::vec3(glm::radians(-90.0f), 0.0f, 0.0f), glm::vec3(1.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f));
	tile_num->model = renderer.LoadModel("9");
	menu_tile->AddChild(tile_num);

	LoadGUI();

	players = nManager.GetClients(); //players point to nManager's clients
	selected_tile = -1;
	tiles_placed = 0;
}
void Game::LoadGUI(){
	main_menu = gManager.AddMenu();
	gManager.AddButton(main_menu, "network_button", 400, 75, 60, 215, false, "Network", sf::Color(32, 69, 137, 255), sf::Color(9, 43, 106, 255), sf::Color(2, 35, 95, 255), "TF2.ttf");
	gManager.AddButton(main_menu, "quit_button", 400, 75, 60, 315, false, "Quit", sf::Color(32, 69, 137, 255), sf::Color(9, 43, 106, 255), sf::Color(2, 35, 95, 255), "TF2.ttf");

	network_menu = gManager.AddMenu();

	gManager.AddText(network_menu, "id_text", 400, 0, true, "Enter your ID", sf::Color::White, "TF2.ttf", 50);
	gManager.AddTextBox(network_menu, "id_text_box", 420, 80, 400, 70, true, "", sf::Color(9, 43, 106, 255), "TF2.ttf");
	gManager.AddText(network_menu, "lobby_text", 400, 150, true, "Enter Lobby to Create/Join", sf::Color::White, "TF2.ttf", 50);
	gManager.AddTextBox(network_menu, "lobby_text_box", 420, 80, 400, 220, true, "", sf::Color(9, 43, 106, 255), "TF2.ttf");
	gManager.AddButton(network_menu, "connect_button", 500, 75, 400, 500, true, "Connect", sf::Color(32, 69, 137, 255), sf::Color(9, 43, 106, 255), sf::Color(2, 35, 95, 255), "TF2.ttf");

	gManager.AddText(network_menu, "menu_message", 400, 375, true, " ", sf::Color(202, 33, 33, 255), "TF2.ttf", 40);

	lobby = gManager.AddMenu();

	gManager.AddButton(lobby, "ready_button", 500, 75, 400, 500, true, "Ready Up", sf::Color(32, 69, 137, 255), sf::Color(9, 43, 106, 255), sf::Color(2, 35, 95, 255), "TF2.ttf");
	gManager.AddButton(lobby, "leave_button", 75, 75, 700, 25, false, "X", sf::Color(232, 23, 23, 255), sf::Color(181, 18, 18, 255), sf::Color(144, 14, 14, 255), "TF2.ttf");
	gManager.AddText(lobby, "client01", 400, (0 * 40) + 10, true, "", sf::Color::White, "TF2.ttf", 50);
	gManager.AddText(lobby, "client02", 400, (1 * 40) + 10, true, "", sf::Color::White, "TF2.ttf", 50);
	gManager.AddText(lobby, "client03", 400, (2 * 40) + 10, true, "", sf::Color::White, "TF2.ttf", 50);
	gManager.AddText(lobby, "client04", 400, (3 * 40) + 10, true, "", sf::Color::White, "TF2.ttf", 50);

	game_hud = gManager.AddMenu();

	gManager.AddText(game_hud, "lobby_count", 400, 0, true, "0", sf::Color::Green, "TF2.ttf", 50);
	gManager.AddText(game_hud, "client01", 5, (0 * 40) + 395, false, "", sf::Color::White, "TF2.ttf", 50);
	gManager.AddText(game_hud, "client02", 5, (1 * 40) + 395, false, "", sf::Color::White, "TF2.ttf", 50);
	gManager.AddText(game_hud, "client03", 5, (2 * 40) + 395, false, "", sf::Color::White, "TF2.ttf", 50);
	gManager.AddText(game_hud, "client04", 5, (3 * 40) + 395, false, "", sf::Color::White, "TF2.ttf", 50);

	gManager.AddTextBox(game_hud, "text_box", 300, 30, 0, 600 - 30, false, "type here.", sf::Color(9, 43, 106, 255), "TF2.ttf");

	gManager.AddButton(game_hud, "send_button", 200, 50, 590, 10, false, "Send Tiles", sf::Color(32, 69, 137, 255), sf::Color(9, 43, 106, 255), sf::Color(2, 35, 95, 255), "TF2.ttf");

	for (int i = 0; i < 7; i++){
		gManager.AddButton(game_hud, "hand0" + std::to_string(i), 60, 60, 310 + 70 * i, 530, false, std::to_string(i), sf::Color(233, 138, 22, 255), sf::Color(187, 110, 17, 255), sf::Color(131, 77, 12, 255), "TF2.ttf");
	}

	gManager.BindMenu(main_menu);
}

void Game::Render(){
	renderer.ShadowMapPass(tiles);
	renderer.Render(objects);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	renderer.Render(tiles);
	
	//render tile cursor
	renderer.Render(tile_cursor);
	glDisable(GL_BLEND);
}
void Game::Render(sf::RenderWindow *_window){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	switch (nManager.state){
	case NetworkManager::GameState::MAIN_MENU:
		//main menu
		renderer.ShadowMapPass(objects);
		renderer.Render(menu_tile);
	case NetworkManager::GameState::NETWORK_MENU:
		//network menu
		break;
	case NetworkManager::GameState::IN_LOBBY:
		//in lobby
		break;
	case NetworkManager::GameState::IN_GAME:
		//in game
		Render();
	default:
		
		break;
	}

	renderer.BindShader(-1);
	gManager.Render(_window);
}

void Game::Update(float _delta){
	renderer.Update(_delta);
	if (inGame){
		for (int i = 0; i < objects.size(); i++){
			objects[i]->Update(_delta);
		}
		for (int i = 0; i < tiles.size(); i++){
			tiles[i]->Update(_delta);
		}
		//white_light->SetPosition(glm::vec4(camera->pos, 1.0f));
	}
	else{
		menu_tile->Update(_delta);
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
	case NetworkManager::GameState::MAIN_MENU:
		//in main menu
		while (_window->pollEvent(event)){
			switch (event.type){
			case sf::Event::Closed:
				nManager.Disconnect();
				isRunning = false;
				break;
			case sf::Event::MouseButtonReleased:
				if (gManager.ButtonClicked("network_button")){
					nManager.state = NetworkManager::GameState::NETWORK_MENU;
					gManager.BindMenu(network_menu);
					menu_tile->SetActive(false);
				}
				if (gManager.ButtonClicked("quit_button")){
					nManager.Disconnect();
					isRunning = false;
				}
				break;
			}
			case sf::Event::KeyPressed:
				if (event.key.code == sf::Keyboard::Escape){
					nManager.Disconnect();
					isRunning = false;
				}
				break;
		}
		break;
	case NetworkManager::GameState::NETWORK_MENU:
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
	case NetworkManager::GameState::IN_LOBBY:
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
				}
				if (gManager.ButtonClicked("leave_button")){
					nManager.Disconnect();
					inLobby = false;
					gManager.BindMenu(network_menu);
					gManager.SetText("lobby_text_box", "");
					gManager.SetText("menu_message", "");
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
			nManager.GetClients(); //set update clients to false*
			printf("client list updated***\n");
			for (int i = 0; i < 4; i++){
				if (i < players->size()){
					std::string _r;
					if (players->at(i).IsReady()){
						_r = ": Ready";
					}
					else{
						_r = ": Not Ready";
					}
					gManager.SetText("client0" + std::to_string(i + 1), players->at(i).ID() + _r);
				}
				else{
					gManager.SetText("client0" + std::to_string(i + 1), "*empty*");
				}
			}
		}
		break;
	case NetworkManager::GameState::IN_GAME:
		if (!inGame){
			inGame = true;
			white_light->SetPosition(glm::vec4(00.0f, 0.0f, -10.0f, 0.0f));
			gManager.BindMenu(game_hud);
			FillHand();
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
					//send tile to server
					if (selected_tile == -1){ break; }
					glm::vec3 h = Raycast(event.mouseButton.x, event.mouseButton.y);
					if (h != glm::vec3(0, 0, 0) && nManager.is_turn){
						h = glm::floor(h * (1.0f / 0.5f) + 0.5f) / (1.0f / 0.5f);
						//add temp tile
						AddTempTile(h, glm::vec3(0.0f, 0.0f, glm::radians(tile_rotation)), selected_tile);
						tiles_placed++;
						//nManager.SendNewTile(h, glm::radians(tile_rotation), selected_tile);
						tile_cursor->SetActive(false);
					}
				}
				break;
			case sf::Event::MouseButtonReleased:
				if (gManager.ButtonClicked("send_button")){
					SendTiles();
				}
				for (int i = 0; i < 7; i++){
					if (gManager.ButtonClicked("hand0" + std::to_string(i))){
						selected_tile = std::stoi(gManager.GetText("hand0" + std::to_string(i)));
						//tile_cursor->texture = renderer.LoadTexture("tile0" + std::to_string(hand[i]) + ".png");
						tile_cursor->GetChild(0)->model = renderer.LoadModel(std::to_string(hand[i]));
					}
				}
				break;
			case sf::Event::MouseMoved:
				if (!gManager.CursorOverGUI()){
					glm::vec3 h = Raycast(event.mouseMove.x, event.mouseMove.y);
					if (h != glm::vec3(0, 0, 0) && nManager.is_turn && selected_tile >= 0){
						h = glm::floor(h * (1.0f / 0.5f) + 0.5f) / (1.0f / 0.5f);
						tile_cursor->position = h;
						tile_cursor->SetRotation(glm::vec3(0.0f, 0.0f, glm::radians(tile_rotation)));
						tile_cursor->GetChild(0)->SetRotation(glm::vec3(glm::radians(-90.0f), 0.0f, glm::radians(-tile_rotation)));
						tile_cursor->SetActive(true);
					}
					else{
						tile_cursor->SetActive(false);
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
						gManager.AddText(game_hud, "chat" + std::to_string(chat.size() - 1), 2, 20 * (chat.size() - 1), false, gManager.GetText("text_box"), sf::Color(202, 33, 33, 255), "TF2.ttf", 20);

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
			gManager.AddText(game_hud, "chat" + std::to_string(chat.size() - 1), 2, 20 * (chat.size() - 1), false, new_message, sf::Color(9, 43, 106, 255), "TF2.ttf", 20);
		}
		if (nManager.UpdateClients()){
			for (int i = 0; i < 4; i++){
				if (i < players->size()){
					if (players->at(i).IsTurn()){
						gManager.SetText("client0" + std::to_string(i + 1), players->at(i).ID() + " <-");
					}
					else{
						gManager.SetText("client0" + std::to_string(i + 1), players->at(i).ID());
					}
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
	if (_key == sf::Keyboard::R){
		tile_rotation += 90.0f;
		if (tile_rotation >= 360.0f){
			tile_rotation -= 360.0f;
		}
		tile_cursor->SetRotation(glm::vec3(0.0f, 0.0f, glm::radians(tile_rotation)));
		tile_cursor->GetChild(0)->SetRotation(glm::vec3(glm::radians(-90.0f), 0.0f, glm::radians(-tile_rotation)));
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
	glm::vec2 m = glm::vec2(mouseX / renderer.ScreenWidth(), mouseY / renderer.ScreenHeight());

	glm::vec3 mts = camera->pos;
	//this contains camera props the are currently private [need to fix this vv]
	float hNear = 2 * tan(glm::radians(45.0f) / 2) * 0.1f;
	float wNear = hNear * renderer.ScreenWidth() / renderer.ScreenHeight();

	mts.x -= wNear / 2;
	mts.y += hNear / 2;
	mts.x += wNear * m.x;
	mts.y -= hNear * m.y;
	mts.z -= 0.1f;

	Ray r = Ray(camera->pos, glm::normalize(mts - camera->pos), 0.0f);
	r.Intersects(*dynamic_cast<AABox*>(cube->bounding_shape));

	if (tiles.size() > 0){
		float dist = 100.0f;
		glm::vec3 final;
		float rot = 0.0f;
		glm::vec3 p;
		int final_tile, final_adjacent;
		for (int i = 0; i < tiles.size(); i++){
			Tile *t = dynamic_cast<Tile*>(tiles[i]);
			for (int a = 0; a < 8; a++){
				if (!t->CheckAdjacent(a)){
					p = t->GetAdjacentPosition(a);
					if (glm::distance(r.Hit(), p) < dist && glm::distance(r.Hit(), p) < 1.0f){
						if (TileValidPlacement(p)){
							dist = glm::distance(r.Hit(), p);
							final = p;
							final_tile = i; final_adjacent = a;
							if (a == 1 || a == 3 || a == 5 || a == 7){
								rot = glm::degrees(t->rotation.z) + 180.0f;
							}
						}
					}
				}
			}
		}
		if (rot > 0){
			tile_rotation = rot;
			if (tile_rotation >= 360.0f){ tile_rotation -= 360.0f; };
		}
		return final;
	}
	return r.Hit();
}
bool Game::TileValidPlacement(glm::vec3 tile_pos){
	for (int i = 0; i < tiles.size(); i++){
		if (glm::distance(tiles[i]->position, tile_pos) < 1.6f){
			return false;
		}
	}
	return true;
}
void Game::AddTempTile(glm::vec3 _pos, glm::vec3 _rot, int _value){
	Tile *tile = new Tile(_pos, _rot, _value);
	tile->model = tile_model;
	tile->shader = tile_cursor->shader;
	mor::GameObject *nm = new mor::GameObject(glm::vec3(0.0f, 0.0f, 0.1f), glm::vec3(glm::radians(-90.0f), 0.0f, -tile->rotation.z), glm::vec3(1.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f));
	nm->model = renderer.LoadModel(std::to_string(tile->GetValue()));
	tile->AddChild(nm);
	
	if (tiles.size() > 0){
		for (int x = 0; x < tiles.size() - 1; x++){
			Tile *a_t = dynamic_cast<Tile*>(tiles[x]);
			tile->AddAdjacent(a_t);
		}
	}

	tiles.push_back(tile);

	//CheckTiles();
}
void Game::DeleteTempTiles(){
	for (int i = 0; i < tiles_placed; i++){
		tiles.erase(tiles.end()-1);
	}
	tiles_placed = 0;
}
void Game::SendTiles(){
	for (int i = tiles_placed; i > 0; i--){
		Tile *t = dynamic_cast<Tile*>(tiles[tiles.size() - i]);
		nManager.SendNewTile(t->position, t->rotation.z, t->GetValue());
	}
	nManager.EndTurn();
	DeleteTempTiles();
}
void Game::AddTiles(std::vector<Tile*> _tiles){
	for (int i = 0; i < _tiles.size(); i++){
		tiles.push_back(_tiles[i]);
		tiles[tiles.size() - 1]->model = tile_model;
		tiles[tiles.size() - 1]->bounding_shape = new AABox(tiles[tiles.size() - 1]->position, 2.0f, 2.0f, 0.5f);
		//add number child object
		mor::GameObject *nm = new mor::GameObject(glm::vec3(0.0f, 0.0f, 0.1f), glm::vec3(glm::radians(-90.0f), 0.0f, -tiles[tiles.size() - 1]->rotation.z), glm::vec3(1.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f));
		nm->model = renderer.LoadModel(std::to_string(dynamic_cast<Tile*>(tiles[tiles.size() - 1])->GetValue()));
		tiles[tiles.size() - 1]->AddChild(nm);

		if (tiles.size() > 1){
			Tile *t = dynamic_cast<Tile*>(tiles[tiles.size() - 1]);
			for (int x = 0; x < tiles.size() - 1; x++){
				Tile *a_t = dynamic_cast<Tile*>(tiles[x]);
				t->AddAdjacent(a_t);
			}
		}
		dynamic_cast<Tile*>(tiles[tiles.size() - 1])->Drop();
	}

	CheckTiles();
}
void Game::FillHand(){
	for (int i = 0; i < 7; i++){
		hand.push_back(rand() % 10);
		gManager.SetText("hand0" + std::to_string(i), std::to_string(hand[i]));
	}
}

void Game::CheckTiles(){
	if (tiles.size() > 1){
		//check each tile
		for (int i = 0; i < tiles.size(); i++){
			Tile *t = dynamic_cast<Tile*>(tiles[i]);
			for (int a = 0; a < 7; a+=2){
				//check = signs of each tile
				int p = i + 4;
				if (p >= 8){ p -= 8; }
				//check for NULL
				if (t->GetAdjacentTile(a)){
					TilePass(t->GetValue(), t->GetAdjacentTile(a)->GetValue(), t->GetAdjacentTile(a), p);
					/*
					if (t->GetValue() != t->GetAdjacentTile(a)->GetValue()){
						printf("conflict at tile index %i: %i != %i \n", i, t->GetValue(), t->GetAdjacentTile(a)->GetValue());
					}*/
				}
			}
		}
	}
}
void Game::TilePass(int iValue, int fValue, Tile *_tile, int previous_index){
	printf("one pass\n");
	bool corners = false;
	for (int i = 1; i < 8; i+=2){
		if (i != previous_index){
			if (_tile->GetAdjacentTile(i)){
				int p = i + 4;
				if (p >= 8){ p -= 8; }
				TilePass(iValue, fValue + _tile->GetAdjacentTile(i)->GetValue(), _tile->GetAdjacentTile(i), p);
				corners = true;
			}
		}
	}
	if (!corners){
		if (iValue != fValue){
			printf("conflict: %i != %i \n", iValue, fValue);
		}
	}
}