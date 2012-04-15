
/***************************************************************************
    DB plugin for NWNX - Implementation of the CNWNXDB class.
    copyright (c) 2006-2008 dumbo (dumbo@nm.ru) & virusman (virusman@virusman.ru)
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

#include <stdint.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include "NWNXdb.h"
#include "HookSCORCO.h"

extern PLUGINLINK *pluginLink;

unsigned char *blob_buffer;
unsigned int   blob_buffer_size;

static const char nib2chr[0x10] = {
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
};

static const unsigned char chr2nib[0x80] = {
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNWNXDB::CNWNXDB()
{
	confKey = "DB";

	request_counter = 0;
	sqlerror_counter = 0;
	scorcoSQL = 0;
	db = 0;
	dbType = dbNONE;
	hookScorco = true;
	memset (&p, 0, sizeof (PARAMETERS));

	last_bind_param_count = -1;

    blob_buffer = (unsigned char *)malloc(NWNX_DB_OBJECT_BUFFER_SIZE);
    blob_buffer[0] = '\0';
    blob_buffer_size = 0;

}
//============================================================================================================================

CNWNXDB::~CNWNXDB()
{
	OnRelease ();
}

//============================================================================================================================

bool CNWNXDB::OnCreate (gline *config, const char* LogDir)
{
	char log[128];
	bool validate = true, startServer = true;

	// call the base class function
	sprintf (log, "%s/nwnx_db.txt", LogDir);
	if (!CNWNXBase::OnCreate(config,log))
		return false;

	// write copy information to the log file
	Log (0, "NWNX2 DB Plugin Version 1.1.0 for Linux\n");
	Log (0, "(c) 2005-2006 dumbo (dumbo@nm.ru)\n");
	Log (0, "(c) 2006-2010 virusman (virusman@virusman.ru)\n");
	Log (0, "(c) 2012 eeriegeek (eeriegeem@yahoo.com)\n");
#ifdef SQLITE_SUPPORT
	Log (0, "NWNX Plugin compiled for SQLite database support\n");
#endif
#ifdef PGSQL_SUPPORT
	Log (0, "NWNX Plugin compiled for PostgreSQL database support\n");
#endif
#ifdef MYSQL_SUPPORT
	Log (0, "NWNX Plugin compiled for MySQL database support\n");
#endif

  if (!LoadConfiguration()) 
    return false;

  if(!pluginLink)
  {
	Log (0, "Plugin link not accessible\n");
  }
  else
  {
  		Log (0, "Plugin link: %08lX\n", pluginLink);
		Log (0, "Plugin link: %08lX\n", pluginLink);
	  hSCOEvent = CreateHookableEvent("NWServer/SCO");
	  hRCOEvent = CreateHookableEvent("NWServer/RCO");
  }

  if (hookScorco)
	{
		scorcoSQL = (char*) malloc(MAXSQL);
		int ret = HookFunctions();
		if (!ret) return ret;
	}

	return Connect();
}

//============================================================================================================================

bool CNWNXDB::Connect()
{
	int ret;

#ifdef MYSQL_SUPPORT
	CMySQL* mysql;
#endif
#ifdef SQLITE_SUPPORT
	CSQLite* sqlite;
#endif
#ifdef PGSQL_SUPPORT
	CPgSQL* pgsql;
#endif

	// create a database instance
	switch (dbType)
	{
#ifdef MYSQL_SUPPORT
		case dbMYSQL:
			mysql = new CMySQL();
			if (strcmp(p.server, "localhost") == 0)
			{
				if (p.socket)
                	Log(2, "o Connecting to localhost, socket=%s\n", p.socket);
				else
					Log(2, "o Connecting to localhost, default socket\n");
			}
			else if (p.server)
			{
				if(p.port)
					Log(2, "o Connecting to %s, port=%d\n", p.server, p.port);
				else
					Log(2, "o Connecting to %s, default port\n", p.server);
			}
			ret = mysql->Connect(p.server, p.user, p.pass, p.db, p.port, p.socket, p.charset);
			db = mysql;
			break;
#endif
#ifdef SQLITE_SUPPORT
		case dbSQLITE:
			sqlite = new CSQLite();
			ret = sqlite->Connect(p.db);
			db = sqlite;
			break;
#endif
#ifdef PGSQL_SUPPORT
		case dbPGSQL:
			pgsql = new CPgSQL();
			//connected = pgsql->Connect(p.server, p.user, p.pass, p.db);
			ret = pgsql->Connect(p.server,p.port,p.socket,p.db,p.charset,p.user,p.pass);
			db = pgsql;
			break;
#endif
		case dbNONE:
			// Do not connect, no database has been compiled in.
			Log (0, "o Not connecting, no valid datasource has been selected.\n");
			ret = -1;
			return true;
	}

	// try to connect to the database
	if (ret < 0)
	{
		Log (0, "! Error while connecting to database: %s\n", db->GetErrorMessage ());
		return false;
	}

	///db->SetBlobBuffer(blob_buffer,&blob_buffer_size);
	//db->bigbuf = NULL;
	//db->bigbuf_size = 0;
	//blob_buffer = NULL;
	//blob_buffer_size = 

	// we successfully connected
	Log (0, "o Connect successful.\n");
	return true;
}

//============================================================================================================================

char* CNWNXDB::OnRequest (char* gameObject, char* Request, char* Parameters)
{
    char* pResult = NULL;

	Log(1,"o OnRequest: Request [%s], Parameters [%32.32s]\n",Request,Parameters);

	     if (strcmp(Request,"EXEC"       ) == 0) pResult = Execute(Parameters);

	else if (strcmp(Request,"EXECUTE"    ) == 0) pResult = Execute(Parameters);
	else if (strcmp(Request,"FETCH"      ) == 0) pResult = Fetch(Parameters,strlen(Parameters));
	else if (strcmp(Request,"ESCAPE"     ) == 0) pResult = Escape((unsigned char *)Parameters,strlen(Parameters));
	else if (strcmp(Request,"P_PREPARE"  ) == 0) pResult = Prepare(Parameters);
	else if (strcmp(Request,"P_EXECUTE"  ) == 0) pResult = ExecutePrepared(Parameters);
	else if (strcmp(Request,"P_FETCH"    ) == 0) pResult = FetchPrepared(Parameters,strlen(Parameters));
	else if (strcmp(Request,"GETDBID"    ) == 0) pResult = GetDbId(Parameters);
	else if (strcmp(Request,"OBJ_FREEZE" ) == 0) pResult = Freeze(Parameters);
	else if (strcmp(Request,"OBJ_THAW"   ) == 0) pResult = Thaw(Parameters);
	else if (strcmp(Request,"OBJ_GETTYPE") == 0) pResult = GetType(Parameters);
	else if (strcmp(Request,"OBJ_ENCODE" ) == 0) pResult = Encode(Parameters);
	else if (strcmp(Request,"OBJ_DECODE" ) == 0) pResult = Decode(Parameters);

	else if (strcmp(Request,"SETSCORCOSQL") == 0) SetScorcoSQL(Parameters);
	else if (strcmp(Request,"STOREOBJECT") == 0)
	{
		int nSize = 0;
		char *pData = SaveObject(strtol(Parameters, NULL, 16), nSize);
		WriteSCO("NWNX", "-", "-", 0, (unsigned char *)pData, nSize);
	}
	else if (strcmp(Request,"RETRIEVEOBJECT") == 0)
	{
		Log(3,"o RETRIEVEOBJECT: Parameters [%s]\n",Parameters);
		dword AreaID;
		float x, y, z, fFacing;
		if(sscanf(Parameters, "%lx¬%f¬%f¬%f¬%f", &AreaID, &x, &y, &z, &fFacing)<5)
		{
			Log(1, "o sscanf error\n");
			return NULL;
		}
		Location lLoc;
		lLoc.AreaID = AreaID;
		lLoc.vect.X = x;
		lLoc.vect.Y = y;
		lLoc.vect.Z = z;
		lLoc.Facing = fFacing;
		int nSize = 0;
		unsigned char *pData = ReadSCO("NWNX", "-", "-", &nSize, &nSize);
		if(pData && nSize)
			lastObjectID = LoadObject((const char *)pData, nSize, lLoc);
		else
			lastObjectID = 0x7F000000;
	}

	return pResult;
}

//============================================================================================================================

char* CNWNXDB::GetDbId(char *Parameters)
{ 
	char* pReturn = Parameters;
	size_t pReturnLen = ((pReturn==NULL)?0:strlen(pReturn));

	if (db==NULL) {
		Log(0,"! GetDbId: The database structure is NULL, is database support compiled?\n");
		if (pReturn!=NULL) pReturn[0] = '\0';
		return pReturn;
    }

	const char* dbid = db->GetDbId();
	size_t dbid_size = strlen(dbid);

	if (dbid_size > pReturnLen) pReturn = (char*)malloc(dbid_size+1);
	
	strcpy(pReturn,dbid);

	return pReturn;
}

// Serializes the game object to a buffer where other methods can access it.
//
char* CNWNXDB::Freeze(char *Parameters)
{

	Log(3,"o FREEZE: Parameters [%s]\n",Parameters);

	// Serialize the game object (Placeable, Merchant, Trigger)
	int nSize = 0;
	char *pData = SaveObject(strtol(Parameters,NULL,16),nSize);

	// Store the serialized object to buffer
	// TODO: full dynamic allocation of bigbuf ?
	if (nSize > NWNX_DB_OBJECT_BUFFER_SIZE) {
		Log(0,"! Object size %u exceeded internal buffer space.\n",(unsigned int)nSize);
		return NULL;
	}
	//if (db->blob==NULL) { db->bigbuf = (unsigned char *)malloc(NWNX_DB_OBJECT_BUFFER_SIZE); }
	memcpy(blob_buffer,(unsigned char *)pData,(unsigned int)nSize);
	blob_buffer[(unsigned int)nSize]='\0';
	blob_buffer_size = (unsigned int)nSize;

	return NULL;
}

//============================================================================================================================

// De-serializes a game object from the buffer.
//
char* CNWNXDB::Thaw(char *Parameters)
{

	dword AreaID;
	float x, y, z, fFacing;

	Log(3,"o THAW: Parameters [%s]\n",Parameters);

	//if(sscanf(Parameters, "%lux¬%f¬%f¬%f¬%f", &AreaID, &x, &y, &z, &fFacing)<5)
	int ret = sscanf(Parameters,"%lx¬%f¬%f¬%f¬%f", &AreaID, &x, &y, &z, &fFacing);
	if(ret<5)
	{
		Log(1, "o sscanf error, ret=%d, [%lx][%f], %s\n",ret,AreaID,fFacing,strerror(errno));
		return NULL;
	}
	Location lLoc;
	lLoc.AreaID = AreaID;
	lLoc.vect.X = x;
	lLoc.vect.Y = y;
	lLoc.vect.Z = z;
	lLoc.Facing = fFacing;
	int nSize = 0;

	//unsigned char *pData = ReadSCO("NWNX", "-", "-", &nSize, &nSize);

	Log (3,"o BigBuf [%08lX] [%lu] [%s]\n",blob_buffer,blob_buffer_size,blob_buffer);

	if(blob_buffer && blob_buffer_size)
		lastObjectID = LoadObject((const char *)blob_buffer, blob_buffer_size, lLoc);
	else
		lastObjectID = 0x7F000000;

	Log (3,"o lastObjectID [%08lX] [%lu]\n",lastObjectID,lastObjectID);

	return NULL;
}

//
// OBJECT_TYPE_CREATURE         = 1;       <-
// OBJECT_TYPE_ITEM             = 2;       <-
// OBJECT_TYPE_TRIGGER          = 4;       <-
// OBJECT_TYPE_DOOR             = 8;
// OBJECT_TYPE_AREA_OF_EFFECT   = 16;
// OBJECT_TYPE_WAYPOINT         = 32;
// OBJECT_TYPE_PLACEABLE        = 64;      <-
// OBJECT_TYPE_STORE            = 128;     <-
// OBJECT_TYPE_ENCOUNTER        = 256;
// OBJECT_TYPE_ALL              = 32767;
// OBJECT_TYPE_INVALID          = 32767;
//
char* CNWNXDB::GetType(char *Parameters)
{
	char *pRet = Parameters;
	if ((Parameters==NULL)||(strlen(Parameters)<4)) pRet = (char*)malloc(4);

	if (strncmp("BIC V3.28",(char*)blob_buffer,9)==0) {
		strcpy(pRet,"1");
	} else if (strncmp("UTI V3.28",(char*)blob_buffer,9)==0) {
		strcpy(pRet,"2");
	} else if (strncmp("UTT V3.28",(char*)blob_buffer,9)==0) {
		strcpy(pRet,"4");
	} else if (strncmp("UTD V3.28",(char*)blob_buffer,9)==0) {
		strcpy(pRet,"8");
	} else if (strncmp("AOE V3.28",(char*)blob_buffer,9)==0) { // Needs corrected
		strcpy(pRet,"16");
	} else if (strncmp("UTW V3.28",(char*)blob_buffer,9)==0) {
		strcpy(pRet,"32");
	} else if (strncmp("UTP V3.28",(char*)blob_buffer,9)==0) {
		strcpy(pRet,"64");
	} else if (strncmp("UTM V3.28",(char*)blob_buffer,9)==0) {
		strcpy(pRet,"128");
	} else if (strncmp("UTE V3.28",(char*)blob_buffer,9)==0) {
		strcpy(pRet,"256");
	} else {
		strcpy(pRet,"0");
	}

	return pRet;
}

// frozen nwn object in buffer converted to hex encoded string object passed to nwscript
char* CNWNXDB::Encode(char* Parameters)
{
	int             n = blob_buffer_size;
	unsigned char * p = blob_buffer;
	char*           q = (char*)malloc((2*blob_buffer_size)+9);

	int i,j;

	for (i=0,j=0; i<n; i+=1,j+=2) { q[j] = nib2chr[p[i]>>4]; q[j+1] = nib2chr[p[i]&0x0F]; }

	q[j] = '\0';

    return q;
}

// hex encoded string object passed in parameters converted to frozen nwn object in buffer
char* CNWNXDB::Decode(char* Parameters)
{
    int            n = strlen(Parameters);
    unsigned char* p = (unsigned char *)Parameters;
    unsigned char* q = blob_buffer;

    int i,j;

    for (i=0,j=0; i<n; i+=2,j+=1) q[j] = (chr2nib[p[i]]<<4)|(chr2nib[p[i+1]]);

    blob_buffer_size = j;

	return NULL;
}

//============================================================================================================================

unsigned long CNWNXDB::OnRequestObject(char *gameObject, char *Request)
{
	if (strncmp(Request, "RETRIEVEOBJECT", 14) == 0)
	{
		return lastObjectID;
	}
	return 0;
}

//============================================================================================================================

char* CNWNXDB::Execute(char *Parameters)
{
	char* pRet = Parameters;

	if (Parameters==NULL) {
		Log(0,"! EXECUTE: Internal error, the Parameter string is NULL.\n");
		return pRet;
	}
	if (Parameters[0]=='\0') {
		Log(0,"! EXECUTE: The query string is empty.\n");
		pRet = (char*)malloc(8);
		strcpy(pRet,"0");
		return pRet;
	}
	if (db==NULL) {
		Log(0,"! EXECUTE: The database structure is NULL, is database support compiled?\n");
		strcpy(pRet,"0");
		return pRet;
	}

	Log(3,"o EXECUTE: Query [%s]\n",Parameters);

	request_counter++;

	int ret = db->Execute(Parameters);

	if (ret < 0) {
		Log(0,"! EXECUTE: Execution of query string [%s] failed, %s\n",Parameters,db->GetErrorMessage());
		strcpy(pRet,"0");
		return pRet;
	}

	Log(3,"o EXECUTE: Query execution complete, [%d] rows effected.\n",ret);

	strcpy(pRet,"1");
	return pRet;
}

//============================================================================================================================

/*
void CNWNXDB::Fetch(char *buffer, unsigned int buffersize)
{
	if (!db) {
		Log (1, "! Error: Tried to execute SQL statement, but no support compiled in.\n");
		buffer[0] = 0x0;
		return;
	}

	unsigned int totalbytes = 0;
	buffer[0] = 0x0;
	// fetch data from recordset
	totalbytes = db->Fetch (buffer, buffersize);
  // log what we received
  if (totalbytes == (unsigned int)-1)
    Log (2, "o Empty set\n");
  else
    Log (2, "o Sent response (%d bytes): %s\n", totalbytes, buffer);
}
*/

