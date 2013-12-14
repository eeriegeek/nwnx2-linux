/******************************************************************************

  nwnx_gdbm_bio - NWNX - GDBM Plugin - Bioware DB compatibility layer.

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

#include "nwnx_gdbm"

/******************************************************************************

  The following set of functions provide a rough compatibility layer for GDBM
  mimicing the Bioware Campaign DB interface for easy migration and testing.

  Changes/Fixes:

  Key Size Limitation - Keys (VarableName) in the Bioware Campaign DB are limited in length
  to 32 characters. This limit is lifted to an effectively unlimited key size.

  Per PC Object Data - As with the Key Size Limitation, the bioware campaign
  db encounters a 32 character size limit on the key used to allow per PC
  data. The player name and character name are used as a key, but the combined
  key is truncated to 32 chars. That limitation is removed here. If the object
  passed as oPlayer is invalid or not a PC it is ignored.

  Type Collision - In the Bioware Campaign DB different types are keyed to the
  same storage record. If an Int named "foo" is stored, it overwrites the
  previously stored string named "foo". Here, the type is added to the key,
  so variables of different types can no longer effect each other even if they
  have the same name.

******************************************************************************/

//
// Store a persistent variable. Creates a database file if none exists.
//
void NWNX_GDBM_SetCampaignString(string sCampaignName, string sVarName, string sString, object oPlayer=OBJECT_INVALID);
void NWNX_GDBM_SetCampaignInt(string sCampaignName, string sVarName, int nInt, object oPlayer=OBJECT_INVALID);
void NWNX_GDBM_SetCampaignFloat(string sCampaignName, string sVarName, float flFloat, object oPlayer=OBJECT_INVALID);
void NWNX_GDBM_SetCampaignVector(string sCampaignName, string sVarName, vector vVector, object oPlayer=OBJECT_INVALID);
void NWNX_GDBM_SetCampaignLocation(string sCampaignName, string sVarName, location locLocation, object oPlayer=OBJECT_INVALID);

int NWNX_GDBM_StoreCampaignObject(string sCampaignName, string sVarName, object oObject, object oPlayer=OBJECT_INVALID);

//
// Retrieve a persistent variable. Returns an empty/invalid value if not found.
//
string NWNX_GDBM_GetCampaignString(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID);
int NWNX_GDBM_GetCampaignInt(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID);
float NWNX_GDBM_GetCampaignFloat(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID);
vector NWNX_GDBM_GetCampaignVector(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID);
location NWNX_GDBM_GetCampaignLocation(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID);

object NWNX_GDBM_RetrieveCampaignObject(string sCampaignName, string sVarName, location locLocation, object oOwner=OBJECT_INVALID, object oPlayer=OBJECT_INVALID);

//
// Delete a persistent variable.
//
void NWNX_GDBM_DeleteCampaignString(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID);
void NWNX_GDBM_DeleteCampaignInt(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID);
void NWNX_GDBM_DeleteCampaignFloat(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID);
void NWNX_GDBM_DeleteCampaignVector(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID);
void NWNX_GDBM_DeleteCampaignLocation(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID);

void NWNX_GDBM_DeleteCampaignObject(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID);

//
// This version of delete operates as the standard one, deleting all variables
// associated with the given key regardless of the variable's base type.
//
void NWNX_GDBM_DeleteCampaignVariable(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID);

//
// Deletes the entire database file.
//
void NWNX_GDBM_DestroyCampaignDatabase(string sCampaignName);


//
// Implementation
//

// Collapse constant strings to reduce instruction count.
string NWNX_GDBM_T1T   = NWNX_GDBM_ARGSEP + "1" + NWNX_GDBM_ARGSEP;
string NWNX_GDBM_T1T1T = NWNX_GDBM_ARGSEP + "1" + NWNX_GDBM_ARGSEP + "1" + NWNX_GDBM_ARGSEP;

