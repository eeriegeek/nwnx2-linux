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

static char* ConvertKeySeparatorToNull (const char* s);
static char* ConvertNullToKeySeparator (const char* s);
static char* split_on_char (char* tokstr, char** toklst, unsigned int toklstlen, char toksep);

//
// Base plugin (CNWNXBase) method implementation
//

CNWNXgdbm::CNWNXgdbm() :
	tmp_filepath(NULL),
	server_home(NULL),
	cfg_filepath(NULL),
	cfg_sync_write(true),
	cfg_lock_file(false),
	cfg_use_scorco(true),
	gdbmpool(NULL)
{
	confKey = "GDBM";
	tmp_filepath = (char*)malloc(FILENAME_MAX+1);
}

CNWNXgdbm::~CNWNXgdbm()
{
	delete gdbmpool;
	free(cfg_filepath);
	free(server_home);
	free(tmp_filepath);
}

bool CNWNXgdbm::OnCreate (gline *config, const char* LogDir)
{
	// Start my log file via the base class method
	//
	snprintf(tmp_filepath, FILENAME_MAX, "%s/nwnx_gdbm.txt", LogDir);
	if (!CNWNXBase::OnCreate(config,tmp_filepath)) return false;

	Log(0,"NWNX - GDBM Plugin Version 2.0.0\n");
	Log(0,"Copyright 2012-2013 eeriegeek (eeriegeek@yahoo.com)\n");
	Log(0,"Distributed under the terms of the GNU General Public License.\n");
	Log(0,"Visit http://www.nwnx.org for more information.\n");

	// Save the server home directory
	//
	ssize_t len = readlink("/proc/self/exe", tmp_filepath, FILENAME_MAX);
	if (len == -1) {
		Log(0,"ERROR: Unable to determine server working directory. %s\n", strerror(errno));
		return false;
	} else {
		tmp_filepath[len] = '\0';
		char* dir = dirname(tmp_filepath);
		if (dir == NULL) {
			Log(0,"ERROR: Unable to determine server working directory. %s\n", strerror(errno));
			return false;
		} else {
			server_home = strcpy((char*)malloc(strlen(dir)+1), dir);
			Log(1,"DB: Found server home directory of [%s]\n", server_home);
		}
	}

	// Read the [GDBM] configuration file section from nwnx2.ini
	//
	if(!nwnxConfig->exists(confKey)) {
		Log(1,"WARNING: GDBM configuration section [%s] not found in nwnx2.ini, using default values.\n", confKey);
	} else {

		char* ptr = NULL;

		// cfg_filepath, default is will be database subdir of server home
		//
		ptr = (char*)((*nwnxConfig)[confKey]["filepath"].c_str());
		if ((ptr == NULL)||(strcmp(ptr,"")==0)) {
			Log(2,"INFO: No filepath found in GDBM configuration section in nwnx2.ini. Will try default location.\n");
		} else {
			cfg_filepath = strcpy((char*)malloc(strlen(ptr)+1),ptr);
			Log(2,"INFO: Found filepath [%s] in GDBM configuration section in nwnx2.ini.\n", cfg_filepath);
		}

		// cfg_sync_write, default is 1
		//
		ptr = (char*)((*nwnxConfig)[confKey]["no_sync"].c_str());
		if ((ptr!=0)&&(ptr[0]=='0')&&(ptr[1]=='\x00')) cfg_sync_write = false;

		// cfg_lock_file, default is 0
		//
		ptr = (char*)((*nwnxConfig)[confKey]["no_lock"].c_str());
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
			Log(0,"ERROR: database filepath [%s] is not a directory.\n", cfg_filepath);
			return false;
		}
	} else {
		Log(0,"ERROR: Cannot stat database filepath [%s]. %s\n", cfg_filepath,strerror(errno));
		return false;
	}

	Log(0,"INFO: Directory path [%s] for physical GDBM database files.\n", cfg_filepath);
	Log(0,"INFO: Settings: sync_write=%d, lock_file=%d, use_scorco=%d\n", cfg_sync_write, cfg_lock_file, cfg_use_scorco);

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
			Log(0,"ERROR: Unable to register NWNX/Core/PluginsLoaded Handler.\n");
			cfg_use_scorco = false;
		}
	}

	return true;
}

