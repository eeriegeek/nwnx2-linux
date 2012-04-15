/***************************************************************************
    CPgSQL.cpp: implementation of the CPgSQL class.
    copyright (c) 2008 virusman (virusman@virusman.ru)
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

#include "pgsql.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include <postgres.h>
#include <catalog/pg_type.h>

#define PG_CONNECT_STRING "host=%s port=%u dbname=%s user=%s password=%s"

CPgSQL::CPgSQL() : CDB()
{
	db_connection = NULL;
	db_result     = NULL;
	current_row   = 0;
}

CPgSQL::~CPgSQL()
{
}

const char *CPgSQL::GetDbId ()
{
	return "PGSQL";
}

bool CPgSQL::Connect ()
{
	// try to establish a default connection
	//return Connect ("localhost", "nwn", "nwnpwd", "nwn");
	return Connect ("localhost",DEF_PGPORT,NULL,"nwn",NULL,"nwn","nwnpwd");
}

//
// server - hostname of server or unix domain socket if starts with "/"
// port - port number
// unix_socket - ignored
// db - database name
// charset - client character set TODO: client_encoding=auto etc. is only 
// available in later (> 9.1?) versions of pgsql. 
//
int CPgSQL::Connect (const char *server, const unsigned int port, const char *unix_socket, const char *db, const char *charset, const char *user, const char *pass)
{
	// Set up the pgsql conect string.
	//
	char *connect_string = (char *)malloc(strlen(PG_CONNECT_STRING)-10+strlen(server)+10+strlen(db)+strlen(user)+strlen(pass)+1);
	sprintf(connect_string, PG_CONNECT_STRING, server, (port==0?DEF_PGPORT:port), db, user, pass);
    DB(printf("PGSQL: Connect String [%s]\n",connect_string);)

	// Attempt the database connection.
	//
	db_connection = PQconnectdb(connect_string);
	if (PQstatus(db_connection) != CONNECTION_OK)
	{
        sprintf(db_last_error_message,"Failure connecting to database, %s\n",PQerrorMessage(db_connection));
		free(connect_string);
		PQfinish(db_connection);
		db_connection = NULL;
		return -1;
	}
	
	free(connect_string);

	// Get the database server version number.
	//
	int db_version = PQserverVersion(db_connection);
	int v1 = db_version / 10000;
	int v2 = (db_version - (v1*10000)) / 100;
	int v3 = (db_version - (v1*10000) - (v2*100));
    //printf("PGSQL: Connection OK, server reports version %d.%d.%d.\n",v1,v2,v3);

    // Set the character set for language encoding.
    //
	if (!SetCharacterSet(charset)) {
		sprintf(db_last_error_message,"Error setting connection character encoding [%s], %s\n", charset, PQerrorMessage(db_connection));
		return -1;
	}

    printf("PGSQL: Connection OK, server reports version %d.%d.%d, connection character set is [%s].\n",
		v1,v2,v3,pg_encoding_to_char(PQclientEncoding(db_connection)));

	return 0;
}

bool CPgSQL::SetCharacterSet (const char *charset)
{
    if ((charset!=NULL)&&(charset[0]!='\0')) return (PQsetClientEncoding(db_connection,charset)==0);
    return true;
}

void CPgSQL::Disconnect ()
{
	PQfinish(db_connection);
    db_connection = NULL;
}

char* CPgSQL::Escape(unsigned char *buffer, unsigned long buffersize)
{
	if (PQstatus(db_connection) != CONNECTION_OK) {
		sprintf(db_last_error_message,"Error escaping string, no database connected, %s\n",PQerrorMessage(db_connection));
		return NULL;
	}
	if (buffer == NULL) return  NULL;
	char* p = (char*)malloc(buffersize*2+1);
	PQescapeStringConn(db_connection,p,(char*)buffer,(size_t)buffersize,NULL);
    return p;
}


int CPgSQL::Execute (const char* query)
{
	if (PQstatus(db_connection) != CONNECTION_OK) {
		sprintf(db_last_error_message,"Error executing statement, no database connected, %s\n",PQerrorMessage(db_connection));
		return -1;
	}

	if (db_result != NULL) {
		PQclear(db_result);
		db_result = NULL;
		current_row = 0;
	}

	//
	//Execute the query.
	//
	db_result = PQexec(db_connection, query);

	if (db_result == NULL) {

        sprintf(db_last_error_message,"Failure executing SQL query [%s], %s\n",query,PQerrorMessage(db_connection));
		PQclear(db_result);
		db_result = NULL;
		return -1;

	} else if (PQresultStatus(db_result) == PGRES_COMMAND_OK) {

		/* May get rid of this and only return row counts for anticipated data rows
		char *n = PQcmdTuples(db_result);
		int row_count = ((n[0]=='\0')?0:atoi(n));
		PQclear(db_result);
		db_result = NULL;
		return row_count;
		*/
		return 0;

	} else if (PQresultStatus(db_result) == PGRES_TUPLES_OK) {

		int row_count = PQntuples(db_result);
		return row_count;

	} else {

		sprintf(db_last_error_message,"Error executing prepared statement, %s\n",PQerrorMessage(db_connection));
		PQclear(db_result);
		db_result = NULL;
		return -1;

	}

    return -1;
}


