
lib_LTLIBRARIES = libbuzz.la
libbuzz_la_CFLAGS = -g $(PGSQL_CFLAGS) $(CURL_CFLAGS) $(JSON_CFLAGS) $(OAUTH_CFLAGS)-I../include -fPIC
libbuzz_la_LDFLAGS= -shared -fPIC -version-info 0:0:0 $(PGSQL_LIBS) $(CURL_LIBS) $(JSON_LIBS) $(OAUTH_LIBS)
libbuzz_la_SOURCES =  buzzDataInterface.c \
                      buzzData_pgsql.c \
                      buzzData_stdio.c \
                      buzz_facebook.c \
                      buzzInterface.c \
                      buzz_twitter.c \
                      buzzValue.c
