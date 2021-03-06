MOD V1.0   F          ц   f  h   B   џџџџ                                                                                                                        >   This module shows how you can use APS/NWNX in your own module.aps_include         й  aps_onload         к  aps_onload         й  area001            м  area001            ў  area001            ч  creaturepalcus     ю  demo_createtable   к  demo_createtable   й  demo_loadvalue  	   к  demo_loadvalue  
   й  demo_storevalue    к  demo_storevalue    й  doorpalcus         ю  encounterpalcus    ю  go                 щ  h                  щ  itempalcus         ю  m                  щ  misc               щ  module             о  p                  щ  placeablepalcus    ю  Repute             і  soundpalcus        ю  storepalcus        ю  triggerpalcus      ю  waypointpalcus     ю                                                                                                                                                                                                                                  F  БD  їI  Є  K  t  M  )2  8  a    ѓ    ј     ѕ  yЂ  ш  aЇ  	  dА  Л  В  Е  дИ  Њ  ~К  Ќ  *Н  Ј  вП  У  Т  Р  UХ  Я  $ж  Р  фи  Щ  ­л  L  љт  №  щц  ќ  хы  a  Fѓ  и  і  ш  ј  l  rћ  ш  // Name     : Avlis Persistence System include
// Purpose  : Various APS/NWNX2 related functions
// Authors  : Ingmar Stieger, Adam Colon, Josh Simon
// Modified : February 16, 2003

// This file is licensed under the terms of the
// GNU GENERAL PUBLIC LICENSE (GPL) Version 2

/************************************/
/* Return codes                     */
/************************************/

int SQL_ERROR = 0;
int SQL_SUCCESS = 1;

/************************************/
/* Function prototypes              */
/************************************/

// Setup placeholders for ODBC requests and responses
void SQLInit();

// Execute statement in sSQL
void SQLExecDirect(string sSQL);

// Position cursor on next row of the resultset
// Call this before using SQLGetData().
// returns: SQL_SUCCESS if there is a row
//          SQL_ERROR if there are no more rows
int SQLFetch();

// * deprecated. Use SQLFetch instead.
// Position cursor on first row of the resultset and name it sResultSetName
// Call this before using SQLNextRow() and SQLGetData().
// returns: SQL_SUCCESS if result set is not empty
//          SQL_ERROR is result set is empty
int SQLFirstRow();

// * deprecated. Use SQLFetch instead.
// Position cursor on next row of the result set sResultSetName
// returns: SQL_SUCCESS if cursor could be advanced to next row
//          SQL_ERROR if there was no next row
int SQLNextRow();

// Return value of column iCol in the current row of result set sResultSetName
string SQLGetData(int iCol);

// Return a string value when given a location
string LocationToString(location lLocation);

// Return a location value when given the string form of the location
location StringToLocation(string sLocation);

// Return a string value when given a vector
string VectorToString(vector vVector);

// Return a vector value when given the string form of the vector
vector StringToVector(string sVector);

// Set oObject's persistent string variable sVarName to sValue
// Optional parameters:
//   iExpiration: Number of days the persistent variable should be kept in database (default: 0=forever)
//   sTable: Name of the table where variable should be stored (default: pwdata)
void SetPersistentString(object oObject, string sVarName, string sValue, int iExpiration=0, string sTable="pwdata");

// Set oObject's persistent integer variable sVarName to iValue
// Optional parameters:
//   iExpiration: Number of days the persistent variable should be kept in database (default: 0=forever)
//   sTable: Name of the table where variable should be stored (default: pwdata)
void SetPersistentInt(object oObject, string sVarName, int iValue, int iExpiration=0, string sTable="pwdata");

// Set oObject's persistent float variable sVarName to fValue
// Optional parameters:
//   iExpiration: Number of days the persistent variable should be kept in database (default: 0=forever)
//   sTable: Name of the table where variable should be stored (default: pwdata)
void SetPersistentFloat(object oObject, string sVarName, float fValue, int iExpiration=0, string sTable="pwdata");

// Set oObject's persistent location variable sVarName to lLocation
// Optional parameters:
//   iExpiration: Number of days the persistent variable should be kept in database (default: 0=forever)
//   sTable: Name of the table where variable should be stored (default: pwdata)
//   This function converts location to a string for storage in the database.
void SetPersistentLocation(object oObject, string sVarName, location lLocation, int iExpiration=0, string sTable="pwdata");

// Set oObject's persistent vector variable sVarName to vVector
// Optional parameters:
//   iExpiration: Number of days the persistent variable should be kept in database (default: 0=forever)
//   sTable: Name of the table where variable should be stored (default: pwdata)
//   This function converts vector to a string for storage in the database.
void SetPersistentVector(object oObject, string sVarName, vector vVector, int iExpiration=0, string sTable ="pwdata");

// Get oObject's persistent string variable sVarName
// Optional parameters:
//   sTable: Name of the table where variable is stored (default: pwdata)
// * Return value on error: ""
string GetPersistentString(object oObject, string sVarName, string sTable="pwdata");

// Get oObject's persistent integer variable sVarName
// Optional parameters:
//   sTable: Name of the table where variable is stored (default: pwdata)
// * Return value on error: 0
int GetPersistentInt(object oObject, string sVarName, string sTable="pwdata");

// Get oObject's persistent float variable sVarName
// Optional parameters:
//   sTable: Name of the table where variable is stored (default: pwdata)
// * Return value on error: 0
float GetPersistentFloat(object oObject, string sVarName, string sTable="pwdata");

// Get oObject's persistent location variable sVarName
// Optional parameters:
//   sTable: Name of the table where variable is stored (default: pwdata)
// * Return value on error: 0
location GetPersistentLocation(object oObject, string sVarname, string sTable="pwdata");

// Get oObject's persistent vector variable sVarName
// Optional parameters:
//   sTable: Name of the table where variable is stored (default: pwdata)
// * Return value on error: 0
vector GetPersistentVector(object oObject, string sVarName, string sTable = "pwdata");

// Delete persistent variable sVarName stored on oObject
// Optional parameters:
//   sTable: Name of the table where variable is stored (default: pwdata)
void DeletePersistentVariable(object oObject, string sVarName, string sTable="pwdata");

// (private function) Replace special character ' with ~
string SQLEncodeSpecialChars(string sString);

// (private function)Replace special character ' with ~
string SQLDecodeSpecialChars(string sString);

