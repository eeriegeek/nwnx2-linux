/******************************************************************************

  nwnx_gdbm - NWNX - GDBM Plugin - Binding to GNU hashed key-value file db.

  Copyright 2012 eeriegeek (eeriegeek@yahoo.com)

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

/******************************************************************************
  The first three GDBM functions here cover major database maintenance. For
  most uses, these will never be needed. Create's only additional function
  over open is the ablility to truncate a database file on open. Reorganize
  is only needed if a very large number of deletes are done, and the physical
  disk space is needed.
******************************************************************************/

// Creates a GDBM file. This is a wrapper around the underlying open method and
// calling create also opens the file for read/write access. Setting bTruncate
// to true destroys the current database file before opening the new one. This
// method adds nothing over open, except the ability to truncate the database.
// Returns TRUE if the GDBM file was created/opened.
//
int NWNX_GdbmCreate(string sDbName, int bTruncate=FALSE);

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


/******************************************************************************
  The following methods handle the normal database open-close use cycle.
******************************************************************************/

// Opens a GDBM file. By default, will open in read-write mode and create a
// new GDBM file if one does not exist. Unlike GdbmCreate, open is a safe
// operation and will not truncate the database. bReadOnly can only be TRUE
// if bCreate is FALSE. Returns TRUE if the GDBM file is successfully opened.
//
int NWNX_GdbmOpen(string sDbName, int bCreate=TRUE, int bReadOnly=FALSE);

// Syncronizes cached data to disk. Call if you are done writing for a while,
// but not ready to close the database yet. Close also syncs data.
//
void NWNX_GdbmSync(string sDbName);

// Closes the database file. Open will need to be called before further access.
//
void NWNX_GdbmClose(string sDbName);

// Closes all open GDBM database files. Call when done with all GDBM file use
// to sync reader/writer counts, and insure all data are written to disk.
// Provided here mainly for testing, NWNX will call this on server shutdown.
//
void NWNX_GdbmCloseAll();


/******************************************************************************
  Data manipulation functions. Insert, update, and delete key/value pairs.
  A GDBM database MUST be opened with GdbmCreate or GdbmOpen before use.
******************************************************************************/

// Stores a key/value pair to the GDBM database. If bReplace is set to FALSE,
// will refuse to overwrite a key currently in the database. Returns FALSE if
// an error occurs (such as storing to closed/non-existent database) or if
// bReplace is FALSE and the store operation would have overwritten a value.
//
int NWNX_GdbmStore(string sDbName, string sKey, string sValue, int bReplace=TRUE);

// Returns TRUE if the specified sKey has a value stored in the GDBM database.
//
int NWNX_GdbmExists(string sDbName, string sKey);

// Fetches the value stored for the specified sKey from the GDBM database.
// Returns an empty string "" if no matching key is found.
//
string NWNX_GdbmFetch(string sDbName, string sKey);

// Deletes the key/value pair specified by sKey from the GDBM database. Returns
// TRUE if the key was found and deleted.
//
int NWNX_GdbmDelete(string sDbName, string sKey);

// All data are passed into NWNX via string. The following are conversion
// routines for Vector, and Location data types. Int and Float can use the
// built-in conversion methods.

// Vectors are saved as ordered triplets. e.g. (1.0,1.0,1.0).

vector NWNX_VECTOR_INVALID = Vector(-9999999.0f,-9999999.0f,-9999999.0f);

string NWNX_VectorToString(vector v);
vector NWNX_StringToVector(string s);

// Locations are saved as ordered quintuplets. e.g. (Area001,1.0,1.0,1.0,90.0).

location NWNX_LOCATION_INVALID = Location(OBJECT_INVALID,Vector(-9999999.0f,-9999999.0f,-9999999.0f),0.0f);

string NWNX_LocationToString(location m);
location NWNX_StringToLocation(string s);


/******************************************************************************
  This is a set of "typed" interface methods for GDBM. Each method converts
  to/from a specific NWN data type for storage as a string in GDBM. The data
  are stored with a type specific key, so for example Int and Vector are not
  confused in the database and a String named "foo" is different than a float
  named "foo".
******************************************************************************/

int NWNX_GdbmStore_String(string sDbName, string sKey, string sValue, int bReplace=TRUE);
int NWNX_GdbmStore_Int(string sDbName, string sKey, int iValue, int bReplace=TRUE);
int NWNX_GdbmStore_Float(string sDbName, string sKey, float fValue, int bReplace=TRUE);
int NWNX_GdbmStore_Vector(string sDbName, string sKey, vector vValue, int bReplace=TRUE);
int NWNX_GdbmStore_Location(string sDbName, string sKey, location mValue, int bReplace=TRUE);