char* CNWNXDB::Fetch(char *buffer, unsigned int buffersize)
{
   	Log(3,"o FETCH: Buffer, Size [%d], Data [%8.8s...]\n",buffersize, buffer);

	char* pRet = buffer;

	if (buffer==NULL) {
		Log(0,"! FETCH: Internal error, the result buffer is NULL.\n");
		return pRet;
	}

	if (db==NULL) {
		Log(0,"! FETCH: The database structure is NULL, is database support compiled?\n");
		pRet[0] = '\0';
		return pRet;
	}

	// Returns pointer to results buffer, will realloc if needed, NULL on error.
	//
	char* pTmp = db->Fetch(buffer,buffersize);

   	Log(3,"o FETCH: Buffers [%p] -> [%p]\n",pRet,pTmp);

	if (pTmp==NULL) {
		Log(0,"! FETCH: Data fetch error, %s\n",db->GetErrorMessage());
		pRet[0] = '\0';
		return pRet;
	} else if (pTmp==pRet) {
		Log(3,"o FETCH: No buffer reallocation was needed.\n");
	} else {
		Log(3,"o FETCH: Buffer reallocation was needed.\n");
		pRet = pTmp;
	}
	
	if (pRet[0] == '\0') {
		Log(3,"o FETCH: Got empty results.\n");
	} else {
		Log(3,"o FETCH: Got row results, length [%u], data [%s]\n",strlen(pRet),pRet);
	}

	return pRet;
}

