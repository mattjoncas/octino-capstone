#pragma once

#include <iostream>
#include <string>
#include "sqlite3.h"

#define DATABASE_NAME "Octino.db"
#define TABLE_NAME "Clients"

class DatabaseManager{

public:
	DatabaseManager();
	~DatabaseManager();

	bool AddClient(std::string player_id, std::string player_pass);
	bool CheckID(std::string player_id);
	bool Login(std::string player_id, std::string player_pass);

	bool AddWin(std::string player_id);

	void PrintDatabase();
private:
	sqlite3 *db;

	bool OpenDatabase();
	void CloseDatabase();
	bool CreateTable();

	int rc;
	char *error;
};