/************************************/
/* Implementation                   */
/************************************/

// Functions for initializing APS and working with result sets

void SQLInit()
{
    int i;
    object oMod = GetModule();

    // Placeholder for ODBC persistence
    string sMemory;

    for (i = 0; i < 8; i++) // reserve 8*128 bytes
        sMemory += "................................................................................................................................";

    SetLocalString(oMod, "NWNX!INIT", "1");
    SetLocalString(oMod, "NWNX!ODBC!SPACER", sMemory);
}

void SQLExecDirect(string sSQL)
{
    SetLocalString(GetModule(), "NWNX!ODBC!EXEC", sSQL);
}

int SQLFetch()
{
    string sRow;
    object oModule = GetModule();
    SetLocalString(oModule, "NWNX!ODBC!FETCH", GetLocalString(oModule, "NWNX!ODBC!SPACER"));
    sRow = GetLocalString(oModule, "NWNX!ODBC!FETCH");
    if (GetStringLength(sRow) > 0)
    {
        SetLocalString(oModule, "NWNX_ODBC_CurrentRow", sRow);
        return SQL_SUCCESS;
    }
    else
    {
        SetLocalString(oModule, "NWNX_ODBC_CurrentRow", "");
        return SQL_ERROR;
    }
}

// deprecated. use SQLFetch().
int SQLFirstRow()
{
    return SQLFetch();
}

// deprecated. use SQLFetch().
int SQLNextRow()
{
    return SQLFetch();
}

string SQLGetData(int iCol)
{
    int iPos;
    string sResultSet = GetLocalString(GetModule(), "NWNX_ODBC_CurrentRow");

    // find column in current row
    int iCount = 0;
    string sColValue = "";

    iPos = FindSubString(sResultSet, "Ќ");
    if ((iPos == -1) && (iCol == 1))
    {
        // only one column, return value immediately
        sColValue = sResultSet;
    }
    else if (iPos == -1)
    {
        // only one column but requested column > 1
        sColValue = "";
    }
    else
    {
        // loop through columns until found
        while (iCount != iCol)
        {
            iCount++;
            if (iCount == iCol)
                sColValue = GetStringLeft(sResultSet, iPos);
            else
            {
                sResultSet = GetStringRight(sResultSet,GetStringLength(sResultSet) - iPos - 1);
                iPos = FindSubString(sResultSet, "Ќ");
            }

            // special case: last column in row
            if (iPos == -1)
                iPos = GetStringLength(sResultSet);
        }
    }

    return sColValue;
}

// These functions deal with various data types. Ultimately, all information
// must be stored in the database as strings, and converted back to the proper
// form when retrieved.

string VectorToString(vector vVector)
{
    return "#POSITION_X#" + FloatToString(vVector.x) + "#POSITION_Y#" + FloatToString(vVector.y) + "#POSITION_Z#" + FloatToString(vVector.z) + "#END#";
}

vector StringToVector(string sVector)
{
    float fX, fY, fZ;
    int iPos, iCount;
    int iLen = GetStringLength(sVector);

    if (iLen > 0)
    {
        iPos = FindSubString(sVector, "#POSITION_X#") + 12;
        iCount = FindSubString(GetSubString(sVector, iPos, iLen - iPos), "#");
        fX = StringToFloat(GetSubString(sVector, iPos, iCount));

        iPos = FindSubString(sVector, "#POSITION_Y#") + 12;
        iCount = FindSubString(GetSubString(sVector, iPos, iLen - iPos), "#");
        fY = StringToFloat(GetSubString(sVector, iPos, iCount));

        iPos = FindSubString(sVector, "#POSITION_Z#") + 12;
        iCount = FindSubString(GetSubString(sVector, iPos, iLen - iPos), "#");
        fZ = StringToFloat(GetSubString(sVector, iPos, iCount));
    }

    return Vector(fX, fY, fZ);
}

string LocationToString(location lLocation)
{
    object oArea = GetAreaFromLocation(lLocation);
    vector vPosition = GetPositionFromLocation(lLocation);
    float fOrientation = GetFacingFromLocation(lLocation);
    string sReturnValue;

    if (GetIsObjectValid(oArea))
        sReturnValue = "#AREA#" + GetTag(oArea) + "#POSITION_X#" + FloatToString(vPosition.x) + "#POSITION_Y#" + FloatToString(vPosition.y) + "#POSITION_Z#" + FloatToString(vPosition.z) + "#ORIENTATION#" + FloatToString(fOrientation) + "#END#";

    return sReturnValue;
}

location StringToLocation(string sLocation)
{
    location lReturnValue;
    object oArea;
    vector vPosition;
    float fOrientation, fX, fY, fZ;

    int iPos, iCount;
    int iLen = GetStringLength(sLocation);

    if (iLen > 0)
    {
        iPos = FindSubString(sLocation, "#AREA#") + 6;
        iCount = FindSubString(GetSubString(sLocation, iPos, iLen - iPos), "#");
        oArea = GetObjectByTag(GetSubString(sLocation, iPos, iCount));

        iPos = FindSubString(sLocation, "#POSITION_X#") + 12;
        iCount = FindSubString(GetSubString(sLocation, iPos, iLen - iPos), "#");
        fX = StringToFloat(GetSubString(sLocation, iPos, iCount));

        iPos = FindSubString(sLocation, "#POSITION_Y#") + 12;
        iCount = FindSubString(GetSubString(sLocation, iPos, iLen - iPos), "#");
        fY = StringToFloat(GetSubString(sLocation, iPos, iCount));

        iPos = FindSubString(sLocation, "#POSITION_Z#") + 12;
        iCount = FindSubString(GetSubString(sLocation, iPos, iLen - iPos), "#");
        fZ = StringToFloat(GetSubString(sLocation, iPos, iCount));

        vPosition = Vector(fX, fY, fZ);

        iPos = FindSubString(sLocation, "#ORIENTATION#") + 13;
        iCount = FindSubString(GetSubString(sLocation, iPos, iLen - iPos), "#");
        fOrientation = StringToFloat(GetSubString(sLocation, iPos, iCount));

        lReturnValue = Location(oArea, vPosition, fOrientation);
    }

    return lReturnValue;
}

// These functions are responsible for transporting the various data types back
// and forth to the database.

