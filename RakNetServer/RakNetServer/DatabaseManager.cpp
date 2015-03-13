#include "DatabaseManager.h"


DatabaseManager::DatabaseManager(){
	if (OpenDatabase()){
		CreateTable();

		if (CheckID("Matt")){
			printf("Client ID already in database.\n");
			if (Login("Matt", "password")){
				printf("login returned true!\n");
			}
			else{
				printf("login failed.\n");
			}
		}
		PrintDatabase();
	}
}

DatabaseManager::~DatabaseManager(){
	CloseDatabase();
}

bool DatabaseManager::OpenDatabase(){
	std::cout << "Opening database ...\n";
	
	rc = sqlite3_open(DATABASE_NAME, &db);
	if (rc){
		std::cerr << "Error opening SQLite3 database: " << sqlite3_errmsg(db) << std::endl << std::endl;
		sqlite3_close(db);
		return false;
	}
	std::cout << "Opened database.\n\n";
	return true;
}
void DatabaseManager::CloseDatabase(){
	std::cout << "Closing database ...\n";
	sqlite3_close(db);
	std::cout << "Closed database.\n\n";
}
bool DatabaseManager::CreateTable(){
	std::cout << "Creating table ...\n";
	std::string s = "CREATE TABLE ";
	s += TABLE_NAME;
	s += " (id STRING PRIMARY KEY, pass STRING, wins INTEGER);";
	const char *sqlCreateTable = "CREATE TABLE Clients (id STRING PRIMARY KEY, pass STRING, wins INTEGER);";
	rc = sqlite3_exec(db, s.c_str(), NULL, NULL, &error);
	if (rc){
		std::cerr << "Error executing SQLite3 statement: " << sqlite3_errmsg(db) << "\n\n";
		sqlite3_free(error);
		return false;
	}
	std::cout << "Created table.\n\n";
	return true;
}

bool DatabaseManager::AddClient(std::string player_id, std::string player_pass){
	std::string s = "INSERT INTO ";
	s += TABLE_NAME;
	s += " VALUES ('";
	s += player_id;
	s += "', '";
	s += player_pass;
	s += "', 0);";
	const char *sqlInsert = "INSERT INTO MyTable VALUES (2, 'hmmm');";
	rc = sqlite3_exec(db, s.c_str(), NULL, NULL, &error);
	if (rc){
		std::cerr << "Error inserting SQLite3 statement: " << sqlite3_errmsg(db) << "\n\n";
		sqlite3_free(error);
		return false;
	}
	std::cout << "Insert into Table complete." << std::endl << std::endl;
	return true;
}
bool DatabaseManager::CheckID(std::string player_id){
	std::string s = "SELECT * FROM ";
	s += TABLE_NAME;
	s += " WHERE ";
	s += "id = '";
	s += player_id;
	s += "';";

	char **results = NULL;
	int rows, columns;
	rc = sqlite3_get_table(db, s.c_str(), &results, &rows, &columns, &error);
	if (rc){
		std::cerr << "Error executing SQLite3 query: " << sqlite3_errmsg(db) << std::endl << std::endl;
		sqlite3_free(error);
	}
	if (rows == 0){ return false; }
	return true;
}
bool DatabaseManager::Login(std::string player_id, std::string player_pass){
	std::string s = "SELECT * FROM ";
	s += TABLE_NAME;
	s += " WHERE ";
	s += "id = '";
	s += player_id;
	s += "' AND pass ='";
	s += player_pass;
	s += "';";

	char **results = NULL;
	int rows, columns;
	rc = sqlite3_get_table(db, s.c_str(), &results, &rows, &columns, &error);
	if (rc){
		std::cerr << "Error executing SQLite3 query: " << sqlite3_errmsg(db) << std::endl << std::endl;
		sqlite3_free(error);
	}
	if (rows == 0){ return false; }
	return true;
}

bool DatabaseManager::AddWin(std::string player_id){
	std::string s = "SELECT wins FROM ";
	s += TABLE_NAME;
	s += " WHERE ";
	s += "id = '";
	s += player_id;
	s += "';";

	char **results = NULL;
	int rows, columns;
	rc = sqlite3_get_table(db, s.c_str(), &results, &rows, &columns, &error);
	if (rc){
		std::cerr << "Error retrieving wins from SQLite3 query: " << sqlite3_errmsg(db) << std::endl << std::endl;
		sqlite3_free(error);
	}
	if (rows == 0){ 
		std::cout << "cannot add win, player id not in database.\n";
		return false; 
	}

	int c_wins = std::stoi(results[rows * columns]);
	std::cout << "wins converted to int = " << c_wins << "\n";
	c_wins++;
	//update players wins
	s = "UPDATE ";
	s += TABLE_NAME;
	s += " SET wins = ";
	s += std::to_string(c_wins);
	s += " WHERE ";
	s += "id = '";
	s += player_id;
	s += "';";
	rc = sqlite3_get_table(db, s.c_str(), &results, &rows, &columns, &error);
	if (rc){
		std::cerr << "Error updating player wins: " << sqlite3_errmsg(db) << std::endl << std::endl;
		sqlite3_free(error);
	}
	return true;
}

void DatabaseManager::PrintDatabase(){
	std::cout << "Retrieving values in Client Table ..." << std::endl;
	std::string s = "SELECT * FROM ";
	s += TABLE_NAME;
	s += ";";
	const char *sqlSelect = "SELECT * FROM MyTable;";
	char **results = NULL;
	int rows, columns;
	rc = sqlite3_get_table(db, s.c_str(), &results, &rows, &columns, &error);
	if (rc){
		std::cerr << "Error executing SQLite3 query: " << sqlite3_errmsg(db) << std::endl << std::endl;
		sqlite3_free(error);
	}
	else{
		// Display Table
		for (int rowCtr = 0; rowCtr <= rows; ++rowCtr){
			for (int colCtr = 0; colCtr < columns; ++colCtr){
				// Determine Cell Position
				int cellPosition = (rowCtr * columns) + colCtr;

				// Display Cell Value
				std::cout.width(12);
				std::cout.setf(std::ios::left);
				std::cout << results[cellPosition] << " ";
			}

			// End Line
			std::cout << std::endl;

			// Display Separator For Header
			if (0 == rowCtr){
				for (int colCtr = 0; colCtr < columns; ++colCtr){
					std::cout.width(12);
					std::cout.setf(std::ios::left);
					std::cout << "~~~~~~~~~~~~ ";
				}
				std::cout << "\n\n";
			}
		}
	}
	sqlite3_free_table(results);
}