//============================================================================================================================
//============================================================================================================================

//
// Escape SQL data and NWNX control tokens
//
char* CNWNXDB::Escape (unsigned char *buffer, unsigned long buffersize)
{
	Log(3,"o ESCAPE: BEFORE [%s]\n",buffer);

	if (db==NULL) {
		Log(0,"! Database NULL, is database support compiled?\n");
		buffer[0] = '\0';
		return NULL;
	}
 
	// Convert binary/control junk and the NWNX tokens to harmless 0xFE (254)
	// 171 = '«' and 172 = '¬', on PC alt code ALt171 = 1/2 Alt172 = 1/4 
	// An alternative to ascii 172 might be to use the ascii line control chars
	// Alt027 - group seperator, and Alt028 - record separator might work well
	// and are non-printable anyway. For now just get rid of 172 and a few
	// odd characters.
	// 
	unsigned long i;
	for (i = 0; i < buffersize; i++) if ((buffer[i]<1)||(buffer[i]==127)||(buffer[i]==172)||(buffer[i]==255)) buffer[i]=0xFE;

	// Call the database implementations SQL escape, the callee is expected to 
	// allocate a new block of mem with malloc and return a pointer to it.
	// If NULL is returned, NWNX should return the original buffer with only the
	// changes made by the binary cleaner above.
	//
	char* pRet = db->Escape(buffer,buffersize);

	Log(3,"o ESCAPE: AFTER  [%s]\n",pRet);

	return pRet;
}

