static const char *RcsId = "$Id$";

//+============================================================================
//
// file :               tango_admin.cpp
//
// description :        C++ source code for the tango_admin utility
//						This utility is a Tango database command line interface
//						Obviously, not all the database features are interfaced
//						by this tool. Only the features needed for the Debian
//						packaging have been implemented. This means:
//						- ping the database server
//						- check if a device is defined in DB
//						- check if a server is defined in DB
//						- create a server in DB
//						- delete a server from the DB
//						- create a property in DB
//						- delete a property from DB
//
// project :            TANGO
//
// author(s) :          E.Taurel
//
// Copyright (C) :      2004,2005,2006,2007,2008,2009,2010
//						European Synchrotron Radiation Facility
//                      BP 220, Grenoble 38043
//                      FRANCE
//
// This file is part of Tango.
//
// Tango is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// Tango is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with Tango.  If not, see <http://www.gnu.org/licenses/>.
//
// $Revision$
//
//-============================================================================

#if HAVE_CONFIG_H
#include <ac_config.h>
#endif

#include <iostream>
#include <anyoption.h>
#include <tango.h>

#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <stdlib.h>

using namespace std;


int ping_database(int);
int check_device(char *);
int add_server(char *,char *,char *);
void list2vect(string &,vector<string> &);
int check_server(char *);
int delete_server(char *,bool);
int add_property(char *,char *,char *);
int delete_property(char *,char *);
int ping_network(int,bool);
int check_net(bool);
int tac_enabled(void);
int ping_device(char *,int);
int check_dev(char *);


