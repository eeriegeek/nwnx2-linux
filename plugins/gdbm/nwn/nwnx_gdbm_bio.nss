/******************************************************************************

  nwnx_gdbm_bio - NWNX - GDBM Plugin - Bioware DB compatibility layer.

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
void GDBM_SetCampaignString(string sCampaignName, string sVarName, string sString, object oPlayer=OBJECT_INVALID);
void GDBM_SetCampaignInt(string sCampaignName, string sVarName, int nInt, object oPlayer=OBJECT_INVALID);
void GDBM_SetCampaignFloat(string sCampaignName, string sVarName, float flFloat, object oPlayer=OBJECT_INVALID);
void GDBM_SetCampaignVector(string sCampaignName, string sVarName, vector vVector, object oPlayer=OBJECT_INVALID);
void GDBM_SetCampaignLocation(string sCampaignName, string sVarName, location locLocation, object oPlayer=OBJECT_INVALID);

//
// Retrieve a persistent variable. Returns an empty/invalid value if not found.
//
string GDBM_GetCampaignString(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID);
int GDBM_GetCampaignInt(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID);
float GDBM_GetCampaignFloat(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID);
vector GDBM_GetCampaignVector(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID);
location GDBM_GetCampaignLocation(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID);

//
// Delete a persistent variable.
//
void GDBM_DeleteCampaignString(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID);
void GDBM_DeleteCampaignInt(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID);
void GDBM_DeleteCampaignFloat(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID);
void GDBM_DeleteCampaignVector(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID);
void GDBM_DeleteCampaignLocation(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID);

//
// This version of delete operates as the standard one, deleting all variables
// associated with the given key regardless of the variable's base type.
//
void GDBM_DeleteCampaignVariable(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID);

//
// Deletes the entire database file.
//
void GDBM_DestroyCampaignDatabase(string sCampaignName);


//
// Implementation
//

void GDBM_SetCampaignString(string sCampaignName, string sVarName, string sString, object oPlayer=OBJECT_INVALID)
{
    string sKey;
    if ( oPlayer != OBJECT_INVALID ) {
        sKey += GetPCPlayerName(oPlayer);
        sKey += NWNX_GDBM_KEYTOK;
        sKey += GetName(oPlayer);
        sKey += NWNX_GDBM_KEYTOK;
    } 
    sKey += sVarName;
    NWNX_GdbmStoreString(sCampaignName,sKey,sString);
}
string GDBM_GetCampaignString(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID)
{
    string sKey;
    if ( oPlayer != OBJECT_INVALID ) {
        sKey += GetPCPlayerName(oPlayer);
        sKey += NWNX_GDBM_KEYTOK;
        sKey += GetName(oPlayer);
        sKey += NWNX_GDBM_KEYTOK;
    }
    sKey += sVarName;
    return(NWNX_GdbmFetchString(sCampaignName,sKey));
}

void GDBM_SetCampaignInt(string sCampaignName, string sVarName, int nInt, object oPlayer=OBJECT_INVALID)
{
    string sKey;
    if ( oPlayer != OBJECT_INVALID ) {
        sKey += GetPCPlayerName(oPlayer);
        sKey += NWNX_GDBM_KEYTOK;
        sKey += GetName(oPlayer);
        sKey += NWNX_GDBM_KEYTOK;
    } 
    sKey += sVarName;
    NWNX_GdbmStoreInt(sCampaignName,sKey,nInt);
}
int GDBM_GetCampaignInt(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID)
{
    string sKey;
    if ( oPlayer != OBJECT_INVALID ) {
        sKey += GetPCPlayerName(oPlayer);
        sKey += NWNX_GDBM_KEYTOK;
        sKey += GetName(oPlayer);
        sKey += NWNX_GDBM_KEYTOK;
    }
    sKey += sVarName;
    return(NWNX_GdbmFetchInt(sCampaignName,sKey));
}

void GDBM_SetCampaignFloat(string sCampaignName, string sVarName, float flFloat, object oPlayer=OBJECT_INVALID)
{
    string sKey;
    if ( oPlayer != OBJECT_INVALID ) {
        sKey += GetPCPlayerName(oPlayer);
        sKey += NWNX_GDBM_KEYTOK;
        sKey += GetName(oPlayer);
        sKey += NWNX_GDBM_KEYTOK;
    }
    sKey += sVarName;
    NWNX_GdbmStoreFloat(sCampaignName,sKey,flFloat);
}
float GDBM_GetCampaignFloat(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID)
{
    string sKey;
    if ( oPlayer != OBJECT_INVALID ) {
        sKey += GetPCPlayerName(oPlayer);
        sKey += NWNX_GDBM_KEYTOK;
        sKey += GetName(oPlayer);
        sKey += NWNX_GDBM_KEYTOK;
    }
    sKey += sVarName;
    return(NWNX_GdbmFetchFloat(sCampaignName,sKey));
}

void GDBM_SetCampaignVector(string sCampaignName, string sVarName, vector vVector, object oPlayer=OBJECT_INVALID)
{
    string sKey;
    if ( oPlayer != OBJECT_INVALID ) {
        sKey += GetPCPlayerName(oPlayer);
        sKey += NWNX_GDBM_KEYTOK;
        sKey += GetName(oPlayer);
        sKey += NWNX_GDBM_KEYTOK;
    }
    sKey += sVarName;
    NWNX_GdbmStoreVector(sCampaignName,sKey,vVector);
}
vector GDBM_GetCampaignVector(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID)
{
    string sKey;
    if ( oPlayer != OBJECT_INVALID ) {
        sKey += GetPCPlayerName(oPlayer);
        sKey += NWNX_GDBM_KEYTOK;
        sKey += GetName(oPlayer);
        sKey += NWNX_GDBM_KEYTOK;
    }
    sKey += sVarName;
    return(NWNX_GdbmFetchVector(sCampaignName,sKey));
}

void GDBM_SetCampaignLocation(string sCampaignName, string sVarName, location locLocation, object oPlayer=OBJECT_INVALID)
{
    string sKey;
    if ( oPlayer != OBJECT_INVALID ) {
        sKey += GetPCPlayerName(oPlayer);
        sKey += NWNX_GDBM_KEYTOK;
        sKey += GetName(oPlayer);
        sKey += NWNX_GDBM_KEYTOK;
    }
    sKey += sVarName;
    NWNX_GdbmStoreLocation(sCampaignName,sKey,locLocation);
}
location GDBM_GetCampaignLocation(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID)
{
    string sKey;
    if ( oPlayer != OBJECT_INVALID ) {
        sKey += GetPCPlayerName(oPlayer);
        sKey += NWNX_GDBM_KEYTOK;
        sKey += GetName(oPlayer);
        sKey += NWNX_GDBM_KEYTOK;
    }
    sKey += sVarName;
    return(NWNX_GdbmFetchLocation(sCampaignName,sKey));
}

int GDBM_StoreCampaignObject(string sCampaignName, string sVarName, object oObject, object oPlayer=OBJECT_INVALID);
int GDBM_StoreCampaignObject(string sCampaignName, string sVarName, object oObject, object oPlayer=OBJECT_INVALID)
{
    return 0;
}
object GDBM_RetrieveCampaignObject(string sCampaignName, string sVarName, location locLocation, object oOwner=OBJECT_INVALID, object oPlayer=OBJECT_INVALID);
object GDBM_RetrieveCampaignObject(string sCampaignName, string sVarName, location locLocation, object oOwner=OBJECT_INVALID, object oPlayer=OBJECT_INVALID)
{
    return OBJECT_INVALID;
}

void GDBM_DeleteCampaignString(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID)
{
    string sKey;
    if ( oPlayer != OBJECT_INVALID ) {
        sKey += GetPCPlayerName(oPlayer);
        sKey += NWNX_GDBM_KEYTOK;
        sKey += GetName(oPlayer);
        sKey += NWNX_GDBM_KEYTOK;
    }
    sKey += sVarName;
    NWNX_GdbmDeleteString(sCampaignName,sKey);
}
void GDBM_DeleteCampaignInt(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID)
{
    string sKey;
    if ( oPlayer != OBJECT_INVALID ) {
        sKey += GetPCPlayerName(oPlayer);
        sKey += NWNX_GDBM_KEYTOK;
        sKey += GetName(oPlayer);
        sKey += NWNX_GDBM_KEYTOK;
    }
    sKey += sVarName;
    NWNX_GdbmDeleteInt(sCampaignName,sKey);
}
void GDBM_DeleteCampaignFloat(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID)
{
    string sKey;
    if ( oPlayer != OBJECT_INVALID ) {
        sKey += GetPCPlayerName(oPlayer);
        sKey += NWNX_GDBM_KEYTOK;
        sKey += GetName(oPlayer);
        sKey += NWNX_GDBM_KEYTOK;
    }
    sKey += sVarName;
    NWNX_GdbmDeleteFloat(sCampaignName,sKey);
}
void GDBM_DeleteCampaignVector(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID)
{
    string sKey;
    if ( oPlayer != OBJECT_INVALID ) {
        sKey += GetPCPlayerName(oPlayer);
        sKey += NWNX_GDBM_KEYTOK;
        sKey += GetName(oPlayer);
        sKey += NWNX_GDBM_KEYTOK;
    }
    sKey += sVarName;
    NWNX_GdbmDeleteVector(sCampaignName,sKey);
}
void GDBM_DeleteCampaignLocation(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID)
{
    string sKey;
    if ( oPlayer != OBJECT_INVALID ) {
        sKey += GetPCPlayerName(oPlayer);
        sKey += NWNX_GDBM_KEYTOK;
        sKey += GetName(oPlayer);
        sKey += NWNX_GDBM_KEYTOK;
    }
    sKey += sVarName;
    NWNX_GdbmDeleteLocation(sCampaignName,sKey);
}
void GDBM_DeleteCampaignObject(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID)
{
    string sKey;
    if ( oPlayer != OBJECT_INVALID ) {
        sKey += GetPCPlayerName(oPlayer);
        sKey += NWNX_GDBM_KEYTOK;
        sKey += GetName(oPlayer);
        sKey += NWNX_GDBM_KEYTOK;
    }
    sKey += sVarName;
    NWNX_GdbmDeleteObject(sCampaignName,sKey);
}

void GDBM_DeleteCampaignVariable(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID)
{
    string sKey;
    if ( oPlayer != OBJECT_INVALID ) {
        sKey += GetPCPlayerName(oPlayer);
        sKey += NWNX_GDBM_KEYTOK;
        sKey += GetName(oPlayer);
        sKey += NWNX_GDBM_KEYTOK;
    }
    sKey += sVarName;
    NWNX_GdbmDeleteString(sCampaignName,sKey);
    NWNX_GdbmDeleteInt(sCampaignName,sKey);
    NWNX_GdbmDeleteFloat(sCampaignName,sKey);
    NWNX_GdbmDeleteVector(sCampaignName,sKey);
    NWNX_GdbmDeleteLocation(sCampaignName,sKey);
    NWNX_GdbmDeleteObject(sCampaignName,sKey);
}

void GDBM_DestroyCampaignDatabase(string sCampaignName)
{
    NWNX_GdbmDestroy(sCampaignName);
}

