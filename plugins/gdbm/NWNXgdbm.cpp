/******************************************************************************

  NWNXgdbm.cpp - Implementation of GDBM plugin for NWNX.

  Copyright 2012-2013 eeriegeek (eeriegeek@yahoo.com)

  This file is part of NWNX.

  NWNX is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  NWNX is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with NWNX.  If not, see <http://www.gnu.org/licenses/>.

******************************************************************************/

#include "NWNXgdbm.h"

#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <libgen.h>

// Access to the global gdbm plugin instance for class variables and logging.
//
extern CNWNXgdbm gdbm;

// The NWNX request mechanism requires all arguments to be passed in a single
// string. Since the strings are probably not NULL safe, alternate separators
// are used. One for argument lists and one for compound data keys. For this
// module, I chose to use the ASCII record separator (RS, 0x1e) and unit
// separator (US, 0x1f) characters. This means that these character's may not
// be used in actual data. One safe and reasonable approach would be to filter
// all user input fields to remove ASCII characters below 0x20 (the space)
// with exceptions for any whitespace (tab, nl, etc.) which might be acceptable.
//
static const char NWNX_GDBM_ARGSEP = '\x1e';
static const char NWNX_GDBM_KEYSEP = '\x1f';

static char* split_on_char (char* tokstr, char** toklst, unsigned int toklstlen, char toksep);

//
// The CNWNXgdbm plugin derives from CNWNXBase, the base NWNX plugin class.
// A few things need to get set up, the confKey needs set, and the OnRequest
// method must be overridden, setting up configuration handling and logging.
//

CNWNXgdbm::CNWNXgdbm() :
	server_home(NULL),
	cfg_filepath(NULL),
	cfg_sync_write(true),
	cfg_lock_file(false),
	cfg_use_scorco(true),
	gdbmpool(NULL)
{
	// Set the NWNX configuration key (protected superclass member.)
	confKey = "GDBM";
}

CNWNXgdbm::~CNWNXgdbm()
{
	delete gdbmpool;
	free(cfg_filepath);
	free(server_home);
}

