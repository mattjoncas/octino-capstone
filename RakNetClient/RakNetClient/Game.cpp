#include "Game.h"

mor::GameObject *cube;
mor::Camera *camera;
mor::Light *white_light;
std::vector<mor::GameObject*> tiles;
std::vector<mor::GameObject*> menu_objects;
mor::GameObject *tile_cursor;

float t = 0.0f;
float tile_rotation = 0.0f;
int selected_tile, hand_position, tiles_placed, m_hand_count, c_hand_count;
//models/shaders/materials
int tile_model, texture_shader, red;

bool inGame = false, inLobby = false, isReady = false, puzzle_mode = false;
int tutorial_stage;

int game_hud, main_menu, settings_menu, network_menu, lobby, create_id_menu, pause_menu;
int previous_menu;

//audio
sf::SoundBuffer s_buffer, w_buffer, l_buffer;
sf::Sound select_sound, win_sound, lose_sound;
sf::Music music;
std::vector<std::string> songs;
bool play_music = true;
//colors
sf::Color header = sf::Color(85, 110, 48, 255);
sf::Color text = sf::Color(85, 110, 48, 255);
sf::Color b_main = sf::Color(114, 156, 52, 255);
sf::Color b_hover = sf::Color(153, 238, 29, 255);
sf::Color b_click = sf::Color(138, 202, 46, 255);
sf::Color e_main = sf::Color(220, 73, 50, 255);
sf::Color e_hover = sf::Color(255, 61, 31, 255);
sf::Color e_click = sf::Color(170, 72, 57, 255);
sf::Color sf_purple = sf::Color(56, 37, 48, 255);

Game::~Game(){
	for (std::vector<mor::GameObject*>::iterator iter = objects.begin(); iter != objects.end(); ++iter){
		delete((*iter));
	}
	for (std::vector<mor::GameObject*>::iterator iter = tiles.begin(); iter != tiles.end(); ++iter){
		delete((*iter));
	}
	for (std::vector<mor::GameObject*>::iterator iter = menu_objects.begin(); iter != menu_objects.end(); ++iter){
		delete((*iter));
	}

	delete(camera);
	delete(tile_cursor);

	gManager.ResetInstance();
}

