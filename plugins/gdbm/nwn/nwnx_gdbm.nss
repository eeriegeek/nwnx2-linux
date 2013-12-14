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
int NWNX_GDBM_Create(string sDbName, int bTruncate = FALSE);

// Reorganizes the contents and hash indexes in the GDBM file and shrinks the
// physical disk file if there is free space. This method is slow and should
// very seldom (if ever) need to be called. GDBM reuses deleted space so this
// method only needs to be called if a large number of deletions occurs that
// are not expected to be consumed soon by new entries. In a normal, growing
// database, this should only happen after major system maintenance.
//
void NWNX_GDBM_Reorganize(string sDbName);

// Delete the GDBM database file. Permanently. You have a backup, right?
// This is not really a GDBM function but a wrapper to file deletion.
//
int NWNX_GDBM_Destroy(string sDbName);

//-----------------------------------------------------------------------------
// The following methods handle the normal database open-close use cycle. If
// using the DML calls with the open option, these methods may not be needed.
//-----------------------------------------------------------------------------

// Opens a GDBM file. By default, will open in read-write mode and create a
// new GDBM file if one does not exist. Unlike GdbmCreate, open is a safe
// operation and will not truncate the database. bReadOnly can only be TRUE
// if bCreate is FALSE. Returns TRUE if the GDBM file is successfully opened.
//
int NWNX_GDBM_Open(string sDbName, int bCreate = TRUE, int bReadOnly = FALSE);

// Syncronizes cached data to disk. Call if you are done writing for a while,
// but not ready to close the database yet. Close also syncs data.
//
void NWNX_GDBM_Sync(string sDbName);

// Closes the database file. Open will need to be called before further access.
//
void NWNX_GDBM_Close(string sDbName);

// Closes all open GDBM database files. Call when done with all GDBM file use
// to sync all reader/writer counts and insure all data are written to disk.
// Provided here mainly for testing, NWNX will call this on server shutdown.
//
void NWNX_GDBM_CloseAll();

//-----------------------------------------------------------------------------
// Primary Data Manipulation (DML) type functions. Insert, update, and delete
// string key/value pairs. A GDBM database MUST be opened with GdbmCreate or
// GdbmOpen before use, however, setting the bOpen flag to TRUE does this
// automatically. These four primary routines are NOT type safe and store
// only string values. For typed storage use the typed interface below.
//-----------------------------------------------------------------------------

// Stores a key/value pair to the GDBM database. If bReplace is set to FALSE,
// will refuse to overwrite a key currently in the database. Returns FALSE if
// an error occurs (such as storing to closed/non-existent database) or if
// bReplace is FALSE and the store operation would have overwritten a value.
//
int NWNX_GDBM_Store(string sDbName, string sKey, string sValue, int bReplace = TRUE, int bOpen = TRUE);

// Returns TRUE if the specified sKey has a value stored in the GDBM database.
//
int NWNX_GDBM_Exists(string sDbName, string sKey, int bOpen = TRUE);

// Fetches the value stored for the specified sKey from the GDBM database.
// Returns an empty string "" if no matching key is found.
//
string NWNX_GDBM_Fetch(string sDbName, string sKey, int bOpen = TRUE);

// Deletes the key/value pair specified by sKey from the GDBM database. Returns
// TRUE if the key was found and deleted.
//
int NWNX_GDBM_Delete(string sDbName, string sKey, int bOpen = TRUE);

//-----------------------------------------------------------------------------
// This set of typed methods constitute the main API for client applications.
// Each method converts to/from a specific NWN data type for storage as a
// string in GDBM. The data are stored with a type specific key, so e.g. Int
// and Vector are not confused in the database and a String named "foo" is
// different than a float named "foo". 
//-----------------------------------------------------------------------------

