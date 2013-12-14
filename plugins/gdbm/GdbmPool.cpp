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
// FUTURE: 
// - Implement pool size limit by closing oldest file.
// - Implement iteration of database file
//

#include "GdbmPool.h"

#define LOG_ERROR(...) printf("ERROR: "__VA_ARGS__)
#define LOG_WARN(...)  printf("WARNING: "__VA_ARGS__)
//#define LOG_INFO(...)  printf("INFO: "__VA_ARGS__)
//#define LOG_DEBUG(...) printf("DBUG: "__VA_ARGS__)

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
	gdbm_database_dir(gdbm_database_dir),
	open_file_limit(512),
	sync_write(sync_write),
	lock_file(lock_file)
{
	LOG_INFO("GDBMPOOL: CONSTRUCTOR\n");
	LOG_INFO("GDBMPOOL: database directory [%s], sync_write [%d], lock_file [%d]\n", gdbm_database_dir, sync_write, lock_file);
	struct rlimit rlim_nofile;
	int r = getrlimit(RLIMIT_NOFILE,&rlim_nofile);
	if (r==-1) {
		LOG_WARN("GDBMPOOL: Cannot get system open file limits, using default. %s\n", strerror(errno));
		LOG_INFO("GDBMPOOL: Setting GDBM open file limit to default [%u]\n", open_file_limit);
	} else {
		LOG_INFO("GDBMPOOL: Found system open file limits: soft [%lu], hard [%lu]\n", rlim_nofile.rlim_cur, rlim_nofile.rlim_max);
		open_file_limit = rlim_nofile.rlim_cur / 2;
		LOG_INFO("GDBMPOOL: Setting GDBM open file limit to 1/2 system soft limit [%u]\n", open_file_limit);
	}
}
GdbmPool::~GdbmPool()
{
	LOG_INFO("GDBMPOOL: DESTRUCTOR\n");
	close_all();
}

//
// Database Lifecycle Methods
//

// return true if created/opened successfully
bool GdbmPool::create(const char* database_name, bool truncate)
{
	if ((database_name==NULL)||(database_name[0]=='\x00')) return false;

	LOG_INFO("CREATE: Creating/Truncating GDBM database name [%s], truncate [%d]\n", database_name, truncate);
	// If a GDBM database of the same name is already open in the pool,
	// close it and remove it from the pool. Send a warning.
	//
	gdbm_file_pool_t::iterator node_itr = gdbm_file_pool.find(database_name);
	if (node_itr != gdbm_file_pool.end()) {
		LOG_WARN("CREATE: Create called on open GDBM database name [%s], will close and proceed.\n", database_name);
		gdbm_close(node_itr->second.dbf);
		gdbm_file_pool.erase(node_itr);
	} else {
		LOG_DEBUG("CREATE: No open dbf name [%s] found while creating, OK\n", database_name);
	}

	// Construct the full pathname from the database name and directory.
	// Construct the read_write flag from the specified options.
	//
	char tmp_filepath[FILENAME_MAX+1];
	snprintf(tmp_filepath, FILENAME_MAX, "%s/%s.gdbm", gdbm_database_dir.c_str(), database_name);
	// One of: GDBM_READER, GDBM_WRITER, GDBM_WRCREAT, GDBM_NEWDB  ( NOT bit flags ! )
	int flags = ( truncate ? GDBM_NEWDB : GDBM_WRCREAT );
	if (sync_write) flags |= GDBM_SYNC;
	if (!lock_file) flags |= GDBM_NOLOCK;

	LOG_INFO("CREATE: Creating/Truncating GDBM database filename [%s] with flags [%#0.2x]\n", tmp_filepath, flags);

	gdbm_file_node_t node;
	node.dbf = gdbm_open(tmp_filepath, 0, flags, 0600, 0);
	if (node.dbf == NULL) {
		LOG_ERROR("CREATE: Call to gdbm_open failed for database filename [%s], %s\n", tmp_filepath, gdbm_strerror(gdbm_errno));
		return false;
	}
	LOG_INFO("CREATE: Creation of GDBM database name [%s] with filepath [%s] OK\n", database_name, tmp_filepath);

	// FUTURE: If limiting pooling, check if the pool is full here, and
	// reallocate based on last access time. Currently just print a 
	// warning that open databases are getting out of hand.
	//
	node.act = time(NULL);
	gdbm_file_pool[database_name] = node;
	LOG_INFO("CREATE: There are now %d opened GDBM databases.\n",gdbm_file_pool.size());
	if ( gdbm_file_pool.size() > open_file_limit ) {
		LOG_WARN("CREATE: Open GDBM file count %u has exceeded soft limit of %u.\n", gdbm_file_pool.size(), open_file_limit);
	}

	return true;
}

