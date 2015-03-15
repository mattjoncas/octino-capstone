#include "Game.h"

mor::GameObject *cube;
mor::Camera *camera;
mor::Light *white_light;
std::vector<mor::GameObject*> tiles;
mor::GameObject *tile_cursor;
mor::GameObject *menu_tile, *tile_num;

float t = 0.0f;
float tile_rotation = 0.0f;
int selected_tile, hand_position, tiles_placed, hand_count;
//materials
int blue, soft_green, gold, orange, chrome, red;
int tile_model, texture_shader;

bool inGame = false, inLobby = false, isReady = false, puzzle_mode = false;

//gui && networking 
gui::GUIManager gManager = gui::GUIManager(true);
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
	white_light = new mor::Light(glm::vec3(0.0, 0.0, 20.0), glm::vec4(0.05, 0.05, 0.05, 1.0), glm::vec4(1.0, 1.0, 1.0, 1.0), glm::vec4(1.0, 1.0, 1.0, 1.0), 300.0f, 1.0f, true);
	white_light->SetPosition(glm::vec3(00.0f, 0.0f, 40.0f));

	renderer.SetCamera(camera);
	renderer.AddLight(white_light);

	blue = renderer.LoadMaterial(glm::vec4(1.0, 1.0, 1.0, 1.0), glm::vec4(0.0, 0.83, 1.0, 1.0), glm::vec4(0.5, 0.5, 0.5, 1.0), 25.0f); //BLUE
	soft_green = renderer.LoadMaterial(glm::vec4(1.0, 1.0, 1.0, 1.0), glm::vec4(0.387, 0.528, 0.086, 1.0), glm::vec4(0.0, 0.0, 0.0, 1.0), 25.0f); //SOFT GREEN [no spec]
	gold = renderer.LoadMaterial(glm::vec4(1.0, 1.0, 1.0, 1.0), glm::vec4(0.31, 0.216, 0.095, 1.0), glm::vec4(1.0, 0.726, 0.181, 1.0), 1.0f); //GOLD
	orange = renderer.LoadMaterial(glm::vec4(1.0, 1.0, 1.0, 1.0), glm::vec4(1.0, 0.368, 0.029, 1.0), glm::vec4(0.638, 1.0, 0.2, 1.0), 50.0f);
	chrome = renderer.LoadMaterial(glm::vec4(1.0, 1.0, 1.0, 1.0), glm::vec4(0.05, 0.05, 0.05, 1.0), glm::vec4(0.5, 0.0, 0.0, 1.0), 1.0f);
	red = renderer.LoadMaterial(glm::vec4(1.0, 1.0, 1.0, 1.0), glm::vec4(0.886, 0.113, 0.113, 1.0), glm::vec4(0.1, 0.1, 0.1, 1.0), 25.0f);

	cube = new mor::GameObject(glm::vec3(0.0f, 0.0f, -50.0f), glm::vec3(glm::radians(0.0f)), glm::vec3(70.0f, 70.0f, 1.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f));
	cube->Init(renderer.LoadModel("cube"), NULL, NULL, NULL);
	cube->shader = renderer.LoadShader("normal_texture_vert.glsl", "normal_texture_frag.glsl");
	cube->texture = renderer.LoadTexture("blank.png");
	cube->normal_map = renderer.LoadTexture("normal.png");
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
	hand_position = -1;
	hand_count = 7;
	tiles_placed = 0;
}
void Game::LoadGUI(){
	sf::Color header = sf::Color(85, 110, 48, 255);
	sf::Color text = sf::Color(85, 110, 48, 255);
	sf::Color b_main = sf::Color(114, 156, 52, 255);
	sf::Color b_hover = sf::Color(153, 238, 29, 255);
	sf::Color b_click = sf::Color(138, 202, 46, 255);

	sf::Color e_main = sf::Color(220, 73, 50, 255);
	sf::Color e_hover = sf::Color(255, 61, 31, 255);
	sf::Color e_click = sf::Color(170, 72, 57, 255);

	main_menu = gManager.AddMenu();
	gManager.AddText(network_menu, "title", renderer.ScreenWidth() / 2, 30, true, "Octino", header, "SG14.ttf", 80);
	gManager.AddButton(main_menu, "network_button", 400, 75, renderer.ScreenWidth() * 0.33, 165, true, "Network", b_main, b_hover, b_click, "alagard.ttf");
	gManager.AddButton(main_menu, "offline_button", 400, 75, renderer.ScreenWidth() * 0.33, 265, true, "Offline", b_main, b_hover, b_click, "8-Bit-Madness.ttf");
	gManager.AddButton(main_menu, "puzzle_button", 400, 75, renderer.ScreenWidth() * 0.33, 365, true, "Puzzle", b_main, b_hover, b_click, "8-Bit-Madness.ttf");
	gManager.AddButton(main_menu, "quit_button", 400, 75, renderer.ScreenWidth() * 0.33, 465, true, "Quit", b_main, b_hover, b_click, "8-Bit-Madness.ttf");

	network_menu = gManager.AddMenu();

	gManager.AddText(network_menu, "id_text", 400, 0, true, "Enter your ID", text, "alagard.ttf", 50);
	gManager.AddTextBox(network_menu, "id_text_box", 420, 80, 400, 70, true, "", b_click, "alagard.ttf");

	gManager.AddText(network_menu, "pass_text", 400, 150, true, "Password", text, "alagard.ttf", 50);
	gManager.AddTextBox(network_menu, "pass_text_box", 420, 80, 400, 220, true, "", b_click, "alagard.ttf");
	gManager.SetTextBoxHidden("pass_text_box", true);
	gManager.AddText(network_menu, "lobby_text", 400, 300, true, "Enter Lobby to Create/Join", text, "alagard.ttf", 50);
	gManager.AddTextBox(network_menu, "lobby_text_box", 420, 80, 400, 370, true, "", b_click, "alagard.ttf");
	gManager.AddButton(network_menu, "connect_button", 500, 75, 400, 500, true, "Connect", b_main, b_hover, b_click, "alagard.ttf");
	gManager.AddButton(network_menu, "random_button", 75, 70, 645, 375, false, "?", b_main, b_hover, b_click, "SG14.ttf");

	gManager.AddText(network_menu, "menu_message", 400, 450, true, " ", e_main, "alagard.ttf", 40);

	lobby = gManager.AddMenu();

	gManager.AddButton(lobby, "ready_button", 500, 75, 400, 500, true, "Ready Up", b_main, b_hover, b_click, "8-Bit-Madness.ttf");
	gManager.AddButton(lobby, "leave_button", 75, 75, 700, 25, false, "X", e_main, e_hover, e_click, "SG14.ttf");
	gManager.AddText(lobby, "client01", 400, (0 * 40) + 10, true, "", b_click, "8-Bit-Madness.ttf", 50);
	gManager.AddText(lobby, "client02", 400, (1 * 40) + 10, true, "", b_click, "8-Bit-Madness.ttf", 50);
	gManager.AddText(lobby, "client03", 400, (2 * 40) + 10, true, "", b_click, "8-Bit-Madness.ttf", 50);
	gManager.AddText(lobby, "client04", 400, (3 * 40) + 10, true, "", b_click, "8-Bit-Madness.ttf", 50);

	game_hud = gManager.AddMenu();

	gManager.AddText(game_hud, "lobby_count", 400, 0, true, "0", sf::Color::Green, "alagard.ttf", 50);
	gManager.AddText(game_hud, "client01", 5, (0 * 40), false, "", sf::Color::White, "alagard.ttf", 50);
	gManager.AddText(game_hud, "client02", 5, (1 * 40), false, "", sf::Color::White, "8-Bit-Madness.ttf", 50);
	gManager.AddText(game_hud, "client03", 5, (2 * 40), false, "", sf::Color::White, "8-Bit-Madness.ttf", 50);
	gManager.AddText(game_hud, "client04", 5, (3 * 40), false, "", sf::Color::White, "alagard.ttf", 50);

	gManager.AddTextBox(game_hud, "text_box", 300, 30, 0, 600 - 30, false, "type here.", b_click, "TF2.ttf");

	gManager.AddButton(game_hud, "end_button", 200, 50, 590, 10, false, "End Turn", b_main, b_hover, b_click, "alagard.ttf");

	for (int i = 0; i < 7; i++){
		gManager.AddButton(game_hud, "hand0" + std::to_string(i), 60, 60, 310 + 70 * i, 530, false, std::to_string(i), e_main, e_hover, e_click, "alagard.ttf");
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

	renderer.BindShader(-1); //this should be in gManager Render()
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
	gui::Event g_event;
	switch (nManager.state){
	case NetworkManager::GameState::MAIN_MENU:
		//in main menu
		while (_window->pollEvent(event)){
			switch (event.type){
			case sf::Event::Closed:
				nManager.Disconnect();
				isRunning = false;
				break;
			case sf::Event::KeyPressed:
				if (event.key.code == sf::Keyboard::Escape){
					nManager.Disconnect();
					isRunning = false;
				}
				break;
			}
			if (gManager.PollEvent(g_event)){
				if (g_event.name == "network_button"){
					nManager.state = NetworkManager::GameState::NETWORK_MENU;
					gManager.BindMenu(network_menu);
					gManager.Select("id_text_box");
					gManager.SetText("menu_message", nManager.Connect());
					menu_tile->SetActive(false);
				}
				if (g_event.name == "offline_button"){
					nManager.state = NetworkManager::GameState::IN_GAME;
					gManager.BindMenu(game_hud);
					menu_tile->SetActive(false);
				}
				if (g_event.name == "puzzle_button"){
					nManager.state = NetworkManager::GameState::IN_GAME;
					gManager.BindMenu(-1);
					puzzle_mode = true;
					menu_tile->SetActive(false);
					GeneratePuzzle();
					AdjustCamera();
				}
				if (g_event.name == "quit_button"){
					nManager.Disconnect();
					isRunning = false;
				}
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
			case sf::Event::KeyPressed:
				if (event.key.code == sf::Keyboard::Escape){
					nManager.Disconnect();
					isRunning = false;
				}
				if (event.key.code == sf::Keyboard::Return){
					gManager.SetText("menu_message",
						nManager.Join(gManager.GetText("id_text_box"), gManager.GetText("pass_text_box"), gManager.GetText("lobby_text_box")));
				}
				break;
			}
		}
		if (gManager.PollEvent(g_event)){
			if (g_event.name == "connect_button"){
				gManager.SetText("menu_message", nManager.Join(gManager.GetText("id_text_box"), gManager.GetText("pass_text_box"), gManager.GetText("lobby_text_box")));
			}
			if (g_event.name == "random_button"){
				//join random lobby
				gManager.SetText("lobby_text_box", nManager.GetRandomLobby());
				if (gManager.GetText("lobby_text_box") == ""){
					gManager.SetText("menu_message", "No lobbies available.");
				}
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
		if (gManager.PollEvent(g_event)){
			if (g_event.name == "ready_button"){
				nManager.ReadyUp();
			}
			if (g_event.name == "leave_button"){
				nManager.Disconnect();
				inLobby = false;
				gManager.BindMenu(network_menu);
				gManager.SetText("lobby_text_box", "");
				gManager.SetText("menu_message", "");
				gManager.BindMenu(main_menu);
				menu_tile->SetActive(true);
			}
		}
		break;
	case NetworkManager::GameState::IN_GAME:
		if (!inGame){
			inGame = true;
			white_light->SetPosition(glm::vec3(0.0f, 0.0f, -10.0f));
			if (!puzzle_mode){
				gManager.BindMenu(game_hud);
				FillHand();
			}
		}
		//multiplayer loop --connected to server--
		if (nManager.IsConnected()){
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
							bool valid = AddTempTile(h, glm::vec3(0.0f, 0.0f, glm::radians(tile_rotation)), selected_tile);
							if (!valid){
								gManager.SetActive("end_button", false);
							}
							else{
								gManager.SetActive("end_button", true);
							}
							AdjustCamera();
							tiles_placed++;
							//nManager.SendNewTile(h, glm::radians(tile_rotation), selected_tile);
							tile_cursor->SetActive(false);
							gManager.SetText("hand0" + std::to_string(hand_position), "");
							gManager.SetActive("hand0" + std::to_string(hand_position), false);
							selected_tile = -1;
							hand_count--;
							float hand_pos_x = gManager.GetPosition("hand0" + std::to_string(hand_position)).x;
							for (int i = 0; i < 7; i++){
								if (gManager.GetPosition("hand0" + std::to_string(i)).x < hand_pos_x && gManager.GetText("hand0" + std::to_string(i)) != ""){
									gManager.Lerp("hand0" + std::to_string(i), gManager.GetPosition("hand0" + std::to_string(i)) + sf::Vector2f(70, 0), 0.5f);
								}
							}
							hand_position = -1;
						}
					}
					if (event.mouseButton.button == sf::Mouse::Right && !gManager.CursorOverGUI()){
						tile_rotation += 90.0f;
						if (tile_rotation >= 360.0f){
							tile_rotation -= 360.0f;
						}
						tile_cursor->SetRotation(glm::vec3(0.0f, 0.0f, glm::radians(tile_rotation)));
						tile_cursor->GetChild(0)->SetRotation(glm::vec3(glm::radians(-90.0f), 0.0f, glm::radians(-tile_rotation)));
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
						if (gManager.GetText("text_box") != "" && gManager.IsSelected("text_box")){
							nManager.SendChatMessage(gManager.GetText("text_box"));
							AddChatMessage(gManager.GetText("text_box"), false);

							gManager.SetText("text_box", "");
							gManager.Unselect();
						}
						else {
							gManager.Select("text_box");
						}
					}
					if (event.key.code == sf::Keyboard::BackSpace){
						if (tiles_placed > 0){
							tiles_placed--;
							Tile *t = dynamic_cast<Tile*>(tiles[tiles.size() - 1]);
							t->RemoveAdjacentTiles();
							//put tile back in hand
							for (int i = 0; i < 7; i++){
								if (gManager.GetText("hand0" + std::to_string(i)) == ""){
									gManager.SetActive("hand0" + std::to_string(i), true);
									gManager.SetPosition("hand0" + std::to_string(i), sf::Vector2f(310, 530));
									hand_count++;
									gManager.Lerp("hand0" + std::to_string(i), sf::Vector2f(310 + 70 * (7 - hand_count), 530), 0.5 * (7 - hand_count));
									gManager.SetText("hand0" + std::to_string(i), std::to_string(t->GetValue()));
									break;
								}
							}

							delete(tiles[tiles.size() - 1]);
							tiles.erase(tiles.end() - 1);

							CheckTiles();
							AdjustCamera();
						}
					}
					break;
				}
			}
			if (gManager.PollEvent(g_event)){
				if (g_event.name == "end_button" && tiles_placed > 0){
					SendTiles();
				}
				for (int i = 0; i < 7; i++){
					if (g_event.name == "hand0" + std::to_string(i)){
						if (gManager.GetText("hand0" + std::to_string(i)) != ""){
							selected_tile = std::stoi(gManager.GetText("hand0" + std::to_string(i)));
							tile_cursor->GetChild(0)->model = renderer.LoadModel(gManager.GetText("hand0" + std::to_string(i)));
							hand_position = i;
						}
					}
				}
			}
			std::string new_message = nManager.GetPendingChatMessage();
			if (new_message.size() > 0){
				AddChatMessage(new_message, true);
			}
			if (nManager.UpdateClients()){
				CheckTiles();
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

				if (nManager.is_turn){
					gManager.SetActive("end_button", true);
				}
			}
			//update tiles if needed
			if (nManager.UpdateTiles()){
				AddTiles(nManager.GetTiles());
			}
		}
		else{
			if (!puzzle_mode){
				//not connected to server [offline play]
				while (_window->pollEvent(event)){
					switch (event.type){
					case sf::Event::Closed:
						isRunning = false;
						break;
					case sf::Event::TextEntered:

						break;
					case sf::Event::MouseButtonPressed:
						if (event.mouseButton.button == sf::Mouse::Left && !gManager.CursorOverGUI()){
							//send tile to server
							if (selected_tile == -1){ break; }
							glm::vec3 h = Raycast(event.mouseButton.x, event.mouseButton.y);
							if (h != glm::vec3(0, 0, 0)){
								h = glm::floor(h * (1.0f / 0.5f) + 0.5f) / (1.0f / 0.5f);
								AddTempTile(h, glm::vec3(0.0f, 0.0f, glm::radians(tile_rotation)), selected_tile);
								tiles_placed++;
								tile_cursor->SetActive(false);
								gManager.SetText("hand0" + std::to_string(hand_position), "");
								gManager.SetActive("hand0" + std::to_string(hand_position), false);
								selected_tile = -1;
								hand_count--;
								float hand_pos_x = gManager.GetPosition("hand0" + std::to_string(hand_position)).x;
								for (int i = 0; i < 7; i++){
									if (gManager.GetPosition("hand0" + std::to_string(i)).x < hand_pos_x && gManager.GetText("hand0" + std::to_string(i)) != ""){
										gManager.Lerp("hand0" + std::to_string(i), gManager.GetPosition("hand0" + std::to_string(i)) + sf::Vector2f(70, 0), 0.5f);
									}
								}
								hand_position = -1;
							}
						}
						if (event.mouseButton.button == sf::Mouse::Right && !gManager.CursorOverGUI()){
							tile_rotation += 90.0f;
							if (tile_rotation >= 360.0f){
								tile_rotation -= 360.0f;
							}
							tile_cursor->SetRotation(glm::vec3(0.0f, 0.0f, glm::radians(tile_rotation)));
							tile_cursor->GetChild(0)->SetRotation(glm::vec3(glm::radians(-90.0f), 0.0f, glm::radians(-tile_rotation)));
						}
						break;
					case sf::Event::MouseMoved:
						if (!gManager.CursorOverGUI()){
							glm::vec3 h = Raycast(event.mouseMove.x, event.mouseMove.y);
							if (h != glm::vec3(0, 0, 0) && selected_tile >= 0){
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
						if (event.key.code == sf::Keyboard::Escape){
							isRunning = false;
						}
						if (event.key.code == sf::Keyboard::BackSpace){
							if (tiles_placed > 0){
								tiles_placed--;
								Tile *t = dynamic_cast<Tile*>(tiles[tiles.size() - 1]);
								t->RemoveAdjacentTiles();
								//put tile back in hand
								for (int i = 0; i < 7; i++){
									if (gManager.GetText("hand0" + std::to_string(i)) == ""){
										gManager.SetActive("hand0" + std::to_string(i), true);
										gManager.SetPosition("hand0" + std::to_string(i), sf::Vector2f(310, 530));
										hand_count++;
										gManager.Lerp("hand0" + std::to_string(i), sf::Vector2f(310 + 70 * (7 - hand_count), 530), 0.5 * (7 - hand_count));
										gManager.SetText("hand0" + std::to_string(i), std::to_string(t->GetValue()));
										break;
									}
								}

								delete(tiles[tiles.size() - 1]);
								tiles.erase(tiles.end() - 1);
							}
						}
						break;
					}
				}
				if (gManager.PollEvent(g_event)){
					if (g_event.name == "end_button" && tiles_placed > 0){
						//SendTiles();
					}
					for (int i = 0; i < 7; i++){
						if (g_event.name == "hand0" + std::to_string(i)){
							if (gManager.GetText("hand0" + std::to_string(i)) != ""){
								selected_tile = std::stoi(gManager.GetText("hand0" + std::to_string(i)));
								tile_cursor->GetChild(0)->model = renderer.LoadModel(gManager.GetText("hand0" + std::to_string(i)));
								hand_position = i;
							}
						}
					}
				}
			}
			else{
				//puzzle mode
				//generate puzzle
				//fill hand
				while (_window->pollEvent(event)){
					switch (event.type){
					case sf::Event::Closed:
						isRunning = false;
						break;
					case sf::Event::TextEntered:

						break;
					case sf::Event::MouseButtonPressed:
						
						break;
					case sf::Event::MouseMoved:
						
						break;
					case sf::Event::KeyPressed:
						if (event.key.code == sf::Keyboard::Escape){
							isRunning = false;
						}
						if (event.key.code == sf::Keyboard::BackSpace){
							GeneratePuzzle();
						}
						if (event.key.code == sf::Keyboard::P){
							printf("tiles checked. ***\n");
							CheckTiles();
						}
						if (event.key.code == sf::Keyboard::O){
							AdjustCamera();
						}
						break;
					}
				}
			}
		}
		break;
		}
	}
}

