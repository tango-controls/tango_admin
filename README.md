# tango_admin

C++ source code for the tango_admin utility
This utility is a Tango database command line interface
Obviously, not all the database features are interfaced by this tool.
Only the features needed for the Debian	packaging have been implemented. 
This means:
  - ping the database server
  - check if a device is defined in DB
  - check if a server is defined in DB
  - create a server in DB
  - delete a server from the DB
  - create a property in DB
  - delete a property from DB