int NWNX_GdbmExists_String(string sDbName, string sKey);
int NWNX_GdbmExists_Int(string sDbName, string sKey);
int NWNX_GdbmExists_Float(string sDbName, string sKey);
int NWNX_GdbmExists_Vector(string sDbName, string sKey);
int NWNX_GdbmExists_Location(string sDbName, string sKey);

string NWNX_GdbmFetch_String(string sDbName, string sKey);
int NWNX_GdbmFetch_Int(string sDbName, string sKey);
float NWNX_GdbmFetch_Float(string sDbName, string sKey);
vector NWNX_GdbmFetch_Vector(string sDbName, string sKey);
location NWNX_GdbmFetch_Location(string sDbName, string sKey);

int NWNX_GdbmDelete_String(string sDbName, string sKey);
int NWNX_GdbmDelete_Int(string sDbName, string sKey);
int NWNX_GdbmDelete_Float(string sDbName, string sKey);
int NWNX_GdbmDelete_Vector(string sDbName, string sKey);
int NWNX_GdbmDelete_Location(string sDbName, string sKey);


/******************************************************************************
  Very simple set of functions to use GDBM. Ignores return values and assumes
  all default parameteters. Opens the database, so no initialization is needed.
  Use them just like the Getter/Setter methods for local variables, just
  replace "Local" in the name with "Gdbm" and specify a db filename instead
  of an object to hold the variables.
******************************************************************************/

void SetGdbmString(string sDbName, string sKey, string sValue);
void SetGdbmInt(string sDbName, string sKey, int iValue);
void SetGdbmFloat(string sDbName, string sKey, float fValue);
void SetGdbmVector(string sDbName, string sKey, vector vValue);
void SetGdbmLocation(string sDbName, string sKey, location mValue);

string GetGdbmString(string sDbName, string sKey);
int GetGdbmInt(string sDbName, string sKey);
float GetGdbmFloat(string sDbName, string sKey);
vector GetGdbmVector(string sDbName, string sKey);
location GetGdbmLocation(string sDbName, string sKey);

void DeleteGdbmString(string sDbName, string sKey);
void DeleteGdbmInt(string sDbName, string sKey);
void DeleteGdbmFloat(string sDbName, string sKey);
void DeleteGdbmVector(string sDbName, string sKey);
void DeleteGdbmLocation(string sDbName, string sKey);


/******************************************************************************
                       Function Implementation
******************************************************************************/

const string NWNX_GDBM_TOKEN  = "¬";
const string NWNX_GDBM_KEYTOK = "§";

string NWNX_BundleArguments( string sArg0,
    string sArg1="", string sArg2="", string sArg3="",
    string sArg4="", string sArg5="", string sArg6="",
    string sArg7="", string sArg8="", string sArg9="" )
{
    if (sArg0=="") return "";
    string s = IntToString(GetStringLength(sArg0))+"|"+sArg0+"|";

    if (sArg1=="") return s; s += IntToString(GetStringLength(sArg1))+"|"+sArg1+"|";
    if (sArg2=="") return s; s += IntToString(GetStringLength(sArg2))+"|"+sArg2+"|";
    if (sArg3=="") return s; s += IntToString(GetStringLength(sArg3))+"|"+sArg3+"|";
    if (sArg4=="") return s; s += IntToString(GetStringLength(sArg4))+"|"+sArg4+"|";
    if (sArg5=="") return s; s += IntToString(GetStringLength(sArg5))+"|"+sArg5+"|";
    if (sArg6=="") return s; s += IntToString(GetStringLength(sArg6))+"|"+sArg6+"|";
    if (sArg7=="") return s; s += IntToString(GetStringLength(sArg7))+"|"+sArg7+"|";
    if (sArg8=="") return s; s += IntToString(GetStringLength(sArg8))+"|"+sArg8+"|";
    if (sArg9=="") return s; s += IntToString(GetStringLength(sArg9))+"|"+sArg9+"|";

    return s;
}

int NWNX_GdbmCreate(string sDbName, int bTruncate=FALSE)
{
    SetLocalString(GetModule(),"NWNX!GDBM!CREATE",sDbName+NWNX_GDBM_TOKEN+(bTruncate?"1":"0"));
    return (GetLocalString(GetModule(),"NWNX!GDBM!CREATE")=="1");
}