char* CNWNXgdbm::OnRequest (char* gameObject, char* Request, char* Parameters)
{
	char* pResult = NULL;

	Log(3,"-----------------\n");
	Log(3,"OnRequest: GOT-REQ [%s]\n",Request);
	Log(3,"OnRequest: GOT-PAR [%s]\n",Parameters);

	// FUTURE: use the perfect hash generator? This should be pretty fast anyway.
	// Only 4 of the keys get beyond the first char and 1 beyond 2 in strcmp.

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
		Log(0,"ERROR: unrecognized GDBM operation [%s] requested\n",Request);
		return pResult;
	}

	Log(3,"OnRequest: RET-PAR [%s]\n",Parameters);
	Log(3,"OnRequest: RET-RES [%s]\n",pResult);

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
	Log(2,"INFO: OnRelease: Called\n");
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
	if ((Parameters==NULL)||(*Parameters=='\0')) {
		Log(0,"ERROR: Missing ALL arguments to CREATE method\n");
		return NULL;
	}
	char* arg = strdup(Parameters);
	strcpy(Parameters,"0");
	char* arglst[2];
	split_on_char(arg,arglst,2,NWNX_GDBM_ARGSEP);
	char* db_name = arglst[0];
	if ((db_name==NULL)||(*db_name=='\0')) { Log(0,"ERROR: Missing argument db_name to CREATE in [%s]\n",Parameters); free(arg); return NULL; }
	char* db_truncate = arglst[1];
	if ((db_truncate==NULL)||(*db_truncate=='\0')) { Log(0,"ERROR: Missing argument db_truncate to CREATE in [%s]\n",Parameters); free(arg); return NULL; }
	Log(2,"INFO: CREATE: db_name [%s], db_truncate [%s]\n", db_name, db_truncate);
	if ( gdbmpool->create(db_name,(db_truncate[0]=='1')) ) {
		Log(2,"INFO: CREATE: db_name [%s], db_truncate [%s], Create OK\n", db_name, db_truncate);
		strcpy(Parameters,"1");
	} else {
		Log(0,"ERROR: CREATE: db_name [%s], db_truncate [%s], FAILED\n", db_name, db_truncate);
	}
	free(arg);
	return NULL;
}

// Arg0 - the name of the database
//
// Ret  - nothing
//
char* CNWNXgdbm::Reorganize(char* gameObject, char* Parameters)
{
	if ((Parameters==NULL)||(*Parameters=='\0')) {
		Log(0,"ERROR: Missing ALL arguments to REORGANIZE method\n");
		return NULL;
	}
	char* db_name = Parameters;
	Log(2,"INFO: REORGANIZE: db_name [%s]\n", db_name);
	gdbmpool->reorganize(db_name);
	return NULL;
}

// Arg0 - the name of the database
//
// Ret  - 1 if unlink succeeds, 0 if anything goes wrong
//
char* CNWNXgdbm::Destroy(char* gameObject, char* Parameters)
{
	if ((Parameters==NULL)||(*Parameters=='\0')) {
		Log(0,"ERROR: Missing ALL arguments to DESTROY method\n");
		return NULL;
	}
	char* db_name = Parameters;
	Log(2,"INFO: DESTROY: db_name [%s]\n", db_name);
	if ( gdbmpool->destroy(db_name) ) {
		Log(2,"INFO: DESTROY: db_name [%s], Create OK\n", db_name);
		strcpy(Parameters,"1");
	} else {
		Log(0,"ERROR: DESTROY: db_name [%s], FAILED\n", db_name);
		strcpy(Parameters,"0");
	}
	return NULL;
}

