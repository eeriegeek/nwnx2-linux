/******************************************************************************

  NWNXgdbm.cpp - Implementation of GDBM plugin for NWNX.

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

#include "NWNXgdbm.h"

#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <sys/resource.h>
#include <libgen.h>

//
// Tokens used to delimit request arguments, TOKEN is used to delimit string
// argument passing. KEYSEC is used to section keys for compound keys.
//
#define NWNX_GDBM_TOKEN  "¬"
#define NWNX_GDBM_KEYSEC "§"

//
// Base plugin (CNWNXBase) method implementation
//

CNWNXgdbm::CNWNXgdbm()
{
	confKey = "GDBM";

    tmp_filepath = (char*)malloc(FILENAME_MAX+1);

    server_home  = NULL;
    cfg_filepath = NULL;

    cfg_sync_write = 1;
    cfg_lock_file  = 0;
}

CNWNXgdbm::~CNWNXgdbm()
{
	Log(4,"DB: Destructor called\n");
    CloseAll();

    free(tmp_filepath); // AdjustAlignment(oSelf,ALIGNMENT_LAWFUL,1)
}

bool CNWNXgdbm::OnCreate (gline *config, const char* LogDir)
{

    //
	// Start my log file via the base class method
	//
	snprintf(tmp_filepath,FILENAME_MAX,"%s/nwnx_gdbm.txt",LogDir);
    if (!CNWNXBase::OnCreate(config,tmp_filepath)) return false;

	Log(0,"NWNX - GDBM Plugin Version 1.0.0\n");
	Log(0,"Copyright 2012 eeriegeek (eeriegeek@yahoo.com)\n");
	Log(0,"Distributed under the terms of the GNU General Public License.\n");
	Log(0,"Visit http://www.nwnx.org for more information.\n");

    //
    // Save the server home directory
    //
    ssize_t len = readlink("/proc/self/exe",tmp_filepath,FILENAME_MAX);
    if (len == -1) {
        Log(0,"ERROR: Unable to determine server working directory. %s\n",strerror(errno));
        return false;
    } else {
        tmp_filepath[len] = '\0';
        char* dir = dirname(tmp_filepath);
        if (dir == NULL) {
            Log(0,"ERROR: Unable to determine server working directory. %s\n",strerror(errno));
            return false;
        } else {
            server_home = strcpy((char*)malloc(strlen(dir)+1),dir);
            Log(1,"DB: Found server home directory of [%s]\n",server_home);
        }
    }
    
    //
	// read my configuration file section
	//
    if(!nwnxConfig->exists(confKey)) {

        Log(1,"DB: GDBM configuration section [%s] not found in nwnx2.ini, using default values.\n",confKey);

	} else {

        char* ptr =  NULL;

        // cfg_filepath
        //
        ptr = (char*)((*nwnxConfig)[confKey]["filepath"].c_str());
        if ((ptr == NULL)||(strcmp(ptr,"")==0)) {

            Log(1,"DB: No filepath found in GDBM configuration section in nwnx2.ini. Will try default location.\n");

        } else {

            cfg_filepath = strcpy((char*)malloc(strlen(ptr)+1),ptr);

            Log(1,"DB: Got filepath [%s] in GDBM configuration section in nwnx2.ini.\n",cfg_filepath);

        }

        // cfg_sync_write
        //
        ptr = (char*)((*nwnxConfig)[confKey]["no_sync"].c_str());
        if ((ptr != NULL)&&(strcmp(ptr,"")!=0)) if (strcmp(ptr,"0")==0) cfg_sync_write = 0;

        // cfg_lock_file
        //
        ptr = (char*)((*nwnxConfig)[confKey]["no_lock"].c_str());
        if ((ptr != NULL)&&(strcmp(ptr,"")!=0)) if (strcmp(ptr,"1")==0) cfg_lock_file = 1;

    }

    //
    // Set to defaults if nothing was specifed in .ini file section.
    //
    if (cfg_filepath == NULL) {
        strcpy(tmp_filepath,server_home);
        strcat(tmp_filepath,"/database");
        cfg_filepath = strcpy((char*)malloc(strlen(tmp_filepath)+1),tmp_filepath);
    }

    struct stat s;
    if (stat(cfg_filepath,&s)==0) {
        if (!S_ISDIR(s.st_mode)) {
            Log(0,"ERROR: filepath = [%s] found GDBM configuration section is not a directory.\n",cfg_filepath);
            return false;
        }
    } else {
        Log(0,"ERROR: Cannot stat filepath = [%s] found GDBM configuration section. %s\n",cfg_filepath,strerror(errno));
        return false;
    }

    Log(0,"INFO: Using filepath [%s] for physical GDBM database files. Flags: sync_write=%d, lock_file=%d\n",cfg_filepath,cfg_sync_write,cfg_lock_file);


    struct rlimit rlim_nofile;
    int r = getrlimit(RLIMIT_NOFILE,&rlim_nofile);
    if (r==-1) {
        Log(0,"WARNING: Cannot get file limits. %s\n",strerror(errno));
    } else {
        Log(0,"INFO: file descriptor limits: soft=%lld, hard=%lld\n", (long long) rlim_nofile.rlim_cur, (long long) rlim_nofile.rlim_max);
    }

	return true;
}

char* CNWNXgdbm::OnRequest (char* gameObject, char* Request, char* Parameters)
{
	char* pResult = NULL;

	Log(3,"-----------------\n");
	Log(3,"OnRequest: GOT-REQ [%s]\n",Request);
	Log(3,"OnRequest: GOT-PAR [%s]\n",Parameters);

    // TODO: use the perfect hash generator? This should be pretty fast anyway.
    // Only 4 of the keys get beyond the first char and 1 beyond 2 in strcmp.

	     if (strcmp(Request,"FETCH"     )==0) pResult = Fetch(gameObject,Parameters);
	else if (strcmp(Request,"STORE"     )==0) pResult = Store(gameObject,Parameters);
	else if (strcmp(Request,"EXISTS"    )==0) pResult = Exists(gameObject,Parameters);
	else if (strcmp(Request,"DELETE"    )==0) pResult = Delete(gameObject,Parameters);
	else if (strcmp(Request,"FIRSTKEY"  )==0) pResult = FirstKey(gameObject,Parameters);
	else if (strcmp(Request,"NEXTKEY"   )==0) pResult = NextKey(gameObject,Parameters);
	else if (strcmp(Request,"OPEN"      )==0) pResult = Open(gameObject,Parameters);
	else if (strcmp(Request,"SYNC"      )==0) pResult = Sync(gameObject,Parameters);
	else if (strcmp(Request,"CLOSE"     )==0) pResult = Close(gameObject,Parameters);
	else if (strcmp(Request,"CLOSEALL"  )==0) pResult = CloseAll(gameObject,Parameters);
	else if (strcmp(Request,"CREATE"    )==0) pResult = Create(gameObject,Parameters);
    else if (strcmp(Request,"REORGANIZE")==0) pResult = Reorganize(gameObject,Parameters);
	else if (strcmp(Request,"DESTROY"   )==0) pResult = Destroy(gameObject,Parameters);

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
	Log(4,"INFO: OnRelease called\n");
	return CNWNXBase::OnRelease();
}

//
// Implementation of Plugin Methods
//

//
// Database Lifecycle Methods
//

// Arg 1 -> the name of the database
// Arg 2 -> truncate flag (default = 0 (off))
// Ret   -> if successful 1, otherwise 0
//
char* CNWNXgdbm::Create(char* gameObject, char* Parameters) 
{
    char* arg = strcpy((char*)malloc(strlen(Parameters)+1),Parameters);

    char* name     = strtok(arg,NWNX_GDBM_TOKEN);
    char* truncate = strtok(NULL,NWNX_GDBM_TOKEN);

	if ( (name==NULL)||(*name=='\0') || (truncate==NULL)||(*truncate=='\0') ) {

		Log(0,"ERROR: Missing argument to CREATE method, was passed [%s]\n",Parameters);

        strcpy(Parameters,"0");

    } else {

		Log(4,"NAM [%s]\n",name);
		Log(4,"TRC [%s]\n",truncate);

        strcpy(Parameters,"0");

        gdbm_file_pool_t::iterator node_itr = gdbm_file_pool.find(name);
        if (node_itr != gdbm_file_pool.end()) {
    	    Log(0,"WARNING: Create called on open GDBM database name [%s], will close and proceed.\n",name);

    	    gdbm_close(node_itr->second.dbf);
    
            gdbm_file_pool.erase(node_itr);
            
        } else {
    	    Log(2,"No open dbf name [%s] found while creating, good.\n",name);
        }

        snprintf(tmp_filepath,FILENAME_MAX,"%s/%s.gdbm",cfg_filepath,name);
     	Log(2,"Creating/Reopening GDBM database file [%s]\n",tmp_filepath);
    
        gdbm_file_node_t node;
    
        if (strcmp(truncate,"1")==0) {
    	    Log(0,"INFO: Creating GDBM database file [%s], with truncation.\n",tmp_filepath);
            node.dbf = gdbm_open(tmp_filepath,0,GDBM_WRCREAT|GDBM_NEWDB|GDBM_NOLOCK|GDBM_SYNC,0600,0);
        } else {
    	    Log(0,"INFO: Creating GDBM database file [%s]\n",tmp_filepath);
            node.dbf = gdbm_open(tmp_filepath,0,GDBM_WRCREAT|GDBM_NOLOCK|GDBM_SYNC,0600,0);
        }
  
        if (node.dbf == NULL) {

            Log(0,"ERROR: Call to gdbm_open failed for database filename [%s], %s\n",tmp_filepath,gdbm_strerror(gdbm_errno));
    
        } else {
    
           	Log(0,"INFO: Creation of GDBM database [%s] successful.\n",tmp_filepath);
    
            node.act = time(NULL);
    
            // FUTURE: if limiting pooling, check if pool full here, and reallocate
                
            gdbm_file_pool[name] = node;

            strcpy(Parameters,"1");
    
        }

    }

    free(arg);

	return NULL;

}

// Arg 1 - the name of the database
// Ret   - nothing
//
char* CNWNXgdbm::Reorganize(char* gameObject, char* Parameters)
{
    char* name = Parameters;

	if ( (name==NULL)||(*name=='\0') ) {

		Log(0,"ERROR: Missing argument to REORGANIZE method, was passed [%s]\n",Parameters);

    } else {

        gdbm_file_pool_t::iterator node_itr = gdbm_file_pool.find(name);

        if (node_itr != gdbm_file_pool.end()) {
	
        	Log(0,"INFO: Reorganizing GDBM database name [%s]\n",name);

	        int ret = gdbm_reorganize(node_itr->second.dbf);
	        if (ret == -1) {
		        Log(0,"ERROR: Call to gdbm_reorganize failed for database name [%s], %s\n",name,gdbm_strerror(gdbm_errno));
        	}

            node_itr->second.act = time(NULL);
 
        } else {
 
        	Log(0,"ERROR: REORGANIZE Could not find open GDBM database name [%s]\n",Parameters);

        }
    }

	return NULL;
}

// Arg 1 - the name of the database
// Ret   - 1 if unlink succeeds, 0 if anything goes wrong
//
char* CNWNXgdbm::Destroy(char* gameObject, char* Parameters)
{
    char* name = Parameters;

	if ( (name==NULL)||(*name=='\0') ) {

		Log(0,"ERROR: Missing argument to DESTROY method, was passed [%s]\n",Parameters);
        strcpy(Parameters,"0");

    } else {

        gdbm_file_pool_t::iterator node_itr = gdbm_file_pool.find(name);

        if (node_itr != gdbm_file_pool.end()) {

    	    Log(0,"WARNING: Destroy called on open GDBM database name [%s], will close and proceed.\n",name);

    	    gdbm_close(node_itr->second.dbf);
    
            gdbm_file_pool.erase(node_itr);

        } else {

    	    Log(2,"No open dbf name [%s] found while destroying, good.\n",name);

        }

        snprintf(tmp_filepath,FILENAME_MAX,"%s/%s.gdbm",cfg_filepath,Parameters);

        Log(0,"INFO: Deleting GDBM database file [%s]\n",tmp_filepath);

    	int ret = unlink(tmp_filepath);
    	if (ret == -1) {

    		Log(0,"ERROR: Call to unlink failed for database filename [%s], %s\n",tmp_filepath,gdbm_strerror(gdbm_errno));
            strcpy(Parameters,"0");

    	} else {

            strcpy(Parameters,"1");

        }

    }

	return NULL;
}

// Arg 1 - the name of the database
// Arg 2 - create flag (default = 1 (on))
// Arg 3 - read-only flag (default = 0 (off))
// Ret   - boolean -> 1 if successful open (or was already open), 0 if failed
//
// Added GDBM_SYNC option to config file, But after a quick test, I couldn't
// see a difference in speed over 10000 inserts with GDBM_SYNC on, so I'll
// leave it on as the default for now. Hopefilly it will protect databases a bit
// more if the server crashes.
//
char* CNWNXgdbm::Open(char* gameObject, char* Parameters) 
{
    char* arg = strcpy((char*)malloc(strlen(Parameters)+1),Parameters);

    char* name      = strtok(arg,NWNX_GDBM_TOKEN);
    char* create    = strtok(NULL,NWNX_GDBM_TOKEN);
    char* read_only = strtok(NULL,NWNX_GDBM_TOKEN);

	if ( (name==NULL)||(*name=='\0') || (read_only==NULL)||(*read_only=='\0') || (create==NULL)||(*create=='\0') ) {

		Log(0,"ERROR: Missing argument to OPEN method, was passed [%s]\n",Parameters);
        strcpy(Parameters,"0");

    } else {

		Log(4,"NAM [%s]\n",name);
		Log(4,"CRT [%s]\n",create);
		Log(4,"RDO [%s]\n",read_only);

        strcpy(Parameters,"0");

        gdbm_file_pool_t::iterator node_itr = gdbm_file_pool.find(name);
    
        if (node_itr != gdbm_file_pool.end()) {

            // In a number of use cases, it is much simpler to simply open the
            // database file before access, rather than verify the state, so
            // redundant open here must just return a success message.
    
    	    Log(2,"Found open dbf name [%s], nothing to do.\n",name);

            node_itr->second.act = time(NULL);

            strcpy(Parameters,"1");
    
        } else {
    
    	    Log(2,"No open dbf name [%s] found, opening.\n",name);
    
            snprintf(tmp_filepath,FILENAME_MAX,"%s/%s.gdbm",cfg_filepath,name);
        	Log(2,"Opening/creating GDBM database file [%s]\n",tmp_filepath);

            int mode = 0;

            if (strcmp(read_only,"1")==0) mode|=GDBM_READER; else if (strcmp(create,"1")==0) mode|=GDBM_WRCREAT; else mode|=GDBM_WRITER;
            if (cfg_sync_write) mode |= GDBM_SYNC;
            if (!cfg_lock_file) mode |= GDBM_NOLOCK;

        	Log(2,"Opening/creating GDBM database file with mode [%08X]\n",mode);

            gdbm_file_node_t node;
    
        	node.dbf = gdbm_open(tmp_filepath,0,GDBM_WRCREAT|GDBM_NOLOCK|GDBM_SYNC,0600,0);
    
        	if (node.dbf == NULL) {

    			Log(0,"ERROR: Call to gdbm_open failed for database filename [%s], %s\n",tmp_filepath,gdbm_strerror(gdbm_errno));

        	} else {
    
            	Log(0,"Opened GDBM database [%s]\n",tmp_filepath);
    
                node.act = time(NULL);
    
                // FUTURE: check if pool full here
                
                gdbm_file_pool[name] = node;

                strcpy(Parameters,"1");
    
            }

        }

    }

    free(arg);

	return NULL;
}

// Arg 1 - the name of the database
// Ret   - nothing
//
char* CNWNXgdbm::Sync(char* gameObject, char* Parameters) 
{

	if ( (Parameters==NULL)||(*Parameters=='\0') ) {

		Log(0,"ERROR: Missing argument to SYNC method, was passed [%s]\n",Parameters);

    } else {

        gdbm_file_pool_t::iterator node_itr = gdbm_file_pool.find(Parameters);

        if (node_itr != gdbm_file_pool.end()) {
	
        	Log(0,"Syncing GDBM database name [%s]\n",Parameters);

        	gdbm_sync(node_itr->second.dbf);
            // returns void, nothing to check

           	Log(0,"Test time 1 [%d]\n",node_itr->second.act);
            node_itr->second.act = time(NULL);
        	Log(0,"Test time 2 [%d]\n",node_itr->second.act);
 
        } else {
 
        	Log(0,"ERROR: SYNC Could not find open GDBM database name [%s]\n",Parameters);

        }
    }

	return NULL;
}


// Arg 1 - the name of the database
// Ret   - nothing
//
char* CNWNXgdbm::Close(char* gameObject, char* Parameters) 
{

	if ( (Parameters==NULL)||(*Parameters=='\0') ) {

		Log(0,"ERROR: Missing argument to CLOSE method, was passed [%s]\n",Parameters);

    } else {

        gdbm_file_pool_t::iterator node_itr = gdbm_file_pool.find(Parameters);
    
        if (node_itr != gdbm_file_pool.end()) {
    	
    	    Log(0,"INFO: Closing GDBM database name [%s]\n",Parameters);
    
    	    gdbm_close(node_itr->second.dbf);
            // returns void, nothing to check
    
            gdbm_file_pool.erase(node_itr);
    
        } else {
    
        	Log(0,"WARNING: CLOSE Could not find open GDBM database name [%s]\n",Parameters);
    
        }
    }

	return NULL;
}

char* CNWNXgdbm::CloseAll(char* gameObject, char* Parameters)
{
    CloseAll();
    return NULL;
}

void CNWNXgdbm::CloseAll()
{
    gdbm_file_pool_t::iterator node_itr = gdbm_file_pool.begin();

    while( node_itr != gdbm_file_pool.end() ) {

        Log(0,"INFO: Closing GDBM database name [%s]\n",node_itr->first.c_str());
        gdbm_close(node_itr->second.dbf);
        gdbm_file_pool.erase(node_itr);
        node_itr++;

    }

}

//
// Data Manipulation Methods
//

// Arg 1 - the name of the database
// Arg 2 - replace flag (default = 1 (TRUE))
// Arg 3 - the key
// Arg 4 - the value
//
// IMPORTANT, the key may contain a seperator so it must be diff from the arg
//            separator or things get confused here.
//
// Ret   - TRUE on successful insert, if replace flag = 0 (FALSE), FALSE is
//         returned for collision and the value is not replaced.
//

char* CNWNXgdbm::Store(char* gameObject, char* Parameters)
{
    char* arg = strcpy((char*)malloc(strlen(Parameters)+1),Parameters);
    char** args = UnBundleArguments(arg);

    datum key, val;
	char* name = args[0];
    if ((name==NULL)||(*name=='\0')) { Log(0,"ERROR: Missing arg1 to STORE in [%s]\n",Parameters); strcpy(Parameters,"0"); return NULL; }
	char* repl = args[1];
    if ((repl==NULL)||(*repl=='\0')) { Log(0,"ERROR: Missing arg2 to STORE in [%s]\n",Parameters); strcpy(Parameters,"0"); return NULL; }
	key.dptr = args[2];
    if ((key.dptr==NULL)||(*key.dptr=='\0')) { Log(0,"ERROR: Missing arg3 to STORE in [%s]\n",Parameters); strcpy(Parameters,"0"); return NULL; }
	val.dptr = args[3];
    if ((val.dptr==NULL)||(*val.dptr=='\0')) { Log(0,"ERROR: Missing arg4 to STORE in [%s]\n",Parameters); strcpy(Parameters,"0"); return NULL; }

/*
    datum key, val;
	char* name = strtok(arg,NWNX_GDBM_TOKEN);
	char* repl = strtok(NULL,NWNX_GDBM_TOKEN);
	key.dptr   = strtok(NULL,NWNX_GDBM_TOKEN);
	val.dptr   = strtok(NULL,NWNX_GDBM_TOKEN); // hmmm, will truncate if val contains TOKEN
*/

	if ( (name==NULL)||(*name=='\0') || (key.dptr==NULL)||(*key.dptr=='\0') || (val.dptr == NULL)||(*val.dptr=='\0') || (repl==NULL)||(*repl=='\0') ) {

		Log(0,"ERROR: Missing argument to STORE method, was passed [%s]\n",Parameters);

        strcpy(Parameters,"0");

    } else {
	
	    key.dsize = strlen(key.dptr)+1;
	    val.dsize = strlen(val.dptr)+1;

		Log(4,"NAM [%s]\n",name);
		Log(4,"KEY [%s]\n",key.dptr);
		Log(4,"RPL [%s]\n",repl);
		Log(4,"VAL [%s]\n",val.dptr);

        strcpy(Parameters,"0");

        gdbm_file_pool_t::iterator node_itr = gdbm_file_pool.find(name);

        if (node_itr != gdbm_file_pool.end()) {

            int flag = ((strcmp(repl,"1")==0)?GDBM_REPLACE:GDBM_INSERT);

       		int ret = gdbm_store(node_itr->second.dbf,key,val,flag);

    		if (ret <= -1) {
    			Log(0,"ERROR: Call to gdbm_store failed for database name [%s] %s\n",name,gdbm_strerror(gdbm_errno));
    		} else if (ret == 0) {
                // ret = 0 indicates success, ret = 1 when GDBM_INSERT indicates key collision -> drop through return "0"
                strcpy(Parameters,"1");
            }

            node_itr->second.act = time(NULL);

        } else {
	        Log(0,"WARNING: STORE: Could not find open GDBM database name [%s]\n",name);
        }

	}

    free(arg);

	return NULL;
}

