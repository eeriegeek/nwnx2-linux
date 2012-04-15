/***************************************************************************
    CMySQL.cpp: implementation of the CMySQL class.
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

#include "mysql.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define NWNX_MYSQL_MIN_FIELD_ALLOC 32

#define NWNX_MYSQL_NEW_SET 0
#define NWNX_MYSQL_OLD_SET 1

CMySQL::CMySQL() : CDB()
{
	db_connection = NULL;
	db_statement  = NULL;
	db_result     = NULL;

	// In theory these structures can be dynamicly allocated, but there are so many
	// other structs dependent on them that I just declare them in the object for now.
	//
	// MYSQL_BIND *bind = (MYSQL_BIND*)malloc(sizeof(MYSQL_BIND) * query_param_count);
	// memset(bind,0,sizeof(MYSQL_BIND) * query_param_count);
	//
	int i;
	for (i = 0; i < NWNX_MYSQL_MAX_PARAMETERS; i++) {
		result_bind_buffer[i] = (unsigned char *)malloc(NWNX_MYSQL_MIN_FIELD_ALLOC);
		memset(result_bind_buffer[i],'\0',NWNX_MYSQL_MIN_FIELD_ALLOC); // make valgrind happy
		//result_bind_buffer[i][0] = '\0';
		result_bind_buffer_size[i] = NWNX_MYSQL_MIN_FIELD_ALLOC;
	}

	fetch_state = NWNX_MYSQL_NEW_SET;
}

CMySQL::~CMySQL()
{
	for (int i = 0; i < NWNX_MYSQL_MAX_PARAMETERS; i++) free(result_bind_buffer[i]);
}

const char *CMySQL::GetDbId ()
{
	return "MYSQL";
}

bool CMySQL::Connect ()
{
	// try to establish a default connection
	return Connect ("localhost", "nwn", "nwnpwd", "nwn", 0, NULL, NULL);
}

int CMySQL::Connect (const char *server, const char *user, const char *pass, const char *db, unsigned int port, const char *unix_socket, const char *charset)
{
	// initialize the mysql structure
	//
	db_connection = mysql_init(NULL);
	if (db_connection == NULL)
	{
		sprintf(db_last_error_message,"Failure initializing SQL database connection.\n");
		return -1;
	}

	// Set any additional database options here.

	// Attempt the database connection.
	//
	// TODO: Test CLIENT_MULTI_RESULTS, CLIENT_MULTI_STATEMENTS Conditional for < 5.5.3 ??
	//
	//if (mysql_real_connect(db_connection,server,user,pass,db,port,unix_socket,CLIENT_MULTI_RESULTS) == NULL)
	if (mysql_real_connect(db_connection,server,user,pass,db,port,unix_socket,CLIENT_MULTI_STATEMENTS) == NULL)
	{
		sprintf(db_last_error_message,"Failure connecting to database, %s\n",mysql_error(db_connection));
		mysql_close(db_connection);
		db_connection = NULL;
		return -1;
	}

	// Get the database server version number. Client must support some functions too.
	//
	// 5.5.3 - in/out and out parameter support, mysql_stmt_next_result()
	// 5.5.3 - multiple result sets from call
	//
	unsigned long db_version = mysql_get_server_version(db_connection);
	//printf("DB Connect succeeded, server reports version %lu\n",db_version);
	int v1 = db_version / 10000;
	int v2 = (db_version - (v1*10000)) / 100;
	int v3 = (db_version - (v1*10000) - (v2*100));

	// Set the character set for language encoding. (e.g. the mysql_escape function)
	//
	if (!SetCharacterSet(charset)) {
        sprintf(db_last_error_message,"Error setting connection character encoding [%s], %s\n", charset, mysql_error(db_connection));
		return -1;
	}

	DB(printf("MYSQL: Connection OK, server reports version %d.%d.%d, connection character set is [%s].\n", v1,v2,v3,mysql_character_set_name(db_connection));)

	return 0;
}

bool CMySQL::SetCharacterSet (const char *charset)
{
	if ((charset!=NULL)&&(charset[0]!='\0')) return (mysql_set_character_set(db_connection,charset)==0);
	return true;
}

void CMySQL::Disconnect ()
{
	if (db_statement) mysql_stmt_free_result(db_statement);
	if (db_statement) mysql_stmt_close(db_statement);
	mysql_close(db_connection);
	db_connection = NULL;
}

// This is the "simple" mode interface, just run a query and fetch results.
//
int CMySQL::Execute (const char* query)
{
	DB(printf("DB: Execute: query [%s]\n",(const char *)query);)

	if (db_connection==NULL) {
		sprintf(db_last_error_message,"Error executing statement, no database connected.");
		return -1;
	}
	// consider using mysql_ping() if last attempt failed or N minutes since last query.

	// Executing a new query will cancel any ongoing result, and start a new query.
	// Since multiple result sets are a possiblity, we will also retrieve and delete
	// any on-going results sets.
	//
	if (db_result != NULL) {
		mysql_free_result(db_result);
		db_result = NULL;
	}
	while (mysql_more_results(db_connection)) {
		mysql_next_result(db_connection);
		db_result = mysql_store_result(db_connection);
		mysql_free_result(db_result);
	}
	db_result = NULL;

	// Execute the query.
	//
	if (mysql_query(db_connection, query)) {
		sprintf(db_last_error_message,"Failure executing SQL query [%s], %s\n",query,mysql_error(db_connection));
		return -1;
	}

	// Store the complete result set in local memory.
	//
	db_result = mysql_store_result(db_connection);
	if (db_result == NULL) {
		// This is the mysql approved way of checking if the query resulted in an error
		// or simply was not expected to return data (e.g. delete statement). If the field
		// count is 0 then no data was returned or expected. 
		if (mysql_field_count(db_connection) == 0) {
			return 0;
		} else {
			sprintf(db_last_error_message,"Failure storing result data, %s\n",mysql_error(db_connection));
			return -1;
		}
	} else {
		// Query successfully executed and result rows were stored back to the client.
		int row_count = mysql_affected_rows(db_connection);
		return row_count;
	}
	return -1;
}

char* CMySQL::Fetch (char *row_buffer, unsigned int row_buffer_size)
{
	char* pRet = row_buffer;

	if (row_buffer==NULL) {
		sprintf(db_last_error_message,"Internal error during fetch, output buffer should never be NULL.\n");
		return NULL;
	}
    DB(printf("DB: Pre-allocated fetch buffer size [%u]\n",row_buffer_size);)
    pRet[0] = '\0';

	if (db_connection==NULL) {
		sprintf(db_last_error_message,"Error fetching data, no database connected\n");
		return NULL;
	}

	if (db_result == NULL) {
		// No results available, not a select statement, or previous out-of-data
		return pRet;
	}

	MYSQL_ROW db_row = mysql_fetch_row(db_result);
	if (db_row==NULL) {
		// NULL means end-of-data when mysql_store_result has been used, but we may just
		// be at the end of a result set. Check if there are more result sets and if so
		// move forward to the next set then return end of data without shutting down
		// the result set. This allows the next fetch to start on the new set.
		if (mysql_more_results(db_connection)) {
			mysql_next_result(db_connection);
			db_result = mysql_store_result(db_connection);
		} else {
			mysql_free_result(db_result);
			db_result = NULL;
		}
		return pRet;
	}

	int num_cols = mysql_num_fields(db_result);
	if (num_cols < 1) {
		return pRet;
	}
	unsigned long* field_lengths = mysql_fetch_lengths(db_result);
	if (field_lengths==NULL) {
		return pRet;
	}

    // The row data will be passed back to nwscript as a string, so we compute
    // the buffer length needed for the incoming data. 32 bytes covers the
    // overhead of the bundling method. If the string passed in is too small
    // to contain the row, a new chunk is allocated.
    //
    unsigned int row_size = 0;
    for (int i = 0; i < num_cols; i++) {
        row_size = row_size + field_lengths[i] + 32;
    }
    DB(printf("DB: FETCH: NEED row_size [%d]\n",row_size);)

	if (row_size > row_buffer_size) {
		pRet = (char*)malloc(row_size);
		pRet[0]='\0';
		DB(printf("DB: Allocated new buffer [%p], size [%u]\n",pRet,row_size);)
	}

	// Loop through each field, copying the row data into the return string buffer.
	//
	MYSQL_FIELD *field_meta = mysql_fetch_fields(db_result);

	char* p = pRet;
	*p++ = '|';

	for (int i = 0; i< num_cols; i++) {

    	//printf("DB: Field [%d] Type [%u], Null [%u], Length [%lu], Data [%4s]\n",i,
		//result_bind[i].buffer_type,*(result_bind[i].is_null),*(result_bind[i].length),(char*)(result_bind[i].buffer));

		switch (field_meta[i].type) {

			// Binary types which may contain a NWN object. Note varchar is probably
			// not binary safe but the mysql interface returns MYSQL_TYPE_VAR_STRING
			// for both varchar and varbinary fields.
			//
			case MYSQL_TYPE_VAR_STRING:
			case MYSQL_TYPE_BLOB:
			case MYSQL_TYPE_LONG_BLOB:
			case MYSQL_TYPE_MEDIUM_BLOB:
			case MYSQL_TYPE_TINY_BLOB:
				if (db_row[i]==NULL) {
					sprintf(p,"2|1|0||");
					p = (char*)rawmemchr(p,'\0');
				} else {
					// Hmn, this is less than ideal, but is there any other way to tell if the
					// blob is a NWN object and thus needs to be placed in the buffer for 
					// de-serialization? This will have to do for now I guess. Also note from
					// the just prior block, we also don't know it's an object if it's NULL.
					if (strncmp("UTI V3.28",db_row[i],9)==0) {
						sprintf(p,"1|0|20|data://nwnx/blob/UTI|");
						p = (char*)rawmemchr(p,'\0');
						memcpy(blob_buffer,db_row[i],field_lengths[i]);
						blob_buffer_size = field_lengths[i];
					} else if (strncmp("BIC V3.28",db_row[i],9)==0) {
						sprintf(p,"1|0|20|data://nwnx/blob/BIC|");
						p = (char*)rawmemchr(p,'\0');
						memcpy(blob_buffer,db_row[i],field_lengths[i]);
						blob_buffer_size = field_lengths[i];
					} else if (strncmp("UTP V3.28",db_row[i],9)==0) {
						sprintf(p,"1|0|20|data://nwnx/blob/UTP|");
						p = (char*)rawmemchr(p,'\0');
						memcpy(blob_buffer,db_row[i],field_lengths[i]);
						blob_buffer_size = field_lengths[i];
					} else if (strncmp("UTM V3.28",db_row[i],9)==0) {
						sprintf(p,"1|0|20|data://nwnx/blob/UTM|");
						p = (char*)rawmemchr(p,'\0');
						memcpy(blob_buffer,db_row[i],field_lengths[i]);
						blob_buffer_size = field_lengths[i];
					} else if (strncmp("UTT V3.28",db_row[i],9)==0) {
						sprintf(p,"1|0|20|data://nwnx/blob/UTT|");
						p = (char*)rawmemchr(p,'\0');
						memcpy(blob_buffer,db_row[i],field_lengths[i]);
						blob_buffer_size = field_lengths[i];
					} else {
						sprintf(p,"2|0|%lu|",field_lengths[i]);
						p = (char*)rawmemchr(p,'\0');
						memcpy(p,(char*)(db_row[i]),field_lengths[i]);
						p += field_lengths[i];
						*p++ = '|';
					}
				}
			break;

			// Integer numeric type, mapped into NWN type int.
			//
			case MYSQL_TYPE_LONG:
			case MYSQL_TYPE_INT24:
			case MYSQL_TYPE_SHORT:
			case MYSQL_TYPE_TINY: // and BOOLEAN
			case MYSQL_TYPE_LONGLONG: // and SERIAL
                if (db_row[i]==NULL) {
					sprintf(p,"3|1|0||");
					p = (char*)rawmemchr(p,'\0');
				} else {
					sprintf(p,"3|0|%lu|",field_lengths[i]);
					p = (char*)rawmemchr(p,'\0');
					memcpy(p,(char*)db_row[i],field_lengths[i]);
					p += field_lengths[i];
					*p++ = '|';
				}
			break;

			// Floating point types mapped into NWN float.
			//
			case MYSQL_TYPE_FLOAT:
			case MYSQL_TYPE_DOUBLE:
			case MYSQL_TYPE_DECIMAL:
			case MYSQL_TYPE_NEWDECIMAL:
                if (db_row[i]==NULL) {
					sprintf(p,"4|1|0||");
					p = (char*)rawmemchr(p,'\0');
				} else {
					sprintf(p,"4|0|%lu|",field_lengths[i]);
					p = (char*)rawmemchr(p,'\0');
					memcpy(p,(char*)db_row[i],field_lengths[i]);
					p += field_lengths[i];
					*p++ = '|';
				}
			break;

			// All others passed back as strings for the application to interpret.
			//
			case MYSQL_TYPE_STRING:
			case MYSQL_TYPE_BIT:
			case MYSQL_TYPE_TIME:
			case MYSQL_TYPE_DATE:
			case MYSQL_TYPE_TIMESTAMP:
			case MYSQL_TYPE_DATETIME:
			case MYSQL_TYPE_YEAR:
			case MYSQL_TYPE_ENUM:
			case MYSQL_TYPE_SET:
			default:
                if (db_row[i]==NULL) {
					sprintf(p,"2|1|0||");
					p = (char*)rawmemchr(p,'\0');
				} else {
					sprintf(p,"2|0|%lu|",field_lengths[i]);
					p = (char*)rawmemchr(p,'\0');
					memcpy(p,(char*)db_row[i],field_lengths[i]);
					p += field_lengths[i];
					*p++ = '|';
				}
			break;

		}

	}

	*p++ = '|';
	*p = '\0';

    DB(printf("DB: buf [%s]\n",pRet);)

	return pRet;
}

//
// Clear the Buffers used to bind prepared query parameters and results. May
// eventually make these fully dynamic.
//
void CMySQL::ResetBind ()
{
    memset(query_bind,0,sizeof(query_bind));
    memset(result_bind,0,sizeof(result_bind));

	int i;
	for (i = 0; i < NWNX_MYSQL_MAX_PARAMETERS; i++) {

		query_bind[i].is_null  = &query_bind_is_null[i];
		query_bind[i].length   = &query_bind_length[i];

		query_bind_is_null[i]  = 0;
		query_bind_length[i]   = 0;

		result_bind[i].is_null = &result_bind_is_null[i];
		result_bind[i].length  = &result_bind_length[i];
		result_bind[i].error   = &result_bind_error[i];

		result_bind[i].buffer        = result_bind_buffer[i];
		result_bind[i].buffer_length = result_bind_buffer_size[i];

		result_bind_is_null[i] = 0;
		result_bind_length[i]  = 0;
		result_bind_error[i]   = 0;
	}
}

//
// Prepare a query for processing, parses bind parameters. After preparation,
// the bind/execute and fetch cycle can be repeated with the same prepared
// query.
//
// Returns the number of expected bind parameters, or -1 on error.
//
int CMySQL::Prepare (char *query)
{
	DB(printf("DB: PREPARE: Query [%s]\n",query);)

	if (db_connection==NULL) {
		sprintf(db_last_error_message,"Missing database connection.");
		return -1;
	}

	if (db_statement != NULL) {
    	mysql_stmt_free_result(db_statement);
		while (mysql_stmt_next_result(db_statement)==0) mysql_stmt_free_result(db_statement);
		if (mysql_stmt_close(db_statement)) {
			DB(printf("DB: Prepare, closing prior SQL statement, %s\n",mysql_stmt_error(db_statement));)
			db_statement = NULL;
		}
	}

	// Initialize the statement.
	//
	db_statement = mysql_stmt_init(db_connection);
	if (db_statement==NULL) {
		sprintf(db_last_error_message,"Error initializing SQL statement [%s], %s",query,mysql_stmt_error(db_statement));
		return -1;
	}

	// Prepare the statement, parses bind parameters.
	//
	if (mysql_stmt_prepare(db_statement,query,strlen(query))) {
		sprintf(db_last_error_message,"Error preparing SQL statement [%s], %s\n",query,mysql_stmt_error(db_statement));
		mysql_stmt_close(db_statement);
		db_statement = NULL;
		return -1;
	}

	unsigned int param_count = mysql_stmt_param_count(db_statement);
	if (param_count > NWNX_MYSQL_MAX_PARAMETERS) {
		sprintf(db_last_error_message,"Number of query parameters [%u] exceeds current internal limit of [%u]",param_count,NWNX_MYSQL_MAX_PARAMETERS);
		return -1;
	}

	DB(printf("DB: PREPARE: Parameter count [%u]\n",param_count);)

	return param_count;
}

void CMySQL::DumpBind(MYSQL_BIND *b, int max) 
{
	for (int i = 0; i < max; i++) {
	printf("DB: [%d] buffer_type[%u], buffer[%p]->[%08lx], buffer_length[%lu], length[%p]->[%d], is_null[%p]->[%02x], is_unsigned[%02x], error[%p]->[%02x]\n", i,
		b[i].buffer_type,
		b[i].buffer, (b[i].buffer?(*((unsigned long*)b[i].buffer)):-1),
		b[i].buffer_length,
		b[i].length,(b[i].length?((int)*(b[i].length)):-1),
		b[i].is_null,*(b[i].is_null),
		b[i].is_unsigned,
		b[i].error,*(b[i].error)
	);
	}
}

//
// One step bind query parameters and execute query. Note that the parameter
// may be zero.
//
// Return effected row count or -1 on error
//
int CMySQL::ExecutePrepared (parsed_bundle_t* bind_params, unsigned int query_param_count)
{
	unsigned int i;

	DB(printf("DB: EXECUTE: BEGIN, Bind parameter count [%u]\n",query_param_count););

	if (db_connection==NULL) {
		sprintf(db_last_error_message,"Missing database connection.");
		return -1;
	}

	if (db_statement == NULL) {
		sprintf(db_last_error_message,"Statement is NULL, was a query prepared?");
		return -1;
	}

	if (query_param_count > NWNX_MYSQL_MAX_PARAMETERS) {
		sprintf(db_last_error_message,"Number of bind parameters [%u] exceeds current internal limit of [%u]",query_param_count,NWNX_MYSQL_MAX_PARAMETERS);
		return -1;
	}

	// Statement was prepared in "Prepare" method, check if the number of
	// parmeters passed here match the number it was prepared with.
	//
	DB(printf("DB: * mysql_stmt_param_count *\n");)
	unsigned int prep_param_count = mysql_stmt_param_count(db_statement);
	if (prep_param_count != query_param_count) {
		sprintf(db_last_error_message,"Prepared parameter count [%u] does not match query parameter count [%u]\n",prep_param_count,query_param_count);
		return -1;
	}

	// Cancel any ongoing result set and restart for new bind parameters. Note that this is
	// a _different_ result free than is used (mysql_free_result) for the simple query call.
	//while (mysql_stmt_next_result(db_statement)==0) mysql_stmt_free_result(db_statement);

    fetch_state = NWNX_MYSQL_NEW_SET;

	// Create the MySQL bind structure block and copy the passed bind parameters ito it.
	//
	//MYSQL_BIND *bind1 = (MYSQL_BIND*)malloc(sizeof(MYSQL_BIND) * query_param_count);
	///memset(bind1,0,sizeof(MYSQL_BIND) * query_param_count);
	//printf("DB: size 1 [%d], addr 1 [%08lX]\n",sizeof(MYSQL_BIND) * query_param_count, &bind1[0]);
	//MYSQL_BIND bind[query_param_count];
	//memset(bind,0,sizeof(bind));
	//printf("DB: size 2 [%d], addr 2 [%08lX]\n",sizeof(bind), &bind[0]);
	//MYSQL_BIND *bind = &bind1[0];
	//unsigned long b_lens[query_param_count];
	//my_bool       b_null[query_param_count];
	//
	ResetBind();

	parsed_bundle_t *param_meta = bind_params += 1;

	// It's ok to use this interface for a query with no parameters. The most likely
	// reason would be to invoke a stored procedure that does not require arguments.
	// Loop through the passed paramters, assigning values to the locations that the
	// MYSQL interface expects them.
	//
	if (query_param_count > 0 ) {

		for (i = 0; i < query_param_count; i++ ) {

			DB(printf("DB: bind_params [%d][%p], Type [%u], Size [%u], Data [%p],[%08x]\n", i,
				&param_meta[i], param_meta[i].b_type, param_meta[i].b_size,
				param_meta[i].b_data,*(param_meta[i].b_data));)
		
			// Referenced Blob
			if (param_meta[i].b_type == 1) {
				query_bind[i].buffer_type   = MYSQL_TYPE_BLOB;
				*(query_bind[i].is_null)    = (my_bool)param_meta[i].b_null;
				query_bind[i].length        = (long unsigned int*)&blob_buffer_size;
				query_bind[i].buffer_length = blob_buffer_size;
				query_bind[i].buffer        = blob_buffer;
				DB(printf("DB: query_bind[%u], is_null [%hhx], length [%lu], data [%s]\n", i, *(query_bind[i].is_null), *(query_bind[i].length), (char*)query_bind[i].buffer );)
			
			// String
			} else if (param_meta[i].b_type == 2) {
				query_bind[i].buffer_type   = MYSQL_TYPE_STRING;
				*(query_bind[i].is_null)    = (my_bool)param_meta[i].b_null;
				query_bind[i].length        = (long unsigned int*)&param_meta[i].b_size;
				query_bind[i].buffer_length = param_meta[i].b_size;
				query_bind[i].buffer        = param_meta[i].b_data;
				DB(printf("DB: query_bind[%u], is_null [%hhx], length [%lu], data [%s]\n", i, *(query_bind[i].is_null), *(query_bind[i].length), (char*)query_bind[i].buffer );)
	
			// Integer
			} else if (param_meta[i].b_type == 3) {
				query_bind[i].buffer_type   = MYSQL_TYPE_LONG;
				*(query_bind[i].is_null)    = (my_bool)param_meta[i].b_null;
				query_bind[i].buffer_length = 0;
				query_bind[i].buffer        = param_meta[i].b_data;
                sscanf((const char*)param_meta[i].b_data,"%d",(int32_t *)param_meta[i].b_data);
				DB(printf("DB: query_bind[%u], is_null [%hhx], length [%lu], data [%s]\n", i, *(query_bind[i].is_null), *(query_bind[i].length), (char*)query_bind[i].buffer);)
	
			// Float
			} else if (param_meta[i].b_type == 4) {
				query_bind[i].buffer_type   = MYSQL_TYPE_FLOAT;
				*(query_bind[i].is_null)    = (my_bool)param_meta[i].b_null;
				query_bind[i].buffer_length = 0;
				query_bind[i].buffer        = param_meta[i].b_data;
                sscanf((const char*)param_meta[i].b_data,"%f",(float *)param_meta[i].b_data);
				DB(printf("DB: query_bind[%u], is_null [%hhx], length [%lu], data [%f]\n", i, *(query_bind[i].is_null), *(query_bind[i].length), *((float*)query_bind[i].buffer) );)
	
			// Error
			} else {
				sprintf(db_last_error_message,"Failed binding SQL statement parameter [%d], unknown type [%u]\n",i,param_meta[i].b_type);
				return -1;
			}
	
		}
	
		DB(printf("DB: * mysql_stmt_bind_param (query) *\n");)

		if (mysql_stmt_bind_param(db_statement,query_bind)) {
			sprintf(db_last_error_message,"Failed to bind SQL statement parameter, %s\n",mysql_stmt_error(db_statement));
			return -1;
		}

	}

	//
	// Execute the Query !!
	//
	DB(printf("DB: * mysql_stmt_execute *\n");)

	if (mysql_stmt_execute(db_statement)) {
		sprintf(db_last_error_message,"Failed executing SQL statement, %s\n",mysql_stmt_error(db_statement));
		return -1;
	}


	// Prepare the result buffers, this has got to be the ugliest interface ever.
	//
	//MYSQL_BIND *bind_result = (MYSQL_BIND*)malloc(sizeof(MYSQL_BIND) * result_field_count);
	//memset(bind_result,0,sizeof(MYSQL_BIND) * result_field_count);
	//printf("DB: size R [%d], addr R [%08lX]\n",sizeof(MYSQL_BIND) * result_field_count, &bind_result[0]);
	
	
	//MYSQL_BIND    bind_result[result_field_count];
	//memset(bind,0,sizeof(bind));
	//unsigned long length[result_field_count];
	//my_bool       is_null[result_field_count];
//	my_bool       error[result_field_count];

/* ******************
	MYSQL_RES *meta_result = mysql_stmt_result_metadata(db_statement);
    if (meta_result == NULL) {
		sprintf(db_last_error_message,"Could not obtain pre-exec result meta-data, %s\n",mysql_stmt_error(db_statement));
		return -1;
    }

	unsigned int result_field_count = mysql_num_fields(meta_result);
	printf("DB: Expecting [%u] columns in result rows\n",result_field_count);

	MYSQL_FIELD *meta_field = NULL;

	printf("DB: looping [%u] columns\n",result_field_count);

	for (i = 0; i < result_field_count; i++) {

        printf("DB: Getting metadata for result field [%d]\n",i);

		meta_field = mysql_fetch_field_direct(meta_result,i);
	    if (meta_field == NULL) {
			sprintf(db_last_error_message,"Failed to get field meta data, %s\n",mysql_stmt_error(db_statement));
			return -1;
   		}

        printf("DB: Result Field, Name [%s], Type, [%d], Length [%lu], MaxLength [%lu]\n",meta_field->name,meta_field->type,meta_field->length,meta_field->max_length);

		result_bind[i].buffer_type = meta_field->type;

		//if ((meta_field->type==MYSQL_TYPE_STRING)||(meta_field->type==MYSQL_TYPE_VAR_STRING)) result_bind[i].buffer = NULL;
		//if ((meta_field->type==MYSQL_TYPE_BLOB)||(meta_field->type==MYSQL_TYPE_LONG_BLOB)||(meta_field->type==MYSQL_TYPE_MEDIUM_BLOB)||(meta_field->type==MYSQL_TYPE_TINY_BLOB)) result_bind[i].buffer = NULL;
		//if ((meta_field->type==MYSQL_TYPE_BLOB)||(meta_field->type==MYSQL_TYPE_LONG_BLOB)||(meta_field->type==MYSQL_TYPE_MEDIUM_BLOB)||(meta_field->type==MYSQL_TYPE_TINY_BLOB)) result_bind[i].buffer = NULL;
		//result_bind[i].length = &length[i];

	}

	mysql_free_result(meta_result);

	DumpBind(&result_bind[0]);

	if (mysql_stmt_bind_result(db_statement,result_bind)) {
		sprintf(db_last_error_message,"Failed to bind result parameters, %s\n",mysql_stmt_error(db_statement));
		return -1;
	}
************* */

