/******************************************************************************

  nwnx_gdbm - NWNX - GDBM Plugin - Binding to GNU hashed key-value file db.

  Copyright 2012-2013 eeriegeek (eeriegeek@yahoo.com)

  This file is part of NWNX.

  NWNX is free software: you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your option) any later
  version.

  NWNX is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along
  with NWNX.  If not, see <http://www.gnu.org/licenses/>.

******************************************************************************/

#include "nwnx_aggregate"

//-----------------------------------------------------------------------------
// The first three GDBM functions here cover major database maintenance. For
// most uses, these will never be needed. Create's only additional function
// over open is the ablility to truncate a database file on open. Reorganize
// is only needed if a very large number of deletes are done, and the physical
// disk space is needed.
//-----------------------------------------------------------------------------

// Creates a GDBM file. This is a wrapper around the underlying open method and
// calling create also opens the file for read/write access. Setting bTruncate
// to true destroys the current database file before opening the new one. This
// method adds nothing over open, except the ability to truncate the database.
// Returns TRUE if the GDBM file was created/opened.
//
int NWNX_GdbmCreate(string sDbName, int bTruncate = FALSE);

// Reorganizes the contents and hash indexes in the GDBM file and shrinks the
// physical disk file if there is free space. This method is slow and should
// very seldom (if ever) need to be called. GDBM reuses deleted space so this
// method only needs to be called if a large number of deletions occurs that
// are not expected to be consumed soon by new entries. In a normal, growing
// database, this should only happen after major system maintenance.
//
void NWNX_GdbmReorganize(string sDbName);

// Delete the GDBM database file. Permanently. You have a backup, right?
// This is not really a GDBM function but a wrapper to file deletion.
//
int NWNX_GdbmDestroy(string sDbName);

//-----------------------------------------------------------------------------
// The following methods handle the normal database open-close use cycle. If
// using the DML calls with the open option, these methods may not be needed.
//-----------------------------------------------------------------------------

// Opens a GDBM file. By default, will open in read-write mode and create a
// new GDBM file if one does not exist. Unlike GdbmCreate, open is a safe
// operation and will not truncate the database. bReadOnly can only be TRUE
// if bCreate is FALSE. Returns TRUE if the GDBM file is successfully opened.
//
int NWNX_GdbmOpen(string sDbName, int bCreate = TRUE, int bReadOnly = FALSE);

// Syncronizes cached data to disk. Call if you are done writing for a while,
// but not ready to close the database yet. Close also syncs data.
//
void NWNX_GdbmSync(string sDbName);

// Closes the database file. Open will need to be called before further access.
//
void NWNX_GdbmClose(string sDbName);

// Closes all open GDBM database files. Call when done with all GDBM file use
// to sync all reader/writer counts and insure all data are written to disk.
// Provided here mainly for testing, NWNX will call this on server shutdown.
//
void NWNX_GdbmCloseAll();

//-----------------------------------------------------------------------------
// Primary Data Manipulation (DML) type functions. Insert, update, and delete key/value
// pairs. A GDBM database MUST be opened with GdbmCreate or GdbmOpen before
// use, however, setting the bOpen flag does this automatically.
//-----------------------------------------------------------------------------

// Stores a key/value pair to the GDBM database. If bReplace is set to FALSE,
// will refuse to overwrite a key currently in the database. Returns FALSE if
// an error occurs (such as storing to closed/non-existent database) or if
// bReplace is FALSE and the store operation would have overwritten a value.
//
int NWNX_GdbmStore(string sDbName, string sKey, string sValue, int bReplace = TRUE, int bOpen = TRUE);

// Returns TRUE if the specified sKey has a value stored in the GDBM database.
//
int NWNX_GdbmExists(string sDbName, string sKey, int bOpen = TRUE);

// Fetches the value stored for the specified sKey from the GDBM database.
// Returns an empty string "" if no matching key is found.
//
string NWNX_GdbmFetch(string sDbName, string sKey, int bOpen = TRUE);

// Deletes the key/value pair specified by sKey from the GDBM database. Returns
// TRUE if the key was found and deleted.
//
int NWNX_GdbmDelete(string sDbName, string sKey, int bOpen = TRUE);

//-----------------------------------------------------------------------------
// This set of "typed" methods constitute the main API for client applications.
// Each method converts to/from a specific NWN data type for storage as a
// string in GDBM. The data are stored with a type specific key, so e.g. Int
// and Vector are not confused in the database and a String named "foo" is
// different than a float named "foo".
//-----------------------------------------------------------------------------