// Arg 1 - the name of the database
// Arg 2 - the key
// Ret   - 1 if key exists
//
char* CNWNXgdbm::Exists(char* gameObject, char* Parameters)
{
    char* arg = strcpy((char*)malloc(strlen(Parameters)+1),Parameters);

    datum key, val;
	char* name = strtok(arg,NWNX_GDBM_TOKEN);
	key.dptr   = strtok(NULL,NWNX_GDBM_TOKEN);

	if ( (name==NULL)||(*name=='\0') || (key.dptr==NULL)||(*key.dptr=='\0') ) {

		Log(0,"ERROR: Missing argument to EXISTS method, was passed [%s]\n",Parameters);

        strcpy(Parameters,"0");

    } else {
	
        key.dsize = strlen(key.dptr)+1;

		Log(4,"NAM [%s]\n",name);
		Log(4,"KEY [%s]\n",key.dptr);

        strcpy(Parameters,"0");

        gdbm_file_pool_t::iterator node_itr = gdbm_file_pool.find(name);

        if (node_itr != gdbm_file_pool.end()) {

    		int ret = gdbm_exists(node_itr->second.dbf,key);

    		if (ret <= -1) {
    			Log(0,"ERROR: Call to gdbm_exists failed for database name [%s], %s\n",name,gdbm_strerror(gdbm_errno));
    		} else if (ret == 1) {
                strcpy(Parameters,"1");
            }

            node_itr->second.act = time(NULL);

        } else {
	        Log(0,"WARNING: EXISTS: Could not find open GDBM database name [%s]\n",name);
        }

	}

    free(arg);

	return NULL;
}