bool CNWNXgdbm::OnCreate (gline *config, const char* LogDir)
{
	// Start my log file via the base class method
	//
	char tmp_filepath[FILENAME_MAX+1];
	snprintf(tmp_filepath, FILENAME_MAX, "%s/nwnx_gdbm.txt", LogDir);
	if ( !CNWNXBase::OnCreate(config, tmp_filepath) ) return false;
	Log(3,"D: GDBM: OnCreate Method called, LogDir [%s], log file [%s]\n", LogDir, tmp_filepath);

	Log(0,"NWNX - GDBM Plugin Version 2.0.0\n");
	Log(0,"Copyright 2012-2013 eeriegeek (eeriegeek@yahoo.com)\n");
	Log(0,"Distributed under the terms of the GNU General Public License.\n");
	Log(0,"Visit http://www.nwnx.org for more information.\n");

	// Save the server home directory, modern Linux specific method.
	//
	ssize_t len = readlink("/proc/self/exe", tmp_filepath, FILENAME_MAX);
	if (len == -1) {
		Log(0,"E: Unable to determine server working directory. %s\n", strerror(errno));
		return false;
	} else {
		tmp_filepath[len] = '\0';
		char* dir = dirname(tmp_filepath);
		if (dir == NULL) {
			Log(0,"E: Unable to determine server working directory. %s\n", strerror(errno));
			return false;
		} else {
			server_home = strcpy((char*)malloc(strlen(dir)+1), dir);
			Log(2,"I: Found server executable home directory [%s]\n", server_home);
		}
	}

	// Read the [GDBM] configuration file section from nwnx2.ini
	//
	if(!nwnxConfig->exists(confKey)) {
		Log(1,"W: GDBM configuration section [%s] not found in nwnx2.ini, using default values.\n", confKey);
	} else {

		char* ptr = NULL;

		// cfg_filepath, default is will be "database" subdir of server home
		//
		ptr = (char*)((*nwnxConfig)[confKey]["filepath"].c_str());
		if ((ptr == NULL)||(strcmp(ptr,"")==0)) {
			Log(2,"I: No filepath found in GDBM configuration section in nwnx2.ini. Will try default location.\n");
		} else {
			cfg_filepath = strcpy((char*)malloc(strlen(ptr)+1),ptr);
			Log(2,"I: Found filepath [%s] in GDBM configuration section in nwnx2.ini.\n", cfg_filepath);
		}

		// cfg_sync_write, default is 1
		//
		ptr = (char*)((*nwnxConfig)[confKey]["sync_write"].c_str());
		if ((ptr!=0)&&(ptr[0]=='0')&&(ptr[1]=='\x00')) cfg_sync_write = false;

		// cfg_lock_file, default is 0
		//
		ptr = (char*)((*nwnxConfig)[confKey]["lock_file"].c_str());
		if ((ptr!=0)&&(ptr[0]=='1')&&(ptr[1]=='\x00')) cfg_lock_file = true;

		// cfg_use_scorco, default is 1
		//
		ptr = (char*)((*nwnxConfig)[confKey]["use_scorco"].c_str());
		if ((ptr!=0)&&(ptr[0]=='0')&&(ptr[1]=='\x00')) cfg_use_scorco = false;

	}

	// Set to defaults if nothing was specifed in .ini file section.
	//
	if (cfg_filepath == NULL) {
		strcpy(tmp_filepath, server_home);
		strcat(tmp_filepath, "/database");
		cfg_filepath = strcpy((char*)malloc(strlen(tmp_filepath)+1), tmp_filepath);
	}

	struct stat s;
	if (stat(cfg_filepath,&s)==0) {
		if (!S_ISDIR(s.st_mode)) {
			Log(0,"E: database filepath [%s] is not a directory.\n", cfg_filepath);
			return false;
		}
	} else {
		Log(0,"E: Cannot stat database filepath [%s]. %s\n", cfg_filepath,strerror(errno));
		return false;
	}

	Log(0,"I: Directory path [%s] for physical GDBM database files.\n", cfg_filepath);
	Log(0,"I: Settings: sync_write [%d], lock_file [%d], use_scorco [%d]\n", cfg_sync_write, cfg_lock_file, cfg_use_scorco);

	// Create the GDBM file pool object
	//
	gdbmpool = new GdbmPool(cfg_filepath, cfg_sync_write, cfg_lock_file);

	// This plugin depends on the SCO/RCO services registerd by the ODMBC
	// driver. Because of this dependency, this event callback is used
	// to ensure that our (the GDBM) callbacks are only registerd after
	// the ODMBC service is available.
	//
	if ( cfg_use_scorco ) {
		HANDLE handlePluginsLoaded = HookEvent("NWNX/Core/PluginsLoaded", PluginsLoaded_EventHandler);
		if ( ! handlePluginsLoaded ) {
			Log(0,"E: Unable to register NWNX/Core/PluginsLoaded Handler.\n");
			cfg_use_scorco = false;
		}
	}

	return true;
}