void Game::Input(){

}
void Game::SFInput(sf::Keyboard::Key _key){
	/*
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
	}*/
}

glm::vec3 Game::GetPosition(){
	return camera->pos;
}

glm::vec3 Game::Raycast(float mouseX, float mouseY){
	glm::vec2 m = glm::vec2(mouseX / renderer.ScreenWidth(), mouseY / renderer.ScreenHeight());

	glm::vec3 mts = camera->pos;
	//this contains camera props the are currently private [need to fix this vv]
	float hNear = 2 * tan(glm::radians(camera->fov) / 2) * camera->fnear;
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
bool Game::AddTempTile(glm::vec3 _pos, glm::vec3 _rot, int _value){
	Tile *tile = new Tile(_pos, _rot, _value);
	tile->model = tile_model;
	tile->shader = tile_cursor->shader;
	mor::GameObject *nm = new mor::GameObject(glm::vec3(0.0f, 0.0f, 0.1f), glm::vec3(glm::radians(-90.0f), 0.0f, -tile->rotation.z), glm::vec3(1.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f));
	nm->model = renderer.LoadModel(std::to_string(tile->GetValue()));
	tile->AddChild(nm);

	tiles.push_back(tile);
	
	if (tiles.size() > 1){
		for (int x = 0; x < tiles.size() - 1; x++){
			Tile *a_t = dynamic_cast<Tile*>(tiles[x]);
			tile->AddAdjacent(a_t);
		}
	}

	return CheckTiles();
}
void Game::DeleteTempTiles(){
	for (int i = 0; i < tiles_placed; i++){
		dynamic_cast<Tile*>(tiles[tiles.size() - 1])->RemoveAdjacentTiles();
		delete(tiles[tiles.size()-1]);
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
	gManager.SetActive("end_button", false);
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
	
	AdjustCamera();
}
void Game::FillHand(){
	for (int i = 0; i < 7; i++){
		hand.push_back(rand() % 10);
		gManager.SetText("hand0" + std::to_string(i), std::to_string(hand[i]));
	}
}

bool Game::CheckTiles(){
	if (tiles.size() > 1){
		//check each tile
		bool bad_placement = false;
		for (int i = 0; i < tiles.size(); i++){
			Tile *t = dynamic_cast<Tile*>(tiles[i]);
			for (int a = 0; a < 7; a+=2){
				//check = signs of each tile
				int p = a + 4;
				if (p >= 8){ p -= 8; }
				//check for NULL
				if (t->GetAdjacentTile(a)){
					std::vector<int> equation;
					equation.push_back(t->GetAdjacentTile(a)->GetValue());
					std::vector<int> e;
					e.push_back(t->GetValue());
					
					if (!TilePass(t, t->GetAdjacentTile(a), p, equation) && !TilePass(t->GetAdjacentTile(a), t, a, e)){
						t->material = red;
						t->GetAdjacentTile(a)->material = red;

						bad_placement = true;
					}
					else{
						t->material = 0;
						t->GetAdjacentTile(a)->material = 0;
					}
				}
			}
		}
		if (bad_placement){
			return false;
		}
		else{
			return true;
		}
	}
}
bool Game::TilePass(Tile *i_tile, Tile *a_tile, int previous_index, std::vector<int> _equation){
	bool test = false;
	bool corners = false;
	for (int i = 1; i < 8; i+=2){
		if (i != previous_index){
			if (a_tile->GetAdjacentTile(i)){
				int p = i + 4;
				if (p >= 8){ p -= 8; }
				int operation;
				if (i == 1){ operation = -2; }
				else if (i == 3){ operation = -3; }
				else if (i == 5){ operation = -4; }
				else if (i == 7){ operation = -1; }
				operation -= abs(glm::degrees(a_tile->rotation.z) / 90.0f);
				if (operation < -4){ operation += 4; }
				_equation.push_back(operation);
				_equation.push_back(a_tile->GetAdjacentTile(i)->GetValue());
				test = TilePass(i_tile, a_tile->GetAdjacentTile(i), p, _equation);
				corners = true;
			}
		}
	}
	if (!corners){
		float l_final, r_final;
		//calculate from left to right
		l_final = _equation[0];
		for (int i = 1; i < _equation.size(); i += 2){
			if (_equation[i] == -1){
				l_final += _equation[i + 1];
			}
			else if (_equation[i] == -2){
				l_final -= _equation[i + 1];
			}
			else if (_equation[i] == -3){
				l_final /= _equation[i + 1];
			}
			else if (_equation[i] == -4){
				l_final *= _equation[i + 1];
			}
		}
		//calculate from right to left
		r_final = _equation[_equation.size() - 1];
		for (int i = _equation.size() - 2; i > 0; i -= 2){
			if (_equation[i] == -1){
				r_final += _equation[i - 1];
			}
			else if (_equation[i] == -2){
				r_final -= _equation[i - 1];
			}
			else if (_equation[i] == -3){
				r_final /= _equation[i - 1];
			}
			else if (_equation[i] == -4){
				r_final *= _equation[i - 1];
			}
		}
		/*
		//print test
		std::cout << i_tile->GetValue() << " = " << _equation[0];
		for (int i = 1; i < _equation.size(); i++){
			if (_equation[i] == -1){
				std::cout << " + ";
			}
			else if (_equation[i] == -2){
				std::cout << " - ";
			}
			else if (_equation[i] == -3){
				std::cout << " / ";
			}
			else if (_equation[i] == -4){
				std::cout << " * ";
			}
			else{
				std::cout << _equation[i];
			}
		}
		std::cout << " \n";*/
		if (i_tile->GetValue() != l_final && i_tile->GetValue() != r_final){
			//printf("conflict: %i != %f || %f \n", i_tile->GetValue(), l_final, r_final);
			return false;
		}
		//bug fix test
		bool c_test = false;
		for (int i = 1; i < 8; i += 2){
			if (i_tile->GetAdjacentTile(i)){
				//printf("ADJACENT PASSS!!!\n");
				c_test = true;
				break;
			}
		}
		if (_equation.size() == 1 && c_test){
			//printf("NOT EQUAL!!\n");
			return false;
		}
		return true;
	}
	return test;
}
void Game::AdjustCamera(){
	if (tiles.size() > 0){
		glm::vec3 average_pos;
		float largest_diff = 0.0f;
		for (int i = 0; i < tiles.size(); i++){
			// x/y
			average_pos.x += tiles[i]->position.x;
			average_pos.y += tiles[i]->position.y;
			//z
			for (int t = 0; t < tiles.size(); t++){
				float diff = glm::distance(glm::vec2(tiles[i]->position), glm::vec2(tiles[t]->position));
				if (diff > largest_diff){
					largest_diff = diff;
				}
			}
		}
		average_pos /= tiles.size();
		if (largest_diff < 6.0f){ largest_diff = 6.0f; }
		average_pos.z = -49.5f + (largest_diff * 2.0f);
		camera->Lerp(average_pos, glm::clamp(glm::distance(average_pos, camera->pos) / 10.0f, 0.5f, 2.0f));
	}
}
void Game::AddChatMessage(std::string _message, bool incoming_message){
	for (int i = 0; i < chat.size(); i++){
		gManager.Lerp("chat" + std::to_string(i), gManager.GetPosition("chat" + std::to_string(i)) + sf::Vector2f(0, -20), 0.1f);
		//gManager.SetPosition("chat" + std::to_string(i), gManager.GetPosition("chat" + std::to_string(i)) + sf::Vector2f(0, -20));
		if (chat.size() > 10 && i < chat.size() - 10){
			gManager.SetActive("chat" + std::to_string(i), false);
		}
	}
	chat.push_back(_message);
	if (incoming_message){
		gManager.AddText(game_hud, "chat" + std::to_string(chat.size() - 1), 4, 540, false, _message, sf::Color(9, 43, 106, 255), "TF2.ttf", 20);
	}
	else{
		gManager.AddText(game_hud, "chat" + std::to_string(chat.size() - 1), 4, 540, false, _message, sf::Color(202, 33, 33, 255), "TF2.ttf", 20);
	}
}

void Game::GeneratePuzzle(){
	for (std::vector<mor::GameObject*>::iterator iter = tiles.begin(); iter != tiles.end(); ++iter){
		delete((*iter));
	}
	tiles.clear();
	tiles_placed = 0;

	camera->SetPosition(glm::vec3(0.0f));
	glm::vec3 h = Raycast(renderer.ScreenWidth() / 2, renderer.ScreenHeight() / 2);
	//place inital tile
	int t_value = rand() % 10;
	h = glm::floor(h * (1.0f / 0.5f) + 0.5f) / (1.0f / 0.5f);
	AddTempTile(h, glm::vec3(0.0f, 0.0f, glm::radians(0.0f)), t_value);
	tiles_placed++;
	//fill puzzle randomly
	int puzzle_size = 4;
	for (int i = 0; i < puzzle_size; i++){
		Tile *t = dynamic_cast<Tile*>(tiles[rand() % tiles.size()]);
		//if tile's =s are full break
		bool test = false;
		for (int j = 0; j < 7; j+=2){
			if (!t->CheckAdjacent(j)){
				test = true;
				break;
			}
		}
		if (!test){
			i--;
			continue;
		}
		float a = 0, b = 0;
		int op;
		test = false;
		while (!test){
			a = rand() % 10;
			b = rand() % 10;

			if (a + b == t->GetValue()){ test = true; op = 0; }
			else if (a - b == t->GetValue()){ test = true; op = 1; }
			else if (b - a == t->GetValue()){ test = true; op = 1; }
			else if (a * b == t->GetValue()){ test = true; op = 2; }
			else if (b != 0){
				if (a / b == t->GetValue()){ test = true; op = 3; }
			}
			else if (a != 0){
				if (b / a == t->GetValue()){ test = true; op = 3; }
			}
		}
		//get a random '=' tile_slot
		int tile_slot = rand() % 8;
		if (tile_slot % 2 != 0){
			tile_slot -= 1;
		}
		//make sure tile_slot is open
		while (t->CheckAdjacent(tile_slot)){
			tile_slot = rand() % 8;
			if (tile_slot % 2 != 0){
				tile_slot -= 1;
			}
		}
		//get rotation based on tile_slot
		if (tile_slot == 0){ tile_rotation = 0.0f; }
		else if (tile_slot == 2){ tile_rotation = 270.0f; }
		else if (tile_slot == 4){ tile_rotation = 180.0f; }
		else if (tile_slot == 6){ tile_rotation = 90.0f; }
		if (op == 2 || op == 3){
			tile_rotation = tile_rotation + 180.0f;
		}
		//random offset
		int f = rand() % 2;
		if (op == 0){
			if (f != 0){
				tile_rotation -= 90.0f;
			}
			//std::cout << a << ", " << b << " + ;";
		}
		else if (op == 1){
			if (f != 0){
				tile_rotation += 90.0f;
			}
			//std::cout << a << ", " << b << " - ;";
		}
		else if (op == 2){
			if (f != 0){
				tile_rotation += 90.0f;
			}
			//std::cout << a << ", " << b << " * ;";
		}
		else if (op == 3){
			if (f != 0){
				tile_rotation -= 90.0f;
			}
			//std::cout << a << ", " << b << " / ;";
		}
		//rotation clamp
		if (tile_rotation >= 360.0f){ tile_rotation -= 360.0f; }
		else if (tile_rotation < 0.0f){ tile_rotation += 360.0f; }
		
		if (TileValidPlacement(t->GetAdjacentPosition(tile_slot))){
			AddTempTile(t->GetAdjacentPosition(tile_slot), glm::vec3(0.0f, 0.0f, glm::radians(tile_rotation)), a);
			tiles_placed++;
		}
		else{
			printf("TileValidPlacement returned false!!! ******\n");
			i--;
			continue;
		}
		
		tile_rotation = glm::degrees(tiles[tiles.size() - 1]->rotation.z) + 180.0f;
		if (tile_rotation >= 360.0f){ tile_rotation -= 360.0f; };
		
		Tile* second_tile = dynamic_cast<Tile*>(tiles[tiles.size() - 1]);
		tile_slot = second_tile->GetAdjacentIndex(op);
		if (TileValidPlacement(second_tile->GetAdjacentPosition(tile_slot))){
			if (AddTempTile(second_tile->GetAdjacentPosition(tile_slot), glm::vec3(0.0f, 0.0f, glm::radians(tile_rotation)), b)){
				tiles_placed++;
			}
			else{
				//ERROR!!
				printf("CHECK TILES RETURNED FALSE!!! ******\n");
				//remove both new tiles
				dynamic_cast<Tile*>(tiles[tiles.size() - 1])->RemoveAdjacentTiles();
				delete(tiles[tiles.size() - 1]);
				tiles.erase(tiles.end() - 1);
				second_tile->RemoveAdjacentTiles();
				delete(tiles[tiles.size() - 1]);
				tiles.erase(tiles.end() - 1);
				tiles_placed --;

				for (int i = 0; i < tiles.size(); i++){
					tiles[i]->material = 0;
				}

				i--;
				continue;
			}
		}
		else{
			printf("TileValidPlacement returned false!!! ******\n");
			second_tile->RemoveAdjacentTiles();
			delete(tiles[tiles.size() - 1]);
			tiles.erase(tiles.end() - 1);
			tiles_placed--;

			for (int i = 0; i < tiles.size(); i++){
				tiles[i]->material = 0;
			}

			i--;
			continue;
		}
	}
	printf("Puzzle generated!\n");
}