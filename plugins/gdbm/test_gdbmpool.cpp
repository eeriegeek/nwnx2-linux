/******************************************************************************

  test_gdbmpool.cpp - Test driver for GdbmPool.cpp.

  Copyright 2013 eeriegeek (eeriegeek@yahoo.com)

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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "GdbmPool.h"

void TEST_DB()
{
	printf("TEST: *** Database Maintenance ***\n");
	{
	GdbmPool gp(".",true,false);
	gp.create("test_db_01",false);
	gp.reorganize("test_db_01");
	gp.destroy("test_db_01"); // should warn
	}
	printf("TEST: ***\n");
	{
	GdbmPool gp(".",false,true);
	gp.create("test_db_01",false);
	// should auto close
	}
	printf("TEST: ***\n");
	{
	GdbmPool gp(".",true,false);
	gp.create("test_db_01",true); // should truncate
	gp.reorganize("test_db_01");
	gp.close("test_db_01");
	gp.destroy("test_db_01");
	}
	printf("TEST: ***\n");

}

void TEST_DM()
{
	printf("TEST: *** Data Manipulation ***\n");

	GdbmPool gp(".",true,false);
	char* tk1_s = "TestKey1"; gdbm_datum_t tk1 (tk1_s,tk1_s+strlen(tk1_s));
	char* tk2_s = "TestKey2"; gdbm_datum_t tk2 (tk2_s,tk2_s+strlen(tk2_s));
	char* tk3_s = "TestKey3"; gdbm_datum_t tk3 (tk3_s,tk3_s+strlen(tk3_s));
	char* tk4_s = "TestKey4"; gdbm_datum_t tk4 (tk4_s,tk4_s+strlen(tk4_s));
	char* tv1_s = "TestValue1";
	char* tv2_s = "TestValue2";
	char* tv3_s = "TestValue3";
	char* foo;
	size_t foo_size;

	gp.open("test_db_02");

	gp.store("test_db_02", tk1, tv1_s, strlen(tv1_s)+1);
	gp.store("test_db_02", tk2, tv2_s, strlen(tv2_s)+1);
	gp.store("test_db_02", tk3, tv3_s, strlen(tv3_s)+1);

	gp.sync("test_db_02");

	printf("TEST: exists, [1] ?= [%d]\n", gp.exists("test_db_02",tk1) );
	printf("TEST: exists, [0] ?= [%d]\n", gp.exists("test_db_02",tk4) );

	gp.fetch("test_db_02",tk1,&foo,&foo_size);
	printf("TEST: fetch, value [%s] ?= [%s], size [%d] ?= [%d]\n", tv1_s, foo, strlen(tv1_s)+1, foo_size);
	free(foo);

	gp.erase("test_db_02",tk1);
	printf("TEST: exists, [0] ?= [%d]\n", gp.exists("test_db_02",tk1) );
	gp.reorganize("test_db_02");
	printf("TEST: exists, [0] ?= [%d]\n", gp.exists("test_db_02",tk1) );

	gp.close("test_db_02");
	gp.destroy("test_db_02");
	printf("TEST: ***\n");
	
	// try to op without open -> error
	printf("TEST: expect 4 errors\n");
	gp.store("test_db_02", tk1, tv1_s, strlen(tv1_s)+1);
	gp.exists("test_db_02",tk1);
	gp.fetch("test_db_02",tk1,&foo,&foo_size);
	free(foo);
	gp.erase("test_db_02",tk1);
	printf("TEST: ***\n");
	gp.store("test_db_02", tk1, tv1_s, strlen(tv1_s)+1, true, true);
	gp.close("test_db_02");
	gp.exists("test_db_02",tk1, true);
	gp.close("test_db_02");
	gp.fetch("test_db_02",tk1,&foo,&foo_size, true);
	free(foo);
	gp.close("test_db_02");
	gp.erase("test_db_02",tk1, true);
	gp.close("test_db_02");
	gp.destroy("test_db_02");
	printf("TEST: ***\n");

}

void TEST_OP_MIX()
{
	printf("TEST: *** Run OP Mix ***\n");

	GdbmPool gp(".",true,false);

	srandom(time(0));

	char db_name[1024];
	char db_key[1024];
	char db_val[1024];

	char* foo;
	size_t foo_size;

	for (int i=0; i<10000; i++) {
		int r_db = random() % 3;
	    sprintf(db_name,"test_db_%02ld",(random()%10));
		for (int j=0; j<10; j++) {
			sprintf(db_key,"%05ldTestKey",(random()%100000));
			gdbm_datum_t k(db_key,db_key+strlen(db_key));
			int r_op = random()%12;
			if (r_op < 6) {
				gp.fetch(db_name,k,&foo,&foo_size,true);
				//printf("OP: FETCH, DB [%s], KEY [%s],  VAL [%s]\n", db_name, db_key, foo);
				free(foo);
			} else if (r_op < 8) {
				sprintf(db_val,"%ldTestValueTestValueTestValueTestValueTestValueTestValueTestValue",random());
				//printf("OP: STORE, DB [%s], KEY [%s], VAL [%s]\n", db_name, db_key,db_val);
				gp.store(db_name,k,db_val,strlen(db_val),true,true);
			} else if (r_op < 10) {
				//printf("OP: EXISTS, DB [%s], KEY [%s]\n", db_name, db_key);
				gp.exists(db_name,k,true);
			} else  {
				//printf("OP: DELETE, DB [%s], KEY [%s]\n", db_name, db_key);
				gp.erase(db_name,k,true);
			}
		}

	}
}

int main(int argc, char **argv)
{
	//TEST_DB();
	TEST_DM();
	//TEST_OP_MIX();

	return 0;
}