// Arg0 - the name of the database
// Arg1 - create flag (default = 1 (on))
// Arg2 - read-only flag (default = 0 (off))
//
// Ret  - boolean -> 1 if successful open (or was already open), 0 if failed
//
// Added GDBM_SYNC option to config file, But after a quick test, I couldn't
// see a difference in speed over 10000 inserts with GDBM_SYNC on, so I'll
// leave it on as the default for now. Hopefilly it will protect databases a bit
// more if the server crashes.
//
char* CNWNXgdbm::Open(char* gameObject, char* Parameters) 
{
	if ((Parameters==NULL)||(*Parameters=='\0')) {
		Log(0,"ERROR: Missing ALL arguments to OPEN method\n");
		return NULL;
	}
	char* arg = strdup(Parameters);
	strcpy(Parameters,"0");
	char* arglst[3];
	split_on_char(arg,arglst,3,NWNX_GDBM_ARGSEP);
	char* db_name = arglst[0];
	if ((db_name==NULL)||(*db_name=='\0')) { Log(0,"ERROR: Missing argument db_name to OPEN in [%s]\n",Parameters); free(arg); return NULL; }
	char* db_create = arglst[1];
	if ((db_create==NULL)||(*db_create=='\0')) { Log(0,"ERROR: Missing argument db_create to OPEN in [%s]\n",Parameters); free(arg); return NULL; }
	char* db_readonly = arglst[2];
	if ((db_readonly==NULL)||(*db_readonly=='\0')) { Log(0,"ERROR: Missing argument db_readonly to OPEN in [%s]\n",Parameters); free(arg); return NULL; }
	Log(2,"INFO: OPEN: db_name [%s], db_create [%s], db_readonly [%s]\n", db_name, db_create, db_readonly);
	if ( gdbmpool->open(db_name,(db_create[0]=='1'),(db_readonly[0]=='1')) ) {
		Log(2,"INFO: OPEN: db_name [%s], OK\n", db_name);
		strcpy(Parameters,"1");
	} else {
		Log(0,"ERROR: Could not open db_name [%s] in OPEN method\n", db_name);
	}
	free(arg);
	return NULL;
}

// Arg0 - the name of the database
//
// Ret   - nothing
//
char* CNWNXgdbm::Sync(char* gameObject, char* Parameters) 
{
	if ((Parameters==NULL)||(*Parameters=='\0')) {
		Log(0,"ERROR: Missing ALL arguments to SYNC method\n");
		return NULL;
	}
	char* db_name = Parameters;
	Log(2,"INFO: SYNC: db_name [%s]\n", db_name);
	gdbmpool->sync(db_name);
	return NULL;
}

// Arg0 - the name of the database
//
// Ret  - nothing
//
char* CNWNXgdbm::Close(char* gameObject, char* Parameters) 
{
	if ((Parameters==NULL)||(*Parameters=='\0')) {
		Log(0,"ERROR: Missing ALL arguments to CLOSE method\n");
		return NULL;
	}
	char* db_name = Parameters;
	Log(2,"INFO: CLOSE: db_name [%s]\n", db_name);
	gdbmpool->close(db_name);
	return NULL;
}

// Args - nothing
//
// Ret  - nothing
//
char* CNWNXgdbm::CloseAll(char* gameObject, char* Parameters)
{
	Log(2,"INFO: CLOSEALL\n");
	gdbmpool->close_all();
	return NULL;
}

//
// Data Manipulation Methods
//

