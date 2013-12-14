
/***************************************************************************
    NWNXFuncs.cpp - Implementation of the CNWNXFuncs class.
    Copyright (C) 2007 Doug Swarin (zac@intertex.net)
    Copyright (C) 2013 eeriegeek (eeriegeek@yahoo.com)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 ***************************************************************************/

#include "NWNXSystem.h"

#include <sys/time.h>

void Func_GetMilliseconds (CGameObject *ob, char *value)
{
    int ret;
	struct timeval now;

	// Keeps first call seconds value and subtracts it away on subsequent
	// calls so int values will not overflow. Result is milliseconds
	// elapsed since first call. Will overflow int in about 24 days.
	//
	static unsigned int first_call_seconds = 0;

	ret = gettimeofday(&now,0);
	if (ret == -1) {
        ret = -errno;
    } else {
		if (first_call_seconds == 0) first_call_seconds = now.tv_sec;
        ret = ( ( now.tv_sec - first_call_seconds ) * 1000 ) + ( now.tv_usec / 1000 );
	}

    snprintf(value, strlen(value), "%d", ret);
}

/* vim: set sw=4: */