char* CNWNXgdbm::OnRequest (char* gameObject, char* Request, char* Parameters)
{
	char* pResult = NULL;

	Log(3,"D: --- REQUEST ---\n");
	Log(3,"D: OnRequest: Request [%#0.8x][%s], Parameters [%#0.8x][%s]\n", Request, Request, Parameters, Parameters);

		 if (strcmp(Request,"FETCH"	    )==0) pResult = Fetch(gameObject,Parameters);
	else if (strcmp(Request,"STORE"	    )==0) pResult = Store(gameObject,Parameters);
	else if (strcmp(Request,"EXISTS"	)==0) pResult = Exists(gameObject,Parameters);
	else if (strcmp(Request,"DELETE"	)==0) pResult = Delete(gameObject,Parameters);
	else if (strcmp(Request,"SETOBJARG" )==0) pResult = SetObjectArguments(gameObject,Parameters);
	else if (strcmp(Request,"OPEN"	    )==0) pResult = Open(gameObject,Parameters);
	else if (strcmp(Request,"SYNC"	    )==0) pResult = Sync(gameObject,Parameters);
	else if (strcmp(Request,"CLOSE"	    )==0) pResult = Close(gameObject,Parameters);
	else if (strcmp(Request,"CLOSEALL"  )==0) pResult = CloseAll(gameObject,Parameters);
	else if (strcmp(Request,"CREATE"	)==0) pResult = Create(gameObject,Parameters);
	else if (strcmp(Request,"REORGANIZE")==0) pResult = Reorganize(gameObject,Parameters);
	else if (strcmp(Request,"DESTROY"   )==0) pResult = Destroy(gameObject,Parameters);
	else { 
		Log(0,"E: unrecognized GDBM operation [%s] requested\n",Request);
		return pResult;
	}

	Log(3,"D: OnRequest: Parameters [%#0.8x][%s], Result [%#0.8x][%s]\n", Parameters, Parameters, pResult, pResult);

	// A return value of NULL tells NWNX that it shouldn't copy any data. If there is
	// a return value it will be passed by PLUGIN code overwriting the Parameters string,
	// in that case the result string must be shorter than the Parameter string!
	// A non-zero pointer returned here tells NWNX to manage the return result for us
	// including copying the contents of the string and reallocating or freeing memory as
	// necessary. The format is a \0 terminated string. NWNX will call free on this pointer.
	
	return pResult;
}

bool CNWNXgdbm::OnRelease()
{
	// Does this ever happen?
	Log(2,"I: OnRelease: Called\n");
	return CNWNXBase::OnRelease();
}

//
// Implementation of Plugin Methods
//

//
// Database Lifecycle Methods
//

// Arg0 -> the name of the database
// Arg1 -> truncate flag (default = 0 (off))
//
// Ret  -> if successful 1, otherwise 0
//
char* CNWNXgdbm::Create(char* gameObject, char* Parameters) 
{
	if (!Parameters) { Log(0,"E: Missing ALL arguments to request\n"); return NULL; }
	char* arg = strdup(Parameters);
	char* arglst[2];
	if ( !ParseDelimitedArgumentList(arg, arglst, 2) ) { free(arg); return NULL; }
	strcpy(Parameters,"0");

	char* db_name    = arglst[0];
	bool db_truncate = (*arglst[1]=='1');

	Log(3,"D: CREATE: calling create, db_name [%s], db_truncate [%d]\n", db_name, db_truncate);
	if ( gdbmpool->create(db_name, db_truncate) ) {
		Log(2,"I: CREATE: create, db_name [%s], db_truncate [%d], create OK\n", db_name, db_truncate);
		strcpy(Parameters,"1");
	} else {
		Log(0,"E: CREATE: create, db_name [%s], db_truncate [%d], failed\n", db_name, db_truncate);
	}

	free(arg);
	return NULL;
}

// Arg0 -> the name of the database
//
// Ret  -> nothing
//
char* CNWNXgdbm::Reorganize(char* gameObject, char* Parameters)
{
	if ( !Parameters ) { Log(0,"E: Missing ALL arguments to request\n"); return NULL; }
	if ( strlen(Parameters)<=0 ) { Log(0,"E: Missing argument to request\n"); return NULL; }

	char* db_name = Parameters;

	Log(2,"I: REORGANIZE: calling reorganize, db_name [%s]\n", db_name);
	gdbmpool->reorganize(db_name);

	strcpy(Parameters,"");
	return NULL;
}

// Arg0 -> the name of the database
//
// Ret  -> 1 if unlink succeeds, 0 if anything goes wrong
//
char* CNWNXgdbm::Destroy(char* gameObject, char* Parameters)
{
	if ( !Parameters ) { Log(0,"E: Missing ALL arguments to request\n"); return NULL; }
	if ( strlen(Parameters)<=0 ) { Log(0,"E: Missing argument to request\n"); return NULL; }

	char* db_name = Parameters;

	Log(2,"I: DESTROY: calling destroy, db_name [%s]\n", db_name);
	if ( gdbmpool->destroy(db_name) ) {
		Log(2,"I: DESTROY: db_name [%s], destroy OK\n", db_name);
		strcpy(Parameters,"1");
	} else {
		Log(0,"E: DESTROY: db_name [%s], destroy failed\n", db_name);
		strcpy(Parameters,"0");
	}

	return NULL;
}