// Arg0 - the name of the database
// Arg1 - replace flag (default = 1 (TRUE))
// Arg2 - open flag (default = 1 (TRUE))
// Arg3 - the key
// Arg4 - the value
//
// IMPORTANT, the key may contain a seperator so it must be diff from the arg
//			separator or things get confused here.
//
// Ret   - TRUE on successful insert, if replace flag = 0 (FALSE), FALSE is
//		 returned for collision and the value is not replaced.
//
char* CNWNXgdbm::Store(char* gameObject, char* Parameters)
{
	if ((Parameters==NULL)||(*Parameters=='\0')) {
		Log(0,"ERROR: Missing ALL arguments to STORE method\n");
		return NULL;
	}
	char* arg = strdup(Parameters);
	strcpy(Parameters,"0");
	char* arglst[5];
	split_on_char(arg,arglst,5,NWNX_GDBM_ARGSEP);
	char* db_name = arglst[0];
	if ((db_name==NULL)||(*db_name=='\0')) { Log(0,"ERROR: Missing argument db_name to STORE in [%s]\n",Parameters); free(arg); return NULL; }
	char* db_replace = arglst[1];
	if ((db_replace==NULL)||(*db_replace=='\0')) { Log(0,"ERROR: Missing argument db_replace to STORE in [%s]\n",Parameters); free(arg); return NULL; }
	char* db_open = arglst[2];
	if ((db_open==NULL)||(*db_open=='\0')) { Log(0,"ERROR: Missing argument db_open to STORE in [%s]\n",Parameters); free(arg); return NULL; }
	char* db_key = arglst[3];
	if ((db_key==NULL)||(*db_key=='\0')) { Log(0,"ERROR: Missing argument db_key to STORE in [%s]\n",Parameters); free(arg); return NULL; }
	char* db_value = arglst[4];
	if ((db_value==NULL)||(*db_value=='\0')) { Log(0,"ERROR: Missing argument db_value to STORE in [%s]\n",Parameters); free(arg); return NULL; }
	Log(3,"DB: STORE: db_name [%s], db_replace [%s], db_open [%s]\n", db_name, db_replace, db_open);
	Log(3,"DB: STORE: db_key [%s], db_value [%s]\n", db_key, db_value);
    if ( gdbmpool->store(db_name, db_key, db_value, strlen(db_value), (db_replace[0]=='1'), (db_open[0]=='1')) ) {
		Log(3,"DB: STORE: db_name [%s], db_key [%s], Key inserted/updated\n", db_name, db_key);
		strcpy(Parameters,"1");
	} else {
		Log(3,"DB: STORE: db_name [%s], db_key [%s], Collision, Key NOT inserted/updated\n", db_name, db_key);
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
	if ((Parameters==NULL)||(*Parameters=='\0')) {
		Log(0,"ERROR: Missing ALL arguments to EXISTS method\n");
		return NULL;
	}
	char* arg = strdup(Parameters);
	strcpy(Parameters,"0");
	char* arglst[3];
	split_on_char(arg,arglst,3,NWNX_GDBM_ARGSEP);
	char* db_name = arglst[0];
	if ((db_name==NULL)||(*db_name=='\0')) { Log(0,"ERROR: Missing argument db_name to EXISTS in [%s]\n",Parameters); free(arg); return NULL; }
	char* db_open = arglst[1];
	if ((db_open==NULL)||(*db_open=='\0')) { Log(0,"ERROR: Missing argument db_open to EXISTS in [%s]\n",Parameters); free(arg); return NULL; }
	char* db_key = arglst[2];
	if ((db_key==NULL)||(*db_key=='\0')) { Log(0,"ERROR: Missing argument db_key to EXISTS in [%s]\n",Parameters); free(arg); return NULL; }
	Log(3,"DB: EXISTS: db_name [%s], db_open [%s], db_key [%s]\n", db_name, db_open, db_key);
    if ( gdbmpool->exists(db_name, db_key) ) {
		Log(3,"DB: EXISTS: db_name [%s], db_key [%s], Key inserted/updated\n", db_name, db_key);
		strcpy(Parameters,"1");
	} else {
		Log(3,"DB: EXISTS: db_name [%s], db_key [%s], Collision, Key NOT inserted/updated\n", db_name, db_key);
	}
	free(arg);
	return NULL;
}

// Arg0 - name of the database
// Arg1 - open flag
// Arg2 - the key
//
// Ret  - the value
//
char* CNWNXgdbm::Fetch(char* gameObject, char* Parameters)
{
	if ((Parameters==NULL)||(*Parameters=='\0')) {
		Log(0,"ERROR: Missing ALL arguments to FETCH method\n");
		return NULL;
	}
	char* arg = strdup(Parameters);
	char* arglst[3];
	split_on_char(arg,arglst,3,NWNX_GDBM_ARGSEP);
	char* db_name = arglst[0];
	if ((db_name==NULL)||(*db_name=='\0')) { Log(0,"ERROR: Missing argument db_name to FETCH in [%s]\n",Parameters); free(arg); return NULL; }
	char* db_open = arglst[1];
	if ((db_open==NULL)||(*db_open=='\0')) { Log(0,"ERROR: Missing argument db_open to FETCH in [%s]\n",Parameters); free(arg); return NULL; }
	char* db_key = arglst[2];
	if ((db_key==NULL)||(*db_key=='\0')) { Log(0,"ERROR: Missing argument db_key to FETCH in [%s]\n",Parameters); free(arg); return NULL; }
	Log(3,"DB: FETCH: db_name [%s], db_open [%s], db_key [%s]\n", db_name, db_open, db_key);
	char* db_value = NULL;
	size_t db_value_size = 0;
    if ( gdbmpool->fetch(db_name, db_key, &db_value, &db_value_size, (db_open[0]=='1')) ) {
		Log(3,"DB: FETCH: db_name [%s], db_key [%s], Found\n", db_name, db_key);
	} else {
		Log(3,"DB: FETCH: db_name [%s], db_key [%s], NOT Found\n", db_name, db_key);
	}
	free(arg);
	return db_value;
}

// Arg0 - name of the database
// Arg1 - open flag
// Arg2 - the key
//
// Ret  - 1 if the key was found, 0 if dne (via Parameters)
//
char* CNWNXgdbm::Delete(char* gameObject, char* Parameters)
{
	if ((Parameters==NULL)||(*Parameters=='\0')) {
		Log(0,"ERROR: Missing ALL arguments to DELETE method\n");
		return NULL;
	}
	char* arg = strdup(Parameters);
	strcpy(Parameters,"0");
	char* arglst[3];
	split_on_char(arg,arglst,3,NWNX_GDBM_ARGSEP);
	Log(3,"DB: DELETE: db_name [%s], db_open [%s], db_key [%s]\n", arglst[0], arglst[1], arglst[2]);
	for (int i=0; i<3; i++) {
		if ( !arglst[i] || (*arglst[i]=='\0') ) {
			Log(0,"ERROR: Missing argument %d to DELETE in [%s]\n",i,Parameters);
			free(arg);
			return NULL;
		}
	}
	//char* db_key = ConvertKeySeparatorToNull(gdbm.sco_rco_arg_db_key.c_str());
	gdbm_datum_t db_key(arglst[3],arglst[3]+strlen(arglst[3]));
	for (size_t i=0; i<db_key.size(); i++) if (db_key[i]==NWNX_GDBM_KEYSEP) db_key[i]='\0';
    if ( gdbmpool->erase(arglst[0], db_key, (*arglst[2]=='1')) ) {
		Log(3,"DB: DELETE: db_name [%s], db_key [%s], Key Deleted\n", arglst[0], arglst[1]);
		strcpy(Parameters,"1");
	} else {
		Log(3,"DB: DELETE: db_name [%s], db_key [%s], Key NOT Found/Deleted\n", arglst[0], arglst[1]);
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
		gdbm.Log(0,"ERROR: Unable to register NWServer/SCO and/or NWServer/RCO event handlers.\n");
		gdbm.cfg_use_scorco = false;
	}
	gdbm.Log(2,"INFO: NWServer/SCO and NWServer/RCO event handlers registered without error.\n");
	return 0;
}

// return nothing
char* CNWNXgdbm::SetObjectArguments(char* gameObject, char* Parameters)
{
	if ( !cfg_use_scorco ) {
		Log(0,"ERROR: Attempt to call SETOBJARG when SCO/RCO handler is disabled.\n");
		return NULL;
	}
	if ((Parameters==NULL)||(*Parameters=='\0')) {
		Log(0,"ERROR: Missing ALL arguments to SETOBJARG method\n");
		return NULL;
	}
	gdbm.Log(3,"DB: SETOBJARG: SCO/RCO Set Object Key, Parameters [%s].\n", Parameters);
    char* arg = strdup(Parameters);
    char* arglst[4];
    split_on_char(arg,arglst,4,NWNX_GDBM_ARGSEP);
	for (int i=0; i<4; i++) {
		if ( !arglst[i] || (*arglst[i]=='\0') ) {
			Log(0,"ERROR: Missing argument %d to SETOBJARG in [%s]\n",i,Parameters);
			free(arg);
			return NULL;
		}
	}
    sco_rco_arg_db_name = arglst[0];
    sco_rco_arg_db_replace = (*arglst[1]=='1');
    sco_rco_arg_db_open = (*arglst[2]=='1');
    sco_rco_arg_db_key = arglst[3];
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
		gdbm.Log(0,"ERROR: Attempt to call SCO when SCO/RCO handler is disabled.\n");
		return NULL;
	}

	// Get the callback event structure and check if the event is for this plugin.
	//
	SCORCOStruct* s = (SCORCOStruct*) p;
	gdbm.Log(3,"DB: SCO_EventHandler: event notification for service [%s].\n", s->key);
	if ( strcmp("GDBM", s->key) != 0 ) return 0; // Decline event if not for GDBM plugin
	gdbm.Log(3,"DB: SCO_EventHandler: event notification for GDBM.\n");

	// Check that valid arguments were set by a prior call to SETOBJARG.
	//
	if ( (gdbm.sco_rco_arg_db_name.length()==0) || (gdbm.sco_rco_arg_db_key.length()==0) ) {
		gdbm.Log(0,"ERROR: SCO_EventHandler: event notification with missing SCO/RCO Argument.\n");
		return 1; // Indicate the event was accepted by this handler
	}
	gdbm.Log(3,"DB: SCO_EventHandler: DB [%s], Key [%s].\n", gdbm.sco_rco_arg_db_name.c_str(), gdbm.sco_rco_arg_db_key.c_str());
	
	//char* db_key = strdup(gdbm.sco_rco_arg_db_key.c_str());
	//size_t db_key_size = strlen(db_key);
	//char* ptr = db_key; while (*ptr) if (*ptr=='0x1f') { *ptr=='\0'; ptr++; }
	//for ( char* p=db_key; *p!='\0';  p++ ) if (*p=='0x1f') *p='\0';

	//char* s = gdbm.sco_rco_arg_db_key.c_str();
	//size_t n = strlen(s);
	//char* d = (char*)malloc(n+1);
	//for (int i=0; i<=n; i++) if (s[i]=='0x1f') d[i]='\0'; else d[i]=s[i];

	// Store the object
	//
	gdbm.Log(3,"DB: SCO_EventHandler: Storing Object [%s], Length [%d].\n", s->pData, s->size);
	char* db_key = ConvertKeySeparatorToNull(gdbm.sco_rco_arg_db_key.c_str());
	bool ok = gdbm.gdbmpool->store(
		gdbm.sco_rco_arg_db_name.c_str(),
		db_key,
		(const char*)s->pData, s->size,
		gdbm.sco_rco_arg_db_replace, gdbm.sco_rco_arg_db_open
	);
    if ( ok ) {
		gdbm.Log(3,"DB: SCO_EventHandler: db_name [%s], db_key [%s], OBJECT inserted/updated\n",
			gdbm.sco_rco_arg_db_name.c_str(), gdbm.sco_rco_arg_db_key.c_str());
	} else {
		gdbm.Log(3,"DB: SCO_EventHandler: db_name [%s], db_key [%s], OBJECT was NOT inserted/updated, already exists\n",
			gdbm.sco_rco_arg_db_name.c_str(), gdbm.sco_rco_arg_db_key.c_str());
	}
	free(db_key);

	return 1; // Indicate the event was accepted by this handler
}

