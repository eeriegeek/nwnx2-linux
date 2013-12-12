/******************************************************************************

  plugin-gdbm.cpp - Singleton object for GDBM plugin for NWNX.

  Copyright 2012-2013 eeriegeek (eeriegeek@yahoo.com)

  This file is part of NWNX.

  NWNX is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  NWNX is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with NWNX.  If not, see <http://www.gnu.org/licenses/>.

******************************************************************************/

#include "NWNXgdbm.h"

CNWNXgdbm gdbm;

PLUGINLINK *pluginLink = NULL;

PLUGININFO pluginInfo={
	sizeof(PLUGININFO),
	"NWNX-GDBM",
	PLUGIN_MAKE_VERSION(2,0,0,0),
	"NWNX plugin for GNU DBM database support",
	"eeriegeek",
	"eeriegeek@yahoo.com",
	"Copyright 2012-1013 eeriegeek, GNU General Public License",
	"http://nwnx.org",
	0
};

extern "C" PLUGININFO* GetPluginInfo(DWORD nwnxVersion) {
	return &pluginInfo;
}

extern "C" int InitPlugin(PLUGINLINK *link) {
	pluginLink = link;
	return 0;
}

extern "C" CNWNXBase* GetClassObject()
{
	return &gdbm;
}