int main(int argc,char *argv[])
{
	AnyOption *opt = new AnyOption();

//
// Add usage menu
//

	opt->addUsage("Usage: " );
 	opt->addUsage(" --help  		Prints this help " );
	opt->addUsage(" --ping-database	[max_time (s)] Ping database " );
	opt->addUsage(" --check-device <dev>    Check if the device is defined in DB");
 	opt->addUsage(" --add-server <exec/inst> <class> <dev list (comma separated)>   Add a server in DB" );
 	opt->addUsage(" --delete-server <exec/inst> [--with-properties]   Delete a server from DB" );
	opt->addUsage(" --check-server <exec/inst>   Check if a device server is defined in DB");
 	opt->addUsage(" --add-property <dev> <prop_name> <prop_value (comma separated for array)>    Add a device property in DB" );
	opt->addUsage(" --delete-property <dev> <prop_name>   Delete a device property from DB ");
	opt->addUsage(" --tac-enabled Check if the TAC (Tango Access Control) is enabled");
	opt->addUsage(" --ping-device <dev> [max_time (s)] Check if the device is running");
	opt->addUsage(" --ping-network [max_time (s)] [-v] Ping network ");

//
// Define the command line options
//

	opt->setFlag("help",'h');
 	opt->setFlag("ping-database");
	opt->setOption("add-server");
	opt->setOption("delete-server");
	opt->setFlag("with-properties");
	opt->setOption("add-property");
	opt->setOption("delete-property");
	opt->setOption("check-device");
	opt->setOption("check-server");
	opt->setOption("ping-device");
	opt->setFlag("ping-network");
	opt->setFlag("tac-enabled");

//
// Process cmd line
//

	opt->processCommandArgs( argc, argv );

	if (!opt->hasOptions())
	{
		opt->printUsage();
		delete opt;
		return 0;
	}

//
// --help option
//

	if (opt->getFlag("help") || opt->getFlag('h'))
	{
		opt->printUsage();
		delete opt;
		return 0;
	}

//
// --ping-database option
//

	if (opt->getFlag("ping-database") == true)
	{
		if (opt->getValue("add-server") != NULL ||
			opt->getValue("delete-server") != NULL ||
		    opt->getValue("add-property") != NULL ||
		    opt->getValue("delete-property") != NULL ||
			opt->getValue("check-device") != NULL ||
			opt->getValue("check-server") != NULL ||
			opt->getFlag("ping-network") == true ||
			opt->getFlag("tac-enabled") == true ||
		    opt->getFlag("with-properties") == true)
			cout << "Can't mix option --ping-database with other option(s)" << endl;

		if (argc > 3)
		{
			cout << "Bad argument number for option --ping-database" << endl;
			opt->printUsage();
			delete opt;
			return 0;
		}

		int ret;
		if (argc == 2)
			ret = ping_database(0);
		else
		{
			int sec = atoi(argv[2]);
			ret = ping_database(sec);
		}

		delete opt;
		return ret;
	}

//
// --check-device option
//


	else if (opt->getValue("check-device") != NULL)
	{
		if (opt->getValue("delete-server") != NULL ||
		    opt->getValue("add-property") != NULL ||
		    opt->getValue("delete-property") != NULL ||
			opt->getValue("add-server") != NULL ||
			opt->getValue("check-server") != NULL ||
			opt->getFlag("ping-network") == true ||
			opt->getFlag("tac-enabled") == true ||
		    opt->getFlag("with-properties") == true)
			cout << "Can't mix option --add-server with other option(s)" << endl;
		else
		{
			if (argc != 3)
			{
				cout << "Bad argument number for option --check_device" << endl;
				opt->printUsage();
				delete opt;
				return 0;
			}

			int ret;
			ret = check_device(opt->getValue("check-device"));

			delete opt;
			return ret;
		}
	}

//
// --add-server option
//


	else if (opt->getValue("add-server") != NULL)
	{
		if (opt->getValue("delete-server") != NULL ||
		    opt->getValue("add-property") != NULL ||
		    opt->getValue("delete-property") != NULL ||
			opt->getValue("check-device") != NULL ||
			opt->getValue("check-server") != NULL ||
			opt->getFlag("ping-network") == true ||
			opt->getFlag("tac-enabled") == true ||
		    opt->getFlag("with-properties") == true)
			cout << "Can't mix option --add-server with other option(s)" << endl;
		else
		{
			if (argc != 5)
			{
				cout << "Bad argument number for option --add-server" << endl;
				opt->printUsage();
				delete opt;
				return 0;
			}

			int ret;
			ret = add_server(opt->getValue("add-server"),opt->getArgv(0),opt->getArgv(1));

			delete opt;
			return ret;
		}
	}

//
// --check-server option
//


	else if (opt->getValue("check-server") != NULL)
	{
		if (opt->getValue("delete-server") != NULL ||
		    opt->getValue("add-property") != NULL ||
		    opt->getValue("delete-property") != NULL ||
			opt->getValue("add-server") != NULL ||
			opt->getValue("check-device") != NULL ||
			opt->getFlag("ping-network") == true ||
			opt->getFlag("tac-enabled") == true ||
		    opt->getFlag("with-properties") == true)
			cout << "Can't mix option --check-server with other option(s)" << endl;
		else
		{
			if (argc != 3)
			{
				cout << "Bad argument number for option --check_server" << endl;
				opt->printUsage();
				delete opt;
				return 0;
			}

			int ret;
			ret = check_server(opt->getValue("check-server"));

			delete opt;
			return ret;
		}
	}

//
// --delete-server option
//

	else if (opt->getValue("delete-server") != NULL)
	{
		if (opt->getValue("add-server") != NULL ||
		    opt->getValue("add-property") != NULL ||
			opt->getValue("check-server") != NULL ||
			opt->getValue("check-device") != NULL ||
			opt->getFlag("ping-network") == true ||
			opt->getFlag("tac-enabled") == true ||
		    opt->getValue("delete-property") != NULL)
			cout << "Can't mix option --delete-server with other option(s)" << endl;
		else
		{
			if ((argc < 3 || argc > 4) ||
				(argc == 3 && strcmp(argv[2],"--with-properties") == 0) ||
				(strcmp(opt->getValue("delete-server"),"--with-properties") == 0))
			{
				cout << "Bad option delete-server usage" << endl;
				opt->printUsage();
				delete opt;
				return 0;
			}

			int ret;
			if (opt->getFlag("with-properties") == true)
				ret = delete_server(opt->getValue("delete-server"),true);
			else
				ret = delete_server(opt->getValue("delete-server"),false);

			delete opt;
			return ret;
		}
	}

//
// --add-property option
//

	else if (opt->getValue("add-property") != NULL)
	{
		if (opt->getValue("delete-server") != NULL ||
		    opt->getValue("delete-property") != NULL ||
			opt->getValue("add-server") != NULL ||
			opt->getValue("check-device") != NULL ||
			opt->getValue("check-server") != NULL ||
		    opt->getFlag("with-properties") == true ||
			opt->getFlag("tac-enabled") == true ||
			opt->getFlag("ping-network") == true ||
			opt->getFlag("ping-database") == true)
			cout << "Can't mix option --add-property with other option(s)" << endl;
		else
		{
			if (argc != 5)
			{
				cout << "Bag argument number for option --add-property" << endl;
				opt->printUsage();
				delete opt;
				return 0;
			}

			int ret;
			ret = add_property(opt->getValue("add-property"),opt->getArgv(0),opt->getArgv(1));

			delete opt;
			return ret;
		}
	}

//
// --delete-property option
//

	else if (opt->getValue("delete-property") != NULL)
	{
		if (opt->getValue("delete-server") != NULL ||
		    opt->getValue("add-property") != NULL ||
			opt->getValue("add-server") != NULL ||
			opt->getValue("check-device") != NULL ||
			opt->getValue("check-server") != NULL ||
		    opt->getFlag("with-properties") == true ||
			opt->getFlag("ping-network") == true ||
			opt->getFlag("tac-enabled") == true ||
			opt->getFlag("ping-database") == true)
			cout << "Can't mix option --delete-property with other option(s)" << endl;
		else
		{
			if (argc != 4)
			{
				cout << "Bag argument number for option --add-property" << endl;
				opt->printUsage();
				delete opt;
				return 0;
			}

			int ret;
			ret = delete_property(opt->getValue("delete-property"),opt->getArgv(0));

			delete opt;
			return ret;
		}
	}

//
// --ping-network option
//

	if (opt->getFlag("ping-network") == true)
	{
		bool verbose = false;
		
		if (opt->getValue("add-server") != NULL ||
			opt->getValue("delete-server") != NULL ||
		    opt->getValue("add-property") != NULL ||
		    opt->getValue("delete-property") != NULL ||
			opt->getValue("check-device") != NULL ||
			opt->getValue("check-server") != NULL ||
			opt->getFlag("ping-database") == true ||
			opt->getFlag("tac-enabled") == true ||
		    opt->getFlag("with-properties") == true)
			cout << "Can't mix option --ping-network with other option(s)" << endl;

		if (argc > 4)
		{
			cout << "Bad argument number for option --ping-network" << endl;
			opt->printUsage();
			delete opt;
			return 0;
		}
		else if (argc == 4)
		{
			if (strcmp(argv[3],"-v") != 0)
			{
				cout << "Bad argument for option --ping-network" << endl;
				opt->printUsage();
				delete opt;
				return 0;
			}
			else
				verbose = true;
		}
		else if (argc == 3)
		{
			if (strcmp(argv[2],"-v") == 0)
			{
				verbose = true;
			}
		}
		
		int ret;
		if (argc == 2)
			ret = ping_network(0,verbose);
		else
		{
			int sec = 0;
			sec = atoi(argv[2]);
			if ((verbose == false) && (sec == 0))
			{
				cout << "Bad argument for option --ping-network" << endl;
				opt->printUsage();
				delete opt;
				return 0;
			}
			ret = ping_network(sec,verbose);
		}

		delete opt;
		return ret;
	}

//
// --tac-enabled option
//

	if (opt->getFlag("tac-enabled") == true)
	{
		if (opt->getValue("add-server") != NULL ||
			opt->getValue("delete-server") != NULL ||
		    opt->getValue("add-property") != NULL ||
		    opt->getValue("delete-property") != NULL ||
			opt->getValue("check-device") != NULL ||
			opt->getValue("check-server") != NULL ||
			opt->getFlag("ping-network") == true ||
		    opt->getFlag("with-properties") == true)
			cout << "Can't mix option --tac-enabled with other option(s)" << endl;

		if (argc > 2)
		{
			cout << "Bad argument number for option --tac-enabled" << endl;
			opt->printUsage();
			delete opt;
			return 0;
		}

		int ret;
		ret = tac_enabled();

		delete opt;
		return ret;
	}

//
// --ping-device option
//


	if (opt->getValue("ping-device") != NULL)
	{
		if (opt->getValue("delete-server") != NULL ||
		    opt->getValue("add-property") != NULL ||
		    opt->getValue("delete-property") != NULL ||
			opt->getValue("add-server") != NULL ||
			opt->getValue("check-server") != NULL ||
			opt->getFlag("ping-network") == true ||
			opt->getFlag("tac-enabled") == true ||
		    opt->getFlag("with-properties") == true)
			cout << "Can't mix option --ping-device with other option(s)" << endl;
		else
		{
			int ret;
			int sec = 0;

			if (argc < 3 || argc > 4)
			{
				cout << "Bad argument number for option --ping_device" << endl;
				opt->printUsage();
				delete opt;
				return 0;
			}
			else if (argc == 4)
			{
				sec = atoi(argv[3]);
			}

			ret = ping_device(opt->getValue("ping-device"),sec);

			delete opt;
			return ret;
		}
	}

//
// Unknown choice
//

	else
	{
		cout << "Wrong usage" << endl;
		opt->printUsage();
	}

	delete opt;
}

