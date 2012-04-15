set(all_deps HookSCORCO CExoString ObjectStorage db NWNXdb plugin-db)

set(CMAKE_VERBOSE_MAKEFILE on)

find_package(PostgreSQL)
if (PostgreSQL_FOUND)

	message("Found PostgreSQL installation!")
	message("PostgreSQL_INCLUDE_DIRS = " "${PostgreSQL_INCLUDE_DIRS}")
	message("PostgreSQL_LIBRARY_DIRS = " "${PostgreSQL_LIBRARY_DIRS}")
	message("PostgreSQL_LIBRARIES    = " "${PostgreSQL_LIBRARIES}")

	include_directories(${PostgreSQL_INCLUDE_DIRS})
	include_directories("${PostgreSQL_INCLUDE_DIRS}/server")
	#include_directories("/usr/include/pgsql/server")
	#include_directories("/usr/pgsql-9.0/include/server")

	add_module(db_pgsql pgsql ${all_deps})
	target_link_libraries(db_pgsql "${PostgreSQL_LIBRARY_DIRS}/libpq.so")
	#target_link_libraries(db_pgsql ${PostgreSQL_LIBRARY_DIRS})
	#target_link_libraries(db_pgsql ${PostgreSQL_LIBRARIES})
	#target_link_libraries(db_pgsql /usr/pgsql-9.0/lib/libpq.so)

	set_target_properties(db_pgsql PROPERTIES COMPILE_FLAGS "-DPGSQL_SUPPORT=1")

    #add_executable(unbun unbun.cpp)

else (PostgreSQL_FOUND)
	message(WARNING "PostgreSQL installation not found")
endif (PostgreSQL_FOUND)

find_package(SQLite3)
if (SQLITE3_FOUND)

	message("Found SQLite3 installation!")
	message("SQLITE3_INCLUDE_DIRS = " "${SQLITE3_INCLUDE_DIRS}")
	message("SQLITE3_LIBRARIES    = " "${SQLITE3_LIBRARIES}")

	include_directories(${SQLITE3_INCLUDE_DIRS})

	add_module(db_sqlite sqlite ${all_deps})
	target_link_libraries(db_sqlite ${SQLITE3_LIBRARIES})

	set_target_properties(db_sqlite PROPERTIES COMPILE_FLAGS "-DSQLITE_SUPPORT=1")

else (SQLITE3_FOUND)
	message(WARNING "SQLite3 installation not found")
endif (SQLITE3_FOUND)

find_package(MySQL)
if (MYSQL_FOUND)

	message("Found MySQL installation!")
	message("MYSQL_INCLUDE_DIR = " "${MYSQL_INCLUDE_DIR}")
	message("MYSQL_LIBRARY     = " "${MYSQL_LIBRARY}")

	include_directories(${MYSQL_INCLUDE_DIR})

	add_module(db_mysql mysql ${all_deps})
	target_link_libraries(db_mysql ${MYSQL_LIBRARY})
	set_target_properties(db_mysql PROPERTIES COMPILE_FLAGS "-DMYSQL_SUPPORT=1")

    #add_executable(tmysql tmysql.cpp db.h mysql.cpp mysql.h)
    #target_link_libraries(tmysql ${MYSQL_LIBRARY})

    #add_executable(t1 t1.cpp db.cpp mysql.cpp mysql.h )
    #target_link_libraries(t1 ${MYSQL_LIBRARY})
	
else (MYSQL_FOUND)
	message(WARNING "MySQL installation not found")
endif (MYSQL_FOUND)

if (0)
endif (0)

