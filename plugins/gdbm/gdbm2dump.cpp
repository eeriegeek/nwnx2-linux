/******************************************************************************

  gdbm2dump.cpp - Utility GDBM file dumper/restorer.

  Copyright 2012-2013 eeriegeek (eeriegeek@yahoo.com)

  This file is part of NWNX.

  NWNX is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  NWNX is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with NWNX.	If not, see <http://www.gnu.org/licenses/>.

******************************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>

#include <string.h>
#include <stdio.h>

#include <gdbm.h>

int main(int argc, char **argv)
{
	datum key, val;

	//
	// gdbm2dump
	// 
	// Reads k-v pairs from the GDBM file and writes a formatted version to
	// a plain file. The output file is designed to be somewhat readable/
	// edittable, but mostly reversable back to GDBM format. The lines ore
	// one per k-v value written in the form:
	//
	//           | key-size | key | value-size | value | \n
	//
	// Note that if editted, the size fields MUST match the size of the
	// corresponding data field. Multiple flat form files can be appended
	// and written back to a GDBM file if needed.
	// 
	if ( strcmp(basename(argv[0]),"gdbm2dump")==0 ) {

		if (argc != 3) { printf("usage: %s <gdbmfile> <dumpfile>\n", basename(argv[0])); return(1); }

		GDBM_FILE dbf = gdbm_open(argv[1], 0, GDBM_READER, 0400, 0);
		if (dbf == NULL) { fprintf(stderr,"Could not open input GDBM file [%s], %s\n", argv[1], gdbm_strerror(gdbm_errno)); return(1); }

		FILE* out = fopen(argv[2], "w");
		if (out == NULL) { fprintf(stderr,"Could not open output file [%s], %s\n", argv[2], strerror(errno)); return(1); }

		fprintf(stderr,"Reading key-value pairs from GDBM file [%s] to flat file [%s]\n", argv[1], argv[2]);

		int row_count = 0;

		key = gdbm_firstkey(dbf);
		while (key.dptr != NULL) {

			row_count++;
			val = gdbm_fetch(dbf,key);

			fprintf(out,"|%d|",key.dsize);
			fwrite(key.dptr,key.dsize,1,out);
			fprintf(out,"|%d|",val.dsize);
			fwrite(val.dptr,val.dsize,1,out);
			fprintf(out,"|\n");

			key = gdbm_nextkey(dbf,key);

		}

		gdbm_close(dbf);
		fclose(out);

		fprintf(stderr,"Read [%d] key-value pairs from GDBM file [%s] to flat file [%s]\n", row_count, argv[1], argv[2]);

	//
	// dump2gdbm
	// 
	// Note: get this utility by copying or linking the name to gdbm2dump.
	// 
	// Convert a flat file in the form written by gdbm2dump back into a GDBM
	// dbf file. Note that the GDBM file is opened in "no-turncate" mode,
	// and stored in "replace" mode so k-v pairs are written in to the file
	// overwriting existing values. This allows updating existing GDB files.
	// If you want only the new k-v pairs, delete the target file first.
	// 
	} else if ( strcmp(basename(argv[0]),"dump2gdbm")==0 ) {

		if (argc != 3) { printf("usage: %s <dumpfile> <gdbmfile>\n", basename(argv[0])); return(1); }

		struct stat s;
		if ( stat(argv[1],&s)!=0 ) { fprintf(stderr,"Could not find input file [%s], %s\n", argv[1], strerror(errno)); return(1); }
		int file_size = s.st_size;
		char* buf = (char*)malloc(file_size+1);

		int fd = open(argv[1], O_RDONLY);
		if (fd == -1) { fprintf(stderr,"Could not open input file [%s], %s\n", argv[1], strerror(errno)); return(1); }

		GDBM_FILE dbf = gdbm_open(argv[2], 0, GDBM_WRITER, 0600, 0);
		if (dbf == NULL) { fprintf(stderr,"Could not open output GDBM file [%s], %s\n", argv[2], gdbm_strerror(gdbm_errno)); return(1); }

		fprintf(stderr,"Writing key-value pairs from flat file [%s] size [%d] bytes to GDBM file [%s]\n", argv[1], file_size, argv[2]);

		// Read the input file into a big buffer
	
		int i = 0;
		int n;
		while ( (n = read(fd, &buf[i], 4096)) > 0 ) {
			i += n;
		}
		assert ( i == file_size );
		buf[i] = '\0';

		close(fd);

		// Write to the output file

		int row_count = 0;
		int len = 0;
		char* p = buf;
		char* np = 0;
		while (1) {

			if ( (p-buf > file_size) || (*p != '|') ) { break; }
			row_count++;
			p++;

			len = atoi(p);
			np = strchr(p,'|');
			p = np + 1;

			key.dsize = len;
			key.dptr  = p;
			//printf("key-size [%d]\n", key.dsize);
			//printf("key      [%s]\n", key.dptr);

			p = p + len + 1; // past '|'

			len = atoi(p);
			np = strchr(p,'|');
			p = np + 1;

			val.dsize = len;
			val.dptr  = p;
			//printf("val-size [%d]\n", val.dsize);
			//printf("val      [%s]\n", val.dptr);

			if ( gdbm_store(dbf, key, val, GDBM_REPLACE) != 0 ) {
				fprintf(stderr,"WARNING: Could not write key-value pair %d to the GDBM file. %s.\n", row_count, gdbm_strerror(gdbm_errno));
			}

			p = p + len + 2; // past '|' + '\n'
			// p now points to '|' for next row, or '\0'

		}

		gdbm_close(dbf);
		free(buf);

		fprintf(stderr,"Wrote [%d] key-value pairs from flat file [%s] to GDBM file [%s]\n", row_count, argv[1], argv[2]);

	}

}