char* CPgSQL::Fetch (char *row_buffer, unsigned int row_buffer_size)
{
	char* pRet = row_buffer;

	if (row_buffer == NULL) {
		sprintf(db_last_error_message,"Internal error during fetch, output buffer should never be NULL.\n");
		return NULL;
	}

	DB(printf("DB: Pre-allocated fetch buffer size [%u]\n",row_buffer_size);)
	pRet[0] = '\0';

	if (PQstatus(db_connection) != CONNECTION_OK) {
		sprintf(db_last_error_message,"Error fetching from statement, no database connected, %s\n",PQerrorMessage(db_connection));
		return NULL;
	}

	if (db_result == NULL) {
		sprintf(db_last_error_message,"Error fetching from statement, NULL result.\n");
		return NULL;
	}
	
	DB(printf("DB: FETCH: resultStatus [%d]\n",PQresultStatus(db_result));)

	if (PQresultStatus(db_result) == PGRES_FATAL_ERROR) {
		sprintf(db_last_error_message,"Error fetching from statement, %s\n",PQerrorMessage(db_connection));
		return NULL;
	}

	int num_rows = PQntuples(db_result);
	DB(printf("DB: FETCH: num_rows [%d]\n",num_rows);)
	if (num_rows <= 0) {
		// no rows returned by execute, assume empty result set
		return pRet;
	}

    int num_cols = PQnfields(db_result);
	DB(printf("DB: FETCH: num_cols [%d]\n",num_cols);)
	if (num_cols <= 0) {
		// no columns returned by execute, assume empty result set
		return pRet;
	}  

	DB(printf("DB: FETCH: Row [%d]\n",current_row);)
	if (current_row >= num_rows) {
		// No more rows, note current_row is 0 based index for data rows
		return pRet;
	}

	// compute the buffer length needed for the incoming data
	// max overhead = 26 bytes "|4294967295|1|4294967295||"
	// max overhead = 16 bytes "|999|1|9999999||"
	unsigned int row_size = 0;
	for (int i = 0; i < num_cols; i++) {
		row_size = row_size + PQgetlength(db_result,current_row,i) + 32;
	}
	DB(printf("DB: FETCH: NEED row_size [%d]\n",row_size);)

	// reallocate buffer if it will be too large for the passed string.
    if (row_size > row_buffer_size) {
		pRet = (char*)malloc(row_size);
		pRet[0]='\0';
		DB(printf("DB: Allocated new buffer [%p], size [%u]\n",pRet,row_size);)
    }

	char* p = pRet;
	*p++ = '|';

	for (int i = 0; i < num_cols; i++) {

		Oid col_type = PQftype(db_result,i);
		int c_len = PQgetlength(db_result,current_row,i);
		char* c_dat = PQgetvalue(db_result,current_row,i);

		DB(printf("DB: Field [%d], Type(OID) [%u], Null [%u], Bin [%d], Length [%d], Data [%4.4s]\n",i,col_type,
		PQgetisnull(db_result,current_row,i),PQfformat(db_result,i),c_len,PQgetvalue(db_result,current_row,i));)
		
		switch (col_type) {

			// Byte Array (or Object), peek inside the binary data and redirect it for
			// de-serialization if it looks like a NWN object, otherwise just try to
			// treat it as a string.
			//
			case BYTEAOID:       // 17
				if (PQgetisnull(db_result,current_row,i)) {
					sprintf(p,"2|1|0||");
					p = (char*)rawmemchr(p,'\0');
				} else {
					// If server version > about 9.0 need libpq at least 5.3 or unescape will not
					// decode the new default hex form of escapement. Should not be a problem
					// if the database and client libs match versions.
					size_t converted_length = 0;
					unsigned char *converted_bytes = PQunescapeBytea((unsigned char *)c_dat,&converted_length);
					DB(printf("DB: Length [%u], Data [%p][%s]\n",converted_length,converted_bytes,(char*)converted_bytes);)
					// If the data contains a recognizable object, copy it to the buffer for conversion
					// into a NWN object. Less than ideal, but is there any other way to tell if the
					// blob is a NWN object and thus needs to be placed in the buffer for 
					// de-serialization? This will have to do for now I guess. Also note from
					// the just prior block, we also don't know it's an object if it's NULL.
					if (strncmp("UTI V3.28",(char*)converted_bytes,9)==0) {
						sprintf(p,"1|0|20|data://nwnx/blob/UTI|");
						p = (char*)rawmemchr(p,'\0');
						memcpy(blob_buffer,(char*)converted_bytes,converted_length);
						blob_buffer_size = converted_length;
					} else if (strncmp("BIC V3.28",(char*)converted_bytes,9)==0) {
						sprintf(p,"1|0|20|data://nwnx/blob/BIC|");
						p = (char*)rawmemchr(p,'\0');
						memcpy(blob_buffer,(char*)converted_bytes,converted_length);
						blob_buffer_size = converted_length;
					} else if (strncmp("UTP V3.28",(char*)converted_bytes,9)==0) {
						sprintf(p,"1|0|20|data://nwnx/blob/UTP|");
						p = (char*)rawmemchr(p,'\0');
						memcpy(blob_buffer,(char*)converted_bytes,converted_length);
						blob_buffer_size = converted_length;
					} else if (strncmp("UTM V3.28",(char*)converted_bytes,9)==0) {
						sprintf(p,"1|0|20|data://nwnx/blob/UTM|");
						p = (char*)rawmemchr(p,'\0');
						memcpy(blob_buffer,(char*)converted_bytes,converted_length);
						blob_buffer_size = converted_length;
					} else if (strncmp("UTT V3.28",(char*)converted_bytes,9)==0) {
						sprintf(p,"1|0|20|data://nwnx/blob/UTT|");
						p = (char*)rawmemchr(p,'\0');
						memcpy(blob_buffer,(char*)converted_bytes,converted_length);
						blob_buffer_size = converted_length;
					} else {
						// If the binary data does not contain a recognized object, try to treat it as a string
						// assumes the encoded string is longer than the binary form since the target buffer
						// is allocated based on the length of the escaped string. Treating this as a string
						// is about all we can do, if there are embedded nuls, the NWN string will be truncared
						// at the first null, but generally speaking, the only thing we should be handling
						// in binary are NWN objects anyway.
						sprintf(p,"2|0|%u|",(uint32_t)converted_length);
						p = (char*)rawmemchr(p,'\0');
						memcpy(p,converted_bytes,converted_length);
						p += converted_length;
						*p++ = '|';
					}
					PQfreemem(converted_bytes);
				}
			break;

			// Boolean type, will be interpreted into a NWN integer, since that is the 
			// type expected in logical expressions. PGSQL keeps a char with 't' or 'f'.
			//
			case BOOLOID:        // 16
				if (PQgetisnull(db_result,current_row,i)) {
					sprintf(p,"3|1|0||");
					p = (char*)rawmemchr(p,'\0');
				} else {
					sprintf(p,"3|0|1|%u|",(PQgetvalue(db_result,current_row,i)[0]=='t'?1:0));
					p = (char*)rawmemchr(p,'\0');
				}
			break;

			// Int type, these types will be interpreted into a NWN integer, which is
			// 32 bits (4 bytes) long. If the value is larger than 32 bits can hold
			// the result is undefined, probably 0 from the NWN StringToInt function.
			//
			case INT2OID:        // 21
			case INT4OID:        // 23
			case INT8OID:        // 20
				if (PQgetisnull(db_result,current_row,i)) {
					sprintf(p,"3|1|0||");
					p = (char*)rawmemchr(p,'\0');
				} else {
					sprintf(p,"3|0|%u|%s|",c_len,PQgetvalue(db_result,current_row,i));
					p = (char*)rawmemchr(p,'\0');
				}
			break;

			// Float type, these types will be interpreted into a NWN float which is
			// 32 bits (4 bytes) long. Undefined if value too large for 32 bit float.
			//
			case FLOAT4OID:      // 700
			case FLOAT8OID:      // 701
			case NUMERICOID:     // 1700 - complex 16 bit number sequence
			case CASHOID:        // 790
				if (PQgetisnull(db_result,current_row,i)) {
					sprintf(p,"4|1|0||");
					p = (char*)rawmemchr(p,'\0');
				} else {
					sprintf(p,"4|0|%u|%s|",c_len,PQgetvalue(db_result,current_row,i));
					p = (char*)rawmemchr(p,'\0');
				}
			break;

			// Text (non-binary) strings. Assumed to be non-binary, may potentially
			// be re-interpretted by the database engine based on character sets,
			// so they should not contain binary data.
			//
			case TEXTOID:        // 25
			case BPCHAROID:      // 1042
			case VARCHAROID:     // 1043

			// Date formats will be passed to the application as strings.  Refer
			// to the PGSQL documentation for exact text formats.
			//
			case DATEOID:        // 1082
			case TIMEOID:        // 1083
			case TIMETZOID:      // 1266
			case TIMESTAMPOID:   // 1114
			case TIMESTAMPTZOID: // 1184
			case INTERVALOID:    // 1186

			// Any remaining unexpected types will also be treated as strings, this
			// should get a representation of them up to the application where they
			// they can potentially be interpreted.
			//
			case CHAROID:      // 18 - internal, char and char(1) actually produce BPCHAROID
			case ABSTIMEOID:   // 702 - deprecated
			case RELTIMEOID:   // 703 - deprecated
			case TINTERVALOID: // 704 - deprecated
			case BITOID:       // 1560 - bit
			case VARBITOID:    // 1562 - bit sting
			default:           // anthing else
				if (PQgetisnull(db_result,current_row,i)) {
					sprintf(p,"2|1|0||");
					p = (char*)rawmemchr(p,'\0');
				} else {
					sprintf(p,"2|0|%u|",(uint32_t)c_len);
					p = (char*)rawmemchr(p,'\0');
					memcpy(p,PQgetvalue(db_result,current_row,i),c_len);
					p += c_len;
					*p++ = '|';
				}
			break;

		}

	}

	current_row += 1;

	*p++ = '|';
	*p = '\0';

    DB(printf("DB: buf [%s]\n",pRet);)

	return pRet;
}