//============================================================================================================================

//
// Returns ptr to "1" on success else ptr to "0". Uses parameter string for return result.
//
char* CNWNXDB::Prepare (char *Parameters)
{
	char* pRet = Parameters;

	if (Parameters==NULL) {
		Log(0,"! PREPARE: Internal error, the query string is NULL.\n");
		return pRet;
	}
	if (Parameters[0]=='\0') {
		Log(0,"! PREPARE: The query string is empty.\n");
		pRet = (char*)malloc(8);
		strcpy(pRet,"0");
		return pRet;
	}
	if (db==NULL) {
		Log(0,"! PREPARE: The database structure is NULL, is database support compiled?\n");
		strcpy(pRet,"0");
		return pRet;
	}

	Log(3,"o PREPARE: Query [%s]\n",Parameters);

	int ret = db->Prepare(Parameters);

	if (ret < 0) {
		last_bind_param_count = -1;
		Log(0,"! PREPARE: Query preparation of query string [%s] failed, %s\n",Parameters,db->GetErrorMessage());
		strcpy(pRet,"0");
		return pRet;
	}

	last_bind_param_count = ret;

	Log(3,"o PREPARE: Prepare complete, expecting [%d] parameters.\n",ret);

	strcpy(pRet,"1");
	return pRet;
}