void Game::Load(){
	srand(time(NULL));
	
	camera = new mor::Camera(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f), renderer.ScreenWidth(), renderer.ScreenHeight(), false);
	white_light = new mor::Light(glm::vec3(0.0, 0.0, 20.0), glm::vec4(0.05, 0.05, 0.05, 1.0), glm::vec4(1.0, 1.0, 1.0, 1.0), glm::vec4(1.0, 1.0, 1.0, 1.0), 300.0f, 1.0f, true);
	white_light->SetPosition(glm::vec3(00.0f, 0.0f, 40.0f));

	renderer.SetCamera(camera);
	renderer.AddLight(white_light);
	
	red = renderer.LoadMaterial(glm::vec4(1.0, 1.0, 1.0, 1.0), glm::vec4(0.886, 0.113, 0.113, 1.0), glm::vec4(0.1, 0.1, 0.1, 1.0), 25.0f);

	cube = new mor::GameObject(glm::vec3(0.0f, 0.0f, -50.0f), glm::vec3(glm::radians(0.0f)), glm::vec3(112.0f, 70.0f, 1.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f));
	cube->Init(renderer.LoadModel("cube"), NULL, NULL, NULL);
	cube->shader = renderer.LoadShader("texture_vert.glsl", "texture_frag.glsl");
	cube->texture = renderer.LoadTexture("background.png");
	cube->bounding_shape = new AABox(cube->position, cube->scale.x, cube->scale.y, cube->scale.z);

	objects.push_back(cube);

	tile_model = renderer.LoadModel("tile");
	texture_shader = renderer.LoadShader("texture_vert.glsl", "texture_frag.glsl");
	tile_cursor = new mor::GameObject(glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f));
	tile_cursor->model = tile_model;
	tile_cursor->shader = renderer.LoadShader("tile_cursor_vert.glsl", "tile_cursor_frag.glsl");
	tile_cursor->SetActive(false);
	mor::GameObject *cursor_num = new mor::GameObject(glm::vec3(0.0f, 0.0f, 0.1f), glm::vec3(glm::radians(-90.0f), 0.0f, 0.0f), glm::vec3(1.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f));
	tile_cursor->AddChild(cursor_num);

	for (int y = 0; y < 5; y++){
		for (int x = 0; x < 5; x++){
			menu_objects.push_back(new mor::GameObject(glm::vec3(x * 5 - 2 * 5, y * 3 - 2 * 3, -15.0f), glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 0.0f)));
			//menu_tile->Init(tile_model, texture_shader, renderer.LoadTexture("tile09.png"), NULL);
			menu_objects[menu_objects.size() - 1]->model = tile_model;
			menu_objects[menu_objects.size() - 1]->material = renderer.LoadMaterial(glm::vec4(1.0), glm::vec4(0.8, 0.8, 0.8, 1.0), glm::vec4(0.25, 0.25, 0.25, 1.0), 100.0f);

			mor::GameObject *tile_num = new mor::GameObject(glm::vec3(0.0f, 0.0f, 0.1f), glm::vec3(glm::radians(-90.0f), 0.0f, 0.0f), glm::vec3(1.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f));
			int n = rand() % 10;
			tile_num->model = renderer.LoadModel(std::to_string(n));
			menu_objects[menu_objects.size() - 1]->AddChild(tile_num);
		}
	}
	LoadGUI();
	LoadAudio();

	players = nManager.GetClients(); //players point to nManager's clients
	selected_tile = -1;
	hand_position = -1;
	m_hand_count = 0, c_hand_count = 0;
	tiles_placed = 0;
}
void Game::LoadGUI(){

	main_menu = gManager.AddMenu();
	gManager.AddText(main_menu, "title", renderer.ScreenWidth() / 2, 50, true, "Octino", sf_purple, "SG14.ttf", 120, 2.5f);
	gManager.AddButton(main_menu, "tutorial_button", 400, 75, renderer.ScreenWidth() / 2, int(renderer.ScreenHeight() * 0.30), true, "Tutorial", "alagard.ttf", "textures/button_texture.png");
	gManager.AddButton(main_menu, "network_button", 400, 75, renderer.ScreenWidth() / 2, int(renderer.ScreenHeight() * 0.30 + 100), true, "Network", "alagard.ttf", "textures/button_texture.png");
	gManager.AddButton(main_menu, "offline_button", 400, 75, renderer.ScreenWidth() / 2, int(renderer.ScreenHeight() * 0.30 + 200), true, "Offline", "alagard.ttf", "textures/button_texture.png");
	gManager.AddButton(main_menu, "puzzle_button", 400, 75, renderer.ScreenWidth() / 2, int(renderer.ScreenHeight() * 0.30 + 300), true, "Puzzle", "alagard.ttf", "textures/button_texture.png");
	gManager.AddButton(main_menu, "quit_button", 400, 75, renderer.ScreenWidth() / 2, int(renderer.ScreenHeight() * 0.30 + 400), true, "Quit", "alagard.ttf", "textures/button_texture.png");

	gManager.AddButton(main_menu, "set_button", 75, 70, 10, renderer.ScreenHeight() - 80, false, "*", e_main, e_hover, e_click, "SG14.ttf");

	settings_menu = gManager.AddMenu(sf_purple);
	gManager.AddText(settings_menu, "title", renderer.ScreenWidth() / 2, 30, true, "Settings", header, "SG14.ttf", 120, NULL);
	gManager.AddSlider(settings_menu, "music_slider", 300, 70, renderer.ScreenWidth() / 2, renderer.ScreenHeight() / 2 - 80, true, "Music", b_main, b_hover, header, "SG14.ttf");
	gManager.AddSlider(settings_menu, "fx_slider", 300, 70, renderer.ScreenWidth() / 2, renderer.ScreenHeight() / 2, true, "FX", b_main, b_hover, header, "SG14.ttf");
	gManager.AddButton(settings_menu, "mute_button", 300, 70, renderer.ScreenWidth() / 2, renderer.ScreenHeight() / 2 + 80, true, "mute", e_main, e_hover, e_click, "SG14.ttf");
	gManager.AddButton(settings_menu, "back_button", 300, 70, renderer.ScreenWidth() / 2, renderer.ScreenHeight() - 90, true, "back", e_main, e_hover, e_click, "SG14.ttf");

	network_menu = gManager.AddMenu(sf_purple);
	int y_offset = int(renderer.ScreenHeight() * 0.1);
	gManager.AddText(network_menu, "id_text", renderer.ScreenWidth() / 2, 0 + y_offset, true, "Enter your ID", text, "alagard.ttf", 50, NULL);
	gManager.AddTextBox(network_menu, "id_text_box", 420, 80, renderer.ScreenWidth() / 2, 70 + y_offset, true, "", b_click, "alagard.ttf");
	gManager.AddText(network_menu, "pass_text", renderer.ScreenWidth() / 2, 150 + y_offset, true, "Password", text, "alagard.ttf", 50, NULL);
	gManager.AddTextBox(network_menu, "pass_text_box", 420, 80, renderer.ScreenWidth() / 2, 220 + y_offset, true, "", b_click, "alagard.ttf");
	gManager.SetTextBoxHidden("pass_text_box", true);
	gManager.AddText(network_menu, "lobby_text", renderer.ScreenWidth() / 2, 300 + y_offset, true, "Enter Lobby to Create/Join", text, "alagard.ttf", 50, NULL);
	gManager.AddTextBox(network_menu, "lobby_text_box", 420, 80, renderer.ScreenWidth() / 2, 370 + y_offset, true, "", b_click, "alagard.ttf");
	gManager.AddButton(network_menu, "connect_button", 400, 75, int(renderer.ScreenWidth() * 0.33), 600 + y_offset, true, "Connect", b_main, b_hover, b_click, "alagard.ttf");
	gManager.AddButton(network_menu, "back_button", 400, 75, int(renderer.ScreenWidth() * 0.66), 600 + y_offset, true, "Back", b_main, b_hover, b_click, "alagard.ttf");
	gManager.AddButton(network_menu, "random_button", 75, 70, renderer.ScreenWidth() / 2 + 245, 375 + y_offset, false, "?", b_main, b_hover, b_click, "SG14.ttf");
	gManager.AddButton(network_menu, "new_button", 75, 70, renderer.ScreenWidth() / 2 + 245, 75 + y_offset, false, "!", b_main, b_hover, b_click, "SG14.ttf");
	gManager.AddText(network_menu, "menu_message", renderer.ScreenWidth() / 2, 500 + y_offset, true, " ", e_main, "alagard.ttf", 40, NULL);

	create_id_menu = gManager.AddMenu(sf_purple);
	
	gManager.AddText(create_id_menu, "title", renderer.ScreenWidth() / 2, y_offset / 2, true, "Create New ID", header, "SG14.ttf", 70, NULL);
	gManager.AddText(create_id_menu, "id_text", renderer.ScreenWidth() / 2, 0 + y_offset * 2, true, "Enter your ID", text, "alagard.ttf", 50, NULL);
	gManager.AddTextBox(create_id_menu, "id_text_box", 420, 80, renderer.ScreenWidth() / 2, 70 + y_offset * 2, true, "", b_click, "alagard.ttf");
	gManager.AddText(create_id_menu, "pass_text", renderer.ScreenWidth() / 2, 150 + y_offset * 2, true, "Password", text, "alagard.ttf", 50, NULL);
	gManager.AddTextBox(create_id_menu, "pass_text_box", 420, 80, renderer.ScreenWidth() / 2, 220 + y_offset * 2, true, "", b_click, "alagard.ttf");
	gManager.SetTextBoxHidden("pass_text_box", true);
	gManager.AddButton(create_id_menu, "create_button", 400, 75, int(renderer.ScreenWidth() * 0.33), 600 + y_offset, true, "Create", b_main, b_hover, b_click, "alagard.ttf");
	gManager.AddButton(create_id_menu, "back_button", 400, 75, int(renderer.ScreenWidth() * 0.66), 600 + y_offset, true, "Back", b_main, b_hover, b_click, "alagard.ttf");
	gManager.AddText(create_id_menu, "menu_message", renderer.ScreenWidth() / 2, 450 + y_offset, true, " ", e_main, "alagard.ttf", 50, NULL);

	lobby = gManager.AddMenu();

	gManager.AddButton(lobby, "ready_button", 500, 75, renderer.ScreenWidth() / 2, renderer.ScreenHeight() - 100, true, "Ready Up", b_main, b_hover, b_click, "alagard.ttf");
	gManager.AddButton(lobby, "leave_button", 75, 75, renderer.ScreenWidth() - 100, 25, false, "X", e_main, e_hover, e_click, "SG14.ttf");
	gManager.AddText(lobby, "client01", renderer.ScreenWidth() / 2, (0 * 40) + int(renderer.ScreenHeight() * 0.33), true, "", b_click, "8-Bit-Madness.ttf", 50, NULL);
	gManager.AddText(lobby, "client02", renderer.ScreenWidth() / 2, (1 * 40) + int(renderer.ScreenHeight() * 0.33), true, "", b_click, "8-Bit-Madness.ttf", 50, NULL);
	gManager.AddText(lobby, "client03", renderer.ScreenWidth() / 2, (2 * 40) + int(renderer.ScreenHeight() * 0.33), true, "", b_click, "8-Bit-Madness.ttf", 50, NULL);
	gManager.AddText(lobby, "client04", renderer.ScreenWidth() / 2, (3 * 40) + int(renderer.ScreenHeight() * 0.33), true, "", b_click, "8-Bit-Madness.ttf", 50, NULL);

	game_hud = gManager.AddMenu();
	
	gManager.AddText(game_hud, "client01", renderer.ScreenWidth() / 2, (0 * 40), true, "", header, "alagard.ttf", 50, NULL);
	gManager.AddText(game_hud, "client02", renderer.ScreenWidth() / 2, (1 * 40), true, "", header, "alagard.ttf", 50, NULL);
	gManager.AddText(game_hud, "client03", renderer.ScreenWidth() / 2, (2 * 40), true, "", header, "alagard.ttf", 50, NULL);
	gManager.AddText(game_hud, "client04", renderer.ScreenWidth() / 2, (3 * 40), true, "", header, "alagard.ttf", 50, NULL);
	gManager.AddTextBox(game_hud, "text_box", 300, 40, 0, renderer.ScreenHeight() - 40, false, "type here.", b_click, "TF2.ttf");
	gManager.AddButton(game_hud, "end_button", 200, 50, renderer.ScreenWidth() / 2, renderer.ScreenHeight() - 140, true, "End Turn", b_main, b_hover, b_click, "alagard.ttf");
	gManager.AddButton(game_hud, "pickup_button", 150, 50, renderer.ScreenWidth() - 160, renderer.ScreenHeight() - 60, false, "Pick Up", b_main, b_hover, b_click, "alagard.ttf");

	pause_menu = gManager.AddMenu(sf::Color(30, 30, 30, int(255 * 0.75)));
	gManager.AddText(pause_menu, "title", renderer.ScreenWidth() / 2, 30, true, "paused", header, "SG14.ttf", 120, NULL);
	gManager.AddButton(pause_menu, "back_button", 300, 70, renderer.ScreenWidth() / 2, renderer.ScreenHeight() - 280, true, "back", b_main, b_hover, b_click, "SG14.ttf");
	gManager.AddButton(pause_menu, "quit_button", 300, 70, renderer.ScreenWidth() / 2, renderer.ScreenHeight() - 140, true, "quit", b_main, b_hover, b_click, "SG14.ttf");

	gManager.BindMenu(main_menu);
}