void NWNX_GdbmReorganize(string sDbName)
{
    SetLocalString(GetModule(),"NWNX!GDBM!REORGANIZE",sDbName);
    return;
}

int NWNX_GdbmDestroy(string sDbName)
{
    SetLocalString(GetModule(),"NWNX!GDBM!DESTROY",sDbName);
    return (GetLocalString(GetModule(),"NWNX!GDBM!DESTROY")=="1");
}

int NWNX_GdbmOpen(string sDbName, int bCreate=TRUE, int bReadOnly=FALSE)
{
    SetLocalString(GetModule(),"NWNX!GDBM!OPEN",sDbName+NWNX_GDBM_TOKEN+(bCreate?"1":"0")+NWNX_GDBM_TOKEN+(bReadOnly?"1":"0"));
    return (GetLocalString(GetModule(),"NWNX!GDBM!OPEN")=="1");
}

void NWNX_GdbmSync(string sDbName)
{
    SetLocalString(GetModule(),"NWNX!GDBM!SYNC",sDbName);
    return;
}

void NWNX_GdbmClose(string sDbName)
{
    SetLocalString(GetModule(),"NWNX!GDBM!CLOSE",sDbName);
    return;
}

void NWNX_GdbmCloseAll()
{
    SetLocalString(GetModule(),"NWNX!GDBM!CLOSEALL","X");
    return;
}

int NWNX_GdbmStore(string sDbName, string sKey, string sValue, int bReplace=TRUE)
{
    SetLocalString(GetModule(),"NWNX!GDBM!STORE",NWNX_BundleArguments(sDbName,(bReplace?"1":"0"),sKey,sValue));
    return (GetLocalString(GetModule(),"NWNX!GDBM!STORE")=="1");
}

int NWNX_GdbmExists(string sDbName, string sKey)
{
    SetLocalString(GetModule(),"NWNX!GDBM!EXISTS",sDbName+NWNX_GDBM_TOKEN+sKey);
    return (GetLocalString(GetModule(),"NWNX!GDBM!EXISTS")=="1");
}

string NWNX_GdbmFetch(string sDbName, string sKey)
{
    SetLocalString(GetModule(),"NWNX!GDBM!FETCH",sDbName+NWNX_GDBM_TOKEN+sKey);
    return GetLocalString(GetModule(),"NWNX!GDBM!FETCH");
}

int NWNX_GdbmDelete(string sDbName, string sKey)
{
    SetLocalString(GetModule(),"NWNX!GDBM!DELETE",sDbName+NWNX_GDBM_TOKEN+sKey);
    return (GetLocalString(GetModule(),"NWNX!GDBM!DELETE")=="1");
}

// Bioware/NWN float keeps about 8-9 signif digits.
// Can return "inf" or "-inf" or underflow precision to 0.0

string NWNX_VectorToString(vector v) {
    return "("+FloatToString(v.x,0,9)+","+FloatToString(v.y,0,9)+","+FloatToString(v.z,0,9)+")";
}

vector NWNX_StringToVector(string s) {

    vector v = NWNX_VECTOR_INVALID;

    int p1 = FindSubString(s,"(",0);
    int c1 = FindSubString(s,",",p1+1);
    int c2 = FindSubString(s,",",c1+1);
    int p2 = FindSubString(s,")",c2+1);

    if ( (p1<0) || (c1<1) || (c2<1) || (p2<0) ) return v;

    string sX = GetSubString(s,p1+1,(c1-p1)-1);
    string sY = GetSubString(s,c1+1,(c2-c1)-1);
    string sZ = GetSubString(s,c2+1,(p2-c2)-1);

    if ( (sX=="") || (sY=="") || (sZ=="") ) return v;
    if ( (sX=="inf") || (sY=="inf") || (sZ=="inf") || (sX=="-inf") || (sY=="-inf") || (sZ=="-inf") ) return v;
    if ( (sX=="nan") || (sY=="nan") || (sZ=="nan") || (sX=="-nan") || (sY=="-nan") || (sZ=="-nan") ) return v;

    v = Vector(StringToFloat(sX),StringToFloat(sY),StringToFloat(sZ));

    return v;
}

string NWNX_LocationToString(location m) {
    vector v = GetPositionFromLocation(m);
    return ("("+GetTag(GetAreaFromLocation(m))+","+FloatToString(v.x,0,9)+","+FloatToString(v.y,0,9)+","+FloatToString(v.z,0,9)+","+FloatToString(GetFacingFromLocation(m),0,9)+")");
}

