
//-----------------------------------------------------------------------------
// This class implements an open file pooling wrapper around GDBM. A map of
// records indexed by the file's base name maintains the open GDBM file record
// so multiple files can be kept active without opening/closing files too
// frequently. Wrapper functions also allow automatic file opening if an
// attempt is made to read/write data to a closed (not yet opened) file.
//-----------------------------------------------------------------------------

#ifndef _GdbmPool_h_
#define _GdbmPool_h_

#include <string>
#include <map>
#include <vector>

#include <gdbm.h>

// Structure holding active info on each open GDBM file in the POOL map.
//
typedef struct gdbm_file_node_s {
	GDBM_FILE dbf; // the GDBM file object
	datum     itr; // FUTURE: iterator
	time_t    act; // FUTURE: access time
} gdbm_file_node_t;

typedef std::map < std::string, gdbm_file_node_t > gdbm_file_pool_t;

typedef std::vector < char > gdbm_datum_t;

class GdbmPool
{
	public:

		// Construct the GdbmPool giving a bas directory in which all the gdbm files will
		// reside. The option sync_write can be turned off for a slight performance gain
		// at increased file corruption risk. The option lock_file can be turned on if
		// there may be other potential writers attempting to access the files.
		//
		GdbmPool(const char* gdbm_database_dir, bool sync_write = true, bool lock_file = false);

		// Destorys the GdbmPool, closes all open gdbm files and frees buffers.
		//
		virtual ~GdbmPool();

		// Major life-cycle/maintenance operations.
		//
		bool create (const char* database_name, bool truncate = false);
		void reorganize (const char* database_name);
		bool destroy (const char* database_name);

		// Basic open/close operations.
		//
		bool open (const char* database_name, bool create = true, bool read_only = false);
		void sync (const char* database_name);
		void close (const char* database_name);
		void close_all ();

		// Basic DML type operations. Each of these specifies a key as a null terminated
		// string (no nulls in key.)  Each method also has an open flag, if open==true,
		// the method will attempt to open the named database in read-write mode and
		// carry our the requested operation, otherwise it is an error for the database
		// to be closed. Delete has beend named erase to avoid collision with op delete.
		//
		bool store (const char* database_name, const char* key, const char* value, size_t value_size, bool replace = true, bool open = false);
		bool exists (const char* database_name, const char* key, bool open = false);
		bool fetch (const char* database_name, const char* key, char** value_ptr, size_t *value_size_ptr, bool open = false);
		bool erase (const char* database_name, const char* key, bool open = false);
		bool erase (const char* database_name, gdbm_datum_t &key, bool open = false);

	private:

		// Hide these from use, no implementation.
		//
		GdbmPool();
		GdbmPool(const GdbmPool& other);
		GdbmPool& operator=(const GdbmPool& rhs);

		// Database directory where all the gdbm files will reside.
		//
		char* gdbm_database_dir;

		// Configuration options for speed/integrity.
		//
		bool sync_write;
	 	bool lock_file;

		// Buffer to handle gdbm file full pathnames. Will malloc in constructor.
		//
		char* tmp_filepath;

		// Maintain a pool of open GDBF files.
		//
		gdbm_file_pool_t gdbm_file_pool;

};

#endif // _GdbmPool_h_