int CPgSQL::Prepare (char *query)
{
	if (PQstatus(db_connection) != CONNECTION_OK) {
		sprintf(db_last_error_message,"Error preparing statement, no database connected, %s\n",PQerrorMessage(db_connection));
		return -1;
	}

	// Clear any ongoing row fetch results in preparation for the new query.
	//
	if (db_result != NULL) {
		PQclear(db_result);
		db_result = NULL;
		current_row = 0;
	}

	PGresult *tmp_db_res;

	tmp_db_res = PQprepare(db_connection, "", query, 0, NULL);
	if ((tmp_db_res == NULL)|| !(PQresultStatus(tmp_db_res) == PGRES_COMMAND_OK)) {
		sprintf(db_last_error_message,"Error preparing SQL statement [%s], %s\n",query,PQerrorMessage(db_connection));
		PQclear(tmp_db_res);
		return -1;
	}
	PQclear(tmp_db_res);

	tmp_db_res = PQdescribePrepared(db_connection, "");
	if ((tmp_db_res == NULL)|| !(PQresultStatus(tmp_db_res) == PGRES_COMMAND_OK)) {
		sprintf(db_last_error_message,"Error describing SQL statement [%s], %s\n",query,PQerrorMessage(db_connection));
		PQclear(tmp_db_res);
		return -1;
	}
	int param_count = PQnparams(tmp_db_res);
	PQclear(tmp_db_res);

    return param_count;
}