// Arg 1 - the name of the database
// Arg 2 - the key
// Ret   - the value
//
char* CNWNXgdbm::Fetch(char* gameObject, char* Parameters)
{
    char* arg = strcpy((char*)malloc(strlen(Parameters)+1),Parameters);

    datum key, val;
	char* name = strtok(arg,NWNX_GDBM_TOKEN);
	key.dptr   = strtok(NULL,NWNX_GDBM_TOKEN);
	val.dptr = NULL;

	if ( (name==NULL)||(*name=='\0') || (key.dptr==NULL)||(*key.dptr=='\0') ) {

		Log(0,"ERROR: Missing argument to FETCH method, was passed [%s]\n",Parameters);

        strcpy(Parameters,"");

    } else {
	
		key.dsize = strlen(key.dptr)+1;

		Log(4,"NAM [%s]\n",name);
		Log(4,"KEY [%s]\n",key.dptr);

        gdbm_file_pool_t::iterator node_itr = gdbm_file_pool.find(name);

        if (node_itr != gdbm_file_pool.end()) {

		    val = gdbm_fetch(node_itr->second.dbf,key);

		    if (val.dptr==NULL) strcpy(Parameters,"");

            node_itr->second.act = time(NULL);

        } else {
	        Log(0,"WARNING: FETCH: Could not find open GDBM database name [%s]\n",name);
            strcpy(Parameters,"");
        }

	}

    free(arg);

	// gdbm allocates this with malloc so NWNX lib is ok to free after use.
	return val.dptr;

}