//// hhnnn
/*
	unsigned int result_field_count = mysql_stmt_field_count(db_statement);
	printf("DB: Got late field count, expecting [%u] columns in next result set rows\n",result_field_count);
    DumpBind(result_bind,result_field_count);
    printf("DB: * mysql_stmt_bind_result *\n");
    if (mysql_stmt_bind_result(db_statement,result_bind)) {
        printf("ERROR: Re-Binding result parameters with resized strings, %s\n",mysql_stmt_error(db_statement));
        return NULL;
    }
*/

	// Causes mysql_stmt_store_result to evaulate the max_length values for returned rows.
	//
	my_bool truebit = 1;
	mysql_stmt_attr_set(db_statement,STMT_ATTR_UPDATE_MAX_LENGTH,&truebit);

	//
	// Fetch ALL the results from the server.
	//
	DB(printf("DB: * mysql_stmt_store_result *\n");)
	if (mysql_stmt_store_result(db_statement)) {
		sprintf(db_last_error_message,"Failed to store results, %s\n",mysql_stmt_error(db_statement));
		return -1;
	}

	// ***********

	DB(printf("DB: * mysql_stmt_affected_rows *\n");)
	my_ulonglong n_rows = mysql_stmt_affected_rows(db_statement);
	if (n_rows == (my_ulonglong)-1) {
		sprintf(db_last_error_message,"Failed to get affected rows, %s\n",mysql_stmt_error(db_statement));
		return -1;
	} else {
		DB(printf("DB: Statement effected %lu rows\n",(unsigned long)n_rows);)
	}

	DB(DumpBind(result_bind,1);)

    return n_rows;
}