int CPgSQL::ExecutePrepared (parsed_bundle_t* bind_params, unsigned int bind_param_count)
{
	DB(printf("DB: Execute [%p], [%d]\n",bind_params,bind_param_count);)

	if (PQstatus(db_connection) != CONNECTION_OK) {
		sprintf(db_last_error_message,"Error executing statement, no database connected, %s\n",PQerrorMessage(db_connection));
		return -1;
	}

	// Clear any ongoing fetch results in preparation for execution with new parameters.
	//
	if (db_result != NULL) {
		PQclear(db_result);
		db_result = NULL;
		current_row = 0;
	}

	PGresult *tmp_db_res;
    tmp_db_res = PQdescribePrepared(db_connection, "");
    if ((tmp_db_res == NULL)|| !(PQresultStatus(tmp_db_res) == PGRES_COMMAND_OK)) {
        sprintf(db_last_error_message,"Error executing prepared statement, unable to get metadata, %s\n",PQerrorMessage(db_connection));
        PQclear(tmp_db_res);
        return -1;
    }
    int param_count = PQnparams(tmp_db_res);
	for (int i=0; i<param_count; i++) {
		Oid o = PQparamtype(tmp_db_res,i);
		DB(printf("DB: Oid [%d]=[%d]\n",i,o);)
	}
    PQclear(tmp_db_res);

	if (bind_param_count != (unsigned int)param_count) {
        sprintf(db_last_error_message,"Error executing prepared SQL statement, bound parameter count [%u] does not match query parameter count [%d]\n",bind_param_count,param_count);
        return -1;
	}

	// Get a pointer to list of parameter metadata blocks (first block is header)
	//
	parsed_bundle_t *param_meta = bind_params += 1;

	if (param_count > 0) {

		//Oid *paramTypes    = (Oid*)malloc(sizeof(Oid) * bind_param_count);
		char **paramValues = (char **)malloc(sizeof(char*) * bind_param_count);
		int *paramLengths  = (int*)malloc(sizeof(int) * bind_param_count);
		int *paramFormats  = (int*)malloc(sizeof(int) * bind_param_count);

		for (int i = 0; i < param_count; i++) {

			DB(printf("DB: bind_params [%d][%p], Type [%u], Size [%u], Data [%p],[%08x]\n", i,
				&param_meta[i], param_meta[i].b_type, param_meta[i].b_size,
				param_meta[i].b_data,*(param_meta[i].b_data));)
		
			// Referenced Blob - The data passed are in the blob buffer. This field must be
			// passed as binary unless we encoded it to string first. This should be faster
			// than encoding and, unlike the row fetch formats, we can mix binary and text
			// parameter passing with paramFormats[]. paramTypes is not needed in this
			// mode since prepare already figured it out when parsing the query. The target
			// column should be BYTEAOID (OID = 17).
			//
			if (param_meta[i].b_type == 1) {
				if (param_meta[i].b_null) {
					paramValues[i] = NULL;
					paramLengths[i] = 0;
					paramFormats[i] = 0;
				} else {
					paramValues[i] = (char*)blob_buffer;
					paramLengths[i] = blob_buffer_size;
					paramFormats[i] = 1;
				}
			
			// In text passing mode everything else gets passed as a string, this works out
			// nicely, since that is how the data must be passed in from NWN script via
			// the SetLocalString function. Passing paramType[] = 0 lets PGSQL coerce the 
			// data into the column type for us!
			//
			} else if ((param_meta[i].b_type == 2)||(param_meta[i].b_type == 3)||(param_meta[i].b_type == 4)) {
				if (param_meta[i].b_null) {
					paramValues[i] = NULL;
					paramLengths[i] = 0;
					paramFormats[i] = 0;
				} else {
					paramValues[i] = (char*)(param_meta[i].b_data);
					paramLengths[i] = param_meta[i].b_size;
					paramFormats[i] = 0;
				}

			} else {
				sprintf(db_last_error_message,"Failed preparing SQL statement while binding parameter [%d], unknown type [%u]\n",i,bind_params[i].b_type);
				return -1;
			}
	
		}

		// Execute the statement with the given parameters. Using the anonymous bind statement
		// rather than nanmed prepared statements. All values retrieved as text - this requires
		// conversion of binary string data after retrieval, but that is much less involved
		// than decoding all types from binary form. Hopefully PGSQL will add the option of
		// passing somthing like dataFormats[] to this function eventually.
		//
		db_result = PQexecPrepared(db_connection, "", param_count, paramValues, paramLengths, paramFormats, 0);

		free(paramValues);
		free(paramLengths);
		free(paramFormats);

	} else {

		// Prepared with no parameter option. Not really an advantage over simple query
		// execution that I can see, but it is ok to prepare a parameterless query and
		// then execute it with no bound parameters. Note however, that this form does not
		// allow multiple statements since it is a form of prepare/bind.
		//
		db_result = PQexecPrepared(db_connection, "", 0, NULL, NULL, NULL, 0);
	
	}

	if (db_result == NULL) {

		sprintf(db_last_error_message,"Error executing prepared statement, %s\n",PQerrorMessage(db_connection));
		PQclear(db_result);
		db_result = NULL;
		return -1;

	} else if (PQresultStatus(db_result) == PGRES_COMMAND_OK) {

		/* May get rid of this and only return row counts for anticipated data rows
		char *n = PQcmdTuples(db_result);
		int row_count = ((n[0]=='\0')?0:atoi(n));
		PQclear(db_result);
		db_result = NULL;
		return row_count;
		*/
		return 0;

	} else if (PQresultStatus(db_result) == PGRES_TUPLES_OK) {

		int row_count = PQntuples(db_result);
		return row_count;

	} else {

		sprintf(db_last_error_message,"Error executing prepared statement, %s\n",PQerrorMessage(db_connection));
		PQclear(db_result);
		db_result = NULL;
		return -1;

	}

    return -1;
}

