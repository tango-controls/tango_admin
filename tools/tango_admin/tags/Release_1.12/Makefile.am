
INCLUDES = -I$(top_srcdir)/lib/cpp/client \
	   -I$(top_srcdir)/lib/cpp/server $(ORB_INCLUDE_PREFIX) \
           -I$(top_srcdir)/lib/cpp/log4tango/include \
           -I$(top_builddir)/lib/cpp/log4tango/include $(LIBZMQ_CFLAGS)

LDADD = -L$(top_builddir)/lib/cpp/server -ltango \
        -L$(top_builddir)/lib/cpp/log4tango/src -llog4tango \
		$(LIBZMQ_LIBS)

bin_PROGRAMS=tango_admin

tango_admin_SOURCES=tango_admin.cpp \
					anyoption.cpp \
					anyoption.h


