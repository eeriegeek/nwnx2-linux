/***************************************************************************
    CDB.cpp: implementaion of the CDB base class.
    Copyright (C) 2012 eeriegeek (eeriegeek@yahoo.com)

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

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "db.h"

CDB::CDB()
{
    //blob_buffer_ptr = NULL;
    //blob_buffer_size_ptr = NULL;
	db_last_error_message[0] = '\0';
}

CDB::~CDB()
{
}

/*
void CDB::SetBlobBuffer(unsigned char *buffer, unsigned int *buffer_size)
{
	blob_buffer_ptr = buffer;
	blob_buffer_size_ptr = buffer_size;
}
*/

const char* CDB::GetErrorMessage ()
{
	return db_last_error_message;
}

///////////////////////////////////////////////////////////////////////////

// Parse (UnBundle) the streamed argument bundle to a list structure.
//
// | TYPE_0 | NULL_0 | SIZE_0 | DATA_0 | TYPE_1 | NULL_1 | SIZE_1 | DATA_1 ||
//
// TYPE, NULL, SIZE -> unsigned 32 bit int, parsable by sscanf %u
//
// DATA, TYPE=0 null message (not used)
// DATA, TYPE=1 external reference -> URL "data://nwnx/blob"
// DATA, TYPE=2 bytes -> unsigned char * of run length SIZE 
// DATA, TYPE=3 int32 -> unsigned char * of run length SIZE parsable by sscanf %d
// DATA, TYPE=4 float -> unsigned char * of run length SIZE parsable by sscanf %f
//
// example: |1|0|20|data://nwnx/blob/BIC|2|0|4|Helo|3|0|2|13|4|0|7|3.14159||
//
// Returns NULL on error or if bundle empty
//
parsed_bundle_t* CDB::ParseBundle(char* bundle) {

	if (bundle==NULL) {
		//printf("ERROR: Invalid/empty serialized data string\n");
		return NULL;
	}

	//printf("UN-BUNDLE Begin, Passed [%s], Length [%d]\n",bundle,strlen(bundle));

	// Empty string = no parameters
	if (*bundle=='\0') {
		return NULL;
	}

	uint32_t alloc_size  = 0;
	uint32_t field_count = 0;

	uint32_t tmp_type = 0;
	uint32_t tmp_null = 0;
	uint32_t tmp_size = 0;
	uint32_t tmp_size_adj = 0;

	char* p = bundle;
	char* q = p + 1;

	int r = 0;

	if ((p==NULL)||(*p!='|')||(*q=='|')) {
		//printf("ERROR: Invalid/empty serialized data string\n");
		return NULL;
	}

	//
	// Pass 1 - Check for format errors and calculate required size of list structure.
	//

	while (*q!='|') {

		// Type

		while (*++p!='|');
		r = sscanf(q,"%u|",&tmp_type);
		if (r!=1) { printf("ERROR: Invalid type field in serialized data, %s\n",strerror(errno)); return NULL; }
		if ((tmp_type<0)||(tmp_type>4)) { printf("ERROR: Type field out of range in serialized data.\n"); return NULL; }
		q = p + 1;

		// Null

		while (*++p!='|');
		r = sscanf(q,"%u|",&tmp_null);
		if (r!=1) { printf("ERROR: Invalid null field in serialized data, %s\n",strerror(errno)); return NULL; }
		if ((tmp_null<0)||(tmp_null>1)) { printf("ERROR: Null field out of range in serialized data.\n"); return NULL; }
		q = p + 1;

		// Size

		while (*++p!='|');
		r = sscanf(q,"%u|",&tmp_size);
		if (r!=1) { printf("ERROR: Invalid size field in serialized data.\n"); return NULL; }
		q = p + 1;

		// allocate word aligned size with room for null byte
		tmp_size_adj = ((tmp_size/4)+1)*4;

		//printf("DB: Pass 1, Type [%u], Null [%u], Size [%u]\n",tmp_type,tmp_null,tmp_size);

		// Data

		switch (tmp_type) {

			case 0: // null field
				// ignore, no header, no data, no error, not currently used
			break;

			case 1: // reference url
			case 2: // unsigned char* (string)
			case 3: // int32_t (32 bit signed int)
			case 4: // float (32 bt float)
				//alloc_size = alloc_size + (((tmp_size/4)+1)*4);
				//alloc_size = alloc_size + tmp_size + 1;
				alloc_size = alloc_size + tmp_size_adj;
			break;

			default:
				//printf("ERROR: Invalid type field in serialized data.\n");
				return NULL;
			break;
		}

		p = p + tmp_size + 1;
		q = p + 1;

		field_count += 1;

		//printf("DB: Pass 1, Field Count [%u], Incremental Size [%u]\n",field_count,alloc_size);

	}

	alloc_size = ( (field_count + 1) * sizeof(parsed_bundle_t) ) + alloc_size;

	parsed_bundle_t* parm = (parsed_bundle_t*)malloc(alloc_size);

	//printf("DB: Pass 1 complete, alloc_size [%u] at [%p]\n",alloc_size,parm);

	//
	// Set the metadata header block data
	//
	parm[0].b_type = 0xFFFFFFFF;
	parm[0].b_null = 0;
	parm[0].b_size = field_count;
	parm[0].b_data = (unsigned char *)&parm[1];

	parsed_bundle_t* parm_meta = &parm[1];

	unsigned char* parm_data = (unsigned char*)(parm + field_count + 1);

	//printf("DB: Pass 1 complete, head [%p], meta [%p], data [%p]\n",parm,parm_meta,parm_data);

	p = bundle;
	q = p + 1;

	//
	// Pass 2 - Parses data values and populates parsed data structure.
	//

	while (*q!='|') {

		// Type

		while (*++p!='|');
		r = sscanf(q,"%u|",&tmp_type);
		q = p + 1;

		// Null

		while (*++p!='|');
		r = sscanf(q,"%u|",&tmp_null);
		q = p + 1;
		// Size

		while (*++p!='|');
		r = sscanf(q,"%u|",&tmp_size);
		q = p + 1;

		tmp_size_adj = ((tmp_size/4)+1)*4;

		// Data

		//printf("DB: Pass 2, Type [%u], Null [%u], Size [%u]\n",tmp_type,tmp_null,tmp_size);

		switch (tmp_type) {

			case 0: break; // ignore null fields

			case 1:
			case 2: // unsigned char*
			case 3: // int32_t (32 bit signed int)
			case 4: // float (32 bit float)

				parm_meta->b_type = tmp_type;
				parm_meta->b_size = tmp_size;
				parm_meta->b_null = tmp_null;
				parm_meta->b_data = parm_data;

				memcpy(parm_meta->b_data,q,tmp_size);
				parm_meta->b_data[tmp_size] = '\0';

				//printf("DB: Parm [%p], Type [%u], Null [%u], Size [%u], Data [%p][%s]\n",parm_meta,parm_meta->b_type,parm_meta->b_null,parm_meta->b_size,parm_meta->b_data,(char*)parm_meta->b_data);

				parm_meta += 1;
				//parm_data = parm_data + (((tmp_size/4)+1)*4);
				//parm_data = parm_data + (tmp_size+1);
				parm_data = parm_data + tmp_size_adj;

			break;
		}

		p = p + tmp_size + 1;
		q = p + 1;

	}

	//printf("DB: UN-BUNDLE End, Fields [%u], Size [%u]\n",field_count,alloc_size);

	//*fields = field_count;

	return parm;
}