// Arg0 -> the name of the database
// Arg1 -> create flag (default = 1 (on))
// Arg2 -> read-only flag (default = 0 (off))
//
// Ret  -> boolean -> 1 if successful open (or was already open), 0 if failed
//
// Added GDBM_SYNC option to config file, But after a quick test, I couldn't
// see a difference in speed over 10000 inserts with GDBM_SYNC on, so I'll
// leave it on as the default for now. Hopefilly it will protect databases a bit
// more if the server crashes.
//
char* CNWNXgdbm::Open(char* gameObject, char* Parameters) 
{
	if ( !Parameters ) { Log(0,"E: Missing ALL arguments to request\n"); return NULL; }
	char* arg = strdup(Parameters);
	char* arglst[3];
	if ( !ParseDelimitedArgumentList(arg, arglst, 3) ) { free(arg); return NULL; }
	strcpy(Parameters,"0");

	char* db_name    = arglst[0];
	bool db_create   = (*arglst[1]=='1');
	bool db_readonly = (*arglst[2]=='1');

	Log(2,"I: OPEN: calling open, db_name [%s], db_create [%d], db_readonly [%d]\n", db_name, db_create, db_readonly);
	if ( gdbmpool->open(db_name, db_create, db_readonly) ) {
		Log(2,"I: OPEN: open, db_name [%s], open OK\n", db_name);
		strcpy(Parameters,"1");
	} else {
		Log(0,"E: OPEN: open db_name [%s], open failed\n", db_name);
	}

	free(arg);
	return NULL;
}

// Arg0 -> the name of the database
//
// Ret  -> nothing
//
char* CNWNXgdbm::Sync(char* gameObject, char* Parameters) 
{
	if ( !Parameters ) { Log(0,"E: Missing ALL arguments to request\n"); return NULL; }
	if ( strlen(Parameters)<=0 ) { Log(0,"E: Missing argument to request\n"); return NULL; }

	char* db_name = Parameters;

	Log(2,"I: SYNC: calling sync, db_name [%s]\n", db_name);
	gdbmpool->sync(db_name);

	strcpy(Parameters,"");
	return NULL;
}

// Arg0 -> the name of the database
//
// Ret  -> nothing
//
char* CNWNXgdbm::Close(char* gameObject, char* Parameters) 
{
	if ( !Parameters ) { Log(0,"E: Missing ALL arguments to request\n"); return NULL; }
	if ( strlen(Parameters)<=0 ) { Log(0,"E: Missing argument to request\n"); return NULL; }

	char* db_name = Parameters;

	Log(2,"I: CLOSE: db_name [%s]\n", db_name);
	gdbmpool->close(db_name);

	strcpy(Parameters,"");
	return NULL;
}

// Args - nothing
//
// Ret  - nothing
//
char* CNWNXgdbm::CloseAll(char* gameObject, char* Parameters)
{
	if ( !Parameters ) { Log(0,"E: Missing ALL arguments to request\n"); return NULL; }

	Log(2,"I: CLOSEALL\n");
	gdbmpool->close_all();

	strcpy(Parameters,"");
	return NULL;
}

//
// Data Manipulation Methods
//

