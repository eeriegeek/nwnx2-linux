/******************************************************************************

  nwnx_base - NWNX - Base NWNX functions.

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

// Initialize the NWNX library, should be called once from OnModuleLoad.
//
void NWNX_Initialize();

//
// Implementation
//
void NWNX_Initialize()
{

    // Send an INIT message to the NWNX library
    //
    SetLocalString(GetModule(),"NWNX!INIT","1");

}