// Returns ptr to "1" on success else ptr to "0". Uses parameter string for return result.
//
char* CNWNXDB::ExecutePrepared(char *Parameters)
{
	char* pRet = Parameters;

	if (Parameters==NULL) {
		Log(0,"! EXECUTE: Internal error, the parameter string is NULL.\n");
		return pRet;
	}
	if (Parameters[0]=='\0') {
		Log(0,"! EXECUTE: The parameter string is empty.\n");
		pRet = (char*)malloc(8);
		strcpy(pRet,"0");
		return pRet;
	}
	if (strlen(Parameters) < 7) {
		Log(0,"! EXECUTE: The parameter string [%s] is too short.\n",Parameters);
		strcpy(pRet,"0");
		return pRet;
	}
	if (db==NULL) {
		Log(0,"! EXECUTE: The database structure is NULL, is database support compiled?\n");
		strcpy(pRet,"0");
		return pRet;
	}
	if (last_bind_param_count < 0) {
		Log(0,"! EXECUTE: A query must be prepared before it can be executed with parameters.\n");
		strcpy(pRet,"0");
		return pRet;
	}

	Log(3,"o EXECUTE: Parameter string [%s]\n",Parameters);

	unsigned int parm_count = 0;

	parsed_bundle_t* parms = NULL;

	if (strcmp(Parameters,"|0|||||")==0) {

		// Shortcut taken for NULL message "|0|||||", a valid message for 0 parameters.
		//
		parm_count = 0;

	} else {

		parms = db->ParseBundle(Parameters);
		if (parms==NULL) {
			Log(0,"! EXECUTE: Invalid parameter string [%s]\n",Parameters);
			strcpy(pRet,"0");
			return pRet;
		}

		parm_count = parms[0].b_size;

	}

	Log(3,"o EXECUTE: Parameter count [%u]\n",parm_count);

	if (parm_count != (unsigned int)last_bind_param_count) {
		Log(0,"! EXECUTE: The number of parameters in [%s] does not match the prepared query.\n",Parameters);
		free(parms);
		strcpy(pRet,"0");
		return pRet;
	}

	// Execute the previously prepared query with the given parameters. Returns the
	// number of rows effected, or -1 on error.
	//
	int ret = db->ExecutePrepared(parms,parm_count);

	free(parms);

	if (ret < 0) {
		Log(0,"! EXECUTE: Query execution failed, %s\n",db->GetErrorMessage());
		strcpy(pRet,"0");
		return pRet;
	}

	Log(3,"o EXECUTE: Query execution complete, [%d] rows effected.\n",ret);

	strcpy(pRet,"1");
	return pRet;
}