// Calls gdbm reorganize on the database, logs and ignores any errors.
void GdbmPool::reorganize(const char* database_name)
{
	if ((database_name==NULL)||(database_name[0]=='\x00')) return;

	gdbm_file_pool_t::iterator node_itr = gdbm_file_pool.find(database_name);
	if (node_itr == gdbm_file_pool.end()) {
		LOG_ERROR("REORGANIZE: Could not find open GDBM database name [%s] to reorganize.\n", database_name);
	} else {
		LOG_INFO("REORGANIZE: Reorganizing GDBM database name [%s]\n", database_name);
		int ret = gdbm_reorganize(node_itr->second.dbf);
		if (ret == -1) {
			LOG_ERROR("REORGANIZE: Call to gdbm_reorganize failed for database name [%s], %s\n", database_name, gdbm_strerror(gdbm_errno));
		}
		node_itr->second.act = time(NULL);
	}
}

// returns true if file unlink ok
bool GdbmPool::destroy(const char* database_name)
{
	if ((database_name==NULL)||(database_name[0]=='\x00')) return false;

	// If a GDBM database of the same name is already open in the pool,
	// close it and remove it from the pool. Send a warning.
	//
	gdbm_file_pool_t::iterator node_itr = gdbm_file_pool.find(database_name);
	if (node_itr != gdbm_file_pool.end()) {
		LOG_WARN("DESTROY: Destroy called on open GDBM database name [%s], will close and proceed.\n", database_name);
		gdbm_close(node_itr->second.dbf);
		gdbm_file_pool.erase(node_itr);
	} else {
		LOG_DEBUG("DESTROY: No open dbf name [%s] found while destroying, OK\n", database_name);
	}

	char tmp_filepath[FILENAME_MAX+1];
	snprintf(tmp_filepath, FILENAME_MAX, "%s/%s.gdbm", gdbm_database_dir.c_str(), database_name);

	LOG_INFO("DESTROY: Calling unlink on GDBM database name [%s], filename [%s]\n", database_name, tmp_filepath);

	int ret = unlink(tmp_filepath);
	if (ret == -1) {
		LOG_ERROR("DESTROY: Call to unlink failed for database filename [%s], %s\n",tmp_filepath,strerror(errno));
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
		LOG_INFO("OPEN: Found open dbf name [%s], nothing to do\n", database_name);
		node_itr->second.act = time(NULL);
		return true;
	}
	LOG_INFO("OPEN: No open dbf name [%s] found, opening\n", database_name);

	char tmp_filepath[FILENAME_MAX+1];
	snprintf(tmp_filepath,FILENAME_MAX,"%s/%s.gdbm",gdbm_database_dir.c_str(),database_name);
	//int flags = 0;
	//if (read_only) flags = GDBM_READER; else if (create) flags = GDBM_WRCREAT; else flags = GDBM_WRITER;
	int flags = ( read_only ? GDBM_READER : ( create ? GDBM_WRCREAT : GDBM_WRITER ) );
	if (sync_write) flags |= GDBM_SYNC;
	if (!lock_file) flags |= GDBM_NOLOCK;

	LOG_INFO("OPEN: Opening/Creating GDBM database name [%s], filename [%s] with flags [%#0.2x]\n",
		database_name, tmp_filepath, flags);

	gdbm_file_node_t node;
	node.dbf = gdbm_open(tmp_filepath, 0, flags, 0600, 0);
	if (node.dbf == NULL) {
		LOG_ERROR("OPEN: Call to gdbm_open failed for database filename [%s], %s\n", tmp_filepath, gdbm_strerror(gdbm_errno));
		return false;
	}
	LOG_INFO("OPEN: Open GDBM database name [%s] OK\n", tmp_filepath);

	// FUTURE
	node.act = time(NULL);
	gdbm_file_pool[database_name] = node;
	LOG_INFO("OPEN: There are now %u opened GDBM databases\n", gdbm_file_pool.size());
	if ( gdbm_file_pool.size() > open_file_limit ) {
		LOG_WARN("OPEN: Open GDBM file count %u has exceeded soft limit of %u.\n", gdbm_file_pool.size(), open_file_limit);
	}

	return true;
}

