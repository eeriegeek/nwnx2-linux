/***************************************************************************
    CMySQL.h: interface for the CMySQL class.
    Copyright (C) 2004 Jeroen Broekhuizen (nwnx@jengine.nl)
    copyright (c) 2006 dumbo (dumbo@nm.ru) & virusman (virusman@virusman.ru)
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

#if !defined _MYSQL_H_
#define _MYSQL_H_

#include "db.h"

#include <mysql.h>

#define NWNX_MYSQL_MAX_PARAMETERS 32

class CMySQL : public CDB
{

public:

	CMySQL();
	~CMySQL();

	const char* GetDbId ();

	bool Connect ();
	int Connect (const char *server, const char *user, const char *pass, const char *db, unsigned int port, const char *unix_socket, const char *charset);
	bool SetCharacterSet (const char *charset);
	void Disconnect ();

	char* Escape(unsigned char *buffer, unsigned long buffersize);

	int Execute (const char* query);
	char* Fetch (char *row_buffer, unsigned int row_buffer_size);

	bool WriteScorcoData (char* SQL, BYTE* pData, int Length);
	BYTE* ReadScorcoData (char* SQL, char *param, bool* pSqlError, int *size);

	int Prepare (char *query);
	int ExecutePrepared (parsed_bundle_t* bind_params, unsigned int bind_param_count);
	char* FetchPrepared (char *row_buffer, unsigned int row_buffer_size);

private:

	//
	// The database connection.
	//
	MYSQL* db_connection;

	//
	// The latest query result for simple queries only. For prepared statements
	// the results are managed with the MYSQL_STMT object and BIND buffers.
	//
	MYSQL_RES* db_result;

	//
	// The latest prepared statment.
	//
	MYSQL_STMT* db_statement;

	//
	// Query bind structs
	//
	MYSQL_BIND    query_bind[NWNX_MYSQL_MAX_PARAMETERS];
	my_bool       query_bind_is_null[NWNX_MYSQL_MAX_PARAMETERS];
	unsigned long query_bind_length[NWNX_MYSQL_MAX_PARAMETERS];

	//
	// Result bind structs	
	//
	MYSQL_BIND     result_bind[NWNX_MYSQL_MAX_PARAMETERS];
	my_bool        result_bind_is_null[NWNX_MYSQL_MAX_PARAMETERS];
	unsigned long  result_bind_length[NWNX_MYSQL_MAX_PARAMETERS];
	my_bool        result_bind_error[NWNX_MYSQL_MAX_PARAMETERS];

	unsigned char* result_bind_buffer[NWNX_MYSQL_MAX_PARAMETERS];
	unsigned int   result_bind_buffer_size[NWNX_MYSQL_MAX_PARAMETERS];

	//
	//
	//
	int fetch_state;

	//
	// Other
	//
	void ResetBind ();
	void DumpBind(MYSQL_BIND *b, int max);

};

#endif