//============================================================================================================================

//
// Returns pointer to a row of data or returns an empty string "" if no data or error.
// Will use the buffer pointed to by result if large enough, otherwise mallocs a new one. 
//
char* CNWNXDB::FetchPrepared(char *buffer, unsigned int buffersize)
{
   	Log(3,"o FETCH: Buf [%s]\n",buffer);

	char* pRet = buffer;

	if (buffer==NULL) {
		Log(0,"! FETCH: Internal error, the result buffer is NULL.\n");
		return pRet;
	}

	if (db==NULL) {
		Log(0,"! FETCH: The database structure is NULL, is database support compiled?\n");
		pRet[0] = '\0';
		return pRet;
	}

	// Returns pointer to results buffer, will realloc if needed, NULL on error.
	//
	char* pTmp = db->FetchPrepared(buffer,buffersize);

   	Log(3,"o FETCH: Buffers [%p] -> [%p]\n",pRet,pTmp);

	if (pTmp==NULL) {
		Log(0,"! FETCH: Data fetch error, %s\n",db->GetErrorMessage());
		pRet[0] = '\0';
		return pRet;
	} else if (pTmp==pRet) {
		Log(3,"o FETCH: No buffer reallocation was needed.\n");
	} else {
		Log(3,"o FETCH: Buffer reallocation was needed.\n");
		pRet = pTmp;
	}
	
	if (pRet[0] == '\0') {
		Log(3,"o FETCH: Got empty results.\n");
	} else {
		Log(3,"o FETCH: Got row results, length [%u], data [%s]\n",strlen(pRet),pRet);
	}

	return pRet;
}

//============================================================================================================================
//============================================================================================================================

void CNWNXDB::SetScorcoSQL(char *request)
{
	if (!db) {
		Log (1, "! Error: Tried to execute SQL-based SCO/RCO, but no support compiled in.\n");
		return;
	}

	memcpy(scorcoSQL, request, strlen(request) + 1);
  Log (2, "o Got request (scorco): %s\n", scorcoSQL);
}

//============================================================================================================================