location NWNX_StringToLocation(string s) {

    location m = NWNX_LOCATION_INVALID;

    int p1 = FindSubString(s,"(",0);
    int c1 = FindSubString(s,",",p1+1);
    int c2 = FindSubString(s,",",c1+1);
    int c3 = FindSubString(s,",",c2+1);
    int c4 = FindSubString(s,",",c3+1);
    int p2 = FindSubString(s,")",c4+1);

    if ((p1<0)||(c1<1)||(c2<1)||(c3<1)||(c4<1)||(p2<0)) return m;

    string sA = GetSubString(s,p1+1,(c1-p1)-1);
    string sX = GetSubString(s,c1+1,(c2-c1)-1);
    string sY = GetSubString(s,c2+1,(c3-c2)-1);
    string sZ = GetSubString(s,c3+1,(c4-c3)-1);
    string sO = GetSubString(s,c4+1,(p2-c4)-1);

    if ( (sA=="")||(sX=="")||(sY=="")||(sZ=="")||(sO=="")) return m;
    if ((sX=="inf")||(sY=="inf")||(sZ=="inf")||(sO=="inf")||(sX=="-inf")||(sY=="-inf")||(sZ=="-inf")||(sO=="-inf")) return m;
    if ((sX=="nan")||(sY=="nan")||(sZ=="nan")||(sO=="nan")||(sX=="-nan")||(sY=="-nan")||(sZ=="-nan")||(sO=="-nan")) return m;

    object oA = GetObjectByTag(sA);

    if (oA==OBJECT_INVALID) return m;

    m = Location(oA,Vector(StringToFloat(sX),StringToFloat(sY),StringToFloat(sZ)),StringToFloat(sO));

    return m;
}

int NWNX_GdbmStore_String(string sDbName, string sKey, string sValue, int bReplace=TRUE)
{
    // NOTE: no type identifier added for string therefore this method's
    // storage form is compatible with the NWNX_GdbmStore method it uses.
    return(NWNX_GdbmStore(sDbName,sKey,sValue,bReplace));
}
int NWNX_GdbmStore_Int(string sDbName, string sKey, int iValue, int bReplace=TRUE)
{
    return(NWNX_GdbmStore(sDbName,sKey+NWNX_GDBM_KEYTOK+"I",IntToString(iValue),bReplace));
}
int NWNX_GdbmStore_Float(string sDbName, string sKey, float fValue, int bReplace=TRUE)
{
    return(NWNX_GdbmStore(sDbName,sKey+NWNX_GDBM_KEYTOK+"F",FloatToString(fValue),bReplace));
}
int NWNX_GdbmStore_Vector(string sDbName, string sKey, vector vValue, int bReplace=TRUE)
{
    return(NWNX_GdbmStore(sDbName,sKey+NWNX_GDBM_KEYTOK+"V",NWNX_VectorToString(vValue),bReplace));
}
int NWNX_GdbmStore_Location(string sDbName, string sKey, location mValue, int bReplace=TRUE)
{
    return(NWNX_GdbmStore(sDbName,sKey+NWNX_GDBM_KEYTOK+"L",NWNX_LocationToString(mValue),bReplace));
}

int NWNX_GdbmExists_String(string sDbName, string sKey)
{
    return(NWNX_GdbmExists(sDbName,sKey));
}
int NWNX_GdbmExists_Int(string sDbName, string sKey)
{
    return(NWNX_GdbmExists(sDbName,sKey+NWNX_GDBM_KEYTOK+"I"));
}
int NWNX_GdbmExists_Float(string sDbName, string sKey)
{
    return(NWNX_GdbmExists(sDbName,sKey+NWNX_GDBM_KEYTOK+"F"));
}
int NWNX_GdbmExists_Vector(string sDbName, string sKey)
{
    return(NWNX_GdbmExists(sDbName,sKey+NWNX_GDBM_KEYTOK+"V"));
}
int NWNX_GdbmExists_Location(string sDbName, string sKey)
{
    return(NWNX_GdbmExists(sDbName,sKey+NWNX_GDBM_KEYTOK+"L"));
}

