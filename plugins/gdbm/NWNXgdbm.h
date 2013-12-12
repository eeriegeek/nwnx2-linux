#ifndef NWNXgdbm_H_
#define NWNXgdbm_H_
/******************************************************************************

  NWNXgdbm.h - Include file for GDBM plugin for NWNX.

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

#include "NWNXBase.h"

#include "GdbmPool.h"

// It's difficult to get a verifiable max length for the NWN player_name and
// character_name, since they are constrained by proportional font width.
// The maximum appears experimentally to be around 260 for first+last name.
// Player name similar. Size 1024 should be large enough for common use
// cases.
//
#define SCO_RCO_OBJECT_ARGS_MAX_LEN 1024

// Main NWNX GDBM plugin object
//
class CNWNXgdbm : public CNWNXBase
{
	public:

		CNWNXgdbm();
		virtual ~CNWNXgdbm();

		bool OnCreate(gline *nwnxConfig, const char* LogDir=NULL);
		char* OnRequest(char* gameObject, char* Request, char* Parameters);
		bool OnRelease();

	private:
					
		char* Create(char* gameObject, char* Parameters);
		char* Reorganize(char* gameObject, char* Parameters);
		char* Destroy(char* gameObject, char* Parameters);

		char* Open(char* gameObject, char* Parameters);
		char* Sync(char* gameObject, char* Parameters);
		char* Close(char* gameObject, char* Parameters);
		char* CloseAll(char* gameObject, char* Parameters);

		char* Store(char* gameObject, char* Parameters);
		char* Exists(char* gameObject, char* Parameters);
		char* Fetch(char* gameObject, char* Parameters);
		char* Delete(char* gameObject, char* Parameters);

		char* SetObjectArguments(char* gameObject, char* Parameters);

		// Set up event callbacks for the new plugin API for SCO/RCO handling.
		//
		static int PluginsLoaded_EventHandler(WPARAM p, LPARAM a);
		static int SCO_EventHandler(WPARAM p, LPARAM a);
		static int RCO_EventHandler(WPARAM p, LPARAM a);

		// Buffer to handle longish file pathnames. Will malloc in constructor.
		//
		char* tmp_filepath;

		// Configuration parameters.
		//
		char* server_home;     // save the servers home directory
		char* cfg_filepath;    // default is "$server_home/database"
		bool cfg_sync_write;   // default is TRUE (GDBM_SYNC)
		bool cfg_lock_file;    // default is FALSE (GDBM_NOLOCK)
		bool cfg_use_scorco;   // default is TRUE, hook handler into SCO/RCO

		// Maintain a pool of open GDBF files.
		//
		GdbmPool* gdbmpool;

		// Holds the last set of arguments specified for the next call to
		// SCO or RCO. (SCO/RCO don't have space for the argument string.
		//
		//char* sco_rco_object_args;
		std::string sco_rco_arg_db_name;
		bool sco_rco_arg_db_replace;
		bool sco_rco_arg_db_open;
		std::string sco_rco_arg_db_key;

};

#endif