// Arg0   -> the name of the database
// Arg1   -> replace flag (default = 1 (TRUE))
// Arg2   -> open flag (default = 1 (TRUE))
// Arg3   -> the key
// Arg4   -> the value
// Return -> TRUE ("1") on successful insert, if replace flag = "0"
//           FALSE is returned for collision and value not replaced
//
char* CNWNXgdbm::Store(char* gameObject, char* Parameters)
{
	if ( !Parameters ) { Log(0,"E: Missing ALL arguments to request\n"); return NULL; }
	char* arg = strdup(Parameters);
	char* arglst[5];
	if ( !ParseDelimitedArgumentList(arg, arglst, 5) ) { free(arg); return NULL; }
	strcpy(Parameters,"0");

	char* db_name   = arglst[0];
	bool db_replace = (*arglst[1]=='1');
	bool db_open    = (*arglst[2]=='1');
	gdbm_datum_t db_key(arglst[3], arglst[3] + strlen(arglst[3]) + 1);
	//Experimental: test storing keys with separator transformed to null char.
	//for (size_t i=0; i<db_key.size(); i++) if (db_key[i]==NWNX_GDBM_KEYSEP) db_key[i]='\0';

	// For the value, assume a C string, and make sure to store the null byte.
	// Object, with potentially embedded nulls, are stored through SCO/RCO.
	char*  db_value      = arglst[4];
	size_t db_value_size = strlen(db_value) + 1;

	Log(3,"D: STORE: calling store, db_name [%s], db_replace [%d], db_open [%d]\n", db_name, db_replace, db_open);
	Log(3,"D: STORE: calling store, db_key [%s], db_value [%s][%u]\n", arglst[3], db_value, db_value_size );
	if ( gdbmpool->store(db_name, db_key, db_value, db_value_size, db_replace, db_open) ) {
		Log(3,"D: STORE: store, db_name [%s], db_key [%s], key-value inserted/updated, OK\n", db_name, arglst[3]);
		strcpy(Parameters,"1");
	} else {
		Log(3,"D: STORE: store, db_name [%s], db_key [%s], key collision or error, value not inserted/updated\n", db_name, arglst[3]);
	}

	free(arg);
	return NULL;
}

// Arg0 - the name of the database
// Arg1 - open flag
// Arg2 - the key
//
// Ret  - 1 if key exists
//
char* CNWNXgdbm::Exists(char* gameObject, char* Parameters)
{
	if ( !Parameters ) { Log(0,"E: Missing ALL arguments to request\n"); return NULL; }
	char* arg = strdup(Parameters);
	char* arglst[3];
	if ( !ParseDelimitedArgumentList(arg, arglst, 3) ) { free(arg); return NULL; }
	strcpy(Parameters,"0");

	char* db_name = arglst[0];
	bool db_open  = (*arglst[1]=='1');
	gdbm_datum_t db_key(arglst[2], arglst[2] + strlen(arglst[2]) + 1);

	Log(3,"D: EXISTS: calling exists, db_name [%s], db_open, [%d], db_key [%s]\n", db_name, db_open, arglst[2]);
	if ( gdbmpool->exists(db_name, db_key, db_open) ) {
		Log(3,"D: EXISTS: exists, db_name [%s], db_key [%s], key found, OK\n", db_name, arglst[2]);
		strcpy(Parameters,"1");
	} else {
		Log(3,"D: EXISTS: exists, db_name [%s], db_key [%s], key not found, OK\n", db_name, arglst[2]);
	}

	free(arg);
	return NULL;
}

// Arg0   -> database name
// Arg1   -> open flag
// Arg2   -> the key
// Return -> the value as a string, NULL if not found or error
//
char* CNWNXgdbm::Fetch(char* gameObject, char* Parameters)
{
	if ( !Parameters ) { Log(0,"E: Missing ALL arguments to request\n"); return NULL; }
	char* arg = strdup(Parameters);
	char* arglst[3];
	if ( !ParseDelimitedArgumentList(arg, arglst, 3) ) { free(arg); return NULL; }
	strcpy(Parameters,"");

	char* db_name = arglst[0];
	bool db_open  = (*arglst[1]=='1');
	gdbm_datum_t db_key(arglst[2], arglst[2] + strlen(arglst[2]) + 1);

	char* db_value = NULL;
	size_t db_value_size = 0;

	Log(3,"D: FETCH: calling fetch, db_name [%s], db_open, [%d], db_key [%s]\n", db_name, db_open, arglst[2]);
	if ( gdbmpool->fetch(db_name, db_key, &db_value, &db_value_size, db_open) ) {
		Log(3,"D: FETCH: fetch, db_name [%s], db_key [%s], fetch OK\n", db_name, arglst[2]);
		// If fetch is true, db_value and db_size should contain the result with
		// db_value a pointer to malloc'ed memory. We will return this pointer
		// and ignore the size parameter, assuming a null terminated C string.
		// The store method should save the null terminator with its string data.
	} else {
		Log(3,"D: FETCH: fetch, db_name [%s], db_key [%s], key not found\n", db_name, arglst[2]);
		// The Parameter string is set to the empty string and NULL returned
		// for empty results (or error conditions.)
	}

	free(arg);
	return db_value;
}