int NWNX_GDBM_StoreString(string sDbName, string sKey, string sValue, int bReplace = TRUE, int bOpen = TRUE);
int NWNX_GDBM_StoreInt(string sDbName, string sKey, int iValue, int bReplace = TRUE, int bOpen = TRUE);
int NWNX_GDBM_StoreFloat(string sDbName, string sKey, float fValue, int bReplace = TRUE, int bOpen = TRUE);
int NWNX_GDBM_StoreVector(string sDbName, string sKey, vector vValue, int bReplace = TRUE, int bOpen = TRUE);
int NWNX_GDBM_StoreLocation(string sDbName, string sKey, location mValue, int bReplace = TRUE, int bOpen = TRUE);
int NWNX_GDBM_StoreObject(string sDbName, string sKey, object oValue, int bReplace = TRUE, int bOpen = TRUE);

int NWNX_GDBM_ExistsString(string sDbName, string sKey, int bOpen = TRUE);
int NWNX_GDBM_ExistsInt(string sDbName, string sKey, int bOpen = TRUE);
int NWNX_GDBM_ExistsFloat(string sDbName, string sKey, int bOpen = TRUE);
int NWNX_GDBM_ExistsVector(string sDbName, string sKey, int bOpen = TRUE);
int NWNX_GDBM_ExistsLocation(string sDbName, string sKey, int bOpen = TRUE);
int NWNX_GDBM_ExistsObject(string sDbName, string sKey, int bOpen = TRUE);

string   NWNX_GDBM_FetchString(string sDbName, string sKey, int bOpen = TRUE);
int      NWNX_GDBM_FetchInt(string sDbName, string sKey, int bOpen = TRUE);
float    NWNX_GDBM_FetchFloat(string sDbName, string sKey, int bOpen = TRUE);
vector   NWNX_GDBM_FetchVector(string sDbName, string sKey, int bOpen = TRUE);
location NWNX_GDBM_FetchLocation(string sDbName, string sKey, int bOpen = TRUE);
object   NWNX_GDBM_FetchObject(string sDbName, string sKey, location mDestination, object oDestination, int bOpen = TRUE);

int NWNX_GDBM_DeleteString(string sDbName, string sKey, int bOpen = TRUE);
int NWNX_GDBM_DeleteInt(string sDbName, string sKey, int bOpen = TRUE);
int NWNX_GDBM_DeleteFloat(string sDbName, string sKey, int bOpen = TRUE);
int NWNX_GDBM_DeleteVector(string sDbName, string sKey, int bOpen = TRUE);
int NWNX_GDBM_DeleteLocation(string sDbName, string sKey, int bOpen = TRUE);
int NWNX_GDBM_DeleteObject(string sDbName, string sKey, int bOpen = TRUE);

//----------------------------------------------------------------------------
// Implementation 
//-----------------------------------------------------------------------------

const string NWNX_GDBM_ARGSEP = "";
const string NWNX_GDBM_KEYSEP = "";

//
//  Maintenance & Lifecycle
//
int NWNX_GDBM_Create(string sDbName, int bTruncate = FALSE)
{
    SetLocalString(GetModule(),"NWNX!GDBM!CREATE", sDbName + NWNX_GDBM_ARGSEP + (bTruncate?"1":"0"));
    return (GetLocalString(GetModule(),"NWNX!GDBM!CREATE")=="1");
}

void NWNX_GDBM_Reorganize(string sDbName)
{
    SetLocalString(GetModule(), "NWNX!GDBM!REORGANIZE", sDbName);
    return;
}

int NWNX_GDBM_Destroy(string sDbName)
{
    SetLocalString(GetModule(), "NWNX!GDBM!DESTROY", sDbName);
    return (GetLocalString(GetModule(),"NWNX!GDBM!DESTROY")=="1");
}

int NWNX_GDBM_Open(string sDbName, int bCreate=TRUE, int bReadOnly=FALSE)
{
    SetLocalString(GetModule(), "NWNX!GDBM!OPEN", sDbName + NWNX_GDBM_ARGSEP + (bCreate?"1":"0") + NWNX_GDBM_ARGSEP + (bReadOnly?"1":"0"));
    return (GetLocalString(GetModule(),"NWNX!GDBM!OPEN")=="1");
}

