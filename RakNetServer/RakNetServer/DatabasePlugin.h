#include "RakPeerInterface.h"
#include "SQLite3ServerPlugin.h"

class DatabasePlugin : public RakNet::SQLite3ServerPlugin{

public:
	DatabasePlugin();
	~DatabasePlugin();

	bool CreateTable(RakNet::RakString dbIdentifier){
		// dbHandles is a member variable of SQLite3Plugin and contains the mappings of identifiers to sql database pointers
		unsigned int idx = dbHandles.GetIndexOf(dbIdentifier);
		if (idx == (unsigned int)-1)
			return false;

		// Store the identifier for the connection state table for use in OnClosedConnection and OnNewConnection
		connectionStateIdentifier = dbIdentifier;

		// Create the table.
		sqlite3_exec(
			// Pointer to sqlite instance previously added with SQLite3Plugin::AddDBHandle()
			dbHandles[idx].dbHandle,
			// Query
			"CREATE TABLE connectionState(systemAddress varchar(64),"
			"rowCreationTime timestamp DATE DEFAULT (datetime('now','localtime')),"
			"lastRowUpdateTime timestamp DATE DEFAULT (datetime('now','localtime')),"
			"rakNetGUID varchar(64))",
			// Ignore per-row callback, callback parameter, and error message destination
			0, 0, 0);

		return true;
	}
	// Implemented event callback from base class PluginInterface2
	virtual void OnClosedConnection(const RakNet::SystemAddress &systemAddress, RakNet::RakNetGUID rakNetGUID, RakNet::PI2_LostConnectionReason lostConnectionReason){
		// Call down to the base class in case it does anything in the future (right now it does nothing)
		SQLite3ServerPlugin::OnClosedConnection(systemAddress, rakNetGUID, lostConnectionReason);

		// Get the database index associated with the table used for this class
		unsigned int idx = dbHandles.GetIndexOf(connectionStateIdentifier);
		if (idx == (unsigned int)-1)
			return;

		// Remove dropped system by primary key system address
		char systemAddressString[64];
		systemAddress.ToString(true, systemAddressString);
		RakNet::RakString query("DELETE FROM connectionState WHERE systemAddress='%s';",
			RakNet::RakString(systemAddressString).SQLEscape().C_String());
		sqlite3_exec(dbHandles[idx].dbHandle, query.C_String(), 0, 0, 0);
	}

	// Implemented event callback from base class PluginInterface2
	virtual void OnNewConnection(const RakNet::SystemAddress &systemAddress, RakNet::RakNetGUID rakNetGUID, bool isIncoming){
		// Call down to the base class in case it does anything in the future (right now it does nothing)
		SQLite3ServerPlugin::OnNewConnection(systemAddress, rakNetGUID, isIncoming);

		// Get the database index associated with the table used for this class
		unsigned int idx = dbHandles.GetIndexOf(connectionStateIdentifier);
		if (idx == (unsigned int)-1)
			return;

		// Store new system's system address and guid. rowCreationTime column is created automatically
		char systemAddressString[64];
		systemAddress.ToString(true, systemAddressString);
		char guidString[128];
		rakNetGUID.ToString(guidString);
		RakNet::RakString query(
			"INSERT INTO connectionState (systemAddress,rakNetGUID) VALUES ('%s','%s');",
			RakNet::RakString(systemAddressString).SQLEscape().C_String(),
			RakNet::RakString(guidString).SQLEscape().C_String());
		sqlite3_exec(dbHandles[idx].dbHandle, query.C_String(), 0, 0, 0);
	}

	RakNet::RakString connectionStateIdentifier;
};

