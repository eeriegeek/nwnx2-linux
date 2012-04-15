/******************************************************************************

  nwnx_db - NWNX - DB Plugin - Database Interface

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

#include "nwnx_aggregate"
#include "nwnx_serializer"

// Initialization of the database. Call once at module startup (OnModuleLoad).
//
void NWNX_DB_Initialize();

// Returns a short string constant identifying the underlying database. This
// is intended allow alternate coding in cases where syntax differs between
// databases implementations. Currently can return MYSQL, PGSQL, and SQLITE.
//
string NWNX_DB_GetDbId();

// Calls the database's native SQL string escape method. Allows the database
// to handle implemention specific quoting needs and proper alternate language
// character set handling. Only needed for DATA embedded in a SQL query.
//
string NWNX_DB_Escape(string sString);

// Basic database query execution. Any valid query may be executed with this
// method. If the statement returns no rows, no further action is required.
// Returns TRUE (1) on success, or FALSE (0) on error. Some databases allow
// multiple statements separated by semicolon as a single query. The results
// returned by multiple queries is database dependent.
//
// MYSQL - returns ALL result sets from multiple queries. The sets can
// (must) be retrieved by looping with NWNX_DB_Fetch() until end-of-data once
// for each data set to be returned. This implies that the developer must
// know how many sets to retrieve.
//
// PGSQL - returns only results from the LAST query in the sequence to return
// a result set. Other results are silently flushed.
//
// SQLITE - does not support multi-query syntax.
//
int NWNX_DB_Execute(string sQuery);

// Retrieve the result of a database query. Gets the next available row of
// data returned from a query into local storage where it can be retrieved
// by the GetField methods. Returns FALSE on end-of-data or error, returns
// TRUE if a row was retrieved. The GetField methods can be used to get
// the value of returned columns.
//
int NWNX_DB_Fetch();

// Check if a returned field is NULL.
//
int NWNX_DB_GetField_IsNull(int nFieldNumber);

// Get the value of a field for a returned row of data. Fields are stored
// internally as a string (except objects) and can be retrieved via the
// typed methods or converted by the application.
//
string NWNX_DB_GetField_String(int nFieldNumber);
int NWNX_DB_GetField_Int(int nFieldNumber);
float NWNX_DB_GetField_Float(int nFieldNumber);
vector NWNX_DB_GetField_Vector(int nFieldNumber);
location NWNX_DB_GetField_Location(int nFieldNumber);
object NWNX_DB_GetField_ObjectToLocation(int nFieldNumber, location mLocation);
object NWNX_DB_GetField_ObjectToContainer(int nFieldNumber, object oContainer);

// Set up a prepared query for execution with parameters. The format for
// parameterized queries varies by database.
//
// MYSQL  - Uses ?  (e.g. select * from foo where id=?)
// PGSQL  - Uses $n (e.g. select * from foo where id=$1)
// SQLITE - (not yet supported)
//
int NWNX_DB_Prepare(string sQueryTemplate);

// Set up the value to pass into a query parameter. Typing is somewhat flexible
// The aggregate types are for convenience and are treated as strings internally.
//
void NWNX_DB_SetParam_String(int nNumber, string sString, int bNull=0);
void NWNX_DB_SetParam_Int(int nNumber, int iInt, int bNull=0);
void NWNX_DB_SetParam_Float(int nNumber, float fFloat, int bNull=0);
void NWNX_DB_SetParam_Vector(int nNumber, vector vVector, int bNull=0);
void NWNX_DB_SetParam_Location(int nNumber, location mLocation, int bNull=0);
void NWNX_DB_SetParam_Object(int nNumber, object oObject, int bNull=0);

// Execute a prepared query, using the previously specified parameters.
//
int NWNX_DB_ExecutePrepared();

// Retrieve the result of a database query. Gets the next available row of
// data returned from a query into local storage where it can be retrieved
// by the GetField methods. Returns FALSE on end-of-data or error, returns
// TRUE if a row was retrieved. The GetField methods can be used to get
// the value of returned columns.
//
int NWNX_DB_FetchPrepared();

//
// Implementation
//

void NWNX_DB_Initialize()
{
    if (GetLocalInt(GetModule(),"NWNX_DB_IsInitialized")) return;

    SetLocalString(GetModule(),"NWNX!DB!GETDBID","XXXXXX");
    string NWNX_DB_DatabaseId = GetLocalString(GetModule(),"NWNX!DB!GETDBID");

    SetLocalString(GetModule(),"NWNX_DB_DatabaseId",NWNX_DB_DatabaseId);

    SetLocalInt(GetModule(),"NWNX_DB_IsInitialized",1);
}

string NWNX_DB_GetDbId()
{
    return(GetLocalString(GetModule(),"NWNX_DB_DatabaseId"));
}

string NWNX_DB_Escape(string sString)
{
    SetLocalString(GetModule(),"NWNX!DB!ESCAPE",sString);
    return(GetLocalString(GetModule(),"NWNX!DB!ESCAPE"));
}

// Unpack a bundled (serialized) set of row data passed from database.
//
int NWNX_DB_UnBundle(string sRow)
{
    object oMod = GetModule();
    int i = 1;
    int p = 1;
    int q = 0;

    while (GetSubString(sRow,p,1)!="|") {

        q = FindSubString(sRow,"|",p);
        string sType = GetSubString(sRow,p,q-p);

        p = q+1;
        q = FindSubString(sRow,"|",p);
        string sNull = GetSubString(sRow,p,q-p);

        p = q+1;
        q = FindSubString(sRow,"|",p);
        string sSize = GetSubString(sRow,p,q-p);

        p = q+1;
        q = p+StringToInt(sSize);
        string sData = GetSubString(sRow,p,q-p);

        SetLocalString(oMod,"_DB_R_T_"+IntToString(i),sType);
        SetLocalInt   (oMod,"_DB_R_N_"+IntToString(i),StringToInt(sNull));
        SetLocalString(oMod,"_DB_R_D_"+IntToString(i),sData);
        i = i + 1;

        p = q+1;

    }

    return 0;
}

int NWNX_DB_Execute(string sQuery)
{
    SetLocalString(GetModule(),"NWNX!DB!EXECUTE",sQuery);
    string sRet = GetLocalString(GetModule(),"NWNX!DB!EXECUTE");
    return (StringToInt(sRet));
}

int NWNX_DB_Fetch()
{
    object oModule = GetModule();
    SetLocalString(oModule,"NWNX!DB!FETCH","XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
    string sRow = GetLocalString(oModule,"NWNX!DB!FETCH");
    if ((sRow!="")&&(sRow!="|0|||||")) {
        NWNX_DB_UnBundle(sRow);
        return 1;
    } else {
        return 0;
    }
}

int NWNX_DB_Prepare(string sQueryTemplate)
{
    SetLocalString(GetModule(),"NWNX!DB!P_PREPARE",sQueryTemplate);
    string sRet = GetLocalString(GetModule(),"NWNX!DB!P_PREPARE");
    return (StringToInt(sRet));
}

void NWNX_DB_SetParam_String(int nNumber, string sString, int bNull=0)
{
    object oMod = GetModule();
    string sNumber = IntToString(nNumber);
    SetLocalString(oMod,"_DB_P_T_"+sNumber,"2");
    if (bNull) {
        SetLocalInt(oMod,"_DB_P_N_"+sNumber,1);
    } else {
        SetLocalString(GetModule(),"_DB_P_D_"+sNumber,sString);
    }
}
void NWNX_DB_SetParam_Int(int nNumber, int iInt, int bNull=0)
{
    object oMod = GetModule();
    string sNumber = IntToString(nNumber);
    SetLocalString(oMod,"_DB_P_T_"+sNumber,"3");
    if (bNull) {
        SetLocalInt(oMod,"_DB_P_N_"+sNumber,1);
    } else {
        SetLocalString(GetModule(),"_DB_P_D_"+sNumber,IntToString(iInt));
    }
}
void NWNX_DB_SetParam_Float(int nNumber, float fFloat, int bNull=0)
{
    object oMod = GetModule();
    string sNumber = IntToString(nNumber);
    SetLocalString(oMod,"_DB_P_T_"+sNumber,"4");
    if (bNull) {
        SetLocalInt(oMod,"_DB_P_N_"+sNumber,1);
    } else {
        SetLocalString(GetModule(),"_DB_P_D_"+sNumber,FloatToString(fFloat,0,9));
    }
}
void NWNX_DB_SetParam_Vector(int nNumber, vector vVector, int bNull=0)
{
    object oMod = GetModule();
    string sNumber = IntToString(nNumber);
    SetLocalString(oMod,"_DB_P_T_"+sNumber,"2");
    if (bNull) {
        SetLocalInt(oMod,"_DB_P_N_"+sNumber,1);
    } else {
        SetLocalString(GetModule(),"_DB_P_D_"+sNumber,NWNX_VectorToString(vVector));
    }
}
void NWNX_DB_SetParam_Location(int nNumber, location mLocation, int bNull=0)
{
    object oMod = GetModule();
    string sNumber = IntToString(nNumber);
    SetLocalString(oMod,"_DB_P_T_"+sNumber,"2");
    if (bNull) {
        SetLocalInt(oMod,"_DB_P_N_"+sNumber,1);
    } else {
        SetLocalString(GetModule(),"_DB_P_D_"+sNumber,NWNX_LocationToString(mLocation));
    }
}

void NWNX_DB_SetParam_Object(int nNumber, object oObject, int bNull=0)
{
    int nObjectType = GetObjectType(oObject);
    string sUrl;
    switch (nObjectType) {
        case OBJECT_TYPE_ITEM:      sUrl = "data://nwnx/blob/UTI"; break;
        case OBJECT_TYPE_CREATURE:  sUrl = "data://nwnx/blob/BIC"; break;
        case OBJECT_TYPE_PLACEABLE: sUrl = "data://nwnx/blob/UTP"; break;
        case OBJECT_TYPE_STORE:     sUrl = "data://nwnx/blob/UTM"; break;
        case OBJECT_TYPE_TRIGGER:   sUrl = "data://nwnx/blob/UTT"; break;
    }
    NWNX_Serializer_Freeze(oObject);
    object oModule = GetModule();
    string sNumber = IntToString(nNumber);
    SetLocalString(oModule,"_DB_P_T_"+sNumber,"1");
    if (bNull) {
        SetLocalInt(oModule,"_DB_P_N_"+sNumber,1);
    } else {
        SetLocalString(oModule,"_DB_P_D_"+sNumber,sUrl);
    }
}

int NWNX_DB_ExecutePrepared()
{
    string sMessage = "|";
    object oMod = GetModule();
    string sData;
    int nNumber = 1;
    string sNumber = IntToString(nNumber);
    string sType = GetLocalString(oMod,"_DB_P_T_"+sNumber);
    if (sType == "") {
        sMessage += "0||||";
    } else {
        while (sType != "") {
            DeleteLocalString(oMod,"_DB_P_T_"+sNumber);
            sMessage += sType + "|";
            int bNull = GetLocalInt(oMod,"_DB_P_N_"+sNumber);
            DeleteLocalInt(oMod,"_DB_P_N_"+sNumber);
            sMessage += (bNull?"1":"0") + "|";
            if (bNull) {
                 sMessage += "0||";
            } else {
                sData = GetLocalString(oMod,"_DB_P_D_"+sNumber);
                DeleteLocalString(oMod,"_DB_P_D_"+sNumber);
                sMessage += IntToString(GetStringLength(sData)) + "|";
                sMessage += sData + "|";
            }
            nNumber += 1;
            sNumber = IntToString(nNumber);
            sType = GetLocalString(oMod,"_DB_P_T_"+sNumber);
        }
    }
    sMessage += "|";
    SetLocalString(GetModule(),"NWNX!DB!P_EXECUTE",sMessage);
    string sRet = GetLocalString(GetModule(),"NWNX!DB!P_EXECUTE");
    return (StringToInt(sRet));
}

int NWNX_DB_FetchPrepared()
{
    if (!GetLocalInt(GetModule(),"NWNX_DB_IsInitialized")) NWNX_DB_Initialize();

    object oModule = GetModule();
    SetLocalString(oModule,"NWNX!DB!P_FETCH","XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
    string sRow = GetLocalString(oModule,"NWNX!DB!P_FETCH");
    if ((sRow!="")&&(sRow!="|0|||||")) {
        NWNX_DB_UnBundle(sRow);
        return 1;
    } else {
        return 0;
    }
}

int NWNX_DB_GetField_IsNull(int nFieldNumber)
{
    return(GetLocalInt(GetModule(),"_DB_R_N_"+IntToString(nFieldNumber)));
}

string NWNX_DB_GetField_String(int nFieldNumber)
{
    return(GetLocalString(GetModule(),"_DB_R_D_"+IntToString(nFieldNumber)));
}
int NWNX_DB_GetField_Int(int nFieldNumber)
{
    return(StringToInt(GetLocalString(GetModule(),"_DB_R_D_"+IntToString(nFieldNumber))));
}
float NWNX_DB_GetField_Float(int nFieldNumber)
{
    return(StringToFloat(GetLocalString(GetModule(),"_DB_R_D_"+IntToString(nFieldNumber))));
}
vector NWNX_DB_GetField_Vector(int nFieldNumber)
{
    return(NWNX_StringToVector(GetLocalString(GetModule(),"_DB_R_D_"+IntToString(nFieldNumber))));
}
location NWNX_DB_GetField_Location(int nFieldNumber)
{
    return(NWNX_StringToLocation(GetLocalString(GetModule(),"_DB_R_D_"+IntToString(nFieldNumber))));
}

object NWNX_DB_GetField_ObjectToContainer(int nFieldNumber, object oContainer)
{
    object oObject = OBJECT_INVALID;
    string sObject = GetLocalString(GetModule(),"_DB_R_D_"+IntToString(nFieldNumber));
    if (sObject=="") {
    } else if (sObject=="data://nwnx/blob/UTI") {
        oObject = RetrieveCampaignObject("NWNX","T",GetLocation(oContainer),oContainer);
    }
    return(oObject);
}

object NWNX_DB_GetField_ObjectToLocation(int nFieldNumber, location mLocation)
{
    object oObject = OBJECT_INVALID;
    string sObject = GetLocalString(GetModule(),"_DB_R_D_"+IntToString(nFieldNumber));
    if (sObject=="") {
    } else if (sObject=="data://nwnx/blob/UTI") {
        oObject = RetrieveCampaignObject("NWNX","T",mLocation,OBJECT_INVALID);
    } else if (sObject=="data://nwnx/blob/BIC") {
        oObject = RetrieveCampaignObject("NWNX","T",mLocation,OBJECT_INVALID);
    } else if (sObject=="data://nwnx/blob/UTP") {
        vector vVec = GetPositionFromLocation(mLocation);
        string sLoc = ObjectToString(GetAreaFromLocation(mLocation))
            + "¬" + FloatToString(vVec.x)
            + "¬" + FloatToString(vVec.y)
            + "¬" + FloatToString(vVec.z)
            + "¬" + FloatToString(GetFacingFromLocation(mLocation)) ;
        SetLocalString(GetModule(),"NWNX!DB!OBJ_THAW",sLoc);
        return (GetLocalObject(GetModule(),"NWNX!DB!RETRIEVEOBJECT"));
        oObject = GetLocalObject(GetModule(),"NWNX!DB!RETRIEVEOBJECT");
    } else if (sObject=="data://nwnx/blob/UTM") {
        vector vVec = GetPositionFromLocation(mLocation);
        string sLoc = ObjectToString(GetAreaFromLocation(mLocation))
            + "¬" + FloatToString(vVec.x)
            + "¬" + FloatToString(vVec.y)
            + "¬" + FloatToString(vVec.z)
            + "¬" + FloatToString(GetFacingFromLocation(mLocation)) ;
        SetLocalString(GetModule(),"NWNX!DB!OBJ_THAW",sLoc);
        return (GetLocalObject(GetModule(),"NWNX!DB!RETRIEVEOBJECT"));
        oObject = GetLocalObject(GetModule(),"NWNX!DB!RETRIEVEOBJECT");
    } else if (sObject=="data://nwnx/blob/UTT") {
        vector vVec = GetPositionFromLocation(mLocation);
        string sLoc = ObjectToString(GetAreaFromLocation(mLocation))
            + "¬" + FloatToString(vVec.x)
            + "¬" + FloatToString(vVec.y)
            + "¬" + FloatToString(vVec.z)
            + "¬" + FloatToString(GetFacingFromLocation(mLocation)) ;
        SetLocalString(GetModule(),"NWNX!DB!OBJ_THAW",sLoc);
        return (GetLocalObject(GetModule(),"NWNX!DB!RETRIEVEOBJECT"));
        oObject = GetLocalObject(GetModule(),"NWNX!DB!RETRIEVEOBJECT");
    }
    return(oObject);
}