void GdbmPool::sync(const char* database_name)
{
	if ((database_name==NULL)||(database_name[0]=='\x00')) return;

	gdbm_file_pool_t::iterator node_itr = gdbm_file_pool.find(database_name);
	if (node_itr == gdbm_file_pool.end()) {
		LOG_WARN("SYNC: Could not find open GDBM database named [%s] to sync.\n", database_name);
		return;
	}
	LOG_INFO("SYNC: Syncing GDBM database name [%s]\n",database_name);
	gdbm_sync(node_itr->second.dbf);
	node_itr->second.act = time(NULL);
}

void GdbmPool::close(const char* database_name)
{
	if ((database_name==NULL)||(database_name[0]=='\x00')) return;

	gdbm_file_pool_t::iterator node_itr = gdbm_file_pool.find(database_name);
	if (node_itr == gdbm_file_pool.end()) {
		LOG_WARN("CLOSE: Could not find open GDBM database name [%s] to close.\n", database_name);
		return;
	}
	LOG_INFO("CLOSE: Closing GDBM database name [%s]\n", database_name);
	gdbm_close(node_itr->second.dbf);
	gdbm_file_pool.erase(node_itr);
}

void GdbmPool::close_all()
{
	gdbm_file_pool_t::iterator node_itr;
	for (node_itr = gdbm_file_pool.begin(); node_itr != gdbm_file_pool.end(); node_itr++) {
		LOG_INFO("CLOSALL: Closing GDBM database name [%s]\n",node_itr->first.c_str());
		gdbm_close(node_itr->second.dbf);
	}
}

//
// Data Manipulation Methods
//

// if replace is false, will not overwrite values
// returns true iff the database was updated
bool GdbmPool::store(const char* database_name, gdbm_datum_t &key, const char* value, size_t value_size, bool replace, bool open)
{
	if ((database_name==NULL)||(database_name[0]=='\x00')) return false;
	if (key.size()==0) return false;
	if ((value==NULL)||(value[0]=='\x00')) return false;

	gdbm_file_pool_t::iterator node_itr = gdbm_file_pool.find(database_name);
	if (node_itr == gdbm_file_pool.end()) {
		if (open) {
			if (!GdbmPool::open(database_name)) return false;
			node_itr = gdbm_file_pool.find(database_name);
			if (node_itr == gdbm_file_pool.end()) {
				LOG_ERROR("STORE: Could not open GDBM database name [%s].\n", database_name);
				return false;
			}
		} else {
			LOG_ERROR("STORE: Could not find open GDBM database name [%s].\n", database_name);
			return false;
		}
	}
	node_itr->second.act = time(NULL);

	datum d_key, d_val;
	d_key.dptr = &key[0];
	d_key.dsize = key.size();
	d_val.dptr = const_cast<char*>(value);
	d_val.dsize = value_size;
	LOG_DEBUG(
		"STORE: database [%s], replace [%d], open [%d], key [%s][%u], value [%s][%u]\n",
		database_name, replace, open, d_key.dptr, d_key.dsize, d_val.dptr, d_val.dsize
	);

	int ret = gdbm_store(node_itr->second.dbf, d_key, d_val, (replace?GDBM_REPLACE:GDBM_INSERT));
	if (ret == -1) {
		// ret == -1 if called by a reader (trying to write)
		LOG_ERROR("STORE: Call to gdbm_store failed for database name [%s] %s\n", database_name, gdbm_strerror(gdbm_errno));
		return false;
	} else if (ret == 0) {
		// ret == 0 indicates successful store (insert or update)
		LOG_DEBUG("STORE: Store OK, Database [%s], Key [%s]\n", database_name, d_key.dptr);
		return true;
	} else if (ret == 1) {
		// ret == 1 when GDBM_INSERT indicates key collision when replace is false
		// the database value is not updated, but not an error
		LOG_DEBUG("STORE: Store OK, NO REPLACE, Database [%s], Key [%s]\n", database_name, d_key.dptr);
		return false;
	}

	return false;
}

