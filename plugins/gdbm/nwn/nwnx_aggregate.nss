/******************************************************************************

  nwnx_aggregate - NWNX - Aggregate type to string serialization.

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

//
// NWN has several structured data types which have native nwscript types.
// In order to safely store these types persistently in the database, they
// need to be serialized in a reversible form. Since the passthrough method
// used by NWNX requires data to be in string form, we will serialize these
// aggragates into a well formed string representation. This form can be
// safely passed through Set/GetLocalString to NWNX and stored in a normal
// database text field. Conversion routines for Vector and Location are
// implemented here. Int and Float types can be stringified with their
// existing IntToString and FloatToString methods.
//
// Vectors are saved as ordered triplets. e.g. (1.0,1.0,1.0).
//
// Locations are saved as ordered quintuplets. e.g. ("AreaTag",1.0,1.0,1.0,90.0).
// 
// Note that Location saves the area Tag, this means that all areas must be
// given different tags for this scheme to work reliably.
//

//
// Interface
//
vector NWNX_VECTOR_INVALID = Vector(-9999999.0f,-9999999.0f,-9999999.0f);

string NWNX_VectorToString(vector v);
vector NWNX_StringToVector(string s);

location NWNX_LOCATION_INVALID = Location(OBJECT_INVALID,Vector(-9999999.0f,-9999999.0f,-9999999.0f),0.0f);

string NWNX_LocationToString(location m);
location NWNX_StringToLocation(string s);


//
// Implementation
//

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
    return ("("+
        GetTag(GetAreaFromLocation(m))+","+
        FloatToString(v.x,0,9)+","+FloatToString(v.y,0,9)+","+FloatToString(v.z,0,9)+","+
        FloatToString(GetFacingFromLocation(m),0,9)+
    ")");
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