// Arg0   -> database name
// Arg1   -> open flag
// Arg2   -> the key
// Return -> 1 if the key was found, 0 if dne (via Parameters)
//
char* CNWNXgdbm::Delete(char* gameObject, char* Parameters)
{
	if ( !Parameters ) { Log(0,"E: Missing ALL arguments to request\n"); return NULL; }
	char* arg = strdup(Parameters);
	char* arglst[3];
	if ( !ParseDelimitedArgumentList(arg, arglst, 3) ) { free(arg); return NULL; }
	strcpy(Parameters,"0");

	char* db_name = arglst[0];
	bool db_open  = (*arglst[1]=='1');
	gdbm_datum_t db_key(arglst[2], arglst[2] + strlen(arglst[2]) + 1);

	Log(3,"D: DELETE: calling erase, db_name [%s], db_open, [%d], db_key [%s]\n", db_name, db_open, arglst[2]);
	if ( gdbmpool->erase(db_name, db_key, db_open) ) {
		Log(3,"D: DELETE: erase, db_name [%s], db_key [%s], delete OK\n", db_name, arglst[2]);
		strcpy(Parameters,"1");
	} else {
		Log(3,"D: DELETE: erase, db_name [%s], db_key [%s], key not found\n", db_name, arglst[2]);
	}

	free(arg);
	return NULL;
}

//
// Object handling methods.
//

//  Event handler for NWNX plugin API initialization event.
//
int CNWNXgdbm::PluginsLoaded_EventHandler(WPARAM p, LPARAM a)
{
	// Register GDBM plugin event callbacks with the NWNX plugin API
	// requesting notification on SCO and RCO events.
	//
	HANDLE handleSCO = HookEvent("NWServer/SCO", SCO_EventHandler);
	HANDLE handleRCO = HookEvent("NWServer/RCO", RCO_EventHandler);
	if ( !handleSCO || !handleRCO ) {
		gdbm.Log(0,"E: Unable to register NWServer/SCO and/or NWServer/RCO event handlers.\n");
		gdbm.cfg_use_scorco = false;
	}
	gdbm.Log(2,"I: NWServer/SCO and NWServer/RCO event handlers registered without error.\n");
	return 0;
}

// Arg0   -> database name
// Arg1   -> replace flag (default = 1 (TRUE))
// Arg2   -> open flag (default = 1 (TRUE))
// Arg3   -> the key
// Return -> nothing
//
char* CNWNXgdbm::SetObjectArguments(char* gameObject, char* Parameters)
{
	if ( !cfg_use_scorco ) {
		Log(0,"E: Attempt to call SETOBJARG when SCO/RCO handler is disabled.\n");
		return NULL;
	}

	if ( !Parameters ) { Log(0,"E: Missing ALL arguments to request\n"); return NULL; }
	char* arg = strdup(Parameters);
	char* arglst[4];
	if ( !ParseDelimitedArgumentList(arg, arglst, 4) ) { free(arg); return NULL; }

	sco_rco_arg_db_name    = arglst[0];
	sco_rco_arg_db_replace = (*arglst[1]=='1');
	sco_rco_arg_db_open    = (*arglst[2]=='1');

	gdbm_datum_t db_key(arglst[3], arglst[3] + strlen(arglst[3]) + 1);
	sco_rco_arg_db_key = db_key;

	strcpy(Parameters,"");
	return NULL;
}