int CNWNXgdbm::RCO_EventHandler(WPARAM p, LPARAM a)
{
	if ( !gdbm.cfg_use_scorco ) {
		gdbm.Log(0,"ERROR: Attempt to call RCO when SCO/RCO handler is disabled.\n");
		return NULL;
	}

	// Get the callback event structure and check if the event is for this plugin.
	//
	SCORCOStruct* s = (SCORCOStruct*) p;
	gdbm.Log(3,"DB: RCO_EventHandler: event notification for service [%s].\n", s->key);
	if ( strcmp("GDBM", s->key) != 0 ) return 0; // Decline event if not for GDBM plugin
	gdbm.Log(3,"DB: RCO_EventHandler: event notification for GDBM.\n");

	// Set return parameters for error conditions.
	//
	s->pData = NULL;
	s->size = 0;

	// Check that valid arguments were set by a prior call to SETOBJARG.
	//
	if ( (gdbm.sco_rco_arg_db_name.length()==0) || (gdbm.sco_rco_arg_db_key.length()==0) ) {
		gdbm.Log(0,"ERROR: RCO_EventHandler: event notification with missing SCO/RCO Argument.\n");
		return 1;
	}
	gdbm.Log(3,"DB: RCO_EventHandler: DB [%s], Key [%s].\n", gdbm.sco_rco_arg_db_name.c_str(), gdbm.sco_rco_arg_db_key.c_str());


	// Retrieve the object
	//
	gdbm.Log(3,"DB: RCO_EventHandler: Retrieving Object.\n");
	char* db_key = ConvertKeySeparatorToNull(gdbm.sco_rco_arg_db_key.c_str());
	char* db_value = NULL;
	size_t db_value_size = 0;
	bool ok = gdbm.gdbmpool->fetch(
		gdbm.sco_rco_arg_db_name.c_str(),
		db_key,
		&db_value, &db_value_size,
		gdbm.sco_rco_arg_db_open
	);
    if ( ok ) {
		gdbm.Log(3,"DB: RCO_EventHandler: db_name [%s], db_key [%s], OBJECT Found, Object [%s], Size [%u]\n",
			gdbm.sco_rco_arg_db_name.c_str(), gdbm.sco_rco_arg_db_key.c_str(), db_value, db_value_size);
	} else {
		gdbm.Log(3,"DB: RCO_EventHandler: db_name [%s], db_key [%s], OBJECT was NOT Found\n",
			gdbm.sco_rco_arg_db_name.c_str(), gdbm.sco_rco_arg_db_key.c_str());
		free(db_value);
		return 1;
	}
	free(db_key);

	// This should never be any object except one we stored, so the object should
	// always be a good one. However, since NWN is known to crash in some senarios
	// with bad GFF files, make a small effort to check validity.
	//
	// Known GFF IDs: "UTI V3.28","BIC V3.28","UTP V3.28","UTM V3.28","UTT V3.28"
	//
	if ( db_value[4]!='V' || db_value[5]!='3' || db_value[6]!='.' || db_value[7]!='2' || db_value[8]!='8' ) {
		gdbm.Log(0,"ERROR: RCO_EventHandler: Object found is not recognized GFF data.\n");
		free(db_value);
		return 1;
	}

	// GDBM mallocs memory returned by the fetch function so returned sent via
	// the pData value the NWN engine should free it correctly.
	//
	s->pData = (unsigned char*)db_value;
	s->size = db_value_size;

	gdbm.Log(3,"DB: RCO_EventHandler, Retrieved GFF Object [%08x], Size [%u]\n", s->pData, s->size);

	return 1; // Indicate the event was accepted by this handler
}

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