// returns true iff the key exists in the database
bool GdbmPool::exists(const char* database_name, gdbm_datum_t &key, bool open) 
{
	if ((database_name==NULL)||(database_name[0]=='\x00')) return false;
	if (key.size()==0) return false;

	gdbm_file_pool_t::iterator node_itr = gdbm_file_pool.find(database_name);
	if (node_itr == gdbm_file_pool.end()) {
		if (open) {
			if (!GdbmPool::open(database_name)) return false;
			node_itr = gdbm_file_pool.find(database_name);
			if (node_itr == gdbm_file_pool.end()) {
				LOG_ERROR("EXISTS: Could not open GDBM database name [%s].\n", database_name);
				return false;
			}
		} else {
			LOG_ERROR("EXISTS: Could not find open GDBM database name [%s].\n", database_name);
			return false;
		}
	}
	node_itr->second.act = time(NULL);

	datum d_key;
	d_key.dptr = &key[0];
	d_key.dsize = key.size();
	LOG_DEBUG(
		"EXISTS: database [%s], open [%d], key [%s][%u]\n",
		database_name, open, d_key.dptr, d_key.dsize
	);

	int ret = gdbm_exists(node_itr->second.dbf,d_key);
	if (ret == -1) {
		// ret == -1 on error (may never occur for this call)
		LOG_ERROR("Call to gdbm_exists failed for database name [%s], %s\n", database_name, gdbm_strerror(gdbm_errno));
		return false;
	} else if (ret) {
		// ret evaluates TRUE (ret != 0) if key exists
		LOG_DEBUG("EXISTS: Exists OK, Database [%s], Key [%s]\n", database_name, d_key.dptr);
		return true;
	}

	return false;
}

// returns true and value_ptr and value_size_ptr are set if non-null value pointer found
// gdbm allocates dptr with malloc so caller should free value_ptr after use.
bool GdbmPool::fetch(const char* database_name, gdbm_datum_t &key, char** value_ptr, size_t *value_size_ptr, bool open)
{
	if ((value_ptr==NULL)||(value_size_ptr==NULL)) return false;
	*value_ptr = NULL;
	*value_size_ptr = 0;
	if ((database_name==NULL)||(database_name[0]=='\x00')) return false;
	if (key.size()==0) return false;

	gdbm_file_pool_t::iterator node_itr = gdbm_file_pool.find(database_name);
	if (node_itr == gdbm_file_pool.end()) {
		if (open) {
			if (!GdbmPool::open(database_name)) return false;
			node_itr = gdbm_file_pool.find(database_name);
			if (node_itr == gdbm_file_pool.end()) {
				LOG_ERROR("FETCH: Could not open GDBM database name [%s].\n", database_name);
				return false;
			}
		} else {
			LOG_ERROR("FETCH: Could not find open GDBM database name [%s].\n", database_name);
			return false;
		}
	}
	node_itr->second.act = time(NULL);

	datum d_key;
	d_key.dptr = &key[0];
	d_key.dsize = key.size();
	LOG_DEBUG(
		"FETCH: database [%s], open [%d], key [%s][%u]\n",
		database_name, open, d_key.dptr, d_key.dsize
	);

	datum d_val;
	d_val.dptr = NULL;
	d_val.dsize = 0;
	d_val = gdbm_fetch(node_itr->second.dbf, d_key);
	if (d_val.dptr == NULL) {
		if (gdbm_errno == GDBM_ITEM_NOT_FOUND) {
			// No ERROR, Item not found
		} else {
			LOG_ERROR("FETCH: Call to gdbm_fetch failed for database name [%s], %s\n", database_name, gdbm_strerror(gdbm_errno));
		}
		return false;
	} else {
		LOG_DEBUG("FETCH: Fetch OK, Database [%s], Key [%s]\n", database_name, d_key.dptr);
		LOG_DEBUG("FETCH: Value [%s] Size [%u]\n", d_val.dptr, d_val.dsize);
		*value_ptr = d_val.dptr;
		*value_size_ptr = d_val.dsize;
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
				LOG_ERROR("ERASE: Could not open GDBM database name [%s].\n", database_name);
				return false;
			}
		} else {
			LOG_ERROR("ERASE: Could not find open GDBM database name [%s].\n", database_name);
			return false;
		}
	}
	node_itr->second.act = time(NULL);

	datum d_key;
	d_key.dptr = &key[0];
	d_key.dsize = key.size();
	LOG_DEBUG(
		"ERASE: database [%s], open [%d], key [%s][%u]\n",
		database_name, open, d_key.dptr, d_key.dsize
    );

	int ret = gdbm_delete(node_itr->second.dbf, d_key);
	if (ret == -1) {
		if (gdbm_errno == GDBM_ITEM_NOT_FOUND) {
			// No ERROR, Item not found
		} else {
			LOG_ERROR("ERASE: Call to gdbm_delete failed for database name [%s], %s\n", database_name, gdbm_strerror(gdbm_errno));
		}
		return false;
	} else if (ret == 0) {
		// ret is 0 if item is found and deleted
		LOG_DEBUG("ERASE: Delete OK, Database [%s], Key [%s]\n", database_name, d_key.dptr);
		return true;
	}

	return false;
}

