/***************************************************************************
    sqlite.cpp: implementation of for CSQLITE class.
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

#include "sqlite.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

CSQLite::CSQLite() : CDB()
{
	db_connection = NULL;
	db_statement  = NULL;

	first_row_read = false;
}

CSQLite::~CSQLite()
{
}

const char *CSQLite::GetDbId ()
{
	return "SQLITE";
}

bool CSQLite::Connect()
{
	return Connect("sqlite.db");
}

int CSQLite::Connect(char *db)
{
	DB(printf("DB: CONNECT, opening sqlite database\n");)

	int ret = sqlite3_open_v2(db, &db_connection, SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE, NULL);
	if (ret != SQLITE_OK)
	{
	    sprintf(db_last_error_message, "Failed to open database, %s", sqlite3_errmsg(db_connection));
	    sqlite3_close(db_connection);
		db_connection = NULL;
		return -1;
	}

/*
	// begin implicit transaction
	rc = sqlite3_prepare(sdb, "BEGIN", -1, &pStmt, NULL);
	if (rc != SQLITE_OK)
		strcpy(db_last_error_message, sqlite3_errmsg(sdb));
	else
	{
		rc = sqlite3_step(pStmt);
		if (rc != SQLITE_DONE)
			strcpy(db_last_error_message, sqlite3_errmsg(sdb));
		finalizeStatement();
	}
*/

	printf("SQLITE: Open OK, library reports version %s.\n",sqlite3_libversion());

	return 0;
}

void CSQLite::Disconnect()
{
	DB(printf("DB: DISCONNECT, closing sqlite database\n");)

    if (db_statement != NULL)
    {
        sqlite3_finalize(db_statement);
        db_statement = NULL;
    }
/*
	// end implicit transaction
	finalizeStatement();
	rc = sqlite3_prepare(db_connection, "COMMIT", -1, &pStmt, NULL);
	if (rc != SQLITE_OK)
		strcpy(db_last_error_message, sqlite3_errmsg(db_connection));
	else
	{
		rc = sqlite3_step(pStmt);
		if (rc != SQLITE_DONE)
			strcpy(db_last_error_message, sqlite3_errmsg(db_connection));
	}
	finalizeStatement();
*/
	sqlite3_close(db_connection);
	db_connection = NULL;

}

int CSQLite::Execute (const char* query)
{
	int rc;

	DB(printf("DB: EXECUTE: Query [%s]\n",query);)

	if (db_statement != NULL)
	{
		sqlite3_finalize(db_statement);
		db_statement = NULL;
	}

	rc = sqlite3_prepare_v2(db_connection, query, -1, &db_statement, NULL);
	if (rc != SQLITE_OK)
	{
	    sprintf(db_last_error_message, "Failed to prepare query [%s], %s", query, sqlite3_errmsg(db_connection));
		return -1;
	}

	first_row_read = false;
	rc = sqlite3_step(db_statement);
	DB(printf("DB: EXECUTE: rc [%d]\n",rc);)
	switch(rc)
	{
		// DONE - the query has completed successfully, nothing more to do.
		case SQLITE_DONE:
			sqlite3_finalize(db_statement);
			db_statement = NULL;
			return 0;
		break;

		// ROW - the query has successfully returned 1 or more rows of data.
		case SQLITE_ROW:
			first_row_read = true;
			return 0;
		break;

		// ERROR, DEFAULT - execution of the query failed close and return error.
		case SQLITE_ERROR:
		case SQLITE_BUSY:
		case SQLITE_MISUSE:
		default:
			sprintf(db_last_error_message, "Failed to execute query [%s], %s", query, sqlite3_errmsg(db_connection));
			sqlite3_finalize(db_statement);
			db_statement = NULL;
			return -1;
		break;
	}

	return -1;
}