void NWNX_GDBM_SetCampaignString(string sCampaignName, string sVarName, string sString, object oPlayer=OBJECT_INVALID)
{
    string sKey;
    if ( oPlayer != OBJECT_INVALID ) {
        sKey += GetPCPlayerName(oPlayer);
        sKey += NWNX_GDBM_KEYSEP;
        sKey += GetName(oPlayer);
        sKey += NWNX_GDBM_KEYSEP;
    } 
    sKey += sVarName;
    sKey += NWNX_GDBM_KEYSEP;
    sKey += "S";
    SetLocalString(
        GetModule(),
        "NWNX!GDBM!STORE",
        sCampaignName + NWNX_GDBM_T1T1T + sKey + NWNX_GDBM_ARGSEP + sString
    );
    // Skip check of status code, since we igore it anyway in this method.
    //NWNX_GDBM_StoreString(sCampaignName,sKey,sString);
}
string NWNX_GDBM_GetCampaignString(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID)
{
    string sKey;
    if ( oPlayer != OBJECT_INVALID ) {
        sKey += GetPCPlayerName(oPlayer);
        sKey += NWNX_GDBM_KEYSEP;
        sKey += GetName(oPlayer);
        sKey += NWNX_GDBM_KEYSEP;
    }
    sKey += sVarName;
    sKey += NWNX_GDBM_KEYSEP;
    sKey += "S";
    SetLocalString(
        GetModule(),
        "NWNX!GDBM!FETCH",
        sCampaignName + NWNX_GDBM_T1T + sKey
    );
    return GetLocalString(GetModule(),"NWNX!GDBM!FETCH");
    //return(NWNX_GDBM_FetchString(sCampaignName,sKey));
}

void NWNX_GDBM_SetCampaignInt(string sCampaignName, string sVarName, int nInt, object oPlayer=OBJECT_INVALID)
{
    string sKey;
    if ( oPlayer != OBJECT_INVALID ) {
        sKey += GetPCPlayerName(oPlayer);
        sKey += NWNX_GDBM_KEYSEP;
        sKey += GetName(oPlayer);
        sKey += NWNX_GDBM_KEYSEP;
    } 
    sKey += sVarName;
    sKey += NWNX_GDBM_KEYSEP;
    sKey += "I";
    SetLocalString(
        GetModule(),
        "NWNX!GDBM!STORE",
        sCampaignName + NWNX_GDBM_T1T1T + sKey + NWNX_GDBM_ARGSEP + IntToString(nInt)
    );
    // Skip check of status code, since we igore it anyway in this method.
    //NWNX_GDBM_StoreInt(sCampaignName,sKey,nInt);
}
int NWNX_GDBM_GetCampaignInt(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID)
{
    string sKey;
    if ( oPlayer != OBJECT_INVALID ) {
        sKey += GetPCPlayerName(oPlayer);
        sKey += NWNX_GDBM_KEYSEP;
        sKey += GetName(oPlayer);
        sKey += NWNX_GDBM_KEYSEP;
    }
    sKey += sVarName;
    sKey += NWNX_GDBM_KEYSEP;
    sKey += "I";
    SetLocalString(
        GetModule(),
        "NWNX!GDBM!FETCH",
        sCampaignName + NWNX_GDBM_T1T + sKey
    );
    return StringToInt(GetLocalString(GetModule(),"NWNX!GDBM!FETCH"));
    //return(NWNX_GDBM_FetchInt(sCampaignName,sKey));
}