int NWNX_GdbmStoreString(string sDbName, string sKey, string sValue, int bReplace = TRUE, int bOpen = TRUE);
int NWNX_GdbmStoreInt(string sDbName, string sKey, int iValue, int bReplace = TRUE, int bOpen = TRUE);
int NWNX_GdbmStoreFloat(string sDbName, string sKey, float fValue, int bReplace = TRUE, int bOpen = TRUE);
int NWNX_GdbmStoreVector(string sDbName, string sKey, vector vValue, int bReplace = TRUE, int bOpen = TRUE);
int NWNX_GdbmStoreLocation(string sDbName, string sKey, location mValue, int bReplace = TRUE, int bOpen = TRUE);
int NWNX_GdbmStoreObject(string sDbName, string sKey, object oValue, int bReplace = TRUE, int bOpen = TRUE);

int NWNX_GdbmExistsString(string sDbName, string sKey, int bOpen = TRUE);
int NWNX_GdbmExistsInt(string sDbName, string sKey, int bOpen = TRUE);
int NWNX_GdbmExistsFloat(string sDbName, string sKey, int bOpen = TRUE);
int NWNX_GdbmExistsVector(string sDbName, string sKey, int bOpen = TRUE);
int NWNX_GdbmExistsLocation(string sDbName, string sKey, int bOpen = TRUE);
int NWNX_GdbmExistsObject(string sDbName, string sKey, int bOpen = TRUE);

string   NWNX_GdbmFetchString(string sDbName, string sKey, int bOpen = TRUE);
int      NWNX_GdbmFetchInt(string sDbName, string sKey, int bOpen = TRUE);
float    NWNX_GdbmFetchFloat(string sDbName, string sKey, int bOpen = TRUE);
vector   NWNX_GdbmFetchVector(string sDbName, string sKey, int bOpen = TRUE);
location NWNX_GdbmFetchLocation(string sDbName, string sKey, int bOpen = TRUE);
object   NWNX_GdbmFetchObject(string sDbName, string sKey, location mDestination, object oDestination, int bOpen = TRUE);

int NWNX_GdbmDeleteString(string sDbName, string sKey, int bOpen = TRUE);
int NWNX_GdbmDeleteInt(string sDbName, string sKey, int bOpen = TRUE);
int NWNX_GdbmDeleteFloat(string sDbName, string sKey, int bOpen = TRUE);
int NWNX_GdbmDeleteVector(string sDbName, string sKey, int bOpen = TRUE);
int NWNX_GdbmDeleteLocation(string sDbName, string sKey, int bOpen = TRUE);
int NWNX_GdbmDeleteObject(string sDbName, string sKey, int bOpen = TRUE);

//-----------------------------------------------------------------------------
// Implementation 
//-----------------------------------------------------------------------------

const string NWNX_GDBM_TOKEN = "";
const string NWNX_GDBM_KEYTOK = "";

//
//  Maintenance & Lifecycle
//
int NWNX_GdbmCreate(string sDbName, int bTruncate = FALSE)
{
    SetLocalString(GetModule(),"NWNX!GDBM!CREATE", sDbName + NWNX_GDBM_TOKEN + (bTruncate?"1":"0"));
    return (GetLocalString(GetModule(),"NWNX!GDBM!CREATE")=="1");
}

void NWNX_GdbmReorganize(string sDbName)
{
    SetLocalString(GetModule(), "NWNX!GDBM!REORGANIZE", sDbName);
    return;
}

int NWNX_GdbmDestroy(string sDbName)
{
    SetLocalString(GetModule(), "NWNX!GDBM!DESTROY", sDbName);
    return (GetLocalString(GetModule(),"NWNX!GDBM!DESTROY")=="1");
}

int NWNX_GdbmOpen(string sDbName, int bCreate=TRUE, int bReadOnly=FALSE)
{
    SetLocalString(GetModule(), "NWNX!GDBM!OPEN", sDbName + NWNX_GDBM_TOKEN + (bCreate?"1":"0") + NWNX_GDBM_TOKEN + (bReadOnly?"1":"0"));
    return (GetLocalString(GetModule(),"NWNX!GDBM!OPEN")=="1");
}

void NWNX_GdbmSync(string sDbName)
{
    SetLocalString(GetModule(), "NWNX!GDBM!SYNC", sDbName);
    return;
}