void Game::Render(){
	renderer.ShadowMapPass(tiles);
	renderer.Render(objects);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	renderer.Render(tiles);
	
	renderer.Render(tile_cursor);
	glDisable(GL_BLEND);
}
void Game::Render(sf::RenderWindow *_window){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	switch (nManager.GetState()){
	case NetworkManager::GameState::MAIN_MENU:
		renderer.ShadowMapPass(objects);
		renderer.Render(menu_objects);
	case NetworkManager::GameState::SETTINGS_MENU:

		break;
	case NetworkManager::GameState::NETWORK_MENU:
		//network menu
		break;
	case NetworkManager::GameState::CREATE_ID_MENU:
		//create id menu
		break;
	case NetworkManager::GameState::IN_LOBBY:
		//in lobby
		break;
	case NetworkManager::GameState::IN_GAME:
		Render();
		break;
	case NetworkManager::GameState::TUTORIAL:
		Render();
		break;
	case NetworkManager::GameState::PAUSED:
		Render();
		break;
	default:

		break;
	}

	renderer.BindShader(-1); //this should be in gManager Render()
	gManager.Render(_window);
}

void Game::Update(float _delta){
	renderer.Update(_delta);
	if (inGame){
		for (int i = 0; i < int(objects.size()); i++){
			objects[i]->Update(_delta);
		}
		for (int i = 0; i < int(tiles.size()); i++){
			tiles[i]->Update(_delta);
		}
	}
	else{
		for (int i = 0; i < int(menu_objects.size()); i++){
			menu_objects[i]->Update(_delta);
		}
	}
}
void Game::Update(float _delta, sf::RenderWindow *_window){
	if (music.getStatus() == sf::Music::Stopped && play_music){
		int s = rand() % songs.size();
		music.openFromFile("audio/" + songs[s]);
		music.play();
	}
	Update(_delta);
	nManager.Update(_delta);
	std::string _message = nManager.GetServerMessage();
	if (_message != ""){
		gManager.SetText("menu_message", _message);
		if (_message == "Account created."){
			gManager.SetText("id_text_box", "");
			gManager.SetText("pass_text_box", "");
		}
	}
	gManager.Update(_window, _delta);
	sf::Event event;
	gui::Event g_event;
	switch (nManager.GetState()){
	case NetworkManager::GameState::MAIN_MENU:
		while (_window->pollEvent(event)){
			switch (event.type){
			case sf::Event::LostFocus:
				gManager.Pause(true);
				break;
			case sf::Event::GainedFocus:
				gManager.Pause(false);
				break;
			case sf::Event::Closed:
				nManager.Disconnect();
				isRunning = false;
				break;
			case sf::Event::KeyPressed:
				if (event.key.code == sf::Keyboard::Escape){
					nManager.Disconnect();
					//isRunning = false;
					SwitchScene("exit");
				}
				break;
			case sf::Event::MouseMoved:
				glm::vec3 _rr = Raycast(event.mouseMove.x, event.mouseMove.y);
				glm::quat q = glm::quat(glm::lookAt(camera->pos, _rr, camera->up));
				for (int i = 0; i < menu_objects.size(); i++){
					menu_objects[i]->SetRotation(q);
				}
				break;
			}
			if (gManager.PollEvent(g_event)){
				if (g_event.type == gui::EventType::Button){
					select_sound.play();
				}
				if (g_event.name == "tutorial_button"){
					nManager.SetState(NetworkManager::GameState::TUTORIAL);
					gManager.BindMenu(game_hud);

					gManager.SetActive("end_button", false);
					gManager.SetActive("pickup_button", false);
					gManager.SetActive("text_box", false);
					gManager.SetText("client01", "");
					gManager.SetText("client02", "");
					gManager.SetText("client03", "");
					gManager.SetText("client04", "");
				}
				else if (g_event.name == "network_button"){
					nManager.SetState(NetworkManager::GameState::NETWORK_MENU);
					gManager.BindMenu(network_menu);
					gManager.Select("id_text_box");
					gManager.SetText("menu_message", nManager.Connect());
				}
				else if (g_event.name == "offline_button"){
					nManager.SetState(NetworkManager::GameState::IN_GAME);
					gManager.BindMenu(game_hud);
				}
				else if (g_event.name == "puzzle_button"){
					nManager.SetState(NetworkManager::GameState::IN_GAME);
					gManager.BindMenu(game_hud);
					gManager.SetActive("end_button", false);
					gManager.SetActive("pickup_button", false);
					gManager.SetActive("text_box", false);
					puzzle_mode = true;
					GeneratePuzzle();
					AdjustCamera();

					gManager.SetText("client01", "");
					gManager.SetText("client02", "");
					gManager.SetText("client03", "");
					gManager.SetText("client04", "");
				}
				else if (g_event.name == "quit_button"){
					nManager.Disconnect();
					isRunning = false;
				}
				else if (g_event.name == "set_button"){
					nManager.SetState(NetworkManager::GameState::SETTINGS_MENU);
					gManager.BindMenu(settings_menu);
				}
				else if (g_event.name == "test_slider"){
					music.setVolume(g_event.value);
				}
			}
			break;
	case NetworkManager::GameState::SETTINGS_MENU:
		while (_window->pollEvent(event)){
			if (event.type == sf::Event::LostFocus){
				gManager.Pause(true);
			}
			else if (event.type == sf::Event::GainedFocus){
				gManager.Pause(false);
			}
		}
		if (gManager.PollEvent(g_event)){
			if (g_event.type == gui::EventType::Button){
				select_sound.play();
			}
			if (g_event.name == "back_button"){
				BindMainMenu();
			}
			else if (g_event.name == "mute_button"){
				if (play_music){
					play_music = false;
					music.stop();
				}
				else{
					play_music = true;
					int s = rand() % songs.size();
					music.openFromFile("audio/" + songs[s]);
					music.play();
				}
			}
			else if (g_event.name == "music_slider"){
				music.setVolume(g_event.value);
			}
			else if (g_event.name == "fx_slider"){
				select_sound.setVolume(g_event.value);
				win_sound.setVolume(g_event.value);
				lose_sound.setVolume(g_event.value);
			}
		}
		break;
	case NetworkManager::GameState::NETWORK_MENU:
		if (!nManager.IsConnected()){
			gManager.SetText("menu_message", "Not connected to server.");
		}
		while (_window->pollEvent(event)){
			switch (event.type){
			case sf::Event::LostFocus:
				gManager.Pause(true);
				break;
			case sf::Event::GainedFocus:
				gManager.Pause(false);
				break;
			case sf::Event::Closed:
				nManager.Disconnect();
				isRunning = false;
				break;
			case sf::Event::TextEntered:
				if (event.text.unicode < 128 && event.text.unicode != 8 && event.text.unicode != 13 && event.text.unicode != 9){
					gManager.TextBoxInput(static_cast<char>(event.text.unicode));
				}
				else if (event.text.unicode == 8){
					gManager.TextBoxInput(NULL);
				}
				else if (event.text.unicode == 9){
					if (gManager.IsSelected("id_text_box")){
						gManager.Unselect();
						gManager.Select("pass_text_box");
					}
					else if (gManager.IsSelected("pass_text_box")){
						gManager.Unselect();
						gManager.Select("lobby_text_box");
					}
					else if (gManager.IsSelected("lobby_text_box")){
						gManager.Unselect();
						gManager.Select("id_text_box");
					}
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
			if (g_event.type == gui::EventType::Button){
				select_sound.play();
			}
			if (g_event.name == "new_button" && nManager.IsConnected()){
				nManager.SetState(NetworkManager::CREATE_ID_MENU);
				gManager.BindMenu(create_id_menu);
				gManager.Select("id_text_box");
				gManager.SetText("menu_message", "Enter new id / password.");
			}
			if (g_event.name == "connect_button" && nManager.IsConnected()){
				gManager.SetText("menu_message", nManager.Join(gManager.GetText("id_text_box"), gManager.GetText("pass_text_box"), gManager.GetText("lobby_text_box")));
			}
			if (g_event.name == "back_button"){
				nManager.SetState(NetworkManager::MAIN_MENU);
				nManager.Disconnect();
				gManager.SetText("lobby_text_box", "");
				gManager.SetText("pass_text_box", "");
				BindMainMenu();
			}
			else if (g_event.name == "connect_button" && !nManager.IsConnected()){
				gManager.SetText("menu_message", nManager.Connect());
			}
			if (g_event.name == "random_button" && nManager.IsConnected()){
				gManager.SetText("lobby_text_box", nManager.GetRandomLobby());
				if (gManager.GetText("lobby_text_box") == ""){
					gManager.SetText("menu_message", "No lobbies available.");
				}
			}
			
		}
		break;
	case NetworkManager::GameState::CREATE_ID_MENU:
		while (_window->pollEvent(event)){
			switch (event.type){
			case sf::Event::LostFocus:
				gManager.Pause(true);
				break;
			case sf::Event::GainedFocus:
				gManager.Pause(false);
				break;
			case sf::Event::Closed:
				nManager.Disconnect();
				isRunning = false;
				break;
			case sf::Event::TextEntered:
				if (event.text.unicode < 128 && event.text.unicode != 8 && event.text.unicode != 13 && event.text.unicode != 9){
					gManager.TextBoxInput(static_cast<char>(event.text.unicode));
				}
				else if (event.text.unicode == 8){
					gManager.TextBoxInput(NULL);
				}
				else if (event.text.unicode == 9){
					if (gManager.IsSelected("id_text_box")){
						gManager.Unselect();
						gManager.Select("pass_text_box");
					}
					else if (gManager.IsSelected("pass_text_box")){
						gManager.Unselect();
						gManager.Select("id_text_box");
					}
				}
				break;
			case sf::Event::KeyPressed:
				if (event.key.code == sf::Keyboard::Return){
					gManager.SetText("menu_message", nManager.CreateNewID(gManager.GetText("id_text_box"), gManager.GetText("pass_text_box")));
				}
				break;
			}
		}
		if (gManager.PollEvent(g_event)){
			if (g_event.type == gui::EventType::Button){
				select_sound.play();
			}
			if (g_event.name == "create_button"){
				gManager.SetText("menu_message", nManager.CreateNewID(gManager.GetText("id_text_box"), gManager.GetText("pass_text_box")));
			}
			if (g_event.name == "back_button"){
				nManager.SetState(NetworkManager::GameState::NETWORK_MENU);
				gManager.BindMenu(network_menu);
				gManager.Select("id_text_box");
			}
		}
		break;
	case NetworkManager::GameState::IN_LOBBY:
		if (!inLobby){
			gManager.BindMenu(lobby);
			inLobby = true;
			nManager.UpdateServer(camera->pos, m_hand_count);
		}
		while (_window->pollEvent(event)){
			switch (event.type){
			case sf::Event::LostFocus:
				gManager.Pause(true);
				break;
			case sf::Event::GainedFocus:
				gManager.Pause(false);
				break;
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
		}
		if (nManager.UpdateClients()){
			nManager.GetClients();
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
			if (g_event.type == gui::EventType::Button){
				select_sound.play();
			}
			if (g_event.name == "ready_button"){
				nManager.ReadyUp();
				nManager.UpdateServer(camera->pos, 7);
			}
			if (g_event.name == "leave_button"){
				nManager.Disconnect();
				inLobby = false;
				gManager.BindMenu(network_menu);
				gManager.SetText("lobby_text_box", "");
				gManager.SetText("menu_message", "");
				BindMainMenu();
			}
		}
		break;
	case NetworkManager::GameState::IN_GAME:
		if (!inGame){
			inGame = true;
			white_light->SetPosition(glm::vec3(0.0f, 0.0f, -10.0f));
			
			if (!puzzle_mode){
				inLobby = false;
				gManager.BindMenu(game_hud);
				gManager.SetActive("end_button", true);
				gManager.SetActive("pickup_button", true);
				gManager.SetActive("text_box", true);
				EmptyHand();
				FillHand(7);
				
				ClearTiles();
			}
		}
		//multiplayer loop --connected to server--
		if (nManager.IsConnected()){
			while (_window->pollEvent(event)){
				switch (event.type){
				case sf::Event::LostFocus:
					gManager.Pause(true);
					break;
				case sf::Event::GainedFocus:
					gManager.Pause(false);
					break;
				case sf::Event::Closed:
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
				case sf::Event::MouseButtonPressed:
					if (event.mouseButton.button == sf::Mouse::Left && !gManager.CursorOverGUI()){
						//send tile to server
						if (selected_tile == -1){ break; }
						glm::vec3 h = Raycast(event.mouseButton.x, event.mouseButton.y);
						if (h != glm::vec3(0, 0, 0) && nManager.is_turn){
							h = glm::floor(h * (1.0f / 0.5f) + 0.5f) / (1.0f / 0.5f);
							bool valid = AddTempTile(h, glm::vec3(0.0f, 0.0f, glm::radians(tile_rotation)), selected_tile);
							if (!valid){
								gManager.SetActive("end_button", false);
							}
							else{
								gManager.SetActive("end_button", true);
							}
							AdjustCamera();
							tiles_placed++;
							tile_cursor->SetActive(false);
							gManager.SetText("hand0" + std::to_string(hand_position), "");
							gManager.SetActive("hand0" + std::to_string(hand_position), false);
							selected_tile = -1;
							c_hand_count--;
							float hand_pos_x = gManager.GetPosition("hand0" + std::to_string(hand_position)).x;
							for (int i = 0; i < m_hand_count; i++){
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
					UpdateTileCursor(event.mouseMove.x, event.mouseMove.y);
					break;
				case sf::Event::KeyPressed:
					if (event.key.code == sf::Keyboard::Escape){
						previous_menu = game_hud;
						nManager.SetState(NetworkManager::GameState::PAUSED);
						gManager.BindMenu(pause_menu);
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
						RemoveTile();
						if (tiles_placed == 0){
							gManager.SetActive("end_button", false);
						}
					}
					break;
				}
			}
			if (gManager.PollEvent(g_event)){
				if (g_event.type == gui::EventType::Button){
					select_sound.play();
				}
				if (g_event.name == "end_button"){
					if (tiles_placed > 0){
						SendTiles();
					}
					nManager.UpdateServer(camera->pos, c_hand_count);
				}
				if (g_event.name == "pickup_button"){
					PickUpTile();
					gManager.SetActive("pickup_button", false);
					gManager.SetActive("end_button", true);
				}
				for (int i = 0; i < m_hand_count; i++){
					if (g_event.name == "hand0" + std::to_string(i)){
						if (gManager.GetText("hand0" + std::to_string(i)) != ""){
							selected_tile = std::stoi(gManager.GetText("hand0" + std::to_string(i)));
							tile_cursor->GetChild(0)->model = renderer.LoadModel(gManager.GetText("hand0" + std::to_string(i)));
							hand_position = i;
						}
					}
				}
				if (g_event.name == "temp_button"){
					inGame = false;
					ClearChat();
					ClearTiles();
					nManager.SetState(NetworkManager::GameState::IN_LOBBY);
					gManager.BindMenu(lobby);
				}
			}
			std::string new_message = nManager.GetPendingChatMessage();
			if (new_message.size() > 0){
				AddChatMessage(new_message, true);
			}
			if (nManager.UpdateClients()){
				for (int i = 0; i < 4; i++){
					if (i < players->size()){
						if (players->at(i).IsTurn()){
							gManager.SetActive("client0" + std::to_string(i + 1), true);
							gManager.SetText("client0" + std::to_string(i + 1), "> " + players->at(i).ID() + " <");
							gManager.SetPosition("client0" + std::to_string(i + 1), sf::Vector2f(renderer.ScreenWidth() / 2, -70));
							gManager.Lerp("client0" + std::to_string(i + 1), sf::Vector2f(renderer.ScreenWidth() / 2, 10), 0.5f);
						}
						else{
							gManager.SetText("client0" + std::to_string(i + 1), players->at(i).ID());
							gManager.SetActive("client0" + std::to_string(i + 1), false);
						}
					}
					else{
						gManager.SetText("client0" + std::to_string(i + 1), "*empty*");
						gManager.SetActive("client0" + std::to_string(i + 1), false);
					}
				}

				if (nManager.is_turn){
					//gManager.SetActive("end_button", true);
					gManager.SetActive("pickup_button", true);
				}
			}
			//update tiles if needed
			if (nManager.UpdateTiles()){
				AddTiles(nManager.GetTiles());
			}
			if (tiles_placed == 0 && c_hand_count == m_hand_count){
				gManager.SetActive("pickup_button", false);
				gManager.SetActive("end_button", true);
			}
			if (nManager.game_done){
				if (c_hand_count == 0){
					win_sound.play();
					gManager.AddText(game_hud, "temp_text", renderer.ScreenWidth() / 2, renderer.ScreenHeight() * 0.33, true, "you win!", sf_purple, "SG14.ttf", 100, 1.0f);
				}
				else{
					lose_sound.play();
					gManager.AddText(game_hud, "temp_text", renderer.ScreenWidth() / 2, renderer.ScreenHeight() * 0.33, true, "you lose!", sf_purple, "SG14.ttf", 100, 1.0f);
				}
				gManager.SetTemporary("temp_text");
				gManager.AddButton(game_hud, "temp_button", 500, 75, renderer.ScreenWidth() / 2, renderer.ScreenHeight() / 2 + 200, true, "Done", b_main, b_hover, b_click, "alagard.ttf");
				gManager.SetTemporary("temp_button");

				nManager.game_done = false;
			}
		}
		else{
			if (!puzzle_mode){
				//not connected to server [offline play]
				while (_window->pollEvent(event)){
					switch (event.type){
					case sf::Event::LostFocus:
						gManager.Pause(true);
						break;
					case sf::Event::GainedFocus:
						gManager.Pause(false);
						break;
					case sf::Event::Closed:
						isRunning = false;
						break;
					case sf::Event::TextEntered:

						break;
					case sf::Event::MouseButtonPressed:
						if (event.mouseButton.button == sf::Mouse::Left && !gManager.CursorOverGUI()){
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
								c_hand_count--;
								float hand_pos_x = gManager.GetPosition("hand0" + std::to_string(hand_position)).x;
								for (int i = 0; i < m_hand_count; i++){
									if (gManager.GetPosition("hand0" + std::to_string(i)).x < hand_pos_x && gManager.GetText("hand0" + std::to_string(i)) != ""){
										gManager.Lerp("hand0" + std::to_string(i), gManager.GetPosition("hand0" + std::to_string(i)) + sf::Vector2f(70, 0), 0.5f);
									}
								}
								hand_position = -1;
								AdjustCamera();
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
						UpdateTileCursor(event.mouseMove.x, event.mouseMove.y);
						break;
					case sf::Event::KeyPressed:
						if (event.key.code == sf::Keyboard::Escape){
							previous_menu = game_hud;
							nManager.SetState(NetworkManager::GameState::PAUSED);
							gManager.BindMenu(pause_menu);
						}
						if (event.key.code == sf::Keyboard::BackSpace){
							RemoveTile();
						}
						break;
					}
				}
				if (gManager.PollEvent(g_event)){
					if (g_event.type == gui::EventType::Button){
						select_sound.play();
					}
					if (g_event.name == "end_button" && tiles_placed > 0){
						//SendTiles();
					}
					for (int i = 0; i < m_hand_count; i++){
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
				while (_window->pollEvent(event)){
					switch (event.type){
					case sf::Event::LostFocus:
						gManager.Pause(true);
						break;
					case sf::Event::GainedFocus:
						gManager.Pause(false);
						break;
					case sf::Event::Closed:
						isRunning = false;
						break;
					case sf::Event::MouseButtonPressed:
						if (event.mouseButton.button == sf::Mouse::Left && !gManager.CursorOverGUI()){
							if (selected_tile == -1){ break; }
							glm::vec3 h = Raycast(event.mouseButton.x, event.mouseButton.y);
							if (h != glm::vec3(0, 0, 0)){
								h = glm::floor(h * (1.0f / 0.5f) + 0.5f) / (1.0f / 0.5f);
								bool solved = AddTempTile(h, glm::vec3(0.0f, 0.0f, glm::radians(tile_rotation)), selected_tile);
								tiles_placed++;
								tile_cursor->SetActive(false);
								gManager.SetText("hand0" + std::to_string(hand_position), "");
								gManager.SetActive("hand0" + std::to_string(hand_position), false);
								selected_tile = -1;
								c_hand_count--;
								float hand_pos_x = gManager.GetPosition("hand0" + std::to_string(hand_position)).x;
								for (int i = 0; i < m_hand_count; i++){
									if (gManager.GetPosition("hand0" + std::to_string(i)).x < hand_pos_x && gManager.GetText("hand0" + std::to_string(i)) != ""){
										gManager.Lerp("hand0" + std::to_string(i), gManager.GetPosition("hand0" + std::to_string(i)) + sf::Vector2f(70, 0), 0.5f);
									}
								}
								hand_position = -1;
								AdjustCamera();

								if (solved && c_hand_count == 0){
									bool single_test = false;
									for (int i = 0; i < tiles.size(); i++){
										Tile *t = dynamic_cast<Tile*>(tiles[i]);
										int a_count = 0;
										for (int a = 0; a < 8; a++){
											if (!t->GetAdjacentTile(a)){
												a_count++;
											}
										}
										if (a_count == 8){
											single_test = true;
											t->material = red;
										}
									}
									if (!single_test){
										win_sound.play();
										gManager.AddText(game_hud, "temp_text", renderer.ScreenWidth() / 2, renderer.ScreenHeight() * 0.33, true, "Puzzle Solved!", sf_purple, "SG14.ttf", 100, 1.0f);
										gManager.SetTemporary("temp_text");
										gManager.AddButton(game_hud, "temp_c_button", 500, 75, renderer.ScreenWidth() / 2, renderer.ScreenHeight() / 2 + 75, true, "Continue", b_main, b_hover, b_click, "alagard.ttf");
										gManager.SetTemporary("temp_c_button");
										gManager.AddButton(game_hud, "temp_e_button", 500, 75, renderer.ScreenWidth() / 2, renderer.ScreenHeight() / 2 + 200, true, "Done", b_main, b_hover, b_click, "alagard.ttf");
										gManager.SetTemporary("temp_e_button");
									}
								}
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
						UpdateTileCursor(event.mouseMove.x, event.mouseMove.y);
						break;
					case sf::Event::KeyPressed:
						if (event.key.code == sf::Keyboard::Escape){
							previous_menu = game_hud;
							nManager.SetState(NetworkManager::GameState::PAUSED);
							gManager.BindMenu(pause_menu);
						}
						if (event.key.code == sf::Keyboard::BackSpace){
							RemoveTile();
						}
						if (event.key.code == sf::Keyboard::P){
							BindMainMenu();
						}
						break;
					}
				}
				if (gManager.PollEvent(g_event)){
					if (g_event.type == gui::EventType::Button){
						select_sound.play();
					}
					for (int i = 0; i < m_hand_count; i++){
						if (g_event.name == "hand0" + std::to_string(i)){
							if (gManager.GetText("hand0" + std::to_string(i)) != ""){
								selected_tile = std::stoi(gManager.GetText("hand0" + std::to_string(i)));
								tile_cursor->GetChild(0)->model = renderer.LoadModel(gManager.GetText("hand0" + std::to_string(i)));
								hand_position = i;
							}
						}
					}
					if (g_event.name == "temp_e_button"){
						ClearTiles();
						BindMainMenu();
					}
					if (g_event.name == "temp_c_button"){
						gManager.RemoveObject(game_hud, "temp_text");
						gManager.RemoveObject(game_hud, "temp_c_button");
						gManager.RemoveObject(game_hud, "temp_e_button");
						GeneratePuzzle();
						AdjustCamera();
					}
				}
			}
		}
		break;
	case NetworkManager::TUTORIAL:
		if (!inGame){
			white_light->SetPosition(glm::vec3(0.0f, 0.0f, -10.0f));
			tutorial_stage = 0;
			EmptyHand();
			glm::vec3 h = Raycast(renderer.ScreenWidth() / 2, renderer.ScreenHeight() / 2);
			h = glm::floor(h * (1.0f / 0.5f) + 0.5f) / (1.0f / 0.5f);
			AddTempTile(h, glm::vec3(0.0f, 0.0f, glm::radians(0.0f)), 5);
			for (int i = 0; i < tiles.size(); i++){
				tiles[i]->shader = 0;
				tiles[i]->material = 0;
			}
			inGame = true;

			AdjustCamera();
			gManager.AddText(game_hud, "t_top", renderer.ScreenWidth() / 2, renderer.ScreenHeight() * 0.15, true, "This is an octino tile.", sf_purple, "alagard.ttf", 50, 1.0f);
			gManager.SetTemporary("t_top");
			gManager.AddText(game_hud, "t_bot", renderer.ScreenWidth() / 2, renderer.ScreenHeight() * 0.75, true, "Use them to make equations.", sf_purple, "alagard.ttf", 50, 1.0f);
			gManager.SetTemporary("t_bot");
		}
		while (_window->pollEvent(event)){
			switch (event.type){
			case sf::Event::LostFocus:
				gManager.Pause(true);
				break;
			case sf::Event::GainedFocus:
				gManager.Pause(false);
				break;
			case sf::Event::Closed:
				isRunning = false;
				break;
			case sf::Event::MouseButtonPressed:
				if (event.mouseButton.button == sf::Mouse::Left && !gManager.CursorOverGUI() && tutorial_stage >= 8){
					if (selected_tile == -1){ break; }
					glm::vec3 h = Raycast(event.mouseButton.x, event.mouseButton.y);
					if (h != glm::vec3(0, 0, 0)){
						h = glm::floor(h * (1.0f / 0.5f) + 0.5f) / (1.0f / 0.5f);
						bool solved = AddTempTile(h, glm::vec3(0.0f, 0.0f, glm::radians(tile_rotation)), selected_tile);
						tiles_placed++;
						tile_cursor->SetActive(false);
						gManager.SetText("hand0" + std::to_string(hand_position), "");
						gManager.SetActive("hand0" + std::to_string(hand_position), false);
						selected_tile = -1;
						c_hand_count--;
						float hand_pos_x = gManager.GetPosition("hand0" + std::to_string(hand_position)).x;
						for (int i = 0; i < m_hand_count; i++){
							if (gManager.GetPosition("hand0" + std::to_string(i)).x < hand_pos_x && gManager.GetText("hand0" + std::to_string(i)) != ""){
								gManager.Lerp("hand0" + std::to_string(i), gManager.GetPosition("hand0" + std::to_string(i)) + sf::Vector2f(70, 0), 0.5f);
							}
						}
						hand_position = -1;
						AdjustCamera();
						if (tutorial_stage == 8 || tutorial_stage == 9){
							tutorial_stage++;
						}
					}
				}
				if (tutorial_stage == 10){
					if (CheckTiles() && tiles.size() > 1){
						gManager.SetActive("end_button", true);
					}
					else{
						gManager.SetActive("end_button", false);
					}
				}
				if (event.mouseButton.button == sf::Mouse::Left){
					if (tutorial_stage < 8){
						tutorial_stage++;
					}
					if (tutorial_stage == 1){
						gManager.SetText("t_top", "Sides represent an equal sign.");
						gManager.SetText("t_bot", "Each colored corner serves as a different math operator.");
					}
					else if (tutorial_stage == 2){
						gManager.AddText(game_hud, "add", renderer.ScreenWidth() / 2 - 250, renderer.ScreenHeight() / 2 - 120, true, "Addition", sf_purple, "alagard.ttf", 50, 1.0f);
						gManager.SetTemporary("add");
					}
					else if (tutorial_stage == 3){
						gManager.AddText(game_hud, "sub", renderer.ScreenWidth() / 2 + 250, renderer.ScreenHeight() / 2 - 120, true, "Subtraction", sf_purple, "alagard.ttf", 50, 1.0f);
						gManager.SetTemporary("sub");
					}
					else if (tutorial_stage == 4){
						gManager.AddText(game_hud, "multi", renderer.ScreenWidth() / 2 - 250, renderer.ScreenHeight() / 2 + 75, true, "Multiplication", sf_purple, "alagard.ttf", 50, 1.0f);
						gManager.SetTemporary("multi");
					}
					else if (tutorial_stage == 5){
						gManager.AddText(game_hud, "div", renderer.ScreenWidth() / 2 + 250, renderer.ScreenHeight() / 2 + 75, true, "Division", sf_purple, "alagard.ttf", 50, 1.0f);
						gManager.SetTemporary("div");
					}
					else if (tutorial_stage == 6){
						gManager.AddText(game_hud, "temp_text", 70, renderer.ScreenHeight() - 75, false, "These are your tiles >>", text, "alagard.ttf", 50, 1.0f);
						gManager.SetTemporary("temp_text");
						gManager.SetText("t_top", "");
						gManager.SetText("t_bot", "");

						FillHand(2);
						hand.clear();
						int num[2] = { 3, 2 };
						for (int i = 0; i < 2; i++){
							hand.push_back(num[i]);
							gManager.SetText("hand0" + std::to_string(i), std::to_string(hand[i]));
						}
					}
					else if (tutorial_stage == 7){
						gManager.RemoveObject(game_hud, "add");
						gManager.RemoveObject(game_hud, "sub");
						gManager.RemoveObject(game_hud, "multi");
						gManager.RemoveObject(game_hud, "div");
						gManager.SetText("t_top", "Select a tile and hover over the board to position it.");
					}
					else if (tutorial_stage == 8){
						gManager.RemoveObject(game_hud, "temp_text");
						gManager.SetText("t_top", "Left click places a tile.");
						gManager.SetText("t_bot", "Right click rotates a tile.");
					}
					else if (tutorial_stage == 9){
						gManager.SetText("t_top", "Good Job! Complete an equation to continue.");
						gManager.SetText("t_bot", "Errors will appear red on the board.");
					}
					else if (tutorial_stage == 10){
						gManager.SetText("t_top", "Backspace will remove the last tile you placed.");
						gManager.SetText("t_bot", "Click the end turn button when done.");
					}
				}
				if (event.mouseButton.button == sf::Mouse::Right && !gManager.CursorOverGUI() && tutorial_stage >= 7){
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
					previous_menu = game_hud;
					nManager.SetState(NetworkManager::GameState::PAUSED);
					gManager.BindMenu(pause_menu);
				}
				if (event.key.code == sf::Keyboard::BackSpace){
					RemoveTile();
					if (!CheckTiles() || tiles.size() == 1){
						gManager.SetActive("end_button", false);
					}
				}
				break;
			}
		}
		if (gManager.PollEvent(g_event)){
			if (g_event.name == "end_button"){
				tutorial_stage++;
				gManager.SetActive("end_button", false);
				gManager.SetText("t_top", "");
				gManager.SetText("t_bot", "");
				gManager.AddText(game_hud, "temp_text", renderer.ScreenWidth() / 2, renderer.ScreenHeight() * 0.33, true, "complete!", header, "SG14.ttf", 120, 1.0f);
				gManager.SetTemporary("temp_text");
				gManager.AddButton(game_hud, "temp_e_button", 500, 75, renderer.ScreenWidth() / 2, renderer.ScreenHeight() / 2 + 200, true, "Back", b_main, b_hover, b_click, "alagard.ttf");
				gManager.SetTemporary("temp_e_button");
			}
			else if (g_event.name == "temp_e_button"){
				
				gManager.SetActive("end_button", true);
				gManager.SetActive("pickup_button", true);
				gManager.SetActive("text_box", true);
				ClearTiles();
				BindMainMenu();
			}
			if (tutorial_stage >= 6){
				for (int i = 0; i < m_hand_count; i++){
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
		break;
		case NetworkManager::GameState::PAUSED:
			while (_window->pollEvent(event)){
				switch (event.type){
					case sf::Event::LostFocus:
						gManager.Pause(true);
						break;
					case sf::Event::GainedFocus:
						gManager.Pause(false);
						break;
					case sf::Event::KeyPressed:
						if (event.key.code == sf::Keyboard::Escape){
							nManager.SetState(nManager.GetPreState());
							gManager.BindMenu(previous_menu);
						}
						break;
				}
			}
			if (gManager.PollEvent(g_event)){
				if (g_event.type == gui::EventType::Button){
					select_sound.play();
				}
				if (g_event.name == "back_button"){
					nManager.SetState(nManager.GetPreState());
					gManager.BindMenu(previous_menu);
				}
				else if (g_event.name == "quit_button"){
					ClearChat();
					ClearTiles();
					BindMainMenu();
				}
			}
			break;
		}
	}
}

void Game::Input(){

}

glm::vec3 Game::GetPosition(){
	return camera->pos;
}

glm::vec3 Game::Raycast(float mouseX, float mouseY){
	glm::vec2 m = glm::vec2(mouseX / renderer.ScreenWidth(), mouseY / renderer.ScreenHeight());

	glm::vec3 mts = camera->pos;
	//this contains camera props the are currently private [need to update this vv]
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
		delete(tiles[tiles.size() - 1]);
		tiles.erase(tiles.end() - 1);
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
void Game::UpdateTileCursor(int mouse_x, int mouse_y){
	if (!gManager.CursorOverGUI()){
		glm::vec3 h = Raycast(mouse_x, mouse_y);
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
}
void Game::AddTiles(std::vector<Tile*> _tiles){
	for (int i = 0; i < _tiles.size(); i++){
		tiles.push_back(_tiles[i]);
		tiles[tiles.size() - 1]->model = tile_model;
		tiles[tiles.size() - 1]->bounding_shape = new AABox(tiles[tiles.size() - 1]->position, 2.0f, 2.0f, 0.5f);
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
void Game::FillHand(int hand_size){
	m_hand_count = hand_size;
	c_hand_count = m_hand_count;
	for (int i = 0; i < m_hand_count; i++){
		//gManager.AddButton(game_hud, "hand0" + std::to_string(i), 60, 60, renderer.ScreenWidth() * 0.62 - 70 * i, renderer.ScreenHeight() - 70, false, std::to_string(i), e_main, e_hover, e_click, "alagard.ttf");
		gManager.AddButton(game_hud, "hand0" + std::to_string(i), 60, 60, renderer.ScreenWidth() * 0.62 - 70 * i, renderer.ScreenHeight() - 70, false, std::to_string(i), "alagard.ttf", "textures/tile.png");
	}
	for (int i = 0; i < m_hand_count; i++){
		if (nManager.IsConnected()){
			int t = rand() % 10;
			hand.push_back(t);
		}
		else{
			hand.push_back(i);
		}
		gManager.SetText("hand0" + std::to_string(i), std::to_string(hand[i]));
	}
}
void Game::EmptyHand(){
	for (int i = 0; i < m_hand_count; i++){
		gManager.RemoveObject(game_hud, "hand0" + std::to_string(i));
	}
	m_hand_count = 0;
	c_hand_count = m_hand_count;
	hand.clear();
}
void Game::PickUpTile(){
	if (c_hand_count != m_hand_count){
		int n = rand() % 10;
		for (int i = 0; i < m_hand_count; i++){
			if (gManager.GetText("hand0" + std::to_string(i)) == ""){
				gManager.SetActive("hand0" + std::to_string(i), true);
				gManager.SetPosition("hand0" + std::to_string(i), sf::Vector2f(renderer.ScreenWidth() * 0.62 - 70 * (m_hand_count - 1), renderer.ScreenHeight() - 70));
				c_hand_count++;
				gManager.Lerp("hand0" + std::to_string(i), sf::Vector2f(renderer.ScreenWidth() * 0.62 - 70 * (c_hand_count - 1), renderer.ScreenHeight() - 70), 0.5 * (m_hand_count - c_hand_count));
				gManager.SetText("hand0" + std::to_string(i), std::to_string(n));
				break;
			}
		}
	}
}

bool Game::CheckTiles(){
	for (int i = 0; i < tiles.size(); i++){
		tiles[i]->material = 0;
		Tile *t = dynamic_cast<Tile*>(tiles[i]);
		for (int l = 0; l < 4; l++){
			t->solved[l] = false;
		}
	}
	bool no_equals = false;
	if (tiles.size() > 1){
		bool bad_placement = false;
		no_equals = true;
		for (int i = 0; i < tiles.size(); i++){
			Tile *t = dynamic_cast<Tile*>(tiles[i]);
			for (int a = 0; a < 7; a+=2){
				int p = a + 4;
				if (p >= 8){ p -= 8; }
				if (t->GetAdjacentTile(a)){
					no_equals = false;
					std::vector<Tile*> _tiles;
					std::vector<int> equation;
					equation.push_back(t->GetAdjacentTile(a)->GetValue());
					std::vector<int> e;
					e.push_back(t->GetValue());
					
					if (TilePass(t, t->GetAdjacentTile(a), p, equation, _tiles)){
						t->solved[a / 2] = true;
						t->GetAdjacentTile(a)->solved[a / 2] = true;
					}
				}
			}
		}
		for (int i = 0; i < tiles.size(); i++){
			Tile *t = dynamic_cast<Tile*>(tiles[i]);
			bool need_check = false;
			for (int a = 0; a < 7; a += 2){
				if (t->GetAdjacentTile(a)){
					need_check = true;
					break;
				}
			}
			if (need_check){
				int s_count = 0;
				for (int l = 0; l < 4; l++){
					if (t->solved[l]){
						s_count++;
						break;
					}
				}
				if (s_count == 0){
					bad_placement = true;
					t->material = red;
				}
			}
		}
		if (no_equals){
			for (int i = 0; i < tiles.size(); i++){
				tiles[i]->material = red;
			}
		}
		if (bad_placement || no_equals){
			return false;
		}
		else{
			return true;
		}
	}
	return false;
}
bool Game::TilePass(Tile *i_tile, Tile *a_tile, int previous_index, std::vector<int> _equation, std::vector<Tile*> _tiles){
	_tiles.push_back(a_tile);
	bool test = true;
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
				if (!TilePass(i_tile, a_tile->GetAdjacentTile(i), p, _equation, _tiles)){
					test = false;
				}
				_equation.clear();
				_equation.push_back(a_tile->GetValue());
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
		//print test
		/*
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
		if (i_tile->GetValue() != r_final && _equation.size() != 1){
			return false;
		}
		//stop
		if (_equation.size() == 1){
			bool c_test = false;
			for (int i = 1; i < 8; i += 2){
				if (i_tile->GetAdjacentTile(i)){
					c_test = true;
					break;
				}
			}
			for (int i = 1; i < 8; i += 2){
				if (a_tile->GetAdjacentTile(i)){
					if (c_test){
						c_test = true;
					}
					break;
				}
			}
			if (!c_test && i_tile->GetValue() != a_tile->GetValue()){
				return false;
			}
			else if (!c_test && i_tile->GetValue() == a_tile->GetValue()){
				return true;
			}
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
			average_pos.x += tiles[i]->position.x;
			average_pos.y += tiles[i]->position.y;
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
		if (chat.size() > 10 && i < chat.size() - 10){
			gManager.RemoveObject(game_hud, "chat" + std::to_string(i));
		}
		else{
			gManager.Lerp("chat" + std::to_string(i), gManager.GetPosition("chat" + std::to_string(i)) + sf::Vector2f(0, -30), 0.1f);
		}
	}
	chat.push_back(_message);
	if (incoming_message){
		gManager.AddText(game_hud, "chat" + std::to_string(chat.size() - 1), 4, renderer.ScreenHeight() - 75, false, _message, sf::Color(9, 43, 106, 255), "TF2.ttf", 30, NULL);
	}
	else{
		gManager.AddText(game_hud, "chat" + std::to_string(chat.size() - 1), 4, renderer.ScreenHeight() - 75, false, _message, sf::Color(202, 33, 33, 255), "TF2.ttf", 30, NULL);
	}
}
void Game::ClearChat(){
	for (int i = 0; i < chat.size(); i++){
		gManager.RemoveObject(game_hud, "chat" + std::to_string(i));
	}
	chat.clear();
}

void Game::GeneratePuzzle(){
	ClearTiles();
	camera->SetPosition(glm::vec3(0.0f, 0.0f, 5.0f));
	glm::vec3 h = Raycast(renderer.ScreenWidth() / 2, renderer.ScreenHeight() / 2);
	//place inital tile
	int t_value = rand() % 10;
	h = glm::floor(h * (1.0f / 0.5f) + 0.5f) / (1.0f / 0.5f);
	AddTempTile(h, glm::vec3(0.0f, 0.0f, glm::radians(0.0f)), t_value);
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
		bool a_first = true;
		int op;
		test = false;
		while (!test){
			a = rand() % 10;
			b = rand() % 10;

			if (a + b == t->GetValue()){ test = true; op = 0; }
			else if (a - b == t->GetValue()){ test = true; a_first = false; op = 1; }
			else if (b - a == t->GetValue()){ test = true; op = 1; }
			else if (a * b == t->GetValue()){ test = true; op = 2; }
			else if (b != 0){
				if (a / b == t->GetValue()){ test = true; a_first = false; op = 3; }
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
		}
		else if (op == 1){
			if (f != 0){
				tile_rotation += 90.0f;
			}
		}
		else if (op == 2){
			if (f != 0){
				tile_rotation += 90.0f;
			}
		}
		else if (op == 3){
			if (f != 0){
				tile_rotation -= 90.0f;
			}
		}
		//rotation clamp
		if (tile_rotation >= 360.0f){ tile_rotation -= 360.0f; }
		else if (tile_rotation < 0.0f){ tile_rotation += 360.0f; }
		
		if (TileValidPlacement(t->GetAdjacentPosition(tile_slot))){
			if (a_first){
				AddTempTile(t->GetAdjacentPosition(tile_slot), glm::vec3(0.0f, 0.0f, glm::radians(tile_rotation)), a);
			}
			else{
				AddTempTile(t->GetAdjacentPosition(tile_slot), glm::vec3(0.0f, 0.0f, glm::radians(tile_rotation)), b);
			}
		}
		else{
			i--;
			continue;
		}
		
		tile_rotation = glm::degrees(tiles[tiles.size() - 1]->rotation.z) + 180.0f;
		if (tile_rotation >= 360.0f){ tile_rotation -= 360.0f; };
		
		Tile* second_tile = dynamic_cast<Tile*>(tiles[tiles.size() - 1]);
		tile_slot = second_tile->GetAdjacentIndex(op);
		if (TileValidPlacement(second_tile->GetAdjacentPosition(tile_slot))){
			bool added;
			if (a_first){
				added = AddTempTile(second_tile->GetAdjacentPosition(tile_slot), glm::vec3(0.0f, 0.0f, glm::radians(tile_rotation)), b);
			}
			else{
				added = AddTempTile(second_tile->GetAdjacentPosition(tile_slot), glm::vec3(0.0f, 0.0f, glm::radians(tile_rotation)), a);
			}
			if (!added){
				dynamic_cast<Tile*>(tiles[tiles.size() - 1])->RemoveAdjacentTiles();
				delete(tiles[tiles.size() - 1]);
				tiles.erase(tiles.end() - 1);
				second_tile->RemoveAdjacentTiles();
				delete(tiles[tiles.size() - 1]);
				tiles.erase(tiles.end() - 1);

				for (int t = 0; t < tiles.size(); t++){
					tiles[t]->material = 0;
				}

				i--;
				continue;
			}
		}
		else{
			second_tile->RemoveAdjacentTiles();
			delete(tiles[tiles.size() - 1]);
			tiles.erase(tiles.end() - 1);

			for (int t = 0; t < tiles.size(); t++){
				tiles[t]->material = 0;
			}

			i--;
			continue;
		}
	}
	
	//remove some random tiles
	std::vector<int> tiles_removed;
	for (int i = 0; i < puzzle_size * 2 - puzzle_size; i++){
		int r = rand() % tiles.size();
		dynamic_cast<Tile*>(tiles[r])->RemoveAdjacentTiles();
		tiles_removed.push_back(dynamic_cast<Tile*>(tiles[r])->GetValue());
		delete(tiles[r]);
		tiles.erase(tiles.begin() + r);
	}
	EmptyHand();
	FillHand(tiles_removed.size());
	for (int i = 0; i < m_hand_count; i++){
		hand[i]  = tiles_removed[i];
		gManager.SetText("hand0" + std::to_string(i), std::to_string(hand[i]));
	}
	for (int i = 0; i < tiles.size(); i++){
		tiles[i]->shader = 0;
		tiles[i]->material = 0;
	}
	printf("Puzzle generated!\n");
}
void Game::RemoveTile(){
	if (tiles_placed > 0){
		tiles_placed--;
		Tile *t = dynamic_cast<Tile*>(tiles[tiles.size() - 1]);
		t->RemoveAdjacentTiles();
		for (int i = 0; i < m_hand_count; i++){
			if (gManager.GetText("hand0" + std::to_string(i)) == ""){
				gManager.SetActive("hand0" + std::to_string(i), true);
				gManager.SetPosition("hand0" + std::to_string(i), sf::Vector2f(renderer.ScreenWidth() * 0.62 - 70 * (m_hand_count - 1), renderer.ScreenHeight() - 70));
				c_hand_count++;
				gManager.Lerp("hand0" + std::to_string(i), sf::Vector2f(renderer.ScreenWidth() * 0.62 - 70 * (c_hand_count - 1), renderer.ScreenHeight() - 70), 0.5 * (m_hand_count - c_hand_count));
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
void Game::ClearTiles(){
	for (std::vector<mor::GameObject*>::iterator iter = tiles.begin(); iter != tiles.end(); ++iter){
		delete((*iter));
	}
	tiles.clear();
	tiles_placed = 0;
}
void Game::BindMainMenu(){
	gManager.SetActive("end_button", true);
	gManager.SetActive("pickup_button", true);
	gManager.SetActive("text_box", true);
	nManager.SetState(NetworkManager::GameState::MAIN_MENU);
	gManager.BindMenu(main_menu);
	inGame = false;
	puzzle_mode = false;
	camera->SetPosition(glm::vec3(0.0f, 0.0f, 5.0f));
	camera->Lerp(glm::vec3(0.0f, 0.0f, 5.0f), 0.01f);
	white_light->SetPosition(glm::vec3(0.0f, 0.0f, 40.0f));
	nManager.Disconnect();
}

void Game::LoadAudio(){
	if (!s_buffer.loadFromFile("audio/sound1.wav")){
		printf("error loading sound\n");
	}
	select_sound.setBuffer(s_buffer);
	if (!w_buffer.loadFromFile("audio/powerup1.wav")){
		printf("error loading sound\n");
	}
	win_sound.setBuffer(w_buffer);
	if (!l_buffer.loadFromFile("audio/lose.wav")){
		printf("error loading sound\n");
	}
	lose_sound.setBuffer(l_buffer);

	songs.push_back("RoccoW_-_09_-_Weeklybeats_2014_9_-_This_Little_Piggy_Danced.wav");
	songs.push_back("RoccoW_-_Fuckaboing.wav");
	songs.push_back("RoccoW_-_07_-_Weeklybeats_2014_7_-_Freaking_Viking.wav");
	songs.push_back("RoccoW_-_Messeah.wav");
	songs.push_back("RoccoW_-_04_-_Weeklybeats_2014_4_-_All_Will_Be_Well.wav");
	songs.push_back("RoccoW_-_Something_Something_Knight.wav");
	songs.push_back("RoccoW_-_Pumped.wav");
	songs.push_back("RoccoW_-_Ideetje_Extended.wav");

	int s = rand() % songs.size();
	if (!music.openFromFile("audio/" + songs[s])){
		printf("error loading song\n");
	}

	music.play();
}