// -------------------------------------------------------------------------
// The statement version of fetch. Gets the next row of data from the data
// queried and buffered in Execute.
//
// Return ptr to data buffer or null on error, "" if out of data
//
//char* CMySQL::FetchPrepared (char *results)
char* CMySQL::FetchPrepared (char *row_buffer, unsigned int row_buffer_size)
{
	char* pRet = row_buffer;

	unsigned int pre_buf_len = strlen(row_buffer);

	DB(printf("DB: Pre-allocated fetch buffer size [%u]\n",pre_buf_len);)

	if (db_connection==NULL) {
		sprintf(db_last_error_message,"Missing database connection\n");
		return NULL;
	}
	if (db_statement == NULL) {
		sprintf(db_last_error_message,"Statement is NULL, was a statement prepared?\n");
		return NULL;
	}

	// TODO: Need a way to detect if EXECUTE failed !

	// db should be connected
	// statement should be prepared
	// result unopened bufferd to client my mysql_stmt_store_result
	
	// Bi-modal operation, this method may be entered in two states.
	//
	// 	1) NEW_SET, A new result set just after an execute operation,
	// 	this is the first loop through. It also applies to the state
	// 	after another result set is found after calling a stored procedure.
	// 	Must eval the meta-data and allocate result bind buffers.
	//
	// 	2) OLD_SET, Continuing row fetches after the single first new set.
	//	Does not require checking the meta-data, or allocating the buffers.
	//

//////////////////////////
	if ( fetch_state == NWNX_MYSQL_NEW_SET ) {
//////////////////////////
	DB(printf("DB: ********************** MODE NEW_SET ***********************\n");)

	unsigned int result_field_count = mysql_stmt_field_count(db_statement);
	DB(printf("DB: Got late field count, expecting [%u] columns in next result set rows\n",result_field_count);)

	// Special case - stored procedures return a final status packet with
	// a field count of zero. This just means END OF DATA.
	//
	if (result_field_count == 0) {
		fetch_state = NWNX_MYSQL_NEW_SET; 
		strcpy(pRet,"|0|||||");
		return pRet;
	} 

	//
	// Check the result meta data.
	//
	DB(printf("DB: * mysql_stmt_result_metadata *\n");)
	MYSQL_RES *late_meta_result = mysql_stmt_result_metadata(db_statement);
    if (late_meta_result == NULL) {
		sprintf(db_last_error_message,"Failed to obtain late result meta-data, %s\n",mysql_stmt_error(db_statement));
		return NULL;
    }
	//unsigned int result_field_count = mysql_num_fields(late_meta_result);
	//printf("DB: Got late meta-data, expecting [%u] columns in next result rows\n",result_field_count);

	//DumpBind(result_bind,result_field_count);

	// for (i = 0; i< result_field_count; i++) {
	// printf("DB: Field [%u] Length [%lu]\n",i,*(result_bind[i].length));
	// }

	//
	// Loop through metadata for results to get maximum result string lengths
	// and to check and allocate local buffers space.
	//
    MYSQL_FIELD *late_meta_field = NULL;

	unsigned int i;
	for (i = 0; i < result_field_count; i++) {

        DB(printf("DB: Getting late metadata for result field [%d]\n",i);)

		late_meta_field = mysql_fetch_field_direct(late_meta_result,i);
	    if (late_meta_field == NULL) {
			sprintf(db_last_error_message,"Failed to get field late meta-data, %s\n",mysql_stmt_error(db_statement));
			return NULL;
   		}

        DB(printf("DB: Late Result FIELD METADATA [%d],  Name [%s], Type, [%d], Length [%lu], MaxLength [%lu]\n",
			i,late_meta_field->name,late_meta_field->type,late_meta_field->length,late_meta_field->max_length);)

		// Note, there are some type conversions we could request here by specifying
		// a different type for the result buffer from the column meta type.
		result_bind[i].buffer_type = late_meta_field->type;

		switch (late_meta_field->type) {

			// Types that need variable length buffer allocation
			case MYSQL_TYPE_STRING:
			case MYSQL_TYPE_VAR_STRING:
			case MYSQL_TYPE_BLOB:
			case MYSQL_TYPE_LONG_BLOB:
			case MYSQL_TYPE_MEDIUM_BLOB:
			case MYSQL_TYPE_TINY_BLOB:
			case MYSQL_TYPE_NEWDECIMAL:
			case MYSQL_TYPE_BIT:
        		//printf("DB: [%d] ALLOCATE\n",late_meta_field->type);
        		DB(printf("DB: BUF meta max [%lu], cur buf [%u]\n",late_meta_field->max_length, result_bind_buffer_size[i]);)
				if ((late_meta_field->max_length+1) > result_bind_buffer_size[i]) {
					free(result_bind_buffer[i]);
					result_bind_buffer[i] = (unsigned char*)malloc(late_meta_field->max_length+1);
					memset(result_bind_buffer[i],'\0',late_meta_field->max_length+1);
					result_bind_buffer_size[i] = (late_meta_field->max_length+1);
        			DB(printf("DB: Alloc new string buffer size, new [%u]\n",result_bind_buffer_size[i]);)
					result_bind[i].buffer = result_bind_buffer[i]; // Point bind to new alloc buffer
					result_bind[i].buffer_length = result_bind_buffer_size[i];
				}
        		DB(printf("DB: Check check [%lu], [%lu], [%u]\n",*(result_bind[i].length),result_bind_length[i],result_bind_buffer_size[i]);)
				result_bind[i].buffer_length = result_bind_buffer_size[i]; // Are you lost yet? I am.
				result_bind_length[i] = 0;
        		DB(printf("DB: Check check [%lu], [%lu], [%u]\n",*(result_bind[i].length),result_bind_length[i],result_bind_buffer_size[i]);)
			break;

			// Types that fit into the default allocation size. The max statically sized
			// object is the time structure. It reports a max_length of 30 but writes a 
			// few bytes past the end of this (caught with valgrind) so the min buffer 
			// should never be smaller than 36 to prevent possible memory corruption.
			/* 
			case MYSQL_TYPE_TINY:
			case MYSQL_TYPE_SHORT:
			case MYSQL_TYPE_INT24:
			case MYSQL_TYPE_LONG:
			case MYSQL_TYPE_LONGLONG:
			case MYSQL_TYPE_FLOAT:
			case MYSQL_TYPE_DOUBLE:
			case MYSQL_TYPE_TIME:
			case MYSQL_TYPE_DATE:
			case MYSQL_TYPE_DATETIME:
			case MYSQL_TYPE_TIMESTAMP:
        		printf("DB: [%d] NO ALLOCATE\n",late_meta_field->type);
			break;
			*/
			default:
			break;

		}

	}

	mysql_free_result(late_meta_result);

	//
	// Must re-bind or mysql does not recognise the new memory allocation of the buffers.
	//
	DB(DumpBind(result_bind,result_field_count);)
	DB(printf("DB: * mysql_stmt_bind_result *\n");)
	if (mysql_stmt_bind_result(db_statement,result_bind)) {
   	   	DB(printf("ERROR: Re-Binding result parameters with resized strings, %s\n",mysql_stmt_error(db_statement));)
   	   	return NULL;
	}


/*
	if (mysql_stmt_fetch(db_statement)) {
 		printf("ERROR: Fetch failed, \n",mysql_stmt_error(db_statement));
		if (mysql_stmt_errno(db_statement) == MYSQL_NO_DATA) {
			printf("DB: Fetch returned no rows\n",mysql_stmt_error(db_statement));
			//results[0] = '\0';
			return false;
		} else {
			printf("ERROR: Fetch failed, \n",mysql_stmt_error(db_statement));
			//results[0] = '\0';
			return false;
		}
	}
	printf("DB: OK\n",mysql_stmt_error(db_statement));
*/

		fetch_state = NWNX_MYSQL_OLD_SET;

	} else { 
		DB(printf("DB: ********************** MODE OLD_SET ***********************\n");)
	}

	DB(printf("DB: * mysql_stmt_fetch *\n");)
	int ret = mysql_stmt_fetch(db_statement);
	if ( ret != 0 ) {

		int err = mysql_stmt_errno(db_statement);

		DB(printf("DB: Fetch, ret [%d], err [%d]\n",ret,err);)

		// Hmmn, the API docs say 1 = error, but the sample code uses
		// if (status == 1 || status == MYSQL_NO_DATA) to break the fetch loop
		// and then proceeds to check for more result sets. 
		// When reading the next row after the row after a next result set
		// I get DB: Fetch, ret [1], err [2053] -> "Attempt to read a row while
		// there is no result set associated with the statement." Perhaps
		// this always is the case at the end of a secondary result set.

		if ( ((ret== 1)&&(err==2053)) || (ret==MYSQL_NO_DATA) ) {
		//if ( (ret== 1) || (ret==MYSQL_NO_DATA) ) {
		//if (ret == MYSQL_NO_DATA)  {

			//printf("DB: Fetch STMT returned end of data\n");
			//pRet[0] = '\0';
			//return pRet;

			// But are we really done? May be more result sets available

			mysql_stmt_free_result(db_statement);
			
			int result_status = mysql_stmt_next_result(db_statement);

			// Status here is equivalent to just after mysql_stmt_execute().

			if (result_status > 0) {

				sprintf(db_last_error_message,"Query for next result set failed, %s\n",mysql_stmt_error(db_statement));
				return NULL;

			} else if (result_status == 0) {

				// there are more results, just keep going (or add result set delimiter?)

				DB(printf("DB: * RESULT SET BREAK *\n");)

				// Need to re-evaluate meta-data and rebind result buffers.

		        fetch_state = NWNX_MYSQL_NEW_SET; 

				strcpy(pRet,"|0|||||");
				return pRet;

			} else if (result_status == -1) {  // -1

				DB(printf("DB: Fetch statement next_row & next_result returned * END OF DATA *\n");)

				mysql_stmt_free_result(db_statement);

				//pRet[0] = '\0';
				strcpy(pRet,"|0|||||");
				return pRet;

			} else {

				// Undefined
				mysql_stmt_free_result(db_statement);

			}


		} else if (ret == MYSQL_DATA_TRUNCATED) {
			DB(printf("DB: Statement fetch returned truncated data.\n");)
			//sprintf(db_last_error_message,"Statement fetch returned truncated data.\n");
			//return NULL;
		} else {
			sprintf(db_last_error_message,"Statement fetch returned error, %s\n",mysql_stmt_error(db_statement));
			return NULL;
		}

	}

	DB(DumpBind(result_bind,mysql_stmt_field_count(db_statement));)

	DB(printf("DB: Fetch, GOT A ROW !!!\n");)

/*
	if (ret
		if (mysql_stmt_errno(db_statement) == MYSQL_NO_DATA) {
			printf("DB: Fetch returned no rows\n",mysql_stmt_error(db_statement));
			results[0] = '\0';
			return NULL;
		} else {
			printf("ERROR: Fetch failed, \n",mysql_stmt_error(db_statement));
			results[0] = '\0';
			return NULL;
		}
	}
*/

	DB(printf("DB: * mysql_stmt_field_count *\n");)
	unsigned result_field_count = mysql_stmt_field_count(db_statement);
	if ((int)result_field_count < 0) {
		sprintf(db_last_error_message,"Could not obtain result field count.\n");
		return NULL;
	}
    	DB(printf("DB: Result Set Field Count [%u]\n",result_field_count);)

	unsigned int buffer_size = 0;

	unsigned int i;

	for (i = 0; i< result_field_count; i++) {
    	DB(printf("DB: Field [%d] Type [%u], Null [%u], Err [%u], Length [%lu]\n",i,result_bind[i].buffer_type,*(result_bind[i].is_null),*(result_bind[i].error),*(result_bind[i].length));)
		buffer_size = buffer_size + (*(result_bind[i].length)) + 16;   // |2|0|12345678901| ~ 16
	}

    DB(printf("DB: Need output buffer size [%u]\n",buffer_size);)

	if (buffer_size > pre_buf_len) {
		pRet = (char*)malloc(buffer_size);
		pRet[0]='\0';
    	DB(printf("DB: Allocated new buffer [%p], size [%u]\n",pRet,buffer_size);)
	}

    DB(printf("DB: buf [%s]\n",pRet);)
	char tbuf[100];
	char* p = pRet;
	*p++ = '|';
	for (i = 0; i< result_field_count; i++) {

    	DB(printf("DB: Field [%d] Type [%u], Null [%u], Length [%lu], Data [%4s]\n",i,
		result_bind[i].buffer_type,*(result_bind[i].is_null),*(result_bind[i].length),(char*)(result_bind[i].buffer));)

		switch (result_bind[i].buffer_type) {

			case MYSQL_TYPE_BLOB:
			case MYSQL_TYPE_LONG_BLOB:
			case MYSQL_TYPE_MEDIUM_BLOB:
			case MYSQL_TYPE_TINY_BLOB:
			case MYSQL_TYPE_VAR_STRING:
			case MYSQL_TYPE_NEWDECIMAL:
				if (*(result_bind[i].is_null)) {
					sprintf(p,"2|1|0||");
					p = (char*)rawmemchr(p,'\0');
				} else {
					// Hmn, this is less than ideal, but is there any other way to tell if the
					// blob is a NWN object and thus needs to be placed in the buffer for 
					// de-serialization? This will have to do for now I guess. Also note from
					// the just prior block, we also don't know it's an object if it's NULL.
					if (strncmp("UTI V3.28",(char*)(result_bind[i].buffer),9)==0) {
						sprintf(p,"1|0|20|data://nwnx/blob/UTI|");
						p = (char*)rawmemchr(p,'\0');
						memcpy(blob_buffer,(char*)(result_bind[i].buffer),*(result_bind[i].length));
						blob_buffer_size = *(result_bind[i].length);
					} else if (strncmp("BIC V3.28",(char*)(result_bind[i].buffer),9)==0) {
						sprintf(p,"1|0|20|data://nwnx/blob/BIC|");
						p = (char*)rawmemchr(p,'\0');
						memcpy(blob_buffer,(char*)(result_bind[i].buffer),*(result_bind[i].length));
						blob_buffer_size = *(result_bind[i].length);
					} else if (strncmp("UTP V3.28",(char*)(result_bind[i].buffer),9)==0) {
						sprintf(p,"1|0|20|data://nwnx/blob/UTP|");
						p = (char*)rawmemchr(p,'\0');
						memcpy(blob_buffer,(char*)(result_bind[i].buffer),*(result_bind[i].length));
						blob_buffer_size = *(result_bind[i].length);
					} else if (strncmp("UTM V3.28",(char*)(result_bind[i].buffer),9)==0) {
						sprintf(p,"1|0|20|data://nwnx/blob/UTM|");
						p = (char*)rawmemchr(p,'\0');
						memcpy(blob_buffer,(char*)(result_bind[i].buffer),*(result_bind[i].length));
						blob_buffer_size = *(result_bind[i].length);
					} else if (strncmp("UTT V3.28",(char*)(result_bind[i].buffer),9)==0) {
						sprintf(p,"1|0|20|data://nwnx/blob/UTT|");
						p = (char*)rawmemchr(p,'\0');
						memcpy(blob_buffer,(char*)(result_bind[i].buffer),*(result_bind[i].length));
						blob_buffer_size = *(result_bind[i].length);
					} else {
						sprintf(p,"2|0|%lu|",*(result_bind[i].length));
						p = (char*)rawmemchr(p,'\0');
						memcpy(p,(char*)(result_bind[i].buffer),*(result_bind[i].length));
						p += *(result_bind[i].length);
						*p++ = '|';
					}
				}
			break;

			case MYSQL_TYPE_LONG:
			case MYSQL_TYPE_INT24:
				if (*(result_bind[i].is_null)) {
					sprintf(p,"3|1|0||");
					p = (char*)rawmemchr(p,'\0');
				} else {
					sprintf(tbuf,"%d",*((int*)result_bind[i].buffer));
					sprintf(p,"3|%u|%u|%s|",((*(result_bind[i].is_null))?1:0),(unsigned int)strlen(tbuf),tbuf);
					p = (char*)rawmemchr(p,'\0');
				}
			break;
			case MYSQL_TYPE_SHORT:
				if (*(result_bind[i].is_null)) {
					sprintf(p,"3|1|0||");
					p = (char*)rawmemchr(p,'\0');
				} else {
					sprintf(tbuf,"%d",*((int16_t*)result_bind[i].buffer));
					sprintf(p,"3|%u|%u|%s|",((*(result_bind[i].is_null))?1:0),(unsigned int)strlen(tbuf),tbuf);
					p = (char*)rawmemchr(p,'\0');
				}
			break;
			case MYSQL_TYPE_TINY:
				if (*(result_bind[i].is_null)) {
					sprintf(p,"3|1|0||");
					p = (char*)rawmemchr(p,'\0');
				} else {
					sprintf(tbuf,"%d",*((char *)result_bind[i].buffer));
					sprintf(p,"3|%u|%u|%s|",((*(result_bind[i].is_null))?1:0),(unsigned int)strlen(tbuf),tbuf);
					p = (char*)rawmemchr(p,'\0');
				}
			break;

			case MYSQL_TYPE_FLOAT:
				if (*(result_bind[i].is_null)) {
					sprintf(p,"4|1|0||");
					p = (char*)rawmemchr(p,'\0');
				} else {
					sprintf(tbuf,"%f",*((float*)result_bind[i].buffer));
					sprintf(p,"4|%u|%u|%s|",((*(result_bind[i].is_null))?1:0),(unsigned int)strlen(tbuf),tbuf);
					p = (char*)rawmemchr(p,'\0');
				}
			break;
			case MYSQL_TYPE_DOUBLE:
				if (*(result_bind[i].is_null)) {
					sprintf(p,"4|1|0||");
					p = (char*)rawmemchr(p,'\0');
				} else {
					sprintf(tbuf,"%f",*((double*)result_bind[i].buffer));
					sprintf(p,"4|%u|%u|%s|",((*(result_bind[i].is_null))?1:0),(unsigned int)strlen(tbuf),tbuf);
					p = (char*)rawmemchr(p,'\0');
				}
			break;

			case MYSQL_TYPE_STRING:
			case MYSQL_TYPE_BIT:
			case MYSQL_TYPE_LONGLONG:
			case MYSQL_TYPE_TIME:
			case MYSQL_TYPE_DATE:
			case MYSQL_TYPE_TIMESTAMP:
			case MYSQL_TYPE_DATETIME:
				if (*(result_bind[i].is_null)) {
					sprintf(p,"2|1|0||");
					p = (char*)rawmemchr(p,'\0');
				} else {
					//sprintf(p,"0||||");
					sprintf(p,"2|1|0||");
					p = (char*)rawmemchr(p,'\0');
				}
			break;

			default:
			break;
		}

	}
	*p++ = '|';
	*p = '\0';
    DB(printf("DB: buf [%s]\n",pRet);)

/*
	printf("DB: Result Data, Type, [%d], Length [%u]\n",result_bind[0].type);

	for (i = 0; i< result_field_count; i++) {
    	printf("DB: Field [%d] Length [%u]\n",i,*(result_bind[i].length));
	}

	// Loop through metadata for results to get maximum result string lengths
	// and to check and allocate local buffers space.
	//
	printf("DB: looping [%d] columns\n",result_field_count);
	for (i = 0; i< result_field_count; i++) {

        printf("DB: Getting late metadata for result field [%d]\n",i);

		meta_field = mysql_fetch_field_direct(meta_result,i);
	    if (meta_field == NULL) {
   	    	printf("ERROR: Getting Meta data, %s\n",mysql_stmt_error(db_statement));
   	    	return false;
   		}

        printf("DB: Late Result Field Metadata, Name [%s], Type, [%d], Length [%u], MaxLength [%u]\n",meta_field->name,meta_field->type,meta_field->length,meta_field->max_length);

		switch (meta_field->type) {
			case MYSQL_TYPE_STRING:
			case MYSQL_TYPE_VAR_STRING:
			case MYSQL_TYPE_BLOB:
        		printf("DB: Check string buffer size, current [%u], max [%u]\n",meta_field->length,result_bind_buffer_size[i]);
				if ((meta_field->max_length+1) > result_bind_buffer_size[i]) {
					free(result_bind_buffer[i]);
					result_bind_buffer[i] = (unsigned char*)malloc(meta_field->max_length+1);
					result_bind_buffer_size[i] = (meta_field->max_length+1);
        			printf("DB: Alloc new string buffer size, new [%u]\n",result_bind_buffer_size[i]);
				}
			break;
			default:
			break;
		}
*/



	return pRet;
}