void NWNX_GdbmClose(string sDbName)
{
    SetLocalString(GetModule(), "NWNX!GDBM!CLOSE", sDbName);
    return;
}

void NWNX_GdbmCloseAll()
{
    SetLocalString(GetModule(), "NWNX!GDBM!CLOSEALL", "X");
    return;
}

//
// Data Manipulation - Basic
//
int NWNX_GdbmStore(string sDbName, string sKey, string sValue, int bReplace = TRUE, int bOpen = FALSE)
{
    SetLocalString(GetModule(), "NWNX!GDBM!STORE", sDbName + NWNX_GDBM_TOKEN + (bReplace?"1":"0") + NWNX_GDBM_TOKEN + (bOpen?"1":"0") + NWNX_GDBM_TOKEN + sKey + NWNX_GDBM_TOKEN + sValue );
    return (GetLocalString(GetModule(), "NWNX!GDBM!STORE")=="1");
}

int NWNX_GdbmExists(string sDbName, string sKey, int bOpen = FALSE)
{
    SetLocalString(GetModule(), "NWNX!GDBM!EXISTS", sDbName + NWNX_GDBM_TOKEN + (bOpen?"1":"0") + NWNX_GDBM_TOKEN + sKey );
    return (GetLocalString(GetModule(),"NWNX!GDBM!EXISTS")=="1");
}

string NWNX_GdbmFetch(string sDbName, string sKey, int bOpen = FALSE)
{
    SetLocalString(GetModule(), "NWNX!GDBM!FETCH", sDbName + NWNX_GDBM_TOKEN + (bOpen?"1":"0") + NWNX_GDBM_TOKEN + sKey );
    return GetLocalString(GetModule(),"NWNX!GDBM!FETCH");
}

int NWNX_GdbmDelete(string sDbName, string sKey, int bOpen = FALSE)
{
    SetLocalString(GetModule(), "NWNX!GDBM!DELETE", sDbName + NWNX_GDBM_TOKEN + (bOpen?"1":"0") + NWNX_GDBM_TOKEN + sKey );
    return (GetLocalString(GetModule(),"NWNX!GDBM!DELETE")=="1");
}

//
// Data Manipulation - Typed Interface
//
int NWNX_GdbmStoreString(string sDbName, string sKey, string sValue, int bReplace = TRUE, int bOpen = TRUE)
{
    return(NWNX_GdbmStore(sDbName, sKey+NWNX_GDBM_KEYTOK+"S", sValue, bReplace));
}
int NWNX_GdbmStoreInt(string sDbName, string sKey, int iValue, int bReplace = TRUE, int bOpen = TRUE)
{
    return(NWNX_GdbmStore(sDbName, sKey+NWNX_GDBM_KEYTOK+"I", IntToString(iValue),bReplace));
}
int NWNX_GdbmStoreFloat(string sDbName, string sKey, float fValue, int bReplace = TRUE, int bOpen = TRUE)
{
    return(NWNX_GdbmStore(sDbName, sKey+NWNX_GDBM_KEYTOK+"F", FloatToString(fValue),bReplace));
}
int NWNX_GdbmStoreVector(string sDbName, string sKey, vector vValue, int bReplace = TRUE, int bOpen = TRUE)
{
    return(NWNX_GdbmStore(sDbName, sKey+NWNX_GDBM_KEYTOK+"V", NWNX_VectorToString(vValue),bReplace));
}
int NWNX_GdbmStoreLocation(string sDbName, string sKey, location mValue, int bReplace = TRUE, int bOpen = TRUE)
{
    return(NWNX_GdbmStore(sDbName, sKey+NWNX_GDBM_KEYTOK+"L", NWNX_LocationToString(mValue),bReplace));
}
int NWNX_GdbmStoreObject(string sDbName, string sKey, object oValue, int bReplace = TRUE, int bOpen = TRUE)
{
    SetLocalString(GetModule(), "NWNX!GDBM!SETOBJARG", sDbName + NWNX_GDBM_TOKEN + (bReplace?"1":"0") + NWNX_GDBM_TOKEN + (bOpen?"1":"0") + NWNX_GDBM_TOKEN + (sKey+NWNX_GDBM_KEYTOK+"O"));
	StoreCampaignObject("NWNX", "GDBM", oValue);
	return 0;
}

