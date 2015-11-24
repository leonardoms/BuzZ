bin_PROGRAMS = test
test_SOURCES = test.c
test_CFLAGS = -I/usr/local/include -L/usr/local/lib -lbuzz $(PGSQL_CFLAGS) $(CURL_CFLAGS) $(JSON_CFLAGS) $(OAUTH_CFLAGS)
test_LDFLAGS = $(PGSQL_LIBS) $(CURL_LIBS) $(JSON_LIBS) $(OAUTH_LIBS)

$(TARGETS): src