//+-------------------------------------------------------------------------
//
// method : 		ping_database
// 
// description : 	This function connect to the database and executes
//					one of its command in order to check the database
//					connectivity.
//
// argument : in : 	- nb_sec : Max time (in sec) to do re-try in case of failure
//
// The function returns 0 is everything is fine. Otherwise, it returns -1
//
//--------------------------------------------------------------------------

int ping_database(int nb_sec)
{
	int ret = 0;

	setenv("SUPER_TANGO","true",1);

	int nb_loop;
	bool infinite = false;

	if (nb_sec == 0)
		nb_loop = 1;
	else if (nb_sec < 0)
	{
		infinite = true;
		nb_loop = 2;
	}
	else
		nb_loop = nb_sec << 1;


	struct timespec ts;
	ts.tv_sec = 0;
	ts.tv_nsec = 500000000;

//
// First sleep for 1 sec before trying to access the db
// This was needed when ported to Natty (Ubuntu 11.04) in the
// tango-db startup script. Db process did not start if tango 
// admin starts pinging db device too early !!
//

	if (nb_loop != 1)
	{
		ts.tv_sec = 1;
		ts.tv_nsec = 0;

		nanosleep(&ts,NULL);
	}

//
// re-try the call every 500 mS
//

	ts.tv_sec = 0;
	ts.tv_nsec = 500000000;

	while(nb_loop > 0)
	{
		try
		{
			Tango::Database db;

			string db_info;
			db_info = db.get_info();
			ret = 0;
			nb_loop = 0;
		}
		catch (Tango::DevFailed &e)
		{
			ret = -1;
			if (infinite == false)
				--nb_loop;
		}

		if (nb_loop != 0)
			nanosleep(&ts,NULL);
	}

	return ret;	
}