// These are the implementations of the SCO (StoreCampaignObject) and RCO
// (RestoreCampaignObject) event handlers for the GDBM plugin. They are
// callback functions registered with the new NWNX plugin API to be notified
// on "SCO" or "RCO" events published by the ODMBC plugin.
//
int CNWNXgdbm::SCO_EventHandler(WPARAM p, LPARAM a)
{
	if ( !gdbm.cfg_use_scorco ) {
		gdbm.Log(0,"E: Attempt to call SCO when SCO/RCO handler is disabled.\n");
		return NULL;
	}

	// Get the callback event structure and check if the event is for this plugin.
	//
	SCORCOStruct* s = (SCORCOStruct*) p;
	gdbm.Log(3,"D: SCO_EventHandler: event notification for service [%s].\n", s->key);
	if ( strcmp("GDBM", s->key) != 0 ) return 0; // Decline event if not for GDBM plugin
	gdbm.Log(3,"D: SCO_EventHandler: event notification for GDBM.\n");

	// Check that valid arguments were set by a prior call to SETOBJARG.
	//
	if ( (gdbm.sco_rco_arg_db_name.length()==0) || (gdbm.sco_rco_arg_db_key.size()==0) ) {
		gdbm.Log(0,"E: SCO_EventHandler: event notification with missing SCO/RCO Argument.\n");
		return 1; // Indicate the event was accepted by this handler
	}
	gdbm.Log(3,"D: SCO_EventHandler: DB [%s], Key [%s].\n", gdbm.sco_rco_arg_db_name.c_str(), &gdbm.sco_rco_arg_db_key[0]);
	
	// Store the object
	//
	gdbm.Log(3,"D: SCO_EventHandler: Storing Object [%s], Length [%d].\n", s->pData, s->size);
	bool ok = gdbm.gdbmpool->store(
		gdbm.sco_rco_arg_db_name.c_str(),
		gdbm.sco_rco_arg_db_key,
		(const char*)s->pData, s->size,
		gdbm.sco_rco_arg_db_replace, gdbm.sco_rco_arg_db_open
	);
	if ( ok ) {
		gdbm.Log(3,"D: SCO_EventHandler: db_name [%s], db_key [%s], OBJECT inserted/updated\n",
			gdbm.sco_rco_arg_db_name.c_str(), &gdbm.sco_rco_arg_db_key[0]);
	} else {
		gdbm.Log(3,"D: SCO_EventHandler: db_name [%s], db_key [%s], OBJECT was NOT inserted/updated, already exists\n",
			gdbm.sco_rco_arg_db_name.c_str(), &gdbm.sco_rco_arg_db_key[0]);
	}

	return 1; // Indicate the event was accepted by this handler
}