void NWNX_GDBM_SetCampaignFloat(string sCampaignName, string sVarName, float flFloat, object oPlayer=OBJECT_INVALID)
{
    string sKey;
    if ( oPlayer != OBJECT_INVALID ) {
        sKey += GetPCPlayerName(oPlayer);
        sKey += NWNX_GDBM_KEYSEP;
        sKey += GetName(oPlayer);
        sKey += NWNX_GDBM_KEYSEP;
    }
    sKey += sVarName;
    NWNX_GDBM_StoreFloat(sCampaignName, sKey, flFloat);
}
float NWNX_GDBM_GetCampaignFloat(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID)
{
    string sKey;
    if ( oPlayer != OBJECT_INVALID ) {
        sKey += GetPCPlayerName(oPlayer);
        sKey += NWNX_GDBM_KEYSEP;
        sKey += GetName(oPlayer);
        sKey += NWNX_GDBM_KEYSEP;
    }
    sKey += sVarName;
    return NWNX_GDBM_FetchFloat(sCampaignName, sKey);
}

void NWNX_GDBM_SetCampaignVector(string sCampaignName, string sVarName, vector vVector, object oPlayer=OBJECT_INVALID)
{
    string sKey;
    if ( oPlayer != OBJECT_INVALID ) {
        sKey += GetPCPlayerName(oPlayer);
        sKey += NWNX_GDBM_KEYSEP;
        sKey += GetName(oPlayer);
        sKey += NWNX_GDBM_KEYSEP;
    }
    sKey += sVarName;
    NWNX_GDBM_StoreVector(sCampaignName, sKey, vVector);
}
vector NWNX_GDBM_GetCampaignVector(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID)
{
    string sKey;
    if ( oPlayer != OBJECT_INVALID ) {
        sKey += GetPCPlayerName(oPlayer);
        sKey += NWNX_GDBM_KEYSEP;
        sKey += GetName(oPlayer);
        sKey += NWNX_GDBM_KEYSEP;
    }
    sKey += sVarName;
    return NWNX_GDBM_FetchVector(sCampaignName, sKey);
}

void NWNX_GDBM_SetCampaignLocation(string sCampaignName, string sVarName, location locLocation, object oPlayer=OBJECT_INVALID)
{
    string sKey;
    if ( oPlayer != OBJECT_INVALID ) {
        sKey += GetPCPlayerName(oPlayer);
        sKey += NWNX_GDBM_KEYSEP;
        sKey += GetName(oPlayer);
        sKey += NWNX_GDBM_KEYSEP;
    }
    sKey += sVarName;
    NWNX_GDBM_StoreLocation(sCampaignName, sKey, locLocation);
}
location NWNX_GDBM_GetCampaignLocation(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID)
{
    string sKey;
    if ( oPlayer != OBJECT_INVALID ) {
        sKey += GetPCPlayerName(oPlayer);
        sKey += NWNX_GDBM_KEYSEP;
        sKey += GetName(oPlayer);
        sKey += NWNX_GDBM_KEYSEP;
    }
    sKey += sVarName;
    return NWNX_GDBM_FetchLocation(sCampaignName, sKey);
}

int NWNX_GDBM_StoreCampaignObject(string sCampaignName, string sVarName, object oObject, object oPlayer=OBJECT_INVALID)
{
    string sKey;
    if ( oPlayer != OBJECT_INVALID ) {
        sKey += GetPCPlayerName(oPlayer);
        sKey += NWNX_GDBM_KEYSEP;
        sKey += GetName(oPlayer);
        sKey += NWNX_GDBM_KEYSEP;
    }
    sKey += sVarName;
    return NWNX_GDBM_StoreObject(sCampaignName, sKey, oObject);
}
object NWNX_GDBM_RetrieveCampaignObject(string sCampaignName, string sVarName, location locLocation, object oOwner=OBJECT_INVALID, object oPlayer=OBJECT_INVALID)
{
    string sKey;
    if ( oPlayer != OBJECT_INVALID ) {
        sKey += GetPCPlayerName(oPlayer);
        sKey += NWNX_GDBM_KEYSEP;
        sKey += GetName(oPlayer);
        sKey += NWNX_GDBM_KEYSEP;
    }
    sKey += sVarName;
    return NWNX_GDBM_FetchObject(sCampaignName, sKey, locLocation, oOwner);
}