char* CMySQL::Escape(unsigned char *buffer, unsigned long buffersize)
{
	if (buffer == NULL) return  NULL;
	if (db_connection == NULL) return NULL;
	char* p = (char*)malloc(buffersize*2+1);
	mysql_real_escape_string(db_connection,p,(char*)buffer,buffersize);
	return p;
}

// -Bind_Prepare ("update a set (a.r=?,a,s=?) where a.pk=?")
// --Bind_Exec    ("bob!BLOB!1234")
// -- if (f(n)=="BLOB") -> SCO
// -

// -Bind_Prepare ("select a,b from t where a.pk=b.pk and a.k=? and b.k=?")
// --Bind_Query    ("bob!larry")
// --Bind_Fetch    ();
// ---Bind_GetRow   ();
// --- if (f(n)=="BLOB") -> RCO
// --
// -

bool CMySQL::WriteScorcoData(char* SQL, BYTE* pData, int Length)
{
    DB(printf("S: [%s]\n",SQL);)
    DB(printf("S: [%8s]... (%d)\n",pData,Length);)

    MYSQL_STMT *stmt = mysql_stmt_init(db_connection);
    if (stmt==NULL) {
        DB(printf("ERROR: Initializing SQL statement [%s], %s\n",SQL,mysql_stmt_error(stmt));)
        return false;
    }

    if (mysql_stmt_prepare(stmt,SQL,strlen(SQL))) {
        DB(printf("ERROR: Preparing SQL statement [%s], %s\n",SQL,mysql_stmt_error(stmt));)
        return false;
    }

    if (mysql_stmt_param_count(stmt)!=1) {
        DB(printf("ERROR: One bind parameter (?) must be present to indicate the object data in SQL statement [%s]\n",SQL);)
        return false;
    }

    MYSQL_BIND bind[1];
    memset(bind,0,sizeof(bind));
    unsigned long b_len = Length;

    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer      = (char *)pData;
    bind[0].is_null     = 0;
    bind[0].length      = &b_len;

    if (mysql_stmt_bind_param(stmt, bind)) {
        DB(printf("ERROR: Binding SQL statement [%s], %s\n",SQL,mysql_stmt_error(stmt));)
        return false;
    }

    // No further binding needed, we only have 1 parameter and 1 row, all set above.

    if (mysql_stmt_execute(stmt)) {
        DB(printf("ERROR: Executing SQL statement [%s], %s\n",SQL,mysql_stmt_error(stmt));)
        return false;
    }

    my_ulonglong n_rows = mysql_stmt_affected_rows(stmt);
    DB(printf("INFO: Statement [%s] effected %lu rows\n",SQL,(unsigned long)n_rows);)

    if (mysql_stmt_close(stmt)) {
        DB(printf("ERROR: Closing SQL statement: %s\n",mysql_stmt_error(stmt));)
        return false;
    }

    return true;
}