// Return NULL on Error, "" on no/end of data, ptr to data on success
//
char* CPgSQL::FetchPrepared (char *row_buffer, unsigned int row_buffer_size)
{
	return (Fetch(row_buffer,row_buffer_size));
}

bool CPgSQL::WriteScorcoData(char* SQL, BYTE* pData, int Length)
{
	int res;
	PGresult *sco_result;
	unsigned int len;
	char* Data = NULL;
	unsigned char *Data2 = NULL;
	char* pSQL = NULL;

	//len = mysql_real_escape_string (&mysql, Data + 1, (const char*)pData, Length);
	Data2 = PQescapeByteaConn(db_connection, pData, Length, &len);
	
	Data = new char[len + 4];
	pSQL = new char[MAXSQL + len + 3];

	memcpy(Data + 2, Data2, len);
	Data[0] = 'E';
	Data[1] = Data[len+1] = 39; //'
	Data[len + 2] = 0x0;
	sprintf(pSQL, SQL, Data);

	sco_result = PQexec (db_connection, pSQL);

	if (sco_result == NULL || PQresultStatus(sco_result) == PGRES_FATAL_ERROR)
		res = 1;
	else
		res = 0;
	
	PQclear (sco_result);
	
	delete[] pSQL;
	delete[] Data;
	PQfreemem(Data2);

	if (res == 0)
		return true;
	else
		return false;
}

BYTE* CPgSQL::ReadScorcoData(char* SQL, char *param, bool* pSqlError, int *size)
{
	PGresult *rcoresult;
	if (strcmp(param, "FETCHMODE") != 0)
	{	
		rcoresult = PQexec (db_connection, SQL);
		if (rcoresult == NULL || PQresultStatus(rcoresult) == PGRES_FATAL_ERROR)
		{
			PQclear (rcoresult);
			*pSqlError = true;
			return NULL;
		}
	}
	else rcoresult=db_result;

	*pSqlError = false;

	if(PQntuples(rcoresult) > 0)
	{
		unsigned char *buf = PQunescapeBytea((unsigned char *) PQgetvalue(rcoresult, 0, 0), (size_t *) size);
		if(!buf) return NULL;

		PQclear (rcoresult);

		// Return pointer to buffer allocated by PQunescapeBytea, should use PQfreemem on windows
		return (BYTE *)buf;
	}
	else
	{
		PQclear (rcoresult);
		return NULL;
	}
	return NULL;
}