void NWNX_GDBM_Sync(string sDbName)
{
    SetLocalString(GetModule(), "NWNX!GDBM!SYNC", sDbName);
    return;
}

void NWNX_GDBM_Close(string sDbName)
{
    SetLocalString(GetModule(), "NWNX!GDBM!CLOSE", sDbName);
    return;
}

void NWNX_GDBM_CloseAll()
{
    SetLocalString(GetModule(), "NWNX!GDBM!CLOSEALL", "");
    return;
}

//
// Data Manipulation - Basic
//
int NWNX_GDBM_Store(string sDbName, string sKey, string sValue, int bReplace = TRUE, int bOpen = FALSE)
{
    SetLocalString(GetModule(), "NWNX!GDBM!STORE", sDbName + NWNX_GDBM_ARGSEP + (bReplace?"1":"0") + NWNX_GDBM_ARGSEP + (bOpen?"1":"0") + NWNX_GDBM_ARGSEP + sKey + NWNX_GDBM_ARGSEP + sValue );
    return (GetLocalString(GetModule(), "NWNX!GDBM!STORE")=="1");
}

int NWNX_GDBM_Exists(string sDbName, string sKey, int bOpen = FALSE)
{
    SetLocalString(GetModule(), "NWNX!GDBM!EXISTS", sDbName + NWNX_GDBM_ARGSEP + (bOpen?"1":"0") + NWNX_GDBM_ARGSEP + sKey );
    return (GetLocalString(GetModule(),"NWNX!GDBM!EXISTS")=="1");
}

string NWNX_GDBM_Fetch(string sDbName, string sKey, int bOpen = FALSE)
{
    SetLocalString(GetModule(), "NWNX!GDBM!FETCH", sDbName + NWNX_GDBM_ARGSEP + (bOpen?"1":"0") + NWNX_GDBM_ARGSEP + sKey );
    return GetLocalString(GetModule(),"NWNX!GDBM!FETCH");
}

int NWNX_GDBM_Delete(string sDbName, string sKey, int bOpen = FALSE)
{
    SetLocalString(GetModule(), "NWNX!GDBM!DELETE", sDbName + NWNX_GDBM_ARGSEP + (bOpen?"1":"0") + NWNX_GDBM_ARGSEP + sKey );
    return (GetLocalString(GetModule(),"NWNX!GDBM!DELETE")=="1");
}

//
// Data Manipulation - Typed Interface
//
int NWNX_GDBM_StoreString(string sDbName, string sKey, string sValue, int bReplace = TRUE, int bOpen = TRUE)
{
    return(NWNX_GDBM_Store(sDbName, sKey+NWNX_GDBM_KEYSEP+"S", sValue, bReplace));
}
int NWNX_GDBM_StoreInt(string sDbName, string sKey, int iValue, int bReplace = TRUE, int bOpen = TRUE)
{
    return(NWNX_GDBM_Store(sDbName, sKey+NWNX_GDBM_KEYSEP+"I", IntToString(iValue),bReplace));
}
int NWNX_GDBM_StoreFloat(string sDbName, string sKey, float fValue, int bReplace = TRUE, int bOpen = TRUE)
{
    return(NWNX_GDBM_Store(sDbName, sKey+NWNX_GDBM_KEYSEP+"F", FloatToString(fValue),bReplace));
}
int NWNX_GDBM_StoreVector(string sDbName, string sKey, vector vValue, int bReplace = TRUE, int bOpen = TRUE)
{
    return(NWNX_GDBM_Store(sDbName, sKey+NWNX_GDBM_KEYSEP+"V", NWNX_VectorToString(vValue),bReplace));
}
int NWNX_GDBM_StoreLocation(string sDbName, string sKey, location mValue, int bReplace = TRUE, int bOpen = TRUE)
{
    return(NWNX_GDBM_Store(sDbName, sKey+NWNX_GDBM_KEYSEP+"L", NWNX_LocationToString(mValue),bReplace));
}
int NWNX_GDBM_StoreObject(string sDbName, string sKey, object oValue, int bReplace = TRUE, int bOpen = TRUE)
{
    int retVal = 0;
    SetLocalString(
        GetModule(),
        "NWNX!GDBM!SETOBJARG",
        sDbName + NWNX_GDBM_ARGSEP + (bReplace?"1":"0") + NWNX_GDBM_ARGSEP + (bOpen?"1":"0") + NWNX_GDBM_ARGSEP + (sKey+NWNX_GDBM_KEYSEP+"O")
    );
    switch (GetObjectType(oValue)) {
        case OBJECT_TYPE_ITEM:
        case OBJECT_TYPE_CREATURE:
	        retVal = StoreCampaignObject("NWNX", "GDBM", oValue);
            break;
        case OBJECT_TYPE_PLACEABLE:
        case OBJECT_TYPE_STORE:
        case OBJECT_TYPE_TRIGGER:
            // TODO: Under construction, may need patch to ODMBC
            //StoreCampaignObject("NWNX","F",oObject);
            //SetLocalString(GetModule(),"NWNX!DB!OBJ_FREEZE",ObjectToString(oObject));
            //SetLocalString(GetModule(), "NWNX!ODBC!STOREOBJECT", ObjectToString(oObject));

            //SetLocalString(GetModule(), "NWNX!ODBC!SAVEOBJECT", "GDBM¬" + ObjectToString(oObject));
            break;
    }
    return retVal; 
}

