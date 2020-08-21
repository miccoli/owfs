libowcapi_la_DEPENDENCIES = $(OWLIB_LD)/libow.la

lib_LTLIBRARIES = libowcapi.la

libowcapi_la_SOURCES = owcapi.c

if HAVE_CYGWIN
 libowcapi_la_LDFLAGS = -low ${LD_EXTRALIBS} -shared -no-undefined 
else
 libowcapi_la_LDFLAGS = -L$(OWLIB_LD) -low \
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE) \
	-release $(LT_RELEASE) ${PTHREAD_LIBS} ${LD_EXTRALIBS} \
	-shared -shrext .so
endif

AM_CFLAGS = -I$(srcdir)/../include \
	-I$(OWLIB_INC) \
	-fexceptions \
	-Wall \
	-W \
	-Wundef \
	-Wshadow \
	-Wpointer-arith \
	-Wcast-qual \
	-Wcast-align \
	-Wstrict-prototypes \
	-Wredundant-decls \
	-D__EXTENSIONS__ \
	${EXTRACFLAGS} \
	${PTHREAD_CFLAGS} \
	${LIBUSB_CFLAGS} \
	${LIBAVAHI_CFLAGS} \
	${PIC_FLAGS}
