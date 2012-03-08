/******************************************************************************

  NWNXgdbm.h - Include file for GDBM plugin for NWNX.

  Copyright 2012 eeriegeek (eeriegeek@yahoo.com)

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

#ifndef NWNXgdbm_H_
#define NWNXgdbm_H_

#include <gdbm.h>

#include <string>
#include <map>

using namespace std;

#include "NWNXBase.h"

//
// Structure holding info about each open GDBM file in the POOL map.
//
typedef struct gdbm_file_node_s {
	GDBM_FILE	dbf;
	datum		itr;
    time_t      act;
} gdbm_file_node_t;

//
// The GDBM file pool. Basically, a string indexed map of GDBM file nodes.
//
typedef map < string, gdbm_file_node_t, less<string> > gdbm_file_pool_t;

//
// Main NWNX plugin object
//
class CNWNXgdbm : public CNWNXBase {

    public:

	CNWNXgdbm();

	virtual ~CNWNXgdbm();

	bool OnCreate(gline *nwnxConfig, const char* LogDir=NULL);

	char* OnRequest(char* gameObject, char* Request, char* Parameters);
  
	bool OnRelease();

    protected:

    private:
					
	char* Create(char* gameObject, char* Parameters);
	char* Reorganize(char* gameObject, char* Parameters);
	char* Destroy(char* gameObject, char* Parameters);

	char* Open(char* gameObject, char* Parameters);
	char* Sync(char* gameObject, char* Parameters);
	char* Close(char* gameObject, char* Parameters);
	char* CloseAll(char* gameObject, char* Parameters);
    void CloseAll();

	char* Store(char* gameObject, char* Parameters);
	char* Exists(char* gameObject, char* Parameters);
	char* Fetch(char* gameObject, char* Parameters);
	char* Delete(char* gameObject, char* Parameters);

	char* FirstKey(char* gameObject, char* Parameters);
	char* NextKey(char* gameObject, char* Parameters);

    //
    // Buffer to handle longish file pathnames. Will malloc in constructor.
    //
    char* tmp_filepath;

    //
    // Configuration parameters
    //
    char*   server_home;        // save the servers home directory
    char*   cfg_filepath;       // default is "$server_home/database"
    int     cfg_sync_write;     // default is TRUE (GDBM_SYNC)
    int     cfg_lock_file;      // default is FALSE (GDBM_NOLOCK)

	//
	// Maintain a pool of open GDBF files, currently unlimited except by system
	//
	gdbm_file_pool_t gdbm_file_pool;

    //
    // Unbundle bundled string of arguments
    //
    char* _arg_list[10+1];
    char** UnBundleArguments(char* args);

};

#endif
