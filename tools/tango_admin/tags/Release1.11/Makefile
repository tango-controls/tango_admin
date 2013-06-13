#=============================================================================
#
# file :        Makefile
#
# description : Makefile for the litle notifd2db command
#
# $Author$
#
# $Revision$
#
# $Log$
# Revision 1.1.1.1  2010/03/12 12:34:36  taurel
# Initial check-in
#
#
#
# copyleft :    European Synchrotron Radiation Facility
#               BP 220, Grenoble 38043
#               FRANCE
#
#=============================================================================

CC = c++
BIN_DIR = ubuntu1204
 
TANGO_HOME   =  /segfs/tango/release
   
INCLUDE_DIRS =  -I. -I$(TANGO_HOME)/$(BIN_DIR)/include
	       
LIB_DIRS     =  -L $(TANGO_HOME)/$(BIN_DIR)/lib
 
       
CXXFLAGS = -g -D_REENTRANT -std=c++0x $(INCLUDE_DIRS)		
LFLAGS =   -g $(LIB_DIRS) \
 	   -ltango -llog4tango \
	   -lCOS4 -lomniORB4 -lomniDynamic4 -lomnithread -lpthread -lzmq -ldl

%.o: %.cpp 
	$(CC) $(CXXFLAGS) -c $<
  
all: tango_admin
 
tango_admin:	anyoption.o tango_admin.o 
	$(CC) anyoption.o tango_admin.o -o $(BIN_DIR)/tango_admin $(LFLAGS)

clean:
	rm -f *.o 
	
