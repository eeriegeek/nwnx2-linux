/******************************************************************************

  GdbmPool.cpp - Implementation of GDBM database file pool.

  Copyright 2013 eeriegeek (eeriegeek@yahoo.com)

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

// Notes:
//
// Assumes KEYS do not contain nulls (uses strlen on them)
//
// FUTURE: 
// - Implement pool size limit by closing oldest file.
// - Implement iteration of database file
//

#include "GdbmPool.h"

#define LOG_ERROR(...) printf("ERROR: "__VA_ARGS__)
#define LOG_WARN(...) printf("WARNING: "__VA_ARGS__)
//#define LOG_INFO(...) printf("INFO: "__VA_ARGS__)
//#define LOG_DEBUG(...) printf("DB: "__VA_ARGS__)

//#define LOG_ERROR(...) ;
//#define LOG_WARN(...) ;
#define LOG_INFO(...) ;
#define LOG_DEBUG(...) ;

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/resource.h>

// Constructor/Destructor
//
GdbmPool::GdbmPool(const char* gdbm_database_dir, bool sync_write, bool lock_file) :
	sync_write(sync_write),
	lock_file(lock_file)
{
	this->gdbm_database_dir = strdup(gdbm_database_dir);
	tmp_filepath = new char[FILENAME_MAX+1];
	// FUTURE
    struct rlimit rlim_nofile;
    int r = getrlimit(RLIMIT_NOFILE,&rlim_nofile);
    if (r==-1) {
        //LOG_DEBUG("Cannot get file limits. %s\n",strerror(errno));
    } else {
        //LOG_DEBUG("Checking file descriptor limits: soft=%lld, hard=%lld\n", (long long) rlim_nofile.rlim_cur, (long long) rlim_nofile.rlim_max);
    }
}
GdbmPool::~GdbmPool()
{
	close_all();
	free(gdbm_database_dir);
	delete [] tmp_filepath;
}

//
// Database Lifecycle Methods
//

// return true if created/opened successfully
bool GdbmPool::create(const char* database_name, bool truncate)
{
	if ((database_name==NULL)||(database_name[0]=='\x00')) return false;

	gdbm_file_pool_t::iterator node_itr = gdbm_file_pool.find(database_name);
	if (node_itr != gdbm_file_pool.end()) {
		LOG_WARN("Create called on open GDBM database name [%s], will close and proceed.\n",database_name);
		gdbm_close(node_itr->second.dbf);
		gdbm_file_pool.erase(node_itr);
	} else {
		LOG_DEBUG("No open dbf name [%s] found while creating, good.\n",database_name);
	}

	snprintf(tmp_filepath,FILENAME_MAX,"%s/%s.gdbm",gdbm_database_dir,database_name);
 	LOG_INFO("Creating/Reopening GDBM database file [%s]\n",tmp_filepath);

	gdbm_file_node_t node;
	if (truncate) {
		LOG_INFO("Creating GDBM database file [%s], with truncation.\n",tmp_filepath);
		node.dbf = gdbm_open(tmp_filepath,0,GDBM_WRCREAT|GDBM_NEWDB|GDBM_NOLOCK|GDBM_SYNC,0600,0);
	} else {
		LOG_INFO("Creating GDBM database file [%s]\n",tmp_filepath);
		node.dbf = gdbm_open(tmp_filepath,0,GDBM_WRCREAT|GDBM_NOLOCK|GDBM_SYNC,0600,0);
	}
 
	if (node.dbf == NULL) {
		LOG_ERROR("Call to gdbm_open failed for database filename [%s], %s\n",tmp_filepath,gdbm_strerror(gdbm_errno));
		return false;
	} else {
	   	LOG_INFO("Creation of GDBM database [%s] successful.\n",tmp_filepath);
		node.act = time(NULL);
		// FUTURE: if limiting pooling, check if pool full here, and reallocate
		gdbm_file_pool[database_name] = node;
		LOG_INFO("There are now %d opened GDBM databases\n",gdbm_file_pool.size());
	}
	return true;
}

// Calls gdbm reorganize on the database, logs and ignores any errors.
void GdbmPool::reorganize(const char* database_name)
{
	if ((database_name==NULL)||(database_name[0]=='\x00')) return;

	gdbm_file_pool_t::iterator node_itr = gdbm_file_pool.find(database_name);
	if (node_itr != gdbm_file_pool.end()) {
		LOG_INFO("Reorganizing GDBM database name [%s]\n",database_name);
		int ret = gdbm_reorganize(node_itr->second.dbf);
		if (ret == -1) {
			LOG_ERROR("Call to gdbm_reorganize failed for database name [%s], %s\n",database_name,gdbm_strerror(gdbm_errno));
		}
		node_itr->second.act = time(NULL);
	} else {
		LOG_ERROR("Reorganize could not find open GDBM database name [%s]\n",database_name);
	}
}

// returns true if file unlink ok
bool GdbmPool::destroy(const char* database_name)
{
	if ((database_name==NULL)||(database_name[0]=='\x00')) return false;

	gdbm_file_pool_t::iterator node_itr = gdbm_file_pool.find(database_name);
	if (node_itr != gdbm_file_pool.end()) {
		LOG_INFO("Destroy called on open GDBM database name [%s], will close and proceed.\n",database_name);
		gdbm_close(node_itr->second.dbf);
		gdbm_file_pool.erase(node_itr);
	} else {
		LOG_DEBUG("No open dbf name [%s] found while destroying, good.\n",database_name);
	}

	snprintf(tmp_filepath,FILENAME_MAX,"%s/%s.gdbm",gdbm_database_dir,database_name);
	LOG_INFO("Deleting GDBM database file [%s]\n",tmp_filepath);

	int ret = unlink(tmp_filepath);
	if (ret == -1) {
		LOG_ERROR("Call to unlink failed for database filename [%s], %s\n",tmp_filepath,strerror(errno));
		return false;
	}
	return true;
}

// return true if the database is opened
bool GdbmPool::open(const char* database_name, bool create, bool read_only)
{
	if ((database_name==NULL)||(database_name[0]=='\x00')) return false;

	gdbm_file_pool_t::iterator node_itr = gdbm_file_pool.find(database_name);
	if (node_itr != gdbm_file_pool.end()) {
		// In a number of use cases, it is much simpler to simply open the
		// database file before access, rather than verify the state, so
		// redundant open here must just return a success message.
		LOG_INFO("Found open dbf name [%s], nothing to do.\n",database_name);
		node_itr->second.act = time(NULL);
		return true;
	}
	LOG_INFO("No open dbf name [%s] found, opening.\n",database_name);

	snprintf(tmp_filepath,FILENAME_MAX,"%s/%s.gdbm",gdbm_database_dir,database_name);
	LOG_INFO("Opening/creating GDBM database file [%s]\n",tmp_filepath);

	int mode = 0;
	if (read_only) mode|=GDBM_READER; else if (create) mode|=GDBM_WRCREAT; else mode|=GDBM_WRITER;
	if (sync_write) mode |= GDBM_SYNC;
	if (lock_file) mode |= GDBM_NOLOCK;
	LOG_INFO("Opening/creating GDBM database file with mode [%08X]\n",mode);

	gdbm_file_node_t node;
	node.dbf = gdbm_open(tmp_filepath,0,GDBM_WRCREAT|GDBM_NOLOCK|GDBM_SYNC,0600,0);
	if (node.dbf == NULL) {
		LOG_ERROR("Call to gdbm_open failed for database filename [%s], %s\n",tmp_filepath,gdbm_strerror(gdbm_errno));
		return false;
	}
	LOG_INFO("Opened GDBM database [%s]\n",tmp_filepath);
	node.act = time(NULL);
	// FUTURE: check if pool full here
	gdbm_file_pool[database_name] = node;
	LOG_INFO("There are now %d opened GDBM databases\n",gdbm_file_pool.size());

	return true;
}

void GdbmPool::sync(const char* database_name)
{
	if ((database_name==NULL)||(database_name[0]=='\x00')) return;

	gdbm_file_pool_t::iterator node_itr = gdbm_file_pool.find(database_name);
	if (node_itr == gdbm_file_pool.end()) {
		LOG_WARN("Could not find open GDBM database named [%s] to sync\n",database_name);
		return;
	}
	LOG_INFO("Syncing GDBM database name [%s]\n",database_name);
	gdbm_sync(node_itr->second.dbf);
	node_itr->second.act = time(NULL);
}

void GdbmPool::close(const char* database_name)
{
	if ((database_name==NULL)||(database_name[0]=='\x00')) return;

	gdbm_file_pool_t::iterator node_itr = gdbm_file_pool.find(database_name);
	if (node_itr == gdbm_file_pool.end()) {
		LOG_WARN("Could not find open GDBM database name [%s] to close\n",database_name);
		return;
	}
	LOG_INFO("Closing GDBM database name [%s]\n",database_name);
	gdbm_close(node_itr->second.dbf);
	gdbm_file_pool.erase(node_itr);
}

void GdbmPool::close_all()
{
	gdbm_file_pool_t::iterator node_itr;
	for (node_itr = gdbm_file_pool.begin(); node_itr != gdbm_file_pool.end(); node_itr++) {
		LOG_INFO("Closing GDBM database name [%s]\n",node_itr->first.c_str());
		gdbm_close(node_itr->second.dbf);
	}
}

//
// Data Manipulation Methods
//

// if replace is false, will not overwrite values
// returns true iff the database was updated
bool GdbmPool::store(const char* database_name, const char* key, const char* value, size_t value_size, bool replace, bool open)
{
	if ((database_name==NULL)||(database_name[0]=='\x00')) return false;
	if ((key==NULL)||(key[0]=='\x00')) return false;
	if ((value==NULL)||(value[0]=='\x00')) return false;

	gdbm_file_pool_t::iterator node_itr = gdbm_file_pool.find(database_name);
	if (node_itr == gdbm_file_pool.end()) {
		if (open) {
			if (!GdbmPool::open(database_name)) return false;
			node_itr = gdbm_file_pool.find(database_name);
			if (node_itr == gdbm_file_pool.end()) {
				LOG_ERROR("Could not open GDBM database name [%s] to store, key [%s]\n",database_name, key);
				return false;
			}
		} else {
			LOG_ERROR("Could not find open GDBM database name [%s] to store, key [%s]\n",database_name, key);
			return false;
		}
	}
	node_itr->second.act = time(NULL);

	datum d_key, d_val;
	d_key.dptr = const_cast<char*>(key);
	d_key.dsize = strlen(key)+1;
	d_val.dptr = const_cast<char*>(value);
	d_val.dsize = value_size;
	LOG_DEBUG("NAM [%s]\n",database_name);
	LOG_DEBUG("KEY [%s]\n",d_key.dptr);
	LOG_DEBUG("VAL [%s] SIZ [%u]\n",d_val.dptr,d_val.dsize);
	LOG_DEBUG("RPL [%d]\n",replace);

	int flag = ((replace)?GDBM_REPLACE:GDBM_INSERT);
	int ret = gdbm_store(node_itr->second.dbf,d_key,d_val,flag);
	if (ret == -1) {
		// ret == -1 if called by a reader
		LOG_ERROR("Call to gdbm_store failed for database name [%s] %s\n",database_name,gdbm_strerror(gdbm_errno));
		return false;
	} else if (ret == 0) {
		// ret == 0 indicates successful store (insert or update)
		return true;
	} else if (ret == 1) {
		// ret == 1 when GDBM_INSERT indicates key collision when replace is false
		// the database value is not updated
		return false;
	}
	return false;
}

// returns true iff the key exists in the database
bool GdbmPool::exists(const char* database_name, const char* key, bool open) 
{
	if ((database_name==NULL)||(database_name[0]=='\x00')) return false;
	if ((key==NULL)||(key[0]=='\x00')) return false;

	gdbm_file_pool_t::iterator node_itr = gdbm_file_pool.find(database_name);
	if (node_itr == gdbm_file_pool.end()) {
		if (open) {
			if (!GdbmPool::open(database_name)) return false;
			node_itr = gdbm_file_pool.find(database_name);
			if (node_itr == gdbm_file_pool.end()) {
				LOG_ERROR("Could not open GDBM database name [%s] to check exist, key [%s]\n",database_name, key);
				return false;
			}
		} else {
			LOG_ERROR("Could not find open GDBM database name [%s] to check exist, key [%s]\n",database_name, key);
			return false;
		}
	}
	node_itr->second.act = time(NULL);

	datum d_key;
	d_key.dptr = const_cast<char*>(key);
	d_key.dsize = strlen(key)+1;
	LOG_DEBUG("NAM [%s]\n",database_name);
	LOG_DEBUG("KEY [%s]\n",d_key.dptr);

	int ret = gdbm_exists(node_itr->second.dbf,d_key);
	if (ret == -1) {
		LOG_ERROR("Call to gdbm_exists failed for database name [%s], %s\n",database_name,gdbm_strerror(gdbm_errno));
		return false;
	} else if (ret) {
		// ret evaluates TRUE if key exists
		return true;
	}
	return false;
}

// returns true and value_ptr and value_size_ptr are set if non-null value pointer found
// gdbm allocates dptr with malloc so caller should free value_ptr after use.
bool GdbmPool::fetch(const char* database_name, const char* key, char** value_ptr, size_t *value_size_ptr, bool open)
{
	if ((value_ptr==NULL)||(value_size_ptr==NULL)) return false;
	*value_ptr = NULL;
	*value_size_ptr = 0;

	if ((database_name==NULL)||(database_name[0]=='\x00')) return false;
	if ((key==NULL)||(key[0]=='\x00')) return false;

	gdbm_file_pool_t::iterator node_itr = gdbm_file_pool.find(database_name);
	if (node_itr == gdbm_file_pool.end()) {
		if (open) {
			if (!GdbmPool::open(database_name)) return false;
			node_itr = gdbm_file_pool.find(database_name);
			if (node_itr == gdbm_file_pool.end()) {
				LOG_ERROR("Could not open GDBM database name [%s] to fetch, key [%s]\n",database_name, key);
				return false;
			}
		} else {
			LOG_ERROR("Could not find open GDBM database name [%s] to fetch, key [%s]\n",database_name, key);
			return false;
		}
	}
	node_itr->second.act = time(NULL);

	datum d_key;
	d_key.dptr = const_cast<char*>(key);
	d_key.dsize = strlen(key)+1;
	LOG_DEBUG("NAM [%s]\n",database_name);
	LOG_DEBUG("KEY [%s]\n",d_key.dptr);

	datum d_val;
	d_val.dptr = NULL;
	d_val.dsize = 0;
	d_val = gdbm_fetch(node_itr->second.dbf,d_key);
	if (d_val.dptr==NULL) return false; // not found
	LOG_DEBUG("VAL [%s] SIZ [%u]\n",d_val.dptr,d_val.dsize);

	*value_ptr = d_val.dptr;
	*value_size_ptr = d_val.dsize;
	return true;
}

// returns true iff the key is found and deleted
bool GdbmPool::erase(const char* database_name, const char* key, bool open)
{
	if ((database_name==NULL)||(database_name[0]=='\x00')) return false;
	if ((key==NULL)||(key[0]=='\x00')) return false;

	gdbm_file_pool_t::iterator node_itr = gdbm_file_pool.find(database_name);
	if (node_itr == gdbm_file_pool.end()) {
		if (open) {
			if (!GdbmPool::open(database_name)) return false;
			node_itr = gdbm_file_pool.find(database_name);
			if (node_itr == gdbm_file_pool.end()) {
				LOG_ERROR("Could not open GDBM database name [%s] to delete, key [%s]\n",database_name, key);
				return false;
			}
		} else {
			LOG_ERROR("Could not find open GDBM database name [%s] to delete, key [%s]\n",database_name, key);
			return false;
		}
	}
	node_itr->second.act = time(NULL);

	datum d_key;
	d_key.dptr = const_cast<char*>(key);
	d_key.dsize = strlen(key)+1;
	LOG_DEBUG("NAM [%s]\n",database_name);
	LOG_DEBUG("KEY [%s]\n",d_key.dptr);

	int ret = gdbm_delete(node_itr->second.dbf, d_key);
	if ((ret == -1)&&(gdbm_errno!=GDBM_ITEM_NOT_FOUND)) {
		LOG_ERROR("Call to gdbm_delete failed for database name [%s], %s\n",database_name,gdbm_strerror(gdbm_errno));
		return false;
	} else if (ret == 0) {
		// ret is 0 if item is found and deleted
		return true;
	}

	return false;
}

bool GdbmPool::erase (const char* database_name, gdbm_datum_t &key, bool open)
{
	if ((database_name==NULL)||(database_name[0]=='\x00')) return false;
	if (key.size()==0) return false;

	gdbm_file_pool_t::iterator node_itr = gdbm_file_pool.find(database_name);
	if (node_itr == gdbm_file_pool.end()) {
		if (open) {
			if (!GdbmPool::open(database_name)) return false;
			node_itr = gdbm_file_pool.find(database_name);
			if (node_itr == gdbm_file_pool.end()) {
				LOG_ERROR("Could not open GDBM database name [%s] to DELETE\n",database_name);
				return false;
			}
		} else {
			LOG_ERROR("Could not find open GDBM database name [%s] to DELETE\n",database_name);
			return false;
		}
	}
	node_itr->second.act = time(NULL);

	datum d_key;
	d_key.dptr = &key[0];
	d_key.dsize = key.size();
	LOG_DEBUG("NAM [%s]\n",database_name);
	LOG_DEBUG("KEY [%s]\n",d_key.dptr);

	int ret = gdbm_delete(node_itr->second.dbf, d_key);
	if ((ret == -1)&&(gdbm_errno!=GDBM_ITEM_NOT_FOUND)) {
		LOG_ERROR("Call to gdbm_delete failed for database name [%s], %s\n",database_name,gdbm_strerror(gdbm_errno));
		return false;
	} else if (ret == 0) {
		// ret is 0 if item is found and deleted
		return true;
	}

	return false;
}
