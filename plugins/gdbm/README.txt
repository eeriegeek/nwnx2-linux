README
------

This NWNX plugin provides a wrapper to the Gnu DBM (GDBM) database library.
GDBM is a simple hash indexed key-value pair flat file database that accepts arbitrary data for both the key and the value.
In addition to access to the basic GDBM database methods this plugin maintains a pool of open gdbm files for increased performance.
The plugin also provides methods for creation, deletion, and maintenance of GDMB database files.

Purpose
-------

This plugin implements an interface to a fast, reliable file based key-value database.
It's architectural role is analogous to the Bioware campaign database.
It does not provide the more advanced data structure, integrity, and query capabilities of more advanced data storage in an RDBMS.
It might serve to fix some limitations on speed and flexibility in the Bioware DBF file implementation for a server not ready implement an RDBMS solution.
It could also act as a temporary index for some larger processing task or interface with an external C/C++ program which can read/write GDBM files.  

A number of limitations in Bioware's default DBF file design have been identified over time.
This plugin can avoid some of those limitations.

1. Key Size Limitation
Keys (VarName) in the Bioware Campaign DB are defined as "VARNAME,C,32" a length limit of 32 characters.
For basic use this is fine, but when doing more complex storage with compound keys the limit is easy to excced.
This plugin based on GDBM has no finite limit on key size except those imposed by memory and storage space.

2. Per PC Data Key Limitation
As with the key size Limitation, the bioware campaign db ("PLAYERID,C,32") places a 32 character size limit on the key used to uniquely identify per PC data.
The player name and character name are used as a key but the combined key is truncated to total of 32 characters.
Because the key size is unlimited in GDBM, the unique player id info can be combined into a compound key with a variable name with unlimited length.
A set of functions implementing this pattern is in nwnx_gdbm_bio.nss. 

3. Type Collision
In the Bioware Campaign DB different types are keyed to the same storage record.
If an Int named "foo" is stored, it overwrites the previously stored string named "foo".
In the implementation of this plugin, a type code is added to the key so variables of different types with the same name can no longer overwrite each other.
As part of this extention type specific delete operations have been added.

4. DBF File Growth
The DBF system used by BioWare uses a deletion mark in combination with an external database garbage collector to manage deleted records.
GDBM manages deleted space internally, re-using space freed by deleted records automatically.
Note that GDBM does not shrink the actual disk file unless the reorganise function is called.
Reorganization is not needed on active, stable or growing databases, only those which have lost large amounts of total data through excess deletion.

5. Object Storage
The basic Campaign functions allow storage of Items and Creatures, this plugin allows for their storage as well.
Storable types have been expanded in NWNX by virusman to include Placeables, Merchants and Triggers.
This GDBM plugin will use the extend plugin API to provide support for storage of these additional object types when available.

Installation
------------

Unpack the plugin in the NWNX plugins directory.
Make sure you have the GDBM normal and development packages installed on your system.
On RedHat / Fedora / CentOS this can be done with "yum install gdbm gdbm-devel".
NWNX currently has two parallel build systems.

If using cmake the plugin should be found in the plugins directory and built automatically.
A CMakeLists.txt file is included as well as a cmake module to find the gdbm includes and library.

If using automake/make a Makefile.in is inclided, but the gdbm plugin is not in the default build plugin list.
To get it to build use the --with-extraplugins option to configure (e.g. ./configure --with-extraplugins="gdbm")

After make completes the nwnx_gdbm.so shared library should be installed by the same method as other plugins.
That is, it should be moved or linked into the NWNX library directory (often just server home).

Of special importance, the GDBM plugin depends on the ODBMC plugin for access to object handling methods (SCO/RCO handling.)
This means that a version (any version) of the ODMBC plugin must be built and installed in the NWNX library directory along with nwnx2.so and nwnx_gdbm.so.
It should not be necessary to even configure the database plugin, it just needs to be present to load.
The SQLite version is probably the easist to build if there are any problems.

Configuration
-------------

This plugin has several configuration options.
It operates with defaults that should work fine for most purposes, thus does not strictly require a config section in nwnx2.ini.
By default, GDBM files are created in the database subdirectory of the NWN server home directory with the extension ".gdbm".
File locking is turned off, file writing is synchronous, and sco/rco is used to provide object handling.
Turning off the sync_write option can produce a large incremental increase in performance at the cost of some risk of data corruption in the event of a crash.
File locking should only be needed for storage on network drives where other applications may be accessing the files.
If the use_scorco option is disabled the plugin will not be able to store or retrieve objects.
In order to override the default settings the following section and settings may be added to nwnx2.ini.

[GDBM]
filepath   = database
sync_write = 1
file_lock  = 0
use_scorco = 1

Note that the GDBM plugin can be used concurrently with either (or both) the Bioware campaign database and any of the odbmc (mysql/pgsql/sqlite) plugins.

Usage
-----

A set of NWN include scripts can be found in the nwn subdirectory of this plugin:

nwnx_aggregate.nss : Functions to convert vector and location types to string
nwnx_gdbm.nss      : Functions to access the full range of GDBM capabilities
nwnx_gdbm_camp.nss : Functions to emulate the Campaign Database interface

There are additional usage notes in the script files themselves.
An erf, nwnx_gdbm_inc.erf is included containing only the listed scripts for easy importation.

Performance
-----------

GDBM is a hashed index flat file database analogous to the DBF file implementation used by Bioware in the campaign database.
As such, its general performance characteristics will be similar.
In testing the GDBM implementation appears to be considerably faster than the campaign database.
Its on disk file size is smaller than the campaign database files as well.
For smaller datasets (up to a few 1000 rows), GDBM performance is comparable and sometimes faster than SQL server configurations.

Tools
-----

The GDBM release has several tools for working with GDMB files, however they are not always built and installed by OS packagers.
This plugin contains a utility programs "gdbm2dump" and "dump2gdbm" which dump a gdbm file to a flat file and convert the flat file back to a GDBM database.
The flat file is structured to be readable, and can be modified with care for testing and major database repairs.

Future
------

Pool limiting
Currently there is no finite limit to the pool map size.
If many database names are opened, the pool of open gdbm files can grow to consume all of the process's fds unless some files are closed.
The actual limits are fairly high; on my Linux system the system default soft limit is 1024 open files, increasable to 4096.
The plugin currently prints a warning to the console if 1/2 the system soft limit is exceeded.
A timestamp field is included in the node struct as a starting point for closing the oldest file after a certain number of open files is reached.

Implement iterator functions.
The GDBM interface allows iteration through all the key-value pairs in a database.
A datum iterator is included in the GDBM node structure future support for this if it seems useful.

Credits
-------

Many thanks to virusman, papillon, dumbo, and all the others who made and continue to make NWNX possible.

