/******************************************************************************

  nwnx_serializer - NWNX - DB Plugin - Object Serialization Interface

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

// Takes a full NWN object passed in oObject (item or creature) and turns
// it into an encoded string form which is safe for non-binary-safe handling
// methods. After any form of transmission or storage, the object may be
// reconstructed from the string by thawing with the Thaw methods.
// The nObjectType parameter is one of the NWN OBJECT_TYPE_* constants.
//
int NWNX_Serializer_TypedFreeze(int nObjectType, object oObject);
int NWNX_Serializer_Freeze(object oObject);

// Utility function to be used with Thaw. Attempts to determine the NWN object
// type OBJECT_TYPE_* from the contents of the blob buffer.
//
int NWNX_Serializer_GetObjectType();

// Takes a frozen object in the form of an encoded string created by Freeze and
// and reconstructs a full NWN object from it. The object is constructed in the
// inventory of the container passed as oContainer. If the oContainer does not
// have an inventory the object is created at the location of oContainer.
// The nObjectType parameter is one of the NWN OBJECT_TYPE_* constants.
//
object NWNX_Serializer_TypedThawToContainer(int nObjectType, object oContainer);
object NWNX_Serializer_ThawToContainer(object oContainer);

// Takes a frozen object in the form of an encoded string as created by Freeze
// and attempts to reconstruct a full NWN object from it. The object is
// constructed at the location given. Items may be thawed to a location or
// in a container, creatures will only be created at a location.
// The nObjectType parameter is one of the NWN OBJECT_TYPE_* constants.
//
object NWNX_Serializer_TypedThawToLocation(int nObjectType, location mLocation);
object NWNX_Serializer_ThawToLocation(location mLocation);

// Encodes the NWN object stored (frozen) to the internal buffer as a string
// of hex digits and returns it to the application.
//
string NWNX_Serializer_EncodeFrozen();

// Takes the passed hex encoded string object and decodes it back to a NWN
// object in the internal buffer where it can be thawed into the application.
//
void NWNX_Serializer_DecodeFrozen(string sFrozenObject);


//
// Implementation
//

int NWNX_Serializer_TypedFreeze(int nObjectType, object oObject)
{
    switch (nObjectType) {
        case OBJECT_TYPE_ITEM:
        case OBJECT_TYPE_CREATURE:
            StoreCampaignObject("NWNX","F",oObject);
            return 1;
        break;
        case OBJECT_TYPE_PLACEABLE:
        case OBJECT_TYPE_STORE:
        case OBJECT_TYPE_TRIGGER:
            SetLocalString(GetModule(),"NWNX!DB!OBJ_FREEZE",ObjectToString(oObject));
            return 1;
        break;
    }
    return 0;
}
int NWNX_Serializer_Freeze(object oObject)
{
    return (NWNX_Serializer_TypedFreeze(GetObjectType(oObject),oObject));
}

//
// OBJECT_TYPE_* constants, types marked with "<-" are supported at this time.
//
// OBJECT_TYPE_CREATURE         = 1;       <-
// OBJECT_TYPE_ITEM             = 2;       <-
// OBJECT_TYPE_TRIGGER          = 4;       <-
// OBJECT_TYPE_DOOR             = 8;
// OBJECT_TYPE_AREA_OF_EFFECT   = 16;
// OBJECT_TYPE_WAYPOINT         = 32;
// OBJECT_TYPE_PLACEABLE        = 64;      <-
// OBJECT_TYPE_STORE            = 128;     <-
// OBJECT_TYPE_ENCOUNTER        = 256;
// OBJECT_TYPE_ALL              = 32767;
// OBJECT_TYPE_INVALID          = 32767;
//
int NWNX_Serializer_GetObjectType()
{
    object oModule = GetModule();
    SetLocalString(oModule,"NWNX!DB!OBJ_GETTYPE","XXXXXXXXXXX");
    return (StringToInt(GetLocalString(oModule,"NWNX!DB!OBJ_GETTYPE")));

}

object NWNX_Serializer_TypedThawToContainer(int nObjectType, object oContainer)
{
    object oObject = OBJECT_INVALID;
    if (nObjectType==OBJECT_TYPE_ITEM) {
        oObject = RetrieveCampaignObject("NWNX","T",GetLocation(oContainer),oContainer);
        return oObject;
    }
    return OBJECT_INVALID;
}
object NWNX_Serializer_ThawToContainer(object oContainer)
{
    return (NWNX_Serializer_TypedThawToContainer(NWNX_Serializer_GetObjectType(),oContainer));
}

object NWNX_Serializer_TypedThawToLocation(int nObjectType, location mLocation)
{
    object oObject = OBJECT_INVALID;
    switch (nObjectType) {
        case OBJECT_TYPE_ITEM:
        case OBJECT_TYPE_CREATURE:
        {
            oObject = RetrieveCampaignObject("NWNX","T",mLocation,OBJECT_INVALID);
            return oObject;
        }
        break;
        case OBJECT_TYPE_PLACEABLE:
        case OBJECT_TYPE_STORE:
        case OBJECT_TYPE_TRIGGER:
        {
            vector vVec = GetPositionFromLocation(mLocation);
            string sLoc = ObjectToString(GetAreaFromLocation(mLocation))
                + "¬" + FloatToString(vVec.x)
                + "¬" + FloatToString(vVec.y)
                + "¬" + FloatToString(vVec.z)
                + "¬" + FloatToString(GetFacingFromLocation(mLocation));
            SetLocalString(GetModule(),"NWNX!DB!OBJ_THAW",sLoc);
            oObject = GetLocalObject(GetModule(),"NWNX!DB!RETRIEVEOBJECT");
            return oObject;
        }
        break;
    }
    return oObject;
}
object NWNX_Serializer_ThawToLocation(location mLocation)
{
    return (NWNX_Serializer_TypedThawToLocation(NWNX_Serializer_GetObjectType(),mLocation));
}

string NWNX_Serializer_EncodeFrozen()
{
    SetLocalString(GetModule(),"NWNX!DB!OBJ_ENCODE","X");
    return GetLocalString(GetModule(),"NWNX!DB!OBJ_ENCODE");
}

void NWNX_Serializer_DecodeFrozen(string sFrozenObject)
{
    SetLocalString(GetModule(),"NWNX!DB!OBJ_DECODE",sFrozenObject);
}