//+-------------------------------------------------------------------------
//
// method : 		check_device
// 
// description : 	This function checks if a device is defined in the DB
//
// argument : in : 	- name : The device name
//
// The function returns 0 is the device is defined. Otherwise, it returns -1
//
//--------------------------------------------------------------------------

int check_device(char *name)
{
	int ret = 0;

	try
	{
		Tango::Database db;

		string d_name(name);
		Tango::DbDevImportInfo dii = db.import_device(d_name);
	}
	catch (Tango::DevFailed &e)
	{
		ret = -1;
	}
	return ret;	
}

//+-------------------------------------------------------------------------
//
// method : 		add_server
// 
// description : 	This function adds a server definition in the DB
//
// argument : in : 	- d_name : The device server name (exec/inst)
//					- c_name : The class name
//					- d_list : The device list
//
// The function returns 0 is everything is fine. Otherwise, it returns -1
//
//--------------------------------------------------------------------------

int add_server(char *d_name,char *c_name,char *d_list)
{
	int ret = 0;

//
// Check ds name syntax
//

	string ds_name(d_name);
	string::size_type pos;

	pos = ds_name.find('/');
#ifdef  __SUNPRO_CC
	int n1 = 0;
	count(ds_name.begin(),ds_name.end(),'/',n1);
	if ((n1 != 1) || pos == 0 || pos == (ds_name.size() - 1))
	{
#else
	if ((count(ds_name.begin(),ds_name.end(),'/') != 1) || pos == 0 || pos == (ds_name.size() - 1))
	{
#endif
		cout << "Wrong syntax for ds name" << endl;
		ret = -1;
		return ret;
	}

//
// Check class name syntax
//

	string class_name(c_name);
#ifdef  __SUNPRO_CC
	count(class_name.begin(),class_name.end(),'/',n1);
	if (n1 != 0)
	{
#else	
	if (count(class_name.begin(),class_name.end(),'/') != 0)
	{
#endif
		cout << "Wrong syntax for class name" << endl;
		ret = -1;
		return ret;
	}

//
// Check device list and device syntax
//

	string dev_list(d_list);
	vector<string> dev_names;

	list2vect(dev_list,dev_names);

	for (unsigned int loop = 0;loop < dev_names.size();++loop)
	{
#ifdef  __SUNPRO_CC
		count(dev_names[loop].begin(),dev_names[loop].end(),'/',n1);
		if (n1 != 2)
		{
#else	
		if (count(dev_names[loop].begin(),dev_names[loop].end(),'/') != 2)
		{
#endif
			cout << "Wrong syntax for device " << dev_names[loop] << endl;
			ret = -1;
			return ret;
		}

		string::size_type pos1,pos2;
		pos1 = dev_names[loop].find('/');
		pos2 = dev_names[loop].rfind('/');

		if (pos1 == 0 || pos2 == dev_names[loop].length() - 1 || pos2 == pos1 + 1)
		{
			cout << "Wrong syntax for device " << dev_names[loop] << endl;
			ret = -1;
			return ret;
		}
	}

//
// Create server in DB
// Dont forget to add the admin device
//

	setenv("SUPER_TANGO","true",1);

	try
	{
		Tango::Database db;

		Tango::DbDevInfos ddi;
		Tango::DbDevInfo tmp_dbi;

		for (unsigned int loop = 0;loop < dev_names.size();++loop)
		{
			tmp_dbi.name = dev_names[loop];
			tmp_dbi._class = class_name;
			tmp_dbi.server = ds_name;
			ddi.push_back(tmp_dbi);
		}
		tmp_dbi.name = "dserver/" + ds_name;
		tmp_dbi._class = "DServer";
		tmp_dbi.server = ds_name;

		ddi.push_back(tmp_dbi);

		db.add_server(ds_name,ddi);
	}
	catch (Tango::DevFailed &e)
	{
		ret = -1;
	}
	return ret;
}

//+-------------------------------------------------------------------------
//
// method : 		check_server
// 
// description : 	This function checks if a device server is defined in the DB
//
// argument : in : 	- d_name : The device server name
//
// The function returns 0 is the device is defined. Otherwise, it returns -1
//
//--------------------------------------------------------------------------

int check_server(char *d_name)
{
	int ret = 0;

	string dev_name = "dserver/";
	string ds_name = d_name;

	dev_name = dev_name + ds_name;

	ret = check_device((char *)dev_name.c_str());

	return ret;
}

//+-------------------------------------------------------------------------
//
// method : 		delete_server
// 
// description : 	This function deletes a device server from the DB
//
// argument : in : 	- d_name : The device server name
//					- with_res : If true, also delte device properties
//
// The function returns 0 is everything is fine. Otherwise, it returns -1
//
//--------------------------------------------------------------------------

int delete_server(char *d_name,bool with_res)
{
	int ret = 0;

	string ds_name(d_name);

//
// Check device server name syntax
//

	string::size_type pos;
	pos = ds_name.find('/');
	
#ifdef  __SUNPRO_CC
	int n1 = 0;
	count(ds_name.begin(),ds_name.end(),'/',n1);
	if (pos == 0 || pos == ds_name.size() - 1 || n1 != 1)
	{
#else
	if (pos == 0 || pos == ds_name.size() - 1 ||
		count(ds_name.begin(),ds_name.end(),'/') != 1)
	{
#endif
		ret = -1;
		return ret;
	}

	ret = check_server(d_name);
	if (ret != 0)
		return ret;


	try
	{

		Tango::Database db;

//
// If we need to remove prop
//

		if (with_res == true)
		{

//
//	First get the ds class list
//

			Tango::DbDatum db_res = db.get_device_class_list(ds_name);
			vector<string> dev_list;
			db_res >> dev_list;

//
// Get device property name for each device
//

			for (unsigned int loop = 0;loop < dev_list.size();++loop)
			{
				vector<string> prop_list;

				db.get_device_property_list(dev_list[loop],"*",prop_list);

//
// Delete all device properties
//

				if (prop_list.empty() == false)
				{
					Tango::DbData dbd;
					
					for (unsigned int ctr = 0;ctr < prop_list.size();++ctr)
						dbd.push_back(Tango::DbDatum(prop_list[ctr]));

					db.delete_device_property(dev_list[loop],dbd);
				}

				++loop;
			}				

		}

//
// Delete device server from db
//


		db.delete_server(ds_name);
	}
	catch (Tango::DevFailed &e)
	{
		ret = -1;
	}

	return ret;
}

//+-------------------------------------------------------------------------
//
// method : 		add_property
// 
// description : 	This function adds a device property in the DB
//
// argument : in : 	- d_name : The device name 
//					- p_name : The property name
//					- p_val : The property value
//
// The function returns 0 is everything is fine. Otherwise, it returns -1
//
//--------------------------------------------------------------------------

int add_property(char *d_name,char *p_name,char *p_val)
{
	int ret = 0;

//
// Check dev name syntax
//

	string dev_name(d_name);
	string::size_type pos1,pos2;

	pos1 = dev_name.find('/');
	pos2 = dev_name.rfind('/');
	
#ifdef  __SUNPRO_CC
	int n1 = 0;
	count(dev_name.begin(),dev_name.end(),'/',n1);
	if ((n1 != 2) || pos1 == 0 || pos2 == (dev_name.size() - 1) || pos2 == pos1 + 1)
	{
#else
	if ((count(dev_name.begin(),dev_name.end(),'/') != 2) || 
		pos1 == 0 || pos2 == (dev_name.size() - 1) || pos2 == pos1 + 1)
	{
#endif
		cout << "Wrong syntax for device name" << endl;
		ret = -1;
		return ret;
	}

//
// Check if the device is defined
//

	if (check_device(d_name) != 0)
		return -1;

//
// Convert prop value(s) into a vector
//

	string prop_val(p_val);
	vector<string> prop_val_list;

	list2vect(prop_val,prop_val_list);

//
// Create server in DB
// Dont forget to add the admin device
//

	try
	{
		Tango::Database db;

		Tango::DbData dbd;
		Tango::DbDatum db_s(p_name);
		
		db_s << prop_val_list;
		dbd.push_back(db_s);

		db.put_device_property(dev_name,dbd);
	}
	catch (Tango::DevFailed &e)
	{
		ret = -1;
	}
	return ret;
}

//+-------------------------------------------------------------------------
//
// method : 		delete_property
// 
// description : 	This function deletes a device property from the DB
//
// argument : in : 	- d_name : The device name 
//					- p_name : The property name
//
// The function returns 0 is everything is fine. Otherwise, it returns -1
//
//--------------------------------------------------------------------------

int delete_property(char *d_name,char *p_name)
{
	int ret = 0;

//
// Check dev name syntax
//

	string dev_name(d_name);
	string::size_type pos1,pos2;

	pos1 = dev_name.find('/');
	pos2 = dev_name.rfind('/');
	
#ifdef  __SUNPRO_CC
	int n1 = 0;
	count(dev_name.begin(),dev_name.end(),'/',n1);
	if ((n1 != 2) || pos2 == (dev_name.size() - 1) || pos2 == pos1 + 1)
	{
#else
	if ((count(dev_name.begin(),dev_name.end(),'/') != 2) || 
		pos1 == 0 || pos2 == (dev_name.size() - 1) || pos2 == pos1 + 1)
	{
#endif
		cout << "Wrong syntax for device name" << endl;
		ret = -1;
		return ret;
	}

//
// Check if the device is defined
//

	if (check_device(d_name) != 0)
		return -1;

//
// Create server in DB
// Dont forget to add the admin device
//

	try
	{
		Tango::Database db;

		Tango::DbData dbd;
		dbd.push_back(Tango::DbDatum(p_name));

		db.delete_device_property(dev_name,dbd);
	}
	catch (Tango::DevFailed &e)
	{
		ret = -1;
	}
	return ret;
}

//+-------------------------------------------------------------------------
//
// method : 		list2vect
// 
// description : 	This function converts a comma separated
//					device list into a vector of strings with one
//					element for each device
//
// argument : in : 	- dev_list : The device list
//					- dev_names : The device vector
//
//--------------------------------------------------------------------------

void list2vect(string &dev_list,vector<string> &dev_names)
{
	string::size_type beg,end;

	bool end_loop = false;
	beg = 0;

	while (end_loop == false)
	{
		end = dev_list.find(',',beg);
		if (end == beg)
		{
			++beg;
			continue;
		}

		if (end == string::npos)
		{
			end = dev_list.length();
			end_loop = true;
		}

		string one_dev;
		one_dev = dev_list.substr(beg,end - beg);
		dev_names.push_back(one_dev);

		beg = end + 1;
		if (beg == dev_list.size())
			end_loop = true;
	}
}

//+-------------------------------------------------------------------------
//
// method : 		ping_network
// 
// description : 	This function periodically chechs the network avaibility
//
// argument : in : 	- nb_sec : Max time (in sec) to do re-try in case of failure
//					- verbose : Boolean flag set to true if some printing is required
//
// The function returns 0 is everything is fine. Otherwise, it returns -1
//
//--------------------------------------------------------------------------

int ping_network(int nb_sec,bool verbose)
{
	int ret = 0;

	int nb_loop;
	bool infinite = false;

	if (nb_sec == 0)
		nb_loop = 1;
	else if (nb_sec < 0)
	{
		infinite = true;
		nb_loop = 2;
	}
	else
		nb_loop = nb_sec << 1;

//
// re-try the call every 500 mS
//

	struct timespec ts;
	ts.tv_sec = 0;
	ts.tv_nsec = 500000000;

	while(nb_loop > 0)
	{

		int res = check_net(verbose);
		if (res == 0)
		{
			ret = 0;
			nb_loop = 0;
		}
		else
		{
			ret = -1;
			if (infinite == false)
				--nb_loop;
		}

		if (nb_loop != 0)
			nanosleep(&ts,NULL);
	}

	return ret;	
}

//+-------------------------------------------------------------------------
//
// method : 		check_net
// 
// description : 	This function connect to the network and check if it is
//					fully ready.
//
// argument : in : 	- verbose : Flag set to true if some printing is required
//
// The function returns 0 is everything is fine. Otherwise, it returns -1
//
//--------------------------------------------------------------------------

int check_net(bool verbose)
{
	int ret = 0;

	char buffer[80];
	string hostname;

	if (gethostname(buffer,80) == 0)
	{
		hostname = buffer;
		if (verbose == true)
			cout << "Host name returned by gethostname function: " << hostname << endl;

  		struct addrinfo hints;

		memset(&hints,0,sizeof(struct addrinfo));

  		hints.ai_flags     = AI_ADDRCONFIG;

  		hints.ai_family    = AF_INET;
  		hints.ai_socktype  = SOCK_STREAM;

  		struct addrinfo	*info;
		struct addrinfo *ptr;
		char tmp_host[512];
		int result;

  		result = getaddrinfo(buffer, NULL, &hints, &info);

		if (result == 0)
		{
			ptr = info;
			if (verbose == true)
				cout << "getaddrinfo() is a success" << endl;
			while (ptr != NULL)
			{
    			if (getnameinfo(ptr->ai_addr,ptr->ai_addrlen,tmp_host,512,0,0,0) != 0)
				{
					if (verbose == true)
						cout << "getnameinfo() call failed" << endl;
					ret = -1;
					break;
				}
				if (verbose == true)
					cout << "Host name as returned by getnameinfo call: " << tmp_host << endl;
				ptr = ptr->ai_next;
			}

			freeaddrinfo(info);
		}
		else
		{
			if (verbose == true)
				cout << "getaddrinfo() call failed with returned value = " << result << endl;
			ret = -1;
		}
	}
	else
	{
		cout << "Cant retrieve server host name" << endl;
		ret = -1;
	}

	return ret;

}

//+-------------------------------------------------------------------------
//
// method : 		tac_enabled
// 
// description : 	This function check in DB if the TAC is enabled
//
// The function returns 0 if the TAC is disabled. Otherwise, it returns 1
//
//--------------------------------------------------------------------------

int tac_enabled(void)
{

	int ret = 1;

	setenv("SUPER_TANGO","true",1);

	try
	{
		Tango::Database db;

		string servicename("AccessControl");
		string instname("tango");
		Tango::DbDatum db_datum = db.get_services(servicename,instname);
		vector<string> service_list;
		db_datum >> service_list;

		if (service_list.empty() == true)
			ret = 0;
	}
	catch (Tango::DevFailed &e)
	{
		ret = 0;
	}

	return ret;
}

//+-------------------------------------------------------------------------
//
// method : 		ping_device
// 
// description : 	This function periodically chechs a device avaibility
//
// argument : in : 	- nb_sec : Max time (in sec) to do re-try in case of failure
//					- dev_name : The device name
//
// The function returns 0 is everything is fine. Otherwise, it returns -1
//
//--------------------------------------------------------------------------

int ping_device(char *dev_name,int nb_sec)
{
	int ret = 0;

	int nb_loop;
	bool infinite = false;

	if (nb_sec == 0)
		nb_loop = 1;
	else if (nb_sec < 0)
	{
		infinite = true;
		nb_loop = 2;
	}
	else
		nb_loop = nb_sec << 1;

//
// re-try the call every 500 mS
//

	struct timespec ts;
	ts.tv_sec = 0;
	ts.tv_nsec = 500000000;

	while(nb_loop > 0)
	{

		int res = check_dev(dev_name);
		if (res == 0)
		{
			ret = 0;
			nb_loop = 0;
		}
		else
		{
			ret = -1;
			if (infinite == false)
				--nb_loop;
		}

		if (nb_loop != 0)
			nanosleep(&ts,NULL);
	}

	return ret;	
}

//+-------------------------------------------------------------------------
//
// method : 		check_dev
// 
// description : 	This function connect to a device and try to ping it
//
// argument : in : 	- dev_name : The device name
//
// The function returns 0 is everything is fine. Otherwise, it returns -1
//
//--------------------------------------------------------------------------

int check_dev(char *dev_name)
{
	int ret = 0;

	try
	{
		Tango::DeviceProxy dev(dev_name);

		dev.ping();
	}
	catch (Tango::DevFailed &e)
	{
		ret = -1;
	}

	return ret;

}
