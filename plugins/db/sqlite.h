/***************************************************************************
    sqlite.h: interface for the CSQLITE class.
    Copyright (C) 2005 Ingmar Stieger (papillon@blackdagger.com)
    Copyright (C) 2007 virusman (virusman@virusman.ru)
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

#if !defined _SQLITE_H_
#define _SQLITE_H_

#include "db.h"

#include <sqlite3.h>

class CSQLite : public CDB
{

public:

	CSQLite();
	~CSQLite();

	const char* GetDbId ();

	bool Connect();
	int Connect(char *db);
	void Disconnect();

	char* Escape(unsigned char *buffer, unsigned long buffersize);

	int Execute (const char* query);
	char* Fetch (char *row_buffer, unsigned int row_buffer_size);

	bool WriteScorcoData(char* SQL, BYTE* pData, int Length);
	BYTE* ReadScorcoData(char* SQL, char *param, bool* pSqlError, int *size);

	int Prepare (char *query);
	int ExecutePrepared (parsed_bundle_t* bind_params, unsigned int bind_param_count);
	char* FetchPrepared (char *row_buffer, unsigned int row_buffer_size);

private:

	sqlite3* db_connection;

	sqlite3_stmt* db_statement;

	bool first_row_read;
	
	void finalizeStatement();

};

#endif