int CNWNXgdbm::RCO_EventHandler(WPARAM p, LPARAM a)
{
	if ( !gdbm.cfg_use_scorco ) {
		gdbm.Log(0,"E: Attempt to call RCO when SCO/RCO handler is disabled.\n");
		return NULL;
	}

	// Get the callback event structure and check if the event is for this plugin.
	//
	SCORCOStruct* s = (SCORCOStruct*) p;
	gdbm.Log(3,"D: RCO_EventHandler: event notification for service [%s].\n", s->key);
	if ( strcmp("GDBM", s->key) != 0 ) return 0; // Decline event if not for GDBM plugin
	gdbm.Log(3,"D: RCO_EventHandler: event notification for GDBM.\n");

	// Set return parameters for error conditions.
	//
	s->pData = NULL;
	s->size = 0;

	// Check that valid arguments were set by a prior call to SETOBJARG.
	//
	if ( (gdbm.sco_rco_arg_db_name.length()==0) || (gdbm.sco_rco_arg_db_key.size()==0) ) {
		gdbm.Log(0,"E: RCO_EventHandler: event notification with missing SCO/RCO Argument.\n");
		return 1;
	}
	gdbm.Log(3,"D: RCO_EventHandler: DB [%s], Key [%s].\n", gdbm.sco_rco_arg_db_name.c_str(), &gdbm.sco_rco_arg_db_key[0]);


	// Retrieve the object
	//
	gdbm.Log(3,"D: RCO_EventHandler: Retrieving Object.\n");
	char* db_value = NULL;
	size_t db_value_size = 0;
	bool ok = gdbm.gdbmpool->fetch(
		gdbm.sco_rco_arg_db_name.c_str(),
		gdbm.sco_rco_arg_db_key,
		&db_value, &db_value_size,
		gdbm.sco_rco_arg_db_open
	);
	if ( ok ) {
		gdbm.Log(3,"D: RCO_EventHandler: db_name [%s], db_key [%s], OBJECT Found, Object [%s], Size [%u]\n",
			gdbm.sco_rco_arg_db_name.c_str(), &gdbm.sco_rco_arg_db_key[0], db_value, db_value_size);
	} else {
		gdbm.Log(3,"D: RCO_EventHandler: db_name [%s], db_key [%s], OBJECT was NOT Found\n",
			gdbm.sco_rco_arg_db_name.c_str(), &gdbm.sco_rco_arg_db_key[0]);
		free(db_value);
		return 1;
	}

	// This should never be any object except one we stored, so the object should
	// always be a good one. However, since NWN is known to crash in some senarios
	// with bad GFF files, make a small effort to check validity.
	//
	// Known GFF IDs: "UTI V3.28","BIC V3.28","UTP V3.28","UTM V3.28","UTT V3.28"
	//
	if ( db_value[4]!='V' || db_value[5]!='3' || db_value[6]!='.' || db_value[7]!='2' || db_value[8]!='8' ) {
		gdbm.Log(0,"E: RCO_EventHandler: Object found is not recognized GFF data.\n");
		free(db_value);
		return 1;
	}

	// GDBM mallocs memory returned by the fetch function so returned sent via
	// the pData value the NWN engine should free it correctly.
	//
	s->pData = (unsigned char*)db_value;
	s->size = db_value_size;

	gdbm.Log(3,"D: RCO_EventHandler, Retrieved GFF Object [%08x], Size [%u]\n", s->pData, s->size);

	return 1; // Indicate the event was accepted by this handler
}

/*
static char* ConvertKeySeparatorToNull (const char* s);
static char* ConvertNullToKeySeparator (const char* s);
static char* ConvertKeySeparatorToNull (const char* s)
{
	size_t n = strlen(s);
	char* d = (char*)malloc(n+1);
	for (size_t i=0; i<=n; i++) if (s[i]==NWNX_GDBM_KEYSEP) d[i]='\0'; else d[i]=s[i];
	return d;
}

static char* ConvertNullToKeySeparator (const char* s)
{
	size_t n = strlen(s);
	char* d = (char*)malloc(n+1);
	for (size_t i=0; i<=n; i++) if (s[i]=='\0') d[i]=NWNX_GDBM_KEYSEP; else d[i]=s[i];
	return d;
}
*/

// argstr -> a modifyiable copy of the parameter string
// arglst -> a pointer array n elements
// argcnt -> the number of arguments n
//
bool CNWNXgdbm::ParseDelimitedArgumentList(char* argstr, char** arglst, int argcnt)
{
	if ( (argstr == NULL) || (*argstr == '\0') || (arglst == NULL) || (argcnt <= 0) ) {
		Log(0,"E: Missing or empty arguments to request\n");
		return false;
	}
	split_on_char(argstr, arglst, argcnt, NWNX_GDBM_ARGSEP);
	for (int i=0; i<argcnt; i++) {
		if ( !arglst[i] || (*arglst[i]=='\0') ) {
			Log(0,"E: Missing argument [%d] to request\n", i);
			return false;
		}
		Log(4,"D: ARG [%d] = [%s]\n", i, arglst[i]);
	}
	return true;
}

static char* split_on_char (char* tokstr, char** toklst, unsigned int toklstlen, char toksep)
{
	register unsigned int i;
	register char* p;
	if ( !tokstr || !toklst || (toklstlen <= 0) ) return 0;
	p = toklst[0] = tokstr;
	for (i = 1; i < toklstlen; i++) {
		while ( (*p != '\0') && (*p != toksep) ) p++;
		if (*p == toksep) { *p++ = '\0'; toklst[i] = p; continue; }
		toklst[i] = 0;
	}
	return tokstr;
}