int NWNX_GdbmExists_String(string sDbName, string sKey, int bOpen = TRUE)
{
    return(NWNX_GdbmExists(sDbName, sKey+NWNX_GDBM_KEYTOK+"S", bOpen));
}
int NWNX_GdbmExists_Int(string sDbName, string sKey, int bOpen = TRUE)
{
    return(NWNX_GdbmExists(sDbName, sKey+NWNX_GDBM_KEYTOK+"I", bOpen));
}
int NWNX_GdbmExists_Float(string sDbName, string sKey, int bOpen = TRUE)
{
    return(NWNX_GdbmExists(sDbName, sKey+NWNX_GDBM_KEYTOK+"F", bOpen));
}
int NWNX_GdbmExists_Vector(string sDbName, string sKey, int bOpen = TRUE)
{
    return(NWNX_GdbmExists(sDbName, sKey+NWNX_GDBM_KEYTOK+"V", bOpen));
}
int NWNX_GdbmExists_Location(string sDbName, string sKey, int bOpen = TRUE)
{
    return(NWNX_GdbmExists(sDbName, sKey+NWNX_GDBM_KEYTOK+"L", bOpen));
}
int NWNX_GdbmExists_Object(string sDbName, string sKey, int bOpen = TRUE)
{
    return(NWNX_GdbmExists(sDbName, sKey+NWNX_GDBM_KEYTOK+"O", bOpen));
}

string NWNX_GdbmFetchString(string sDbName, string sKey, int bOpen = TRUE)
{
    return(NWNX_GdbmFetch(sDbName, sKey+NWNX_GDBM_KEYTOK+"S", bOpen));
}
int NWNX_GdbmFetchInt(string sDbName, string sKey, int bOpen = TRUE)
{
    return(StringToInt(NWNX_GdbmFetch(sDbName, sKey+NWNX_GDBM_KEYTOK+"I", bOpen)));
}
float NWNX_GdbmFetchFloat(string sDbName, string sKey, int bOpen = TRUE)
{
    return(StringToFloat(NWNX_GdbmFetch(sDbName, sKey+NWNX_GDBM_KEYTOK+"F", bOpen)));
}
vector NWNX_GdbmFetchVector(string sDbName, string sKey, int bOpen = TRUE)
{
    return(NWNX_StringToVector(NWNX_GdbmFetch(sDbName, sKey+NWNX_GDBM_KEYTOK+"V", bOpen)));
}
location NWNX_GdbmFetchLocation(string sDbName, string sKey, int bOpen = TRUE)
{
    return(NWNX_StringToLocation(NWNX_GdbmFetch(sDbName, sKey+NWNX_GDBM_KEYTOK+"L", bOpen)));
}
object NWNX_GdbmFetchObject(string sDbName, string sKey, location mDestination, object oDestination, int bOpen = TRUE)
{
    SetLocalString(GetModule(), "NWNX!GDBM!SETOBJARG", sDbName + NWNX_GDBM_TOKEN + "0" + NWNX_GDBM_TOKEN + (bOpen?"1":"0") + NWNX_GDBM_TOKEN + (sKey+NWNX_GDBM_KEYTOK+"O"));
	return RetrieveCampaignObject("NWNX", "GDBM", mDestination, oDestination, OBJECT_INVALID);
}

int NWNX_GdbmDeleteString(string sDbName, string sKey, int bOpen = TRUE)
{
    return(NWNX_GdbmDelete(sDbName, sKey+NWNX_GDBM_KEYTOK+"S", bOpen));
}
int NWNX_GdbmDeleteInt(string sDbName, string sKey, int bOpen = TRUE)
{
    return(NWNX_GdbmDelete(sDbName, sKey+NWNX_GDBM_KEYTOK+"I", bOpen));
}
int NWNX_GdbmDeleteFloat(string sDbName, string sKey, int bOpen = TRUE)
{
    return(NWNX_GdbmDelete(sDbName, sKey+NWNX_GDBM_KEYTOK+"F", bOpen));
}
int NWNX_GdbmDeleteVector(string sDbName, string sKey, int bOpen = TRUE)
{
    return(NWNX_GdbmDelete(sDbName, sKey+NWNX_GDBM_KEYTOK+"V", bOpen));
}
int NWNX_GdbmDeleteLocation(string sDbName, string sKey, int bOpen = TRUE)
{
    return(NWNX_GdbmDelete(sDbName, sKey+NWNX_GDBM_KEYTOK+"L", bOpen));
}
int NWNX_GdbmDeleteObject(string sDbName, string sKey, int bOpen = TRUE)
{
    return(NWNX_GdbmDelete(sDbName, sKey+NWNX_GDBM_KEYTOK+"O", bOpen));
}