// Arg 1 - the name of the database
// Arg 2 - the key
// Ret   - 1 if the key was found, 0 if dne (via Parameters)
//
char* CNWNXgdbm::Delete(char* gameObject, char* Parameters)
{
    char* arg = strcpy((char*)malloc(strlen(Parameters)+1),Parameters);

    datum key;
	char* name = strtok(arg,NWNX_GDBM_TOKEN);
	key.dptr   = strtok(NULL,NWNX_GDBM_TOKEN);

	if ( (name==NULL)||(*name=='\0') || (key.dptr==NULL)||(*key.dptr=='\0') ) {

		Log(0,"ERROR: Missing argument to DELETE method, was passed [%s]\n",Parameters);

        strcpy(Parameters,"0");

    } else {
	
		key.dsize = strlen(key.dptr)+1;

		Log(4,"NAM [%s]\n",name);
		Log(4,"KEY [%s]\n",key.dptr);

        strcpy(Parameters,"0");

        gdbm_file_pool_t::iterator node_itr = gdbm_file_pool.find(name);

        if (node_itr != gdbm_file_pool.end()) {

		    int ret = gdbm_delete(node_itr->second.dbf,key);

    		if ((ret <= -1)&&(gdbm_errno!=GDBM_ITEM_NOT_FOUND)) {
    			Log(0,"ERROR: Call to gdbm_delete failed for database name [%s], %s\n",name,gdbm_strerror(gdbm_errno));
    		} else if (ret == 0) {
                strcpy(Parameters,"1");
            }

            node_itr->second.act = time(NULL);

        } else {
	        Log(0,"WARNING: DELETE: Could not find open GDBM database name [%s]\n",name);
        }

	}

    free(arg);

	return NULL;
}


