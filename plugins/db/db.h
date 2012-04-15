/***************************************************************************
    CDB.h: interface for the CDB base class.
    Copyright (C) 2004 Jeroen Broekhuizen (nwnx@jengine.nl)
    copyright (c) 2006 dumbo (dumbo@nm.ru)
    Copyright (C) 2012 eeriegeek (eeriegeek@yahoo.com)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 ***************************************************************************/

#if !defined _DB_H_
#define _DB_H_

#define MAXSQL 1024

//#define MAXRESULT 1024*1024

#include <stdint.h>

#define EXTRA_DEBUG 0

#if defined EXTRA_DEBUG && EXTRA_DEBUG > 0
#define DB(X) X
#else
#define DB(X) 
#endif

typedef unsigned char BYTE;

//
// Header block structure for parmeters passed to the prepare/bind/fetch series
// of methods. Values bundled and streamed from the script vm are parsed and
// saved into a list of these structures.
//
typedef struct {
	uint32_t b_type;
	uint32_t b_size;
	uint32_t b_null;
	unsigned char* b_data;
} parsed_bundle_t;

extern "C" unsigned char *blob_buffer;
extern "C" unsigned int   blob_buffer_size;

class CDB
{

public:

	CDB();
	~CDB();

	// Get a short string identifying the current database type. Values currently
	// anticipated are: MYSQL, PGSQL, SQLITE, future possibilities: ODBC, etc.
	//
	virtual const char* GetDbId () = 0;

	virtual bool Connect () = 0;
	virtual void Disconnect () = 0;

	// Calls the native database escape method. Returns malloced memory block.
	//
	virtual char* Escape (unsigned char *buffer, unsigned long buffersize) = 0;

	//
	// Execute / Fetch, basic database interface.
	//

	// Execute any valid query string. If the query returns data rows, return the
	// expected number of rows (0..N), otherwise 0 on success, and -1 on error.
	//
	virtual int Execute (const char* query) = 0;

	// Fetch rows. Returns the bundled row for return to nwscript, or an empty
	// string if no more rows are found. Will place the result in buffer if
	// it is big enough, otherwise will malloc a new block and return a ptr.
	//
	virtual char* Fetch (char *row_buffer, unsigned int row_buffer_size) = 0;

	virtual bool WriteScorcoData (char* SQL, BYTE* pData, int Length) = 0;
	virtual BYTE* ReadScorcoData (char* SQL, char *param, bool* pSqlError, int *size) = 0;

	//
	// Prepare / ExecutePrepared / FetchPrepared, statement database interface.
	//

	// Utility routine to unbundle parameter string from nwscript and parse it.
	//
	parsed_bundle_t* ParseBundle(char* bundle);

	// Prepare a query for execution with 0 or more parameters.
	//
	virtual int Prepare (char *query) = 0;

	// Bind the parameters to the prepared query and execute it.
	//
	virtual int ExecutePrepared (parsed_bundle_t* bind_params, unsigned int bind_param_count) = 0;

	// Fetches the next available row from a prepared query. Constructs a bundled
	// string containing the column values in row_buffer if it is big enough, else
	// it mallocs a new buffer. The string returned is null terminated.
	//
	virtual char* FetchPrepared (char *row_buffer, unsigned int row_buffer_size) = 0;

	// Returns the last recorded error message from the DB interface.
	//
	const char* GetErrorMessage ();

protected:

	//
	// String buffer to store the last error message.
	//
	char db_last_error_message[1024];

};

#endif