char* CSQLite::Fetch (char *row_buffer, unsigned int row_buffer_size)
{
	int rc;

	unsigned int total, len, i, maxCol, totalbytes;
	const char* pCol;

    char* pRet = row_buffer;
    pRet[0] = '\0';

	if (db_statement == NULL) {
		return pRet;
	}

	if (first_row_read) {
		first_row_read = false;
		rc = SQLITE_ROW;
	} else {
		rc = sqlite3_step(db_statement);
	}
	switch(rc)
	{
		// DONE - the query has completed successfully, nothing more to do.
		case SQLITE_DONE:
			sqlite3_finalize(db_statement);
			db_statement = NULL;
			return pRet;
		break;

		// ROW - the query has successfully retuned 1 or more rows.
		case SQLITE_ROW:
			// OK
		break;

		// ERROR, DEFAULT - execution of the query failed close and return error.
		case SQLITE_ERROR:
		case SQLITE_BUSY:
		case SQLITE_MISUSE:
		default:
			sprintf(db_last_error_message, "Failed to fetch row, %d, %s", rc, sqlite3_errmsg(db_connection));
			sqlite3_finalize(db_statement);
			db_statement = NULL;
			return NULL;
		break;
	}

	int num_cols = sqlite3_column_count(db_statement);
	if (num_cols <= 0) {
		// Likely we got here somehow after a non-row returning statement like update.
		return pRet;
	}
	DB(printf("DB: FETCH: num_cols [%d]\n",num_cols);)

	unsigned int row_size = 0;
	for (int i = 0; i < num_cols; i++) {
		row_size = row_size + sqlite3_column_bytes(db_statement,i) + 32;
	}
	DB(printf("DB: FETCH: row_size [%u]\n",row_size);)

    if (row_size > row_buffer_size) {
        pRet = (char*)malloc(row_size);
        pRet[0]='\0';
        DB(printf("DB: FETCH: Allocated new buffer [%p], size [%u]\n",pRet,row_size);)
    }

	char* p = pRet;
	*p++ = '|';

	for (int i = 0; i < num_cols; i++) {

		int col_type = sqlite3_column_type(db_statement,i);
		int col_size = sqlite3_column_bytes(db_statement,i);

/*
		char* c_dat = PQgetvalue(db_result,current_row,i);
		printf("DB: Field [%d], Type(OID) [%u], Null [%u], Bin [%d], Length [%d], Data [%4.4s]\n",i,c_oid,
		PQgetisnull(db_result,current_row,i),PQfformat(db_result,i),c_len,PQgetvalue(db_result,current_row,i));
*/
		DB(printf("DB: FETCH: Field [%d], Type [%d], Size [%d]\n",i,col_type,col_size);)
		
		switch (col_type)
		{
			// Note that SQLITE is slightly different than the other databases here,
			// it drops all type information for NULL fields, so all NULL values 
			// are returned here as strings, even if the column type was int or float.
			// This should work out ok, since we pass strings back anyway with late
			// conversion to other types. NULL is typeless anyway in SQL.
			//
			case SQLITE_NULL: 
				sprintf(p,"2|1|0||");
				p = (char*)rawmemchr(p,'\0');
			break;

			case SQLITE_BLOB:
			{
				unsigned char *blob = (unsigned char *)sqlite3_column_blob(db_statement,i);
				// NULL here means an empty string, not NULL SQL field.
				if (blob == NULL) {
					sprintf(p,"2|0|0||");
					p = (char*)rawmemchr(p,'\0');
				} else {
					if (strncmp("UTI V3.28",(char*)blob,9)==0) {
						sprintf(p,"1|0|20|data://nwnx/blob/UTI|");
						p = (char*)rawmemchr(p,'\0');
						memcpy(blob_buffer,(char*)blob,col_size);
						blob_buffer_size = col_size;
					} else if (strncmp("BIC V3.28",(char*)blob,9)==0) {
						sprintf(p,"1|0|20|data://nwnx/blob/BIC|");
						p = (char*)rawmemchr(p,'\0');
						memcpy(blob_buffer,(char*)blob,col_size);
						blob_buffer_size = col_size;
					} else if (strncmp("UTP V3.28",(char*)blob,9)==0) {
						sprintf(p,"1|0|20|data://nwnx/blob/UTP|");
						p = (char*)rawmemchr(p,'\0');
						memcpy(blob_buffer,(char*)blob,col_size);
						blob_buffer_size = col_size;
					} else if (strncmp("UTM V3.28",(char*)blob,9)==0) {
						sprintf(p,"1|0|20|data://nwnx/blob/UTM|");
						p = (char*)rawmemchr(p,'\0');
						memcpy(blob_buffer,(char*)blob,col_size);
						blob_buffer_size = col_size;
					} else if (strncmp("UTT V3.28",(char*)blob,9)==0) {
						sprintf(p,"1|0|20|data://nwnx/blob/UTT|");
						p = (char*)rawmemchr(p,'\0');
						memcpy(blob_buffer,(char*)blob,col_size);
						blob_buffer_size = col_size;
					} else {
						sprintf(p,"2|0|%u|",(uint32_t)col_size);
						p = (char*)rawmemchr(p,'\0');
						memcpy(p,blob,col_size);
						p += col_size;
						*p++ = '|';
					}
				}
			}
			break;

			case SQLITE_INTEGER:
				sprintf(p,"3|0|%u|%d|",col_size,sqlite3_column_int(db_statement,i));
				p = (char*)rawmemchr(p,'\0');
			break;

			case SQLITE_FLOAT:
				sprintf(p,"4|0|%u|%f|",col_size,sqlite3_column_double(db_statement,i));
				p = (char*)rawmemchr(p,'\0');
			break;

			case SQLITE_TEXT: 
			default:
			{
				const unsigned char *text = sqlite3_column_text(db_statement,i);
				// According to the docs, this should never return NULL, still...
				if (text == NULL) {
					sprintf(p,"2|0|0||");
					p = (char*)rawmemchr(p,'\0');
				} else {
					sprintf(p,"2|0|%u|",(uint32_t)col_size);
					p = (char*)rawmemchr(p,'\0');
					memcpy(p,text,col_size);
					p += col_size;
					*p++ = '|';
				}
			}
			break;

		}

	}

	*p++ = '|';
	*p = '\0';

    DB(printf("DB: buf [%s]\n",pRet);)

	return pRet;
}