/* TODO */

// Arg 1 - the name of the database
// Ret   - the first key
//
char* CNWNXgdbm::FirstKey(char* gameObject, char* Parameters)
{
    Log(0,"ERROR: FIRSTKEY: Call to unimplemented feature with [%s]\n",Parameters);
    return NULL;
}

// Arg 1 - the name of the database
// Ret   - the next key
//
char* CNWNXgdbm::NextKey(char* gameObject, char* Parameters)
{
    Log(0,"ERROR: NEXTKEY: Call to unimplemented feature with [%s]\n",Parameters);
    return NULL;
}

//
// Parser for length specified argument bundles. This is needed when more than the 
// last argument passed via the SetLocalString could have unsafe user input text
// Currently only required for the store op since it requires both a key and a 
// value which could potentially contain parse tokens.
//
// Takes a mutable argument list of chars* in the form "len1|str1|len2|str2|" and
// returns array of ptrs to null terminated args. Nulls are embedded in the passed
// string and the pointers reference offsets in the passed argument string.
//
char** CNWNXgdbm::UnBundleArguments(char* args) {

    int i = 0;
    int len = 0;

    char* p = args;
    char* q = NULL;

    while (*p!='\0') {
        q = strchr(p,'|');
        if (q==NULL) break; // format error
        *q = '\0';
        q++;
        if (sscanf(p,"%d",&len)<1) break; // format error
		//Log(5,"i %d, len [%d]\n",i,len);
        p = q + len;
        *p = '\0';
        p++;
		//Log(5,"i %d, arg [%s]\n",i,q);
        _arg_list[i] = q;
        i++;
        if (i > 9) break; // max args
    }
    _arg_list[i] = NULL; 
    return _arg_list;
}

