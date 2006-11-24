AM_CFLAGS = @CFLAGS@

INCLUDES = \
   $(OPENWSMAN_CFLAGS) 

	
wsman_cunit_tests_LDADD = \
       $(OPENWSMAN_LIBS) -lcunit


wsman_cunit_tests_SOURCES = run_tests.c \
		    common.c \
		    common.h \
		    identify.c \
		    invoke.c \
		    enumeration.c \
		    transfer_get.c \
		    pull.c \
		    transfer_put.c 

noinst_PROGRAMS = wsman_cunit_tests 

