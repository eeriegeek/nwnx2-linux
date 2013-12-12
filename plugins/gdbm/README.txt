README:

This NWNX plugin provides a wrapper to the Gnu GDBM database functions. GDBM
is a simple key-value pair hashed index database that accepts arbitrary data
for both the key and the value. In addition to access to the database methods,
this plugin also maintains a pool of open gdbm files and adds methods for
creation and deletion of GDMB database files.

Purpose:

This plugin implements an interface to a fast file based key-value database.
It's role falls somewhere between the Bioware campaign database and more
advanced data storage in an RDBMS. It could serve as means to fix some of
the limitations on speed and key size for a growing server not ready to move
to a SQL server or as a temporary index for some larger processing task.

A number of limitation in the default DBF file design have been identified
over the years. This plugin attempts to address some of those limitations.

1. Key Size Limitation - Keys (VarName) in the Bioware Campaign DB are
defined as "VARNAME,C,32" a length limit of 32 characters. For basic use this
is fine, but when doing more complex storage with compound keys, the limit
is easy to excced. This GDBM based plugin has no finite limit on key size except those
imposed by memory and storage space.

2. Per PC Data Key Limitation - As with the key size Limitation, the bioware campaign
db ("PLAYERID,C,32") places a 32 character size limit on the key used to 
uniquely identify per PC data. The player name and character name are used as a key,
but the combined key is truncated to 32 chars. Because the key size is unlimited,
the unique player id info can be combined in a compound key with the variable
name with unlimited length. A set of functions implementing this pattern is
in nwnx_gdbm_bio.nss. 

3. Type Collision - In the Bioware Campaign DB different types are keyed to the
same storage record. If an Int named "foo" is stored, it overwrites the
previously stored string named "foo". Here, the type is added to the key,
so variables of different types can no longer effect each other even if they
have the same name. As part of this extention, Type specific delete operations
have been added.

4. DBF File Growth - The DBF system used by BioWare uses a deletion mark 
in combination with an external database garbage collector to manage
deleted records. GDBM manages deleted space internally, re-using space
freed by deleted records. Note that GDBM does not shrink the actual
disk file unless the reorganise function is called. This is not needed
on stable or growing databases, only those which have lost large 
amounts of total data through excess deletion.

5. Object Storage - The basic Campaign functions allow storage of Items
and Creatures. These types have been expanded in NWNX2 by virusman to
include Placeables, Merchants and Triggers. This GDBM plugin uses the
extend plugin API to provide support for storage of these additional
object types.

Installation:

Unpack the plugin in the NWNX plugins directory. Make sure you have the GDBM
normal and development packages installed on your system. On RedHat / Fedora
this can be done with "yum install gdbm gdbm-devel". If the gdbm plugin is not
in the list of default plugins to build you will need to run configure with
the extra plugins option:
  
    ./configure --with-extraplugins="plugins/gdbm"

After make completes, the nwnx_gdbm.so shared library should be installed
by the same method as other plugins, i.e. by moving or linking it into the
NWNX server home directory. A CMakeLists.txt file is included for compatibility
with the core-2.8 cmake structure by elven. A cmake module is included to
help find the gdbm library.

Configuration:

This plugin does not require a config section in nwnx2.ini. It operates with
defaults that should work fine for most purposes. By default, GDBM files
are created in the database subdirectory of the NWN server home directory
with the extension ".gdbm". File locking is turned off and writing is
synchronous. The following section can be added to change the defaults.
Note that the sync option seemed to have little performance impact in my
initial testing and is therefore left enabled b default. It is also
probably safer than no_sync. File locking should only be needed for storage
on network drives where other applications may be accessing the files.

[GDBM]
filepath   = database
sync_write = 1
file_lock  = 0

Also note that the GDBM plugin can be used concurrently with either (or both)
the Bioware campaign database and the odbmc (mysql/pgsql/sqlite) plugin.

Usage:

A set of NWN include scripts can be found in the nwn subdirectory of this
plugin (nwnx_gdbm.nss, nwnx_gdbm_bio.nss, nwnx_gdbm_aps.nss. See the file
nwnx_gdbm.nss for more information on usage. An erf, nwnx_gdbm_inc.erf is
included. It contains only the above scripts for easy importation.

Performance:

The performance of GDMB seems to be on par with well configured local SQL
databases like mysql. For small data sets it is slightly faster.  It is not,
however, a functional replacement for a SQL database. GDBM is functionally
similar to, but significantly faster than, the default Bioware campaign
database. In addition it does not have the same key size limits as the
campaign database. Its on disk file size is also much smaller.

Other:

The GDBM release has several tools for working with GDMB files, however
they are not always built and installed by OS packagers. I have included
a small utility "gdbm2dump" which dumps a gdbm file to a flat file which
can be viewed with any editor for testing. Future release will likely
include the ability to reverse the process "dump2gdbm" and possibly
to dump the data in SQL insert statement form for migration into an RDBMS. 

Future:

 TODO:  Pool limiting, currently there is no finite limit to the pool map
 size. The pool of open gdbm files will grow to consume all fd's unless 
 files are closed. This is ok, but lightly dangerous. A timestamp field
 is included in the node struct as a starting point for closing the oldest
 file after a certain number of open files is reached.

 TODO: Implement the iterator functions. A datum iterator is included in
 the GDBM node structure for this.

 TODO: Utility to dump/restore to flat file. Dump to SQL.

Credits:

Many thanks to virusman, papillon, dumbo, and all the others who made
and continue to make NWNX possible.