/*
BOOL CMySQL::WriteScorcoData(char* SQL, BYTE* pData, int Length)
{
	int res;
	unsigned long len;
	char* Data = new char[Length * 2 + 1 + 2];
	char* pSQL = new char[MAXSQL + Length * 2 + 1];

	len = mysql_real_escape_string (&mysql, Data + 1, (const char*)pData, Length);
	Data[0] = Data[len + 1] = 39; //'
	Data[len + 2] = 0x0; 
	sprintf(pSQL, SQL, Data);

	MYSQL_RES *result = mysql_store_result (&mysql);
	res = mysql_query(&mysql, (const char *) pSQL);

	mysql_free_result(result);
	delete[] pSQL;
	delete[] Data;

	if (res == 0)
		return true;
	else
		return false;
}
*/

BYTE* CMySQL::ReadScorcoData(char* SQL, char *param, bool* pSqlError, int *size)
{
	MYSQL_RES *rcoresult;
	if (strcmp(param, "FETCHMODE") != 0)
	{	
		if (mysql_query(db_connection, (const char *) SQL) != 0)
		{
			*pSqlError = true;
			return NULL;
		}

		/*if (result)
		{
      mysql_free_result(result);
      result = NULL;
		}*/
		rcoresult = mysql_store_result (db_connection);
		if (!rcoresult)
		{
			*pSqlError = true;
			return NULL;
		}
	}
	else rcoresult=db_result;

	MYSQL_ROW row;
	*pSqlError = false;
	row = mysql_fetch_row(rcoresult);
	if (row)
	{
		unsigned long* length = mysql_fetch_lengths(rcoresult);
		// allocate buf for result!
		char* buf = new char[*length];
		if (!buf) return NULL;

		memcpy(buf, row[0], length[0]);
		*size = length[0];
		mysql_free_result(rcoresult);
 
		// Return pointer to buffer allocated by new[], safe to free() on linux
		return (BYTE*)buf;
	}
	else
	{
		mysql_free_result(rcoresult);
		return NULL;
	}
}

