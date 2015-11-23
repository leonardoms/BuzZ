
lib_LTLIBRARIES = libbuzz.la
libbuzz_la_CFLAGS = -I../include -I/usr/include/postgresql/ -fPIC
libbuzz_la_LDFLAGS= -shared -fPIC -version-info 0:0:0
libbuzz_la_SOURCES =  buzzDataInterface.c \
                      buzzData_pgsql.c \
                      buzzData_stdio.c \
                      buzz_facebook.c \
                      buzzInterface.c \
                      buzz_twitter.c \
                      buzzValue.c