//
// Delete
//
void NWNX_GDBM_DeleteCampaignString(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID)
{
    string sKey;
    if ( oPlayer != OBJECT_INVALID ) {
        sKey += GetPCPlayerName(oPlayer);
        sKey += NWNX_GDBM_KEYSEP;
        sKey += GetName(oPlayer);
        sKey += NWNX_GDBM_KEYSEP;
    }
    sKey += sVarName;
    NWNX_GDBM_DeleteString(sCampaignName,sKey);
}
void NWNX_GDBM_DeleteCampaignInt(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID)
{
    string sKey;
    if ( oPlayer != OBJECT_INVALID ) {
        sKey += GetPCPlayerName(oPlayer);
        sKey += NWNX_GDBM_KEYSEP;
        sKey += GetName(oPlayer);
        sKey += NWNX_GDBM_KEYSEP;
    }
    sKey += sVarName;
    NWNX_GDBM_DeleteInt(sCampaignName,sKey);
}
void NWNX_GDBM_DeleteCampaignFloat(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID)
{
    string sKey;
    if ( oPlayer != OBJECT_INVALID ) {
        sKey += GetPCPlayerName(oPlayer);
        sKey += NWNX_GDBM_KEYSEP;
        sKey += GetName(oPlayer);
        sKey += NWNX_GDBM_KEYSEP;
    }
    sKey += sVarName;
    NWNX_GDBM_DeleteFloat(sCampaignName,sKey);
}
void NWNX_GDBM_DeleteCampaignVector(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID)
{
    string sKey;
    if ( oPlayer != OBJECT_INVALID ) {
        sKey += GetPCPlayerName(oPlayer);
        sKey += NWNX_GDBM_KEYSEP;
        sKey += GetName(oPlayer);
        sKey += NWNX_GDBM_KEYSEP;
    }
    sKey += sVarName;
    NWNX_GDBM_DeleteVector(sCampaignName,sKey);
}
void NWNX_GDBM_DeleteCampaignLocation(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID)
{
    string sKey;
    if ( oPlayer != OBJECT_INVALID ) {
        sKey += GetPCPlayerName(oPlayer);
        sKey += NWNX_GDBM_KEYSEP;
        sKey += GetName(oPlayer);
        sKey += NWNX_GDBM_KEYSEP;
    }
    sKey += sVarName;
    NWNX_GDBM_DeleteLocation(sCampaignName,sKey);
}
void NWNX_GDBM_DeleteCampaignObject(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID)
{
    string sKey;
    if ( oPlayer != OBJECT_INVALID ) {
        sKey += GetPCPlayerName(oPlayer);
        sKey += NWNX_GDBM_KEYSEP;
        sKey += GetName(oPlayer);
        sKey += NWNX_GDBM_KEYSEP;
    }
    sKey += sVarName;
    NWNX_GDBM_DeleteObject(sCampaignName,sKey);
}

void NWNX_GDBM_DeleteCampaignVariable(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID)
{
    string sKey;
    if ( oPlayer != OBJECT_INVALID ) {
        sKey += GetPCPlayerName(oPlayer);
        sKey += NWNX_GDBM_KEYSEP;
        sKey += GetName(oPlayer);
        sKey += NWNX_GDBM_KEYSEP;
    }
    sKey += sVarName;
    NWNX_GDBM_DeleteString(sCampaignName,sKey);
    NWNX_GDBM_DeleteInt(sCampaignName,sKey);
    NWNX_GDBM_DeleteFloat(sCampaignName,sKey);
    NWNX_GDBM_DeleteVector(sCampaignName,sKey);
    NWNX_GDBM_DeleteLocation(sCampaignName,sKey);
    NWNX_GDBM_DeleteObject(sCampaignName,sKey);
}

//
// Destroy
//
void NWNX_GDBM_DestroyCampaignDatabase(string sCampaignName)
{
    NWNX_GDBM_Destroy(sCampaignName);
}