int NWNX_GDBM_ExistsString(string sDbName, string sKey, int bOpen = TRUE)
{
    return(NWNX_GDBM_Exists(sDbName, sKey+NWNX_GDBM_KEYSEP+"S", bOpen));
}
int NWNX_GDBM_ExistsInt(string sDbName, string sKey, int bOpen = TRUE)
{
    return(NWNX_GDBM_Exists(sDbName, sKey+NWNX_GDBM_KEYSEP+"I", bOpen));
}
int NWNX_GDBM_ExistsFloat(string sDbName, string sKey, int bOpen = TRUE)
{
    return(NWNX_GDBM_Exists(sDbName, sKey+NWNX_GDBM_KEYSEP+"F", bOpen));
}
int NWNX_GDBM_ExistsVector(string sDbName, string sKey, int bOpen = TRUE)
{
    return(NWNX_GDBM_Exists(sDbName, sKey+NWNX_GDBM_KEYSEP+"V", bOpen));
}
int NWNX_GDBM_ExistsLocation(string sDbName, string sKey, int bOpen = TRUE)
{
    return(NWNX_GDBM_Exists(sDbName, sKey+NWNX_GDBM_KEYSEP+"L", bOpen));
}
int NWNX_GDBM_ExistsObject(string sDbName, string sKey, int bOpen = TRUE)
{
    return(NWNX_GDBM_Exists(sDbName, sKey+NWNX_GDBM_KEYSEP+"O", bOpen));
}