string NWNX_GdbmFetch_String(string sDbName, string sKey)
{
    return(NWNX_GdbmFetch(sDbName,sKey));
}
int NWNX_GdbmFetch_Int(string sDbName, string sKey)
{
    return(StringToInt(NWNX_GdbmFetch(sDbName,sKey+NWNX_GDBM_KEYTOK+"I")));
}
float NWNX_GdbmFetch_Float(string sDbName, string sKey)
{
    return(StringToFloat(NWNX_GdbmFetch(sDbName,sKey+NWNX_GDBM_KEYTOK+"F")));
}
vector NWNX_GdbmFetch_Vector(string sDbName, string sKey)
{
    return(NWNX_StringToVector(NWNX_GdbmFetch(sDbName,sKey+NWNX_GDBM_KEYTOK+"V")));
}
location NWNX_GdbmFetch_Location(string sDbName, string sKey)
{
    return(NWNX_StringToLocation(NWNX_GdbmFetch(sDbName,sKey+NWNX_GDBM_KEYTOK+"L")));
}

int NWNX_GdbmDelete_String(string sDbName, string sKey)
{
    return(NWNX_GdbmDelete(sDbName,sKey));
}
int NWNX_GdbmDelete_Int(string sDbName, string sKey)
{
    return(NWNX_GdbmDelete(sDbName,sKey+NWNX_GDBM_KEYTOK+"I"));
}
int NWNX_GdbmDelete_Float(string sDbName, string sKey)
{
    return(NWNX_GdbmDelete(sDbName,sKey+NWNX_GDBM_KEYTOK+"F"));
}
int NWNX_GdbmDelete_Vector(string sDbName, string sKey)
{
    return(NWNX_GdbmDelete(sDbName,sKey+NWNX_GDBM_KEYTOK+"V"));
}
int NWNX_GdbmDelete_Location(string sDbName, string sKey)
{
    return(NWNX_GdbmDelete(sDbName,sKey+NWNX_GDBM_KEYTOK+"L"));
}

void SetGdbmString(string sDbName, string sKey, string sValue)
{
    NWNX_GdbmOpen(sDbName);
    NWNX_GdbmStore_String(sDbName,sKey,sValue);
}
void SetGdbmInt(string sDbName, string sKey, int iValue)
{
    NWNX_GdbmOpen(sDbName);
    NWNX_GdbmStore_Int(sDbName,sKey,iValue);
}
void SetGdbmFloat(string sDbName, string sKey, float fValue)
{
    NWNX_GdbmOpen(sDbName);
    NWNX_GdbmStore_Float(sDbName,sKey,fValue);
}
void SetGdbmVector(string sDbName, string sKey, vector vValue)
{
    NWNX_GdbmOpen(sDbName);
    NWNX_GdbmStore_Vector(sDbName,sKey,vValue);
}
void SetGdbmLocation(string sDbName, string sKey, location mValue)
{
    NWNX_GdbmOpen(sDbName);
    NWNX_GdbmStore_Location(sDbName,sKey,mValue);
}

string GetGdbmString(string sDbName, string sKey)
{
    NWNX_GdbmOpen(sDbName);
    return(NWNX_GdbmFetch_String(sDbName,sKey));
}
int GetGdbmInt(string sDbName, string sKey)
{
    NWNX_GdbmOpen(sDbName);
    return(NWNX_GdbmFetch_Int(sDbName,sKey));
}
float GetGdbmFloat(string sDbName, string sKey)
{
    NWNX_GdbmOpen(sDbName);
    return(NWNX_GdbmFetch_Float(sDbName,sKey));
}
vector GetGdbmVector(string sDbName, string sKey)
{
    NWNX_GdbmOpen(sDbName);
    return(NWNX_GdbmFetch_Vector(sDbName,sKey));
}
location GetGdbmLocation(string sDbName, string sKey)
{
    NWNX_GdbmOpen(sDbName);
    return(NWNX_GdbmFetch_Location(sDbName,sKey));
}

void DeleteGdbmString(string sDbName, string sKey)
{
    NWNX_GdbmOpen(sDbName);
    NWNX_GdbmDelete_String(sDbName,sKey);
}
void DeleteGdbmInt(string sDbName, string sKey)
{
    NWNX_GdbmOpen(sDbName);
    NWNX_GdbmDelete_Int(sDbName,sKey);
}
void DeleteGdbmFloat(string sDbName, string sKey)
{
    NWNX_GdbmOpen(sDbName);
    NWNX_GdbmDelete_Float(sDbName,sKey);
}
void DeleteGdbmVector(string sDbName, string sKey)
{
    NWNX_GdbmOpen(sDbName);
    NWNX_GdbmDelete_Vector(sDbName,sKey);
}
void DeleteGdbmLocation(string sDbName, string sKey)
{
    NWNX_GdbmOpen(sDbName);
    NWNX_GdbmDelete_Location(sDbName,sKey);
}

