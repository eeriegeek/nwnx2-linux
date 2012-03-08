/******************************************************************************

  nwnx_gdbm_aps - NWNX - GDBM Plugin - APS/SQLDB compatibility layer.

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
  styled after the easy to use persistence functions provided in the file
  aps_include included with the nwnx_odbmc plugin. They are provided to allow
  easy switching between databases for testing purposes.

  NOTE: This file is really only provided for testing purposes. If you have
  a working system using the NWNX database bindings, there is likely no reason
  to consider switching to a GDBM based database!

  There is not a perfect correspondence in functionality between the two
  systems. GDBM is a very simple hash structure, and does not provide for data
  expiration so that parameter is ignored. The object parameter is also
  unneccessary for the GDBM methods and is ignored as well.

  TODO: Object handling is not yet implemented.

******************************************************************************/

void SetPersistentString(object oObject, string sVarName, string sValue, int iExpiration=0, string sTable="pwdata");
void SetPersistentInt(object oObject, string sVarName, int iValue, int iExpiration=0, string sTable="pwdata");
void SetPersistentFloat(object oObject, string sVarName, float fValue, int iExpiration=0, string sTable="pwdata");
void SetPersistentVector(object oObject, string sVarName, vector vVector, int iExpiration=0, string sTable="pwdata");
void SetPersistentLocation(object oObject, string sVarName, location lLocation, int iExpiration=0, string sTable="pwdata");

string GetPersistentString(object oObject, string sVarName, string sTable="pwdata");
int GetPersistentInt(object oObject, string sVarName, string sTable="pwdata");
float GetPersistentFloat(object oObject, string sVarName, string sTable="pwdata");
vector GetPersistentVector(object oObject, string sVarName, string sTable="pwdata");
location GetPersistentLocation(object oObject, string sVarName, string sTable="pwdata");

void DeletePersistentVariable(object oObject, string sVarName, string sTable = "pwdata");


//
// Implementation
//

void SetPersistentString(object oObject, string sVarName, string sValue, int iExpiration=0, string sTable="pwdata")
{
    SetGdbmString(sTable,sVarName,sValue);
}
void SetPersistentInt(object oObject, string sVarName, int iValue, int iExpiration=0, string sTable="pwdata")
{
    SetGdbmInt(sTable,sVarName,iValue);
}
void SetPersistentFloat(object oObject, string sVarName, float fValue, int iExpiration=0, string sTable="pwdata")
{
    SetGdbmFloat(sTable,sVarName,fValue);
}
void SetPersistentVector(object oObject, string sVarName, vector vVector, int iExpiration=0, string sTable="pwdata")
{
    SetGdbmVector(sTable,sVarName,vVector);
}
void SetPersistentLocation(object oObject, string sVarName, location lLocation, int iExpiration=0, string sTable="pwdata")
{
    SetGdbmLocation(sTable,sVarName,lLocation);
}
string GetPersistentString(object oObject, string sVarName, string sTable="pwdata")
{
    return(GetGdbmLocation(sTable,sVarName));
}
int GetPersistentInt(object oObject, string sVarName, string sTable="pwdata")
{
    return(GetGdbmInt(sTable,sVarName));
}
float GetPersistentFloat(object oObject, string sVarName, string sTable="pwdata")
{
    return(GetGdbmFloat(sTable,sVarName));
}
vector GetPersistentVector(object oObject, string sVarName, string sTable="pwdata")
{
    return(GetGdbmVector(sTable,sVarName));
}
location GetPersistentLocation(object oObject, string sVarName, string sTable="pwdata")
{
    return(GetGdbmLocation(sTable,sVarName));
}

void DeletePersistentVariable(object oObject, string sVarName, string sTable = "pwdata")
{
    NWNX_GdbmOpen(sTable);
    NWNX_GdbmDelete_String(sTable,sVarName);
    NWNX_GdbmDelete_Int(sTable,sVarName);
    NWNX_GdbmDelete_Float(sTable,sVarName);
    NWNX_GdbmDelete_Vector(sTable,sVarName);
    NWNX_GdbmDelete_Location(sTable,sVarName);
}

