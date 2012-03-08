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

  Key Size Limitation - Keys in the Bioware Campaign DB are limited in length
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

  TODO: Object handling is not yet implemented.

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
    string sKey = GetPCPlayerName(oPlayer);
    if (sKey!="") sKey += NWNX_GDBM_KEYTOK+GetName(oPlayer)+NWNX_GDBM_KEYTOK;
    sKey += sVarName;
    SetGdbmString(sCampaignName,sKey,sString);
}
string GDBM_GetCampaignString(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID)
{
    string sKey = GetPCPlayerName(oPlayer);
    if (sKey!="") sKey += NWNX_GDBM_KEYTOK+GetName(oPlayer)+NWNX_GDBM_KEYTOK;
    sKey += sVarName;
    return(GetGdbmString(sCampaignName,sKey));
}

void GDBM_SetCampaignInt(string sCampaignName, string sVarName, int nInt, object oPlayer=OBJECT_INVALID)
{
    string sKey = GetPCPlayerName(oPlayer);
    if (sKey!="") sKey += NWNX_GDBM_KEYTOK+GetName(oPlayer)+NWNX_GDBM_KEYTOK;
    sKey += sVarName;
    SetGdbmInt(sCampaignName,sKey,nInt);
}
int GDBM_GetCampaignInt(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID)
{
    string sKey = GetPCPlayerName(oPlayer);
    if (sKey!="") sKey += NWNX_GDBM_KEYTOK+GetName(oPlayer)+NWNX_GDBM_KEYTOK;
    sKey += sVarName;
    return(GetGdbmInt(sCampaignName,sKey));
}

void GDBM_SetCampaignFloat(string sCampaignName, string sVarName, float flFloat, object oPlayer=OBJECT_INVALID)
{
    string sKey = GetPCPlayerName(oPlayer);
    if (sKey!="") sKey += NWNX_GDBM_KEYTOK+GetName(oPlayer)+NWNX_GDBM_KEYTOK;
    sKey += sVarName;
    SetGdbmFloat(sCampaignName,sKey,flFloat);
}
float GDBM_GetCampaignFloat(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID)
{
    string sKey = GetPCPlayerName(oPlayer);
    if (sKey!="") sKey += NWNX_GDBM_KEYTOK+GetName(oPlayer)+NWNX_GDBM_KEYTOK;
    sKey += sVarName;
    return(GetGdbmFloat(sCampaignName,sKey));
}

void GDBM_SetCampaignVector(string sCampaignName, string sVarName, vector vVector, object oPlayer=OBJECT_INVALID)
{
    string sKey = GetPCPlayerName(oPlayer);
    if (sKey!="") sKey += NWNX_GDBM_KEYTOK+GetName(oPlayer)+NWNX_GDBM_KEYTOK;
    sKey += sVarName;
    SetGdbmVector(sCampaignName,sKey,vVector);
}
vector GDBM_GetCampaignVector(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID)
{
    string sKey = GetPCPlayerName(oPlayer);
    if (sKey!="") sKey += NWNX_GDBM_KEYTOK+GetName(oPlayer)+NWNX_GDBM_KEYTOK;
    sKey += sVarName;
    return(GetGdbmVector(sCampaignName,sKey));
}

void GDBM_SetCampaignLocation(string sCampaignName, string sVarName, location locLocation, object oPlayer=OBJECT_INVALID)
{
    string sKey = GetPCPlayerName(oPlayer);
    if (sKey!="") sKey += NWNX_GDBM_KEYTOK+GetName(oPlayer)+NWNX_GDBM_KEYTOK;
    sKey += sVarName;
    SetGdbmLocation(sCampaignName,sKey,locLocation);
}
location GDBM_GetCampaignLocation(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID)
{
    string sKey = GetPCPlayerName(oPlayer);
    if (sKey!="") sKey += NWNX_GDBM_KEYTOK+GetName(oPlayer)+NWNX_GDBM_KEYTOK;
    sKey += sVarName;
    return(GetGdbmLocation(sCampaignName,sKey));
}

void GDBM_DeleteCampaignString(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID)
{
    string sKey = GetPCPlayerName(oPlayer);
    if (sKey!="") sKey += NWNX_GDBM_KEYTOK+GetName(oPlayer)+NWNX_GDBM_KEYTOK;
    sKey += sVarName;
    DeleteGdbmString(sCampaignName,sKey);
}
void GDBM_DeleteCampaignInt(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID)
{
    string sKey = GetPCPlayerName(oPlayer);
    if (sKey!="") sKey += NWNX_GDBM_KEYTOK+GetName(oPlayer)+NWNX_GDBM_KEYTOK;
    sKey += sVarName;
    DeleteGdbmInt(sCampaignName,sKey);
}
void GDBM_DeleteCampaignFloat(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID)
{
    string sKey = GetPCPlayerName(oPlayer);
    if (sKey!="") sKey += NWNX_GDBM_KEYTOK+GetName(oPlayer)+NWNX_GDBM_KEYTOK;
    sKey += sVarName;
    DeleteGdbmFloat(sCampaignName,sKey);
}
void GDBM_DeleteCampaignVector(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID)
{
    string sKey = GetPCPlayerName(oPlayer);
    if (sKey!="") sKey += NWNX_GDBM_KEYTOK+GetName(oPlayer)+NWNX_GDBM_KEYTOK;
    sKey += sVarName;
    DeleteGdbmVector(sCampaignName,sKey);
}
void GDBM_DeleteCampaignLocation(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID)
{
    string sKey = GetPCPlayerName(oPlayer);
    if (sKey!="") sKey += NWNX_GDBM_KEYTOK+GetName(oPlayer)+NWNX_GDBM_KEYTOK;
    sKey += sVarName;
    DeleteGdbmLocation(sCampaignName,sKey);
}

void GDBM_DeleteCampaignVariable(string sCampaignName, string sVarName, object oPlayer=OBJECT_INVALID)
{
    string sKey = GetPCPlayerName(oPlayer);
    if (sKey!="") sKey += NWNX_GDBM_KEYTOK+GetName(oPlayer)+NWNX_GDBM_KEYTOK;
    sKey += sVarName;
    NWNX_GdbmOpen(sCampaignName);
    NWNX_GdbmDelete_String(sCampaignName,sKey);
    NWNX_GdbmDelete_Int(sCampaignName,sKey);
    NWNX_GdbmDelete_Float(sCampaignName,sKey);
    NWNX_GdbmDelete_Vector(sCampaignName,sKey);
    NWNX_GdbmDelete_Location(sCampaignName,sKey);
}

void GDBM_DestroyCampaignDatabase(string sCampaignName)
{
    NWNX_GdbmDestroy(sCampaignName);
}