bool CNWNXDB::OnRelease ()
{
  Log (0, "o Shutdown.\n");

  if (db) db->Disconnect ();

	if (scorcoSQL)
    free(scorcoSQL);

	// release the server
	if (p.server)
		free (p.server);

	// release memory
	if (dbType == dbMYSQL || dbType == dbPGSQL)
	{
		free (p.user);
		free (p.pass);
		free (p.db);
	}
	return true;
}

//============================================================================================================================

int CNWNXDB::WriteSCO(char* database, char* key, char* player, int flags, unsigned char * pData, int size)
{
  Log(3, "o SCO: db='%s', key='%s', player='%s', flags=%08lX, pData=%08lX, size=%08lX\n", database, key, player, flags, pData, size);

  if(strcmp(key,"-") == 0)
  {

	if (!db) {
		Log (1, "! Error: Tried to execute SQL-based SCO/RCO, but no support compiled in.\n");
		return 0;
	}

	  if (size > 0)
		{
			//Log ("o Writing scorco data.\n");
			if (!db->WriteScorcoData(scorcoSQL, pData, size))
	      Log (1, "! SQL Error: %s\n", db->GetErrorMessage ());
	    else
	      return 1;
		}
	  return 0;
  }
	else if (strcmp(key,"F")==0) 
	{
		// Serialize (Freeze) object to buffer (Item, Creature)
		if (size > NWNX_DB_OBJECT_BUFFER_SIZE) {
			Log(0,"! Object size %u exceeded internal buffer space.\n",(unsigned int)size);
			return 0;
		}
		//if (db->bigbuf==NULL) { db->bigbuf = (unsigned char *)malloc(NWNX_DB_OBJECT_BUFFER_SIZE); }
		memcpy(blob_buffer,(unsigned char *)pData,(unsigned int)size);
		blob_buffer[(unsigned int)size]='\0';
		blob_buffer_size = (unsigned int)size;
		//db->SetBlobBuffer(blob_buffer,&blob_buffer_size);
	    return 1;
	}
  else 
  {
	  SCORCOStruct scoInfo = {
		database,
		key,
		player,
		pData,
		size
	  };
	  NotifyEventHooks(hSCOEvent,(WPARAM)&scoInfo,0);

	  return 1;
  }
}

//============================================================================================================================

unsigned char* CNWNXDB::ReadSCO(char* database, char* key, char* player, int* arg4, int* size)
{
  *arg4 = 0x4f;

  Log(3, "o RCO(0): db='%s', key='%s', player='%s', arg4=%08lX, size=%08lX\n", database, key, player, arg4, size);

  if(strcmp(key,"-") == 0)
  {
	if (!db) {
		Log (1, "! Error: Tried to execute SQL-based SCO/RCO, but no support compiled in.\n");
		return NULL;
	}

	BYTE* pData;
	bool sqlError;
	
	//Log ("o Reading scorco data.\n");
	pData = db->ReadScorcoData(scorcoSQL, key, &sqlError, size);
	
  Log(3, "o RCO(1): db='%s', key='%s', player='%s', arg4=%08lX, size=%08lX\n", database, key, player, *arg4, *size);
  Log(3, "o RCO(2): err=%lX, pData=%08lX, pData='%s'\n", sqlError, pData, pData);

  if (!sqlError && pData)
		return pData;
	else 
	{
		if (sqlError)
			Log (1, "! SQL Error: %s\n", db->GetErrorMessage ());
	}
  return NULL;
  }
	else if (strcmp(key,"T")==0) 
	{
        if ((blob_buffer!=NULL)&&(blob_buffer_size>0)) {
        	Log(3,"o RCO(t): bigbuf=%08lX, bigbuf_size=%08lX, *bigbuf=[%8s]\n", blob_buffer, blob_buffer_size, blob_buffer);
			//unsigned char * pData = (unsigned char *)malloc(db->bigbuf_size+1);
			//char* buf = new char[*length];
			unsigned char *pData = new unsigned char[blob_buffer_size+1];
        	memcpy(pData,blob_buffer,blob_buffer_size);
        	pData[blob_buffer_size]='\0';
			*size = blob_buffer_size;
        	Log(3,"o RCO(t): pData=%08lX, size=%08lX, buff=[%8s]\n", pData, *size, pData);
			return pData;
		} else {
            Log(0,"! No object in buffer.\n");
			return NULL;
		}
	} else
  {
	SCORCOStruct rcoInfo = {
		database,
		key,
		player,
		NULL,
		0
	};
	NotifyEventHooks(hRCOEvent,(WPARAM)&rcoInfo,0);

	if(rcoInfo.pData && rcoInfo.size)
	{
		*size = rcoInfo.size;
		Log(3, "o RCO(n1): db='%s', key='%s', player='%s', arg4=%08lX, size=%08lX\n", database, key, player, *arg4, *size);
		Log(3, "o RCO(n2): pData=%08lX, size=%08lX, pData='%s'\n", rcoInfo.pData, rcoInfo.size, rcoInfo.pData);
		return rcoInfo.pData;
	}
	return NULL;
  }
}