void SetPersistentString(object oObject, string sVarName, string sValue, int iExpiration=0, string sTable="pwdata")
{
    string sPlayer;
    string sTag;

    if (GetIsPC(oObject))
    {
        sPlayer = SQLEncodeSpecialChars(GetPCPlayerName(oObject));
        sTag = SQLEncodeSpecialChars(GetName(oObject));
    }
    else
    {
        sPlayer = "~";
        sTag = GetTag(oObject);
    }

    sVarName = SQLEncodeSpecialChars(sVarName);
    sValue = SQLEncodeSpecialChars(sValue);

    string sSQL = "SELECT player FROM " + sTable + " WHERE player='" + sPlayer +
                  "' AND tag='" + sTag + "' AND name='" + sVarName + "'";
    SQLExecDirect(sSQL);

    if (SQLFetch() == SQL_SUCCESS)
    {
        // row exists
        sSQL = "UPDATE " + sTable + " SET val='" + sValue +
               "',expire=" + IntToString(iExpiration) + " WHERE player='"+ sPlayer +
               "' AND tag='" + sTag + "' AND name='" + sVarName + "'";
        SQLExecDirect(sSQL);
    }
    else
    {
        // row doesn't exist
        sSQL = "INSERT INTO " + sTable + " (player,tag,name,val,expire) VALUES" +
               "('" + sPlayer + "','" + sTag + "','" + sVarName + "','" +
               sValue + "'," + IntToString(iExpiration) + ")";
        SQLExecDirect(sSQL);
    }
}

string GetPersistentString(object oObject, string sVarName, string sTable="pwdata")
{
    string sPlayer;
    string sTag;

    if (GetIsPC(oObject))
    {
        sPlayer = SQLEncodeSpecialChars(GetPCPlayerName(oObject));
        sTag = SQLEncodeSpecialChars(GetName(oObject));
    }
    else
    {
        sPlayer = "~";
        sTag = GetTag(oObject);
    }

    sVarName = SQLEncodeSpecialChars(sVarName);

    string sSQL = "SELECT val FROM " + sTable + " WHERE player='" + sPlayer +
               "' AND tag='" + sTag + "' AND name='" + sVarName + "'";
    SQLExecDirect(sSQL);

    if (SQLFetch() == SQL_SUCCESS)
        return SQLDecodeSpecialChars(SQLGetData(1));
    else
    {
        return "";
        // If you want to convert your existing persistent data to APS, this
        // would be the place to do it. The requested variable was not found
        // in the database, you should
        // 1) query it's value using your existing persistence functions
        // 2) save the value to the database using SetPersistentString()
        // 3) return the string value here.
    }
}

void SetPersistentInt(object oObject, string sVarName, int iValue, int iExpiration=0, string sTable="pwdata")
{
    SetPersistentString(oObject, sVarName, IntToString(iValue), iExpiration, sTable);
}

int GetPersistentInt(object oObject, string sVarName, string sTable="pwdata")
{
    return StringToInt(GetPersistentString(oObject, sVarName, sTable));
}

void SetPersistentFloat(object oObject, string sVarName, float fValue, int iExpiration=0, string sTable="pwdata")
{
    SetPersistentString(oObject, sVarName, FloatToString(fValue), iExpiration, sTable);
}

float GetPersistentFloat(object oObject, string sVarName, string sTable="pwdata")
{
    return StringToFloat(GetPersistentString(oObject, sVarName, sTable));
}

void SetPersistentLocation(object oObject, string sVarName, location lLocation, int iExpiration=0, string sTable="pwdata")
{
    SetPersistentString(oObject, sVarName, LocationToString(lLocation), iExpiration, sTable);
}

location GetPersistentLocation(object oObject, string sVarName, string sTable="pwdata")
{
    return StringToLocation(GetPersistentString(oObject, sVarName, sTable));
}

void SetPersistentVector(object oObject, string sVarName, vector vVector, int iExpiration=0, string sTable ="pwdata")
{
    SetPersistentString(oObject, sVarName, VectorToString(vVector), iExpiration, sTable);
}

vector GetPersistentVector(object oObject, string sVarName, string sTable = "pwdata")
{
    return StringToVector(GetPersistentString(oObject, sVarName, sTable));
}

void DeletePersistentVariable(object oObject, string sVarName, string sTable="pwdata")
{
    string sPlayer;
    string sTag;

    if (GetIsPC(oObject))
    {
        sPlayer = SQLEncodeSpecialChars(GetPCPlayerName(oObject));
        sTag = SQLEncodeSpecialChars(GetName(oObject));
    }
    else
    {
        sPlayer = "~";
        sTag = GetTag(oObject);
    }

    sVarName = SQLEncodeSpecialChars(sVarName);
    string sSQL = "DELETE FROM " + sTable + " WHERE player='" + sPlayer +
                  "' AND tag='" + sTag + "' AND name='" + sVarName + "'";
    SQLExecDirect(sSQL);
}

// Problems can arise with SQL commands if variables or values have single quotes
// in their names. These functions are a replace these quote with the tilde character

string SQLEncodeSpecialChars(string sString)
{
    if (FindSubString(sString, "'") == -1) // not found
        return sString;

    int i;
    string sReturn = "";
    string sChar;

    // Loop over every character and replace special characters
    for (i = 0; i < GetStringLength(sString); i++)
    {
        sChar = GetSubString(sString, i, 1);
        if (sChar == "'")
            sReturn += "~";
        else
            sReturn += sChar;
    }
    return sReturn;
}

string SQLDecodeSpecialChars(string sString)
{
    if (FindSubString(sString, "~") == -1) // not found
        return sString;

    int i;
    string sReturn = "";
    string sChar;

    // Loop over every character and replace special characters
    for (i = 0; i < GetStringLength(sString); i++)
    {
        sChar = GetSubString(sString, i, 1);
        if (sChar == "~")
            sReturn += "'";
        else
            sReturn += sChar;
    }
    return sReturn;
}
NCS V1.0B  Є          џџџј  џџџќ   џџџј  џџџќ*     +  џџџј          ђ џџџј  џџџќ    џџџ№  џџџќџџџє         Мџџџќ  ................................................................................................................................#џџџј  џџџќџџџє $џџџ№ џџџќ џџџ: 1 	NWNX!INITџџџ№   9џџџќ  NWNX!ODBC!SPACERџџџ№   9 џџџє  // Name     : Avlis Persistence System OnModuleLoad
// Purpose  : Initialize APS on module load event
// Authors  : Ingmar Stieger
// Modified : January 27, 2003

