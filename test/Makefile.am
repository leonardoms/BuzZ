bin_PROGRAMS = test facebook
test_SOURCES = test.c
facebook_SOURCES = facebook.c
test_CFLAGS = -g -I/usr/local/include -L/usr/local/lib -lbuzz $(PGSQL_CFLAGS) $(CURL_CFLAGS) $(JSON_CFLAGS) $(OAUTH_CFLAGS)
test_LDFLAGS = $(PGSQL_LIBS) $(CURL_LIBS) $(JSON_LIBS) $(OAUTH_LIBS)
facebook_CFLAGS = -g -I/usr/local/include -L/usr/local/lib -lbuzz $(PGSQL_CFLAGS) $(CURL_CFLAGS) $(JSON_CFLAGS) $(OAUTH_CFLAGS)
facebook_LDFLAGS = $(PGSQL_LIBS) $(CURL_LIBS) $(JSON_LIBS) $(OAUTH_LIBS)


$(TARGETS): src
