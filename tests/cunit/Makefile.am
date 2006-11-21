AM_CFLAGS = @CFLAGS@

INCLUDES = \
   $(OPENWSMAN_CFLAGS) 

	
wsman_cunit_tests_LDADD = \
       $(OPENWSMAN_LIBS) -lcunit -lcurl


wsman_cunit_tests_SOURCES = run_tests.c \
		    common.c \
		    common.h \
		    identify.c \
		    enumeration.c \
		    transfer_get.c \
		    transfer_put.c 

noinst_PROGRAMS = wsman_cunit_tests 