// This file is licensed under the terms of the
// GNU GENERAL PUBLIC LICENSE (GPL) Version 2

#include "aps_include"

void main()
{
    // Init placeholders for ODBC gateway
    SQLInit();
}

ARE V3.28   A   D  Ј  $#  2   D&  =   &   
  !1    џџџџ    (          
      Ш   
      №   
        
      @  
      h  
        
      И  
      р  
        
      0  
      X  
        
      Ј  
      а  
      ј  
         
      H  
      p  
        
      Р  
      ш  
        
      8  
      `  
        
      А  
      и  
         
      (  
      P  
      x  
         
      Ш  
      №  
        
      @  
      h  
        
      И  
      р  
        
      0  
      X  
        
      Ј  
      а  
      ј  
         
      H  
      p  
        
      Р  
      ш  
      	  
      8	  
      `	  
      	  
      А	  
      и	  
       
  
      (
  
      P
  
      x
  
          џџџџ      џџџџ         
                         '   
      /                         	          
                       ddШ                  22d                 22d       џџџ                  h~                                        2                                                                                                 !         "   3      #   4      $   5      %   6      &   7      '         (         )          *           +           ,           -           .           /          0          1         (         )         *           +           ,           -           .           /          0          1         (         )          *           +           ,           -           .           /          0          1         (         )         *           +           ,           -           .           /          0          1         (         )         *           +           ,           -           .           /          0          1         (         )         *           +           ,           -           .           /          0          1         (         )          *           +           ,           -           .           /          0          1         (         )         *           +           ,           -           .           /          0          1         (         )         *           +           ,           -           .           /          0          1         (         )         *           +           ,           -           .           /          0          1         (         )         *           +           ,           -           .           /          0          1         (         )         *           +           ,           -           .           /          0          1         (         )         *           +           ,           -           .           /          0          1         (         )         *           +           ,           -           .           /          0          1         (         )          *           +           ,           -           .           /          0          1         (         )         *           +           ,           -           .           /          0          1         (         )         *           +           ,           -           .           /          0          1         (         )         *           +           ,           -           .           /          0          1         (         )         *           +           ,           -           .           /          0          1         (         )          *           +           ,           -           .           /          0          1         (         )         *           +           ,           -           .           /          0          1         (         )         *           +           ,           -           .           /          0          1         (         )         *           +           ,           -           .           /          0          1         (         )         *           +           ,           -           .           /          0          1         (         )         *           +           ,           -           .           /          0          1         (         )         *           +           ,           -           .           /          0          1         (         )         *           +           ,           -           .           /          0          1         (         )         *           +           ,           -           .           /          0          1         (         )         *           +           ,           -           .           /          0          1         (         )          *           +           ,           -           .           /          0          1         (         )          *           +           ,           -           .           /          0          1         (         )          *           +           ,           -           .           /          0          1         (         )         *           +           ,           -           .           /          0          1         (         )         *           +           ,           -           .           /          0          1         (         )          *           +           ,           -           .           /          0          1         (         )         *           +           ,           -           .           /          0          1         (         )         *           +           ,           -           .           /          0          1         (         )         *           +           ,           -           .           /          0          1         (         )          *           +           ,           -           .           /          0          1         (         )          *           +           ,           -           .           /          0          1         (         )          *           +           ,           -           .           /          0          1         (         )         *           +           ,           -           .           /          0          1         (         )         *           +           ,           -           .           /          0          1         (         )         *           +           ,           -           .           /          0          1         (         )         *           +           ,           -           .           /          0          1         (         )         *           +           ,           -           .           /          0          1         (         )         *           +           ,           -           .           /          0          1         (         )         *           +           ,           -           .           /          0          1         (         )          *           +           ,           -           .           /          0          1         (         )         *           +           ,           -           .           /          0          1         (         )         *           +           ,           -           .           /          0          1         (         )         *           +           ,           -           .           /          0          1         (         )          *           +           ,           -           .           /          0          1         (         )         *           +           ,           -           .           /          0          1         (         )          *           +           ,           -           .           /          0          1         (         )          *           +           ,           -           .           /          0          1         (         )         *           +           ,           -           .           /          0          1         (         )         *           +           ,           -           .           /          0          1         (         )          *           +           ,           -           .           /          0          1         (         )          *           +           ,           -           .           /          0          1         (         )          *           +           ,           -           .           /          0          1         (         )         *           +           ,           -           .           /          0          1         (         )          *           +           ,           -           .           /          0          1         (         )         *           +           ,           -           .           /          0          1      ID              Creator_ID      Version         Tag             Name            ResRef          Comments        Expansion_List  Flags           ModSpotCheck    ModListenCheck  MoonAmbientColorMoonDiffuseColorMoonFogAmount   MoonFogColor    MoonShadows     SunAmbientColor SunDiffuseColor SunFogAmount    SunFogColor     SunShadows      IsNight         LightingScheme  ShadowOpacity   DayNightCycle   ChanceRain      ChanceSnow      ChanceLightning WindPower       LoadScreenID    PlayerVsPlayer  NoRest          Width           Height          OnEnter         OnExit          OnHeartbeat     OnUserDefined   Tileset         Tile_List       Tile_ID         Tile_OrientationTile_Height     Tile_MainLight1 Tile_MainLight2 Tile_SrcLight1  Tile_SrcLight2  Tile_AnimLoop1  Tile_AnimLoop2  Tile_AnimLoop3     Area001   џџџџ          Area 001area001        tms01                            	   
                                                                      !   "   #   $   %   &   '   (   )   *   +   ,   -   .   /   0   1   2   3   4   5   6   7   8   9   :   ;   <   =   >   ?   @   A   B   C   D   E   F   G   H   I   J   K   L   M   N   O   P   Q   R   S   T   U   V   W   X   Y   Z   [   \   ]   ^   _   `   a   b   c   d   e   f   g   h   i   j   k   l   m   n   o   p   q   r   s   t   u   v   w   x   y   z   {   |   }   ~                                                                                                          Ё   Ђ   Ѓ   Є   Ѕ   І   Ї   Ј   Љ   Њ   Ћ   Ќ   ­   Ў   Џ   А   Б   В   Г   Д   Е   Ж   З   И   Й   К   Л   М   Н   О   П   Р   С   Т   У   Ф   Х   Ц   Ч   Ш   Щ   Ъ   Ы   Ь   Э   Ю   Я   а   б   в   г   д   е   ж   з   и   й   к   л   м   н   о   п   р   с   т   у   ф   х   ц   ч   ш   щ   ъ   ы   ь   э   ю   я   №   ё   ђ   ѓ   є   ѕ   і   ї   ј   љ   њ   ћ   ќ   §   ў   џ                      	  
                                               !  "  #  $  %  &  '  (  )  *  +  ,  -  .  /  0  1  2  3  4  5  6  7  8  9  :  ;  <  =  >  ?  @  A  B  C  D  E  F  G  H  I  J  K  L  M  N  O  P  Q  R  S  T  U  V  W  X  Y  Z  [  \  ]  ^  _  `  a  b  c  d  e  f  g  h  i  j  k  l  m  n  o  p  q  r  s  t  u  v  w  x  y  z  {  |  }  ~                                                                       Ё  Ђ  Ѓ  Є  Ѕ  І  Ї  Ј  Љ  Њ  Ћ  Ќ  ­  Ў  Џ  А  Б  В  Г  Д  Е  Ж  З  И  Й  К  Л  М  Н  О  П  Р  С  Т  У  Ф  Х  Ц  Ч  Ш  Щ  Ъ  Ы  Ь  Э  Ю  Я  а  б  в  г  д  е  ж  з  и  й  к  л  м  н  о  п  р  с  т  у  ф  х  ц  ч  ш  щ  ъ  ы  ь  э  ю  я  №  ё  ђ  ѓ  є  ѕ  і  ї  ј  љ  њ  ћ  ќ  §  ў  џ                     	  
                                               !  "  #  $  %  &  '  (  )  *  +  ,  -  .  /  0  1  2  3  4  5  6  7  8  9  :  ;  <  =  >  ?  @  A  B  C  D  E  F  G  H  I  J  K  L  M  N  O  P  Q  R  S  T  U  V  W  X  Y  Z  [  \  ]  ^  _  `  a  b  c  d  e  f  g  h  i  j  k  l  m  n  o  p  q  r  s  t  u  v  w  x  y  z  {  |  }  ~                                                                       Ё  Ђ  Ѓ  Є  Ѕ  І  Ї      @                           	   
                                                                      !   "   #   $   %   &   '   (   )   *   +   ,   -   .   /   0   1   2   3   4   5   6   7   8   9   :   ;   <   =   >   ?   @   GIC V3.28      h      ј   
     u     $   1  0   џџџџ    	   	   	      	   
      	                                                                                             
   	       
   	   '   
   	   N   Creature List   Door List       Encounter List  List            SoundList       StoreList       TriggerList     WaypointList    Placeable List  Comment         #   Freestanding Merchant's Placard - 1#   Freestanding Merchant's Placard - 3#   Freestanding Merchant's Placard - 4                                                                        GIT V3.28      t   Е   №  I     o  я  д  У  0   џџџџ$   
   d       	   	   L   6   	   $  6   	   ќ  6                                                                   "                        	   _    
                                                                                
                !         S         Ѓ                               А                                                                                !   щ      "          #           $          %          &           '          (       
   )   Б       *          +         ,         -          .          /          0           1          2   Е      3   Ж      4   З      5   И      6   Й      7   К      8   Л      9   М      :   Н      ;   О      <   П      =   Р       >           ?           @           A           B         C   С      D   Т      E   3B   F   ЁI9B   G      7   H   ЯЩ=
      г         є         "        r                                                                                                             !   ъ      "          #           $          %          &           '          (       
   )         *          +         ,         -          .          /          0           1          2        3        4        5        6        7        8        9        :        ;        <        =         >           ?           @           A           B         C        D        E   ,&B   F   Њ9B   G      7   H   эЩ=
      Ё        Т        ё        A                              N                                                                               !   ы      "          #           $          %          &           '          (       
   )   O      *          +         ,         -          .          /          0           1          2   S     3   T     4   U     5   V     6   W     7   X     8   Y     9   Z     :   [     ;   \     <   ]     =   ^      >           ?           @           A           B         C   _     D   `     E   I":B   F   фџ9B   G      7   H   ЁЩНAreaProperties  AmbientSndDay   AmbientSndNight AmbientSndDayVolAmbientSndNitVolEnvAudio        MusicBattle     MusicDay        MusicNight      MusicDelay      Creature List   Door List       Encounter List  List            SoundList       StoreList       TriggerList     WaypointList    Placeable List  Tag             LocName         Description     TemplateResRef  AutoRemoveKey   CloseLockDC     Conversation    Interruptable   Faction         Plot            KeyRequired     Lockable        Locked          OpenLockDC      PortraitId      TrapDetectable  TrapDetectDC    TrapDisarmable  DisarmDC        TrapFlag        TrapOneShot     TrapType        KeyName         AnimationState  Appearance      HP              CurrentHP       Hardness        Fort            Ref             Will            OnClosed        OnDamaged       OnDeath         OnDisarm        OnHeartbeat     OnLock          OnMeleeAttacked OnOpen          OnSpellCastAt   OnTrapTriggered OnUnlock        OnUserDefined   HasInventory    BodyBag         Static          Type            Useable         OnInvDisturbed  OnUsed          X               Y               Z               Bearing            FreestandingMerchantsPlacard2.   `9            Create database table "pwdata"L   _9         <   A carefully constructed marker denoting a point of interest.plc_placard1                  demo_createtable   FreestandingMerchantsPlacard3*   `9            Store variable in databaseL   b9         <   A carefully constructed marker denoting a point of interest.plc_placard3                  demo_storevalue   FreestandingMerchantsPlacard4+   `9            Load variable from databaseL   c9         <   A carefully constructed marker denoting a point of interest.plc_placard4                  demo_loadvalue                        	       
                                                                      !   "   #   $   %   &   '   (   )   *   +   ,   -   .   /   0   1   2   3   4   5   6   7   8   9   :   ;   <   =   >   ?   @   A   B   C   D   E   F   G   H   I   J   K   L   M   N   O   P   Q   R   S   T   U   V   W   X   Y   Z   [   \   ]   ^   _   `   a   b   c   d   e   f   g   h   i   j   k   l   m   n   o   p   q   r   s   t   u   v   w   x   y   z   {   |   }   ~                                                                                                          Ё   Ђ   Ѓ   Є   Ѕ   І   Ї   Ј   Љ   Њ   Ћ   Ќ   ­   Ў   Џ   А   Б   В   Г   Д                                               ITP V3.28   =     y   Р      	       	  р  р
    џџџџ                                                               (          0          8          @          H          P          X          `          h          p          x                                                             Ј          А          И          Р          Ш          а          и          р          ш          №          ј                                                         (         0         8         @         H         P         X         `         h         p         x                                                       Ј         А         И         Р         Ш         а         и                      %                 ї         0         &        L         '                  (                  )                  *                  Щ          	         8                  9                  :                          d         G         "         H         #         I         $         J         %         1        x         2                  3                  4                  5                  6                                    Щ          2         ,                 -         
         .                  ї         1         8                  ;        Є         <                  =                  >                  +         /         ?                  /                  #        М         
                  B                  Щ                   D                  C                  k                   E         !         K        м                   &                   '                   (                   *                   )         !          +         #          ,         Щ          -                                              !                  "                  #                  $                  L         .   MAIN            STRREF          LIST            ID                                      	   
                                                                      !   "   #   $   %   &   '   (   )   *   +   ,   -   .   /   0   1   2   3   4   5   6   7   8   9   :   ;   <   =   >   ?   @   A   B   C   D   E   F   G   H   I   J   K   L   M   N   O   P   Q   R   S   T   U   V   W   X   Y   Z   [   \   ]   ^   _   `   a   b   c   d   e   f   g   h   i   j   k   l   m   n   o   p   q   r   s   t   u   v   w   x         -   6   <            	   
                        $   %                                                                               !   "   #      &   '   (   )   *   +   ,      .   /   0   1   2   3   4   5      7   8   9   :   ;   NCS V1.0B  ѕ          џџџј  џџџќ   џџџј  џџџќ*     +  џџџј   DROP TABLE pwdata   a Table 'pwdata' deleted. J  v CREATE TABLE pwdata (  player VARCHAR(64) default NULL,# tag VARCHAR(64) default NULL,# name VARCHAR(64) default NULL,# 	val TEXT,# &expire SMALLINT UNSIGNED default NULL,# last TIMESTAMP(14) NOT NULL,# KEY idx (player,tag,name)# )#    - Table 'pwdata' created. J  v  џџџќ  NWNX!ODBC!EXEC  ђ   9 џџџќ  // Name     : Demo create table
// Purpose  : Create a table for persistent data
// Authors  : Ingmar Stieger
// Modified : January 30, 2003

// This file is licensed under the terms of the
// GNU GENERAL PUBLIC LICENSE (GPL) Version 2

#include "aps_include"

void main()
{
    SQLExecDirect("DROP TABLE pwdata");
    SendMessageToPC(GetLastUsedBy(), "Table 'pwdata' deleted.");


    // For Access
    /*
    SQLExecDirect("CREATE TABLE pwdata (" +
                    "player text(64)," +
                    "tag text(64)," +
                    "name text(64)," +
                    "val memo," +
                    "expire text(4)," +
                    "last date)");
    */
    // Example for MySQL

    SQLExecDirect("CREATE TABLE pwdata (" +
                    "player VARCHAR(64) default NULL," +
                    "tag VARCHAR(64) default NULL," +
                    "name VARCHAR(64) default NULL," +
                    "val TEXT," +
                    "expire SMALLINT UNSIGNED default NULL," +
                    "last TIMESTAMP(14) NOT NULL," +
                    "KEY idx (player,tag,name)" +
                    ")" );

    SendMessageToPC(GetLastUsedBy(), "Table 'pwdata' created.");
}
NCS V1.0B  	          џџџј  џџџќ   џџџј  џџџќ*     +  џџџј   pwdata demoName J     Vџџџј  џџџќ "Retrieved variable from database: џџџј # J  v џџџќ  џџџє   й    Rџџџ№  s    џџџє  џџџќџџџ№   §   }џџџј  џџџќ    6-  ~џџџє  џџџќџџџє   Јџџџј  џџџќџџџь    )џџџь  џџџќ SELECT val FROM џџџф #  WHERE player='#џџџ№ # ' AND tag='#џџџє # ' AND name='#џџџш # '#џџџј  џџџќџџџќ    т   'џџџќ      <      1   iџџџр  џџџ№    8 џџџќ    &-   џџџр  џџџ№     џџџќ џџџє џџџє   'џџџј   B        .џџџќ џџџє  џџџќ     џџџќ      џџџј  џџџќ    џџџ№  џџџќџџџє џџџь   ;     Ѓ   џџџ№ џџџш   Aџџџј  џџџќџџџќ  '#    )џџџј  ~#џџџє  џџџќ    (- џџџј џџџј #џџџє  џџџќџџџє $џџџ№ џџџќ џџџLџџџј џџџш  џџџ№     џџџќ џџџє џџџќ  џџџќ  NWNX!ODBC!EXEC  ђ   9 џџџќ    ђ џџџј  џџџќ NWNX!ODBC!SPACERџџџј   5 NWNX!ODBC!FETCHџџџє   9 NWNX!ODBC!FETCHџџџј   5џџџє  џџџќџџџј   ;         [џџџј  NWNX_ODBC_CurrentRowџџџє   9'џџџќ џџџ№  џџџє    e џџџќ    S-    NWNX_ODBC_CurrentRowџџџє   9'џџџј џџџ№  џџџє     џџџќ џџџј   NWNX_ODBC_CurrentRow  ђ   5џџџј  џџџќ    џџџј  џџџќ  џџџј  џџџќ Ќџџџ№   Bџџџь  џџџќџџџ№     џџџќ     џџџш          "џџџє џџџј  џџџќ   B- џџџ№           џџџј  џџџќ   - џџџј џџџш      іџџџј $џџџє џџџќџџџј џџџш      /џџџ№ џџџ№   ?џџџј  џџџќ    b- џџџє   ;џџџь      џџџ№   >џџџ№  џџџќ Ќџџџ№   Bџџџь  џџџќџџџ№         'џџџє   ;џџџь  џџџќ     џџўўџџџќ џџџф  џџџь     џџџќ џџџ№ џџџќ   ~џџџј   B        .џџџќ џџџє  џџџќ     џџџќ      џџџј  џџџќ    џџџ№  џџџќџџџє џџџь   ;     Ѓ   џџџ№ џџџш   Aџџџј  џџџќџџџќ  ~#    )џџџј  '#џџџє  џџџќ    (- џџџј џџџј #џџџє  џџџќџџџє $џџџ№ џџџќ џџџLџџџј џџџш  џџџ№     џџџќ џџџє џџџќ  // Name     : Demo load value
// Purpose  : Load a value from the database
// Authors  : Ingmar Stieger
// Modified : January 27, 2003

// This file is licensed under the terms of the
// GNU GENERAL PUBLIC LICENSE (GPL) Version 2

#include "aps_include"

void main()
{
    string sString = GetPersistentString(GetLastUsedBy(), "demoName");
    SendMessageToPC(GetLastUsedBy(), "Retrieved variable from database: " + sString);
}
NCS V1.0B  Е          џџџј  џџџќ   џџџј  џџџќ*     +  џџџј   pwdata     	testValue demoName J     5 Stored 'testValue' in database. J  v  џџџє   й    Rџџџ№  s   џџџє  џџџќџџџ№   §   пџџџј  џџџќ    6-  ~џџџє  џџџќџџџє   Јџџџј  џџџќџџџь    џџџь  џџџќџџџш    mџџџш  џџџќ SELECT player FROM џџџм #  WHERE player='#џџџ№ # ' AND tag='#џџџє # ' AND name='#џџџш # '#џџџј  џџџќџџџќ    #   G'џџџќ      в UPDATE џџџм # 
 SET val='#џџџф # 	',expire=#џџџр   \#  WHERE player='#џџџ№ # ' AND tag='#џџџє # ' AND name='#џџџш # '#џџџј  џџџќџџџќ    E    б-  INSERT INTO џџџм # $ (player,tag,name,val,expire) VALUES# ('#џџџ№ # ','#џџџє # ','#џџџш # ','#џџџф # ',#џџџр   \# )#џџџј  џџџќџџџќ    t џџџє џџџь   'џџџј   B        .џџџќ џџџє  џџџќ     џџџќ      џџџј  џџџќ    џџџ№  џџџќџџџє џџџь   ;     Ѓ   џџџ№ џџџш   Aџџџј  џџџќџџџќ  '#    )џџџј  ~#џџџє  џџџќ    (- џџџј џџџј #џџџє  џџџќџџџє $џџџ№ џџџќ џџџLџџџј џџџш  џџџ№     џџџќ џџџє џџџќ  џџџќ  NWNX!ODBC!EXEC  ђ   9 џџџќ    ђ џџџј  џџџќ NWNX!ODBC!SPACERџџџј   5 NWNX!ODBC!FETCHџџџє   9 NWNX!ODBC!FETCHџџџј   5џџџє  џџџќџџџј   ;         [џџџј  NWNX_ODBC_CurrentRowџџџє   9'џџџќ џџџ№  џџџє    e џџџќ    S-    NWNX_ODBC_CurrentRowџџџє   9'џџџј џџџ№  џџџє     џџџќ џџџј  // Name     : Demo store value
// Purpose  : Store a value in the database
// Authors  : Ingmar Stieger
// Modified : January 27, 2003

// This file is licensed under the terms of the
// GNU GENERAL PUBLIC LICENSE (GPL) Version 2

#include "aps_include"

void main()
{
    SetPersistentString(GetLastUsedBy(), "demoName", "testValue");
    SendMessageToPC(GetLastUsedBy(), "Stored 'testValue' in database.");
}
ITP V3.28      Ш      м             X   t  8   џџџџ                                                               (          0          8          @          H          P                                                            !                  "                  #                  $                  N                  O        (         P                  Q                  R            MAIN            STRREF          LIST            ID                                      	   
                                                                        	   
      ITP V3.28      Ш      м             X   t  4   џџџџ                                                               (          0          8          @          H          P                       Њ                  Ћ         	         б                  Љ                                                       !                  "                  #                  $                  Ї            MAIN            STRREF          ID              LIST                                    	   
                                                                        	   
   UTI V3.28      D           <  ;   w  H   П     џџџџ                        L                           %   
      1                                      	   PУ      
                                                                    
      7   TemplateResRef  BaseItem        LocalizedName   Description     DescIdentified  Tag             Charges         Cost            Stolen          StackSize       Plot            AddCost         Identified      Cursed          ModelPart1      PropertiesList  PaletteID       Comment         go   џџџџ          go   џџџџ       џџџџ       go                                	   
                            UTI V3.28      D           <  8   t  H   М     џџџџ                                                   #   
      /                                     	          
                                                                    
      4   TemplateResRef  BaseItem        LocalizedName   Description     DescIdentified  Tag             Charges         Cost            Stolen          StackSize       Plot            AddCost         Identified      Cursed          ModelPart1      PropertiesList  PaletteID       Comment         h   џџџџ          h   џџџџ       џџџџ       h                                	   
                            ITP V3.28   T   (  Љ        t  +        ?    џџџџ                                                    $          ,          4          <          D          L          T          \          d          l          t          |                                                  Є          Ќ          Д          М          Ф          Ь          д          м          ф          ь          є          ќ                                              $         ,         4         <         D         L         X         `         h         p         x                                                       Ј         А         И         Р         Ш         а         и         р         ш         №         ј                                                        (         0         8         @         H         P         X         `         h         p         x                                                          O                  К                   @   
                   
      	            
                  
                        в                  S         	         Я                                             :         T        T         І                  U         
         V                  W        d                  7         X                  Џ         ?                  ;                                    8         8                          <         К         Ј         Y                  №                  Z                  [                  њ                  F        @                  9         ]        Р         ^                  _                  \                  +                  a                  b                           6                 Ь                             ф   
               &         !                  "                  #                  $                  L         5         є        ь         d                e                  f                  g                  ъ        (        j                   h                  i                  k        8        l         !         m         "         n         #         o         $         +         %         p         &         ы        T        q         '         r         (         s         )         t         *         Ё         =         w         .         x         /         y        l        z         0         {         1         |         2         щ         3         Ђ        |        v         +         Ѓ         ,         Є         -         Ѕ         >         ю         4   MAIN            STRREF          LIST            ID              NAME            RESREF             gogo   hh   mm   pp   miscmisc                        	   
                                                                      !   "   #   $   %   &   '   (   )   *   +   ,   -   .   /   0   1   2   3   4   5   6   7   8   9   :   ;   <   =   >   ?   @   A   B   C   D   E   F   G   H   I   J   K   L   M   N   O   P   Q   R   S   T   U   V   W   X   Y   Z   [   \   ]   ^   _   `   a   b   c   d   e   f   g   h   i   j   k   l   m   n   o   p   q   r   s   t   u   v   w   x   y   z   {   |   }   ~                                                                                                          Ё   Ђ   Ѓ   Є   Ѕ   І   Ї   Ј               (   )   0   1               	   
                                                         	                !   $   %   &   '                        "   #      *   ,   -   .   /      +   
   2   6   :   A   G   H   I   M   N   S      3   4   5      7   8   9      ;   <   =   >   ?   @      B   C   D   E   F      J   K   L      O   P   Q   R   UTI V3.28      D           <  8   t  H   М     џџџџ                                                   #   
      /                                      	          
                                                                    
      4   TemplateResRef  BaseItem        LocalizedName   Description     DescIdentified  Tag             Charges         Cost            Stolen          StackSize       Plot            AddCost         Identified      Cursed          ModelPart1      PropertiesList  PaletteID       Comment         m   џџџџ          m   џџџџ       џџџџ       m                                	   
                            UTI V3.28      D           <  A   }  H   Х     џџџџ                                                   )   
      5                                      	          
                                                                     
      =   TemplateResRef  BaseItem        LocalizedName   Description     DescIdentified  Tag             Charges         Cost            Stolen          StackSize       Plot            AddCost         Identified      Cursed          ModelPart1      PropertiesList  PaletteID       Comment         misc   џџџџ          misc   џџџџ       џџџџ       misc                                	   
                            IFO V3.28      P   1     1   Ќ  Ф   p  Р   0     џџџџ    0      .                 
                                              
      8         B              
   	         
            ћГB      ЖB         7      Џ
ЩМ      Cь?                                                                            \         
                   Ё         Ќ         ­         Ў         Џ         А          Б      !   В      "   Г      #   Д      $   Е      %   Ж      &   З      '   И      (   Й      )   К      *   Л      +         ,         -         .   М      /         0      Mod_ID          Mod_MinGameVer  Mod_Creator_ID  Mod_Version     Expansion_Pack  Mod_Name        Mod_Tag         Mod_Description Mod_IsSaveGame  Mod_CustomTlk   Mod_Entry_Area  Mod_Entry_X     Mod_Entry_Y     Mod_Entry_Z     Mod_Entry_Dir_X Mod_Entry_Dir_Y Mod_Expan_List  Mod_DawnHour    Mod_DuskHour    Mod_MinPerHour  Mod_StartMonth  Mod_StartDay    Mod_StartHour   Mod_StartYear   Mod_XPScale     Mod_OnHeartbeat Mod_OnModLoad   Mod_OnModStart  Mod_OnClientEntrMod_OnClientLeavMod_OnActvtItem Mod_OnAcquirItemMod_OnUsrDefinedMod_OnUnAqreItemMod_OnPlrDeath  Mod_OnPlrDying  Mod_OnPlrEqItm  Mod_OnPlrLvlUp  Mod_OnSpawnBtnDnMod_OnPlrRest   Mod_OnPlrUnEqItmMod_OnCutsnAbortMod_StartMovie  Mod_CutSceneListMod_GVar_List   Mod_Area_list   Area_Name       Mod_HakList     Mod_CacheNSSList   c!ЊKЇъ*ЇњЏю   1.60   џџџџ          aps_demo   MODULEN   џџџџ       >   This module shows how you can use APS/NWNX in your own module.    area001 
aps_onload                area001                            	   
                                                                      !   "   #   $   %   &   '   (   )   *   +   ,   -   /   0                             UTI V3.28      P           D  8   |  l   ш     џџџџ           P                       1                           #   
      /                                    	   
       
                                                                                                                           џ                     d             
      4   TemplateResRef  BaseItem        LocalizedName   Description     DescIdentified  Tag             Charges         Cost            Stolen          StackSize       Plot            AddCost         Identified      Cursed          ModelPart1      ModelPart2      ModelPart3      PropertiesList  PropertyName    Subtype         CostTable       CostValue       Param1          Param1Value     ChanceAppear    PaletteID       Comment         p   џџџџ          p   џџџџ       џџџџ       p                                	   
                                                         ITP V3.28      X  0        и      и  М     h   џџџџ                                                               (          0          8          @          H          P          X          `          h          p          x                                                  Є          Ќ          Д                                         ~                                    8         	                  
                                             Ђ#                  Ј#                          8                             !                  "                  #                  $                                    К                 P         є                  Ь                                   Я                  <                 }            MAIN            STRREF          ID              LIST                                    	   
                                                                      !   "   #   $   %   &   '   (   )   *   +   ,   -   .   /                              	   
                                                FAC V3.28      p  M          5   С  4  ѕ  l   џџџџ<                                      $         0          D         P         \         h         t                                    Є      	   А      
   М         Ш         д         р         ь         ј                                 (                      џџџџ
                         џџџџ
                        џџџџ
                        џџџџ
                        џџџџ
      )                                                                           2                            2                            2                           d                                                                                                                                           d                           2                           d                                                       2                           d                           d                                                       2                           d                           d   FactionList     FactionParentID FactionName     FactionGlobal   RepList         FactionID1      FactionID2      FactionRep         PC   Hostile   Commoner   Merchant   Defender                        	   
                                                                          !   "   #   $   %   &   '   (   )   *   +   ,   -   .   /   0   1   2   3   4   5   6   7   8   9   :   ;   <   =   >   ?   @   A   B   C   D   E   F   G   H   I   J   K   L                                 	   
                                                ITP V3.28      д            @      @  `      8   џџџџ                                                               (          0          8          @          H          P          X                       &                  9ї                  є                  є                  є                                                        !                  "                  #                  $                  є            MAIN            STRREF          ID              LIST                                    	   
                                                                              	   
      ITP V3.28            L             8   Ф  $   џџџџ                                                               (          0                                                                              !                  "                  #                  $            MAIN            STRREF          ID              LIST                                    	   
                                          ITP V3.28      ј      l     Ќ      Ќ  x   $  H   џџџџ                                                               (          0          8          @          H          P          X          `          h          p                       :                                    Љ#                                                       !                  "                  #                  $                  Л        0         НЯ                  Я                  а                  б                  ОЯ            MAIN            STRREF          ID              LIST                                    	   
                                                                              
                  	                     ITP V3.28            L             8   Ф  $   џџџџ                                                               (          0                                                            !                  "                  #                  $                              MAIN            STRREF          LIST            ID                                      	   
                                          