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

//-----------------------------------------------------------------------------
// The following set of functions provide a compatibility layer for GDBM
// patterned after the Bioware campaign database interface. Functions are
// be argument for argument and functionally compatible while providing
// the addtional support available in GDBM. See the README file for details,
// but is short key size limitations are removed, type collisions avoided,
// and less maintenance is required.
//-----------------------------------------------------------------------------

// Store a persistent string. Stores the string to persistent file storage
// in a GDBM database file. Creates and opens the GDBM file if needed.
// All names and strings are case sensitive.
//
// sCampaignName   -> Essentially the database file name. This string identifies
//                    the database in which the persistent variable will be
//                    stored. The actual file will be named bt this value with a
//                    ".gdbm" file extension. The name should not contain any
//                    characters which are not safe in filenames.
// sVarName        -> The name (or key) of the variable to store. Length unlimited.
// sString         -> The value of the string to store persistently. Lengthe unlimited.
// oPlayer         -> A PC object. If this argument is provided (not OBJECT_INVALID)
//                    the key/value pair is uniquely associated with the given
//                    player. Variables stored with the same key but other PC
//                    objects will be stored and retrieved seperately.
//
void NWNX_GDBM_SetCampaignString(string sCampaignName, string sVarName, string sString, object oPlayer=OBJECT_INVALID);

// Store a persistent int. See NWNX_GDBM_SetCampaignString for details.
//
void NWNX_GDBM_SetCampaignInt(string sCampaignName, string sVarName, int nInt, object oPlayer=OBJECT_INVALID);

// Store a persistent float. See NWNX_GDBM_SetCampaignString for details.
//
void NWNX_GDBM_SetCampaignFloat(string sCampaignName, string sVarName, float flFloat, object oPlayer=OBJECT_INVALID);

// Store a persistent vector. See NWNX_GDBM_SetCampaignString for details.
//
void NWNX_GDBM_SetCampaignVector(string sCampaignName, string sVarName, vector vVector, object oPlayer=OBJECT_INVALID);

// Store a persistent location. See NWNX_GDBM_SetCampaignString for details.
//
void NWNX_GDBM_SetCampaignLocation(string sCampaignName, string sVarName, location locLocation, object oPlayer=OBJECT_INVALID);

// Store a persistent object. See NWNX_GDBM_SetCampaignString for details.
//
int NWNX_GDBM_StoreCampaignObject(string sCampaignName, string sVarName, object oObject, object oPlayer=OBJECT_INVALID);

// Retrieve a persistent string. Retrieves the string stored via the given key
// from persistent GDBM file storage.
//
// Returns an empty string if the key is not found.
//
string NWNX_GDBM_GetCampaignString(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID);

// Retrieve a persistent int.
// Returns 0 if the key is not found.
//
int NWNX_GDBM_GetCampaignInt(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID);

// Retrieve a persistent float.
// Returns 0.0 if the key is not found.
//
float NWNX_GDBM_GetCampaignFloat(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID);

// Retrieve a persistent vector.
// Returns NWNX_VECTOR_INVALID if the key is not found.
//
vector NWNX_GDBM_GetCampaignVector(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID);

// Retrieve a persistent location.
// Returns NWNX_LOCATION_INVALID if the key is not found.
//
location NWNX_GDBM_GetCampaignLocation(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID);

// Retrieve a persistent object.
// Returns OBJECT_INVALID if the key is not found.
//
object NWNX_GDBM_RetrieveCampaignObject(string sCampaignName, string sVarName, location locLocation, object oOwner=OBJECT_INVALID, object oPlayer=OBJECT_INVALID);

// Delete a persistent string.
//
void NWNX_GDBM_DeleteCampaignString(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID);

// Delete a persistent int.
//
void NWNX_GDBM_DeleteCampaignInt(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID);

// Delete a persistent float.
//
void NWNX_GDBM_DeleteCampaignFloat(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID);

// Delete a persistent vector.
//
void NWNX_GDBM_DeleteCampaignVector(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID);

// Delete a persistent location.
//
void NWNX_GDBM_DeleteCampaignLocation(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID);

// Delete a persistent object.
//
void NWNX_GDBM_DeleteCampaignObject(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID);

// Delete all type variable with the given key. This version of delete operates
// like the standard one, deleting all variables associated with the given key
// regardless of the variable's base type.
//
void NWNX_GDBM_DeleteCampaignVariable(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID);

// Deletes the entire database file. Use with caution!
//
void NWNX_GDBM_DestroyCampaignDatabase(string sCampaignName);

//-----------------------------------------------------------------------------
// Implementation
//-----------------------------------------------------------------------------

// Note, several of the functions have been "unrolled" rather than call internal
// functions. This was done during performance testing to reduce instruction
// count and left in place for now. Also some constant strings were folded for
// the same purpose.

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
    // Skip check of status code, since we ignore it anyway in this method.
    //NWNX_GDBM_StoreString(sCampaignName, sKey, sString);
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
    //return NWNX_GDBM_FetchString(sCampaignName, sKey);
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
    //NWNX_GDBM_StoreInt(sCampaignName, sKey, nInt);
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
    //return NWNX_GDBM_FetchInt(sCampaignName, sKey);
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