string NWNX_GDBM_FetchString(string sDbName, string sKey, int bOpen = TRUE)
{
    return(NWNX_GDBM_Fetch(sDbName, sKey+NWNX_GDBM_KEYSEP+"S", bOpen));
}
int NWNX_GDBM_FetchInt(string sDbName, string sKey, int bOpen = TRUE)
{
    return(StringToInt(NWNX_GDBM_Fetch(sDbName, sKey+NWNX_GDBM_KEYSEP+"I", bOpen)));
}
float NWNX_GDBM_FetchFloat(string sDbName, string sKey, int bOpen = TRUE)
{
    return(StringToFloat(NWNX_GDBM_Fetch(sDbName, sKey+NWNX_GDBM_KEYSEP+"F", bOpen)));
}
vector NWNX_GDBM_FetchVector(string sDbName, string sKey, int bOpen = TRUE)
{
    return(NWNX_StringToVector(NWNX_GDBM_Fetch(sDbName, sKey+NWNX_GDBM_KEYSEP+"V", bOpen)));
}
location NWNX_GDBM_FetchLocation(string sDbName, string sKey, int bOpen = TRUE)
{
    return(NWNX_StringToLocation(NWNX_GDBM_Fetch(sDbName, sKey+NWNX_GDBM_KEYSEP+"L", bOpen)));
}
object NWNX_GDBM_FetchObject(string sDbName, string sKey, location mDestination, object oDestination, int bOpen = TRUE)
{
    object oReturn = OBJECT_INVALID;
    SetLocalString(GetModule(), "NWNX!GDBM!SETOBJARG", sDbName + NWNX_GDBM_ARGSEP + "0" + NWNX_GDBM_ARGSEP + (bOpen?"1":"0") + NWNX_GDBM_ARGSEP + (sKey+NWNX_GDBM_KEYSEP+"O"));
    int nObjectType = OBJECT_TYPE_ITEM;
    switch (nObjectType) {
        case OBJECT_TYPE_ITEM:
        case OBJECT_TYPE_CREATURE:
	        oReturn = RetrieveCampaignObject("NWNX", "GDBM", mDestination, oDestination, OBJECT_INVALID);
            break;
        case OBJECT_TYPE_PLACEABLE:
        case OBJECT_TYPE_STORE:
        case OBJECT_TYPE_TRIGGER:
            // TODO: Under construction
            //SetLocalString(GetModule(), "NWNX!ODBC!WRITEOBJECT", ObjectToString(oObject));
            //SetLocalString(GetModule(), "NWNX!ODBC!RETRIEVEOBJECT", ObjectToString(GetAreaFromLocation(lLocation))+"¬"+FloatToString(vLocation.x)+"¬"+FloatToString(vLocation.y)+"¬"+FloatToString(vLocation.z)+"¬"+FloatToString(GetFacingFromLocation(lLocation)));
            //oReturn = GetLocalObject(GetModule(), "NWNX!ODBC!RETRIEVEOBJECT");

            //SetLocalString(GetModule(), "NWNX!ODBC!LOADOBJECT", ObjectToString(GetAreaFromLocation(mDestination))+"¬"+FloatToString(vLocation.x)+"¬"+FloatToString(vLocation.y)+"¬"+FloatToString(vLocation.z)+"¬"+FloatToString(GetFacingFromLocation(lLocation)));
            //oReturn = GetLocalObject(GetModule(), "NWNX!ODBC!LOADOBJECT", "GDBM¬" + ObjectToString(oObject));
            break;
    }

    return oReturn;
}

int NWNX_GDBM_DeleteString(string sDbName, string sKey, int bOpen = TRUE)
{
    return(NWNX_GDBM_Delete(sDbName, sKey+NWNX_GDBM_KEYSEP+"S", bOpen));
}
int NWNX_GDBM_DeleteInt(string sDbName, string sKey, int bOpen = TRUE)
{
    return(NWNX_GDBM_Delete(sDbName, sKey+NWNX_GDBM_KEYSEP+"I", bOpen));
}
int NWNX_GDBM_DeleteFloat(string sDbName, string sKey, int bOpen = TRUE)
{
    return(NWNX_GDBM_Delete(sDbName, sKey+NWNX_GDBM_KEYSEP+"F", bOpen));
}
int NWNX_GDBM_DeleteVector(string sDbName, string sKey, int bOpen = TRUE)
{
    return(NWNX_GDBM_Delete(sDbName, sKey+NWNX_GDBM_KEYSEP+"V", bOpen));
}
int NWNX_GDBM_DeleteLocation(string sDbName, string sKey, int bOpen = TRUE)
{
    return(NWNX_GDBM_Delete(sDbName, sKey+NWNX_GDBM_KEYSEP+"L", bOpen));
}
int NWNX_GDBM_DeleteObject(string sDbName, string sKey, int bOpen = TRUE)
{
    return(NWNX_GDBM_Delete(sDbName, sKey+NWNX_GDBM_KEYSEP+"O", bOpen));
}