char* CSQLite::Escape(unsigned char *buffer, unsigned long buffersize)
{
	if (buffer == NULL) return  NULL;
	char* p = (char*)malloc(buffersize*2+1);
	sqlite3_snprintf(buffersize*2+1,p,"%q",buffer);
    return p;
}

int CSQLite::Prepare (char *query)
{
	printf("ERROR: Unimplemented method Prepare called.\n");
    return 0;
}

int CSQLite::ExecutePrepared (parsed_bundle_t* bind_params, unsigned int bind_param_count)
{
	printf("ERROR: Unimplemented method ExecutePrepared called.\n");
    return 0;
}

char* CSQLite::FetchPrepared (char *row_buffer, unsigned int row_buffer_size)
{
	printf("ERROR: Unimplemented method FetchPrepared called.\n");
	row_buffer[0] = '\0';
	return NULL;
}

bool CSQLite::WriteScorcoData(char* SQL, BYTE* pData, int Length)
{
	int rc;
	char* pSQL = (char*) malloc(strlen(SQL)+1);
	sprintf(pSQL, SQL, "?");

	finalizeStatement();
	rc = sqlite3_prepare(db_connection, pSQL, -1, &db_statement, NULL);
	free(pSQL);

	if (rc != SQLITE_OK)
	{
		strcpy(db_last_error_message, sqlite3_errmsg(db_connection));
		finalizeStatement();
		return false;
	}

	rc = sqlite3_bind_blob(db_statement, 1, (const void*) pData, Length, SQLITE_STATIC);
	rc = sqlite3_step(db_statement);   
	finalizeStatement();

	if (rc != SQLITE_DONE)
	{
		strcpy(db_last_error_message, sqlite3_errmsg(db_connection));
		return false;
	}

	return true;
}

BYTE* CSQLite::ReadScorcoData(char* SQL, char *param, bool* pSqlError, int *size)
{
	int rc;
	const void* pBlob;

	if (strcmp(param, "FETCHMODE") != 0)
	{	
		finalizeStatement();
		rc = sqlite3_prepare(db_connection, SQL, -1, &db_statement, NULL);
		if (rc != SQLITE_OK)
		{
			strcpy(db_last_error_message, sqlite3_errmsg(db_connection));
			finalizeStatement();
			*pSqlError = true;
			return NULL;
		}
	}

	// execute step
	rc = sqlite3_step(db_statement);
	if (rc == SQLITE_ERROR)
	{
		strcpy(db_last_error_message, sqlite3_errmsg(db_connection));
		*pSqlError = true;
		finalizeStatement();
		return NULL;
	}

	*pSqlError = false;
	if (rc == SQLITE_ROW)
	{
		*size = sqlite3_column_bytes(db_statement, 0);
/*
		if (*size > MAXRESULT)
		{
			sprintf(db_last_error_message, "Critical error - object too large (>%ld bytes).\n", MAXRESULT);
			*pSqlError = true;
			return NULL;
		}
*/
		pBlob = sqlite3_column_blob(db_statement, 0);
		char* buf = new char[*size];
		memcpy(buf, pBlob, *size);

		// Return pointer to buffer allocated by new[], safe to free() on linux, pBlob
		// points to memory that will be managed by sqlite.
		return (BYTE *)buf;	
	}
	else if (rc == SQLITE_DONE)
	{
		finalizeStatement();
	}
	return NULL;
}

void CSQLite::finalizeStatement()
{
	if (db_statement)
	{
		sqlite3_finalize(db_statement);
		db_statement = NULL;
	}
}

