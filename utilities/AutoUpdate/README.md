===================================================================
xTuple Auto/Mass Updater script
===================================================================

This script is used to mass update xTuple systems via a Linux script.

With this script you can update multiple xTuple (.gz) package files into 
multiple xTuple systems.

WARNING:
At this stage the script does not handle update errors and will continue on regardless.
The script *assumes* each xTuple system to be updated would be able to be updated manually with
the selected package files.

No Warrantees or Guarantees are implied in this script.  Use at your own risk.

======================================
INSTRUCTIONS
======================================

1] You must use at least version 2.4 of xTuple Updater
2] Modify the script to identify the Updater binary location (UPDATERLOCATION) 
3] Place the package .gz files to update into the folder (You can specify an alternate folder).  If packages must be loaded in a particular order then create a packagelist.txt file and list the full name of packages that you want to update in the correct sequence. 
4] Create a file with the systems to update in the format hostname:port/database.  One system per line.  If you specify a package file location, this server list must be in the same location
5] Call the xtuple_autoupdater script with the appropriate parameters

./xtuple_autoupdater serverlist

OR

./xtuple_autoupdater hostname:port/database, hostname2:port2/database2 etc

To specify an alternate location

./xtuple_autoupdater -l /alternatelocation serverlist