//============================================================================================================================
bool CNWNXDB::LoadConfiguration ()
{
	char buffer[256];

	if(!nwnxConfig->exists(confKey)) {
		Log (0, "o Critical Error: Section [%s] not found in nwnx2.ini\n", confKey);
		return false;
	}

  // see what mode should be used
	strcpy(buffer, (char*)((*nwnxConfig)[confKey]["source"].c_str()));
	dbType = dbNONE;

#ifdef MYSQL_SUPPORT
	if (strcasecmp (buffer, "MYSQL") == 0) {
		// load in the settings for a direct mysql connection
		p.server    = strdup((char*)((*nwnxConfig)[confKey]["server"].c_str()));
		p.user      = strdup((char*)((*nwnxConfig)[confKey]["user"].c_str()));
		p.pass      = strdup((char*)((*nwnxConfig)[confKey]["pass"].c_str()));
		if(!*p.pass)
			p.pass  = strdup((char*)((*nwnxConfig)[confKey]["pwd"].c_str()));
		p.db        = strdup((char*)((*nwnxConfig)[confKey]["db"].c_str()));
		p.port      = atoi((char*)((*nwnxConfig)[confKey]["port"].c_str()));
		p.socket    = strdup((char*)((*nwnxConfig)[confKey]["socket"].c_str()));
		if(p.socket && !*p.socket)
			p.socket = NULL;		//use default socket if it's not specified in the config
		p.charset   = strdup((char*)((*nwnxConfig)[confKey]["charset"].c_str()));
		dbType = dbMYSQL;
	}
#endif
/*
  else if (strcasecmp (buffer, "ODBC") == 0) {
		p.server = strdup((char*)((*nwnxConfig)[confKey]["dsn"].c_str()));
		dbType = dbODBC;
	}*/
#ifdef SQLITE_SUPPORT
	if (strcasecmp (buffer, "SQLITE") == 0) {
		// load in the settings for the internal database
		p.db = strdup((char*)((*nwnxConfig)[confKey]["file"].c_str()));
		dbType = dbSQLITE;
	}
#endif
#ifdef PGSQL_SUPPORT
	if (strcasecmp (buffer, "PGSQL") == 0) {
		  // load in the settings for a direct pgsql connection
		  p.server	  = strdup((char*)((*nwnxConfig)[confKey]["server"].c_str()));
		  p.user	  = strdup((char*)((*nwnxConfig)[confKey]["user"].c_str()));
		  p.pass	  = strdup((char*)((*nwnxConfig)[confKey]["pass"].c_str()));
		  if(!*p.pass)
			p.pass    = strdup((char*)((*nwnxConfig)[confKey]["pwd"].c_str()));
		  p.db		  = strdup((char*)((*nwnxConfig)[confKey]["db"].c_str()));
		  dbType = dbPGSQL;
	}
#endif
	if (dbType == dbNONE) {
		p.db = NULL;
		Log(0, "o Warning: Datasource must be one of:\n");
#ifdef SQLITE_SUPPORT
		Log (0, "oo SQLITE\n");
#endif
#ifdef PGSQL_SUPPORT
		Log (0, "oo PGSQL\n");
#endif
#ifdef MYSQL_SUPPORT
		Log (0, "oo MYSQL\n");
#endif
		Log (0, "o Only providing SCO/RCO hooks to other plugins.\n");
	}

	// check if scorco should be hooked
	strcpy(buffer, (char*)((*nwnxConfig)[confKey]["hookscorco"].c_str()));
	if (strcasecmp (buffer, "false") == 0)
		hookScorco = false;

	return true;
}


