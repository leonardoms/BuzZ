#ifndef BUZZ_DATA_PGSQL_H
#define BUZZ_DATA_PGSQL_H

#include "buzzDataInterface.h"
#include "buzzValue.h"

#define PGSQL_DATA_TABLE_DEFAULT  "universe"

typedef struct _buzzDataPgSQL   buzzDataPgSQL;

#define BUZZ_DATA_PGSQL(obj)  ( (buzzDataPgSQL*)obj )

buzzDataPgSQL*
buzz_data_create_pgsql();

void
buzz_data_pgsql_configure( buzzDataPgSQL *bzpgsql, const char *host, const char *port,
                           const char *database, const char *user, const char* passwd,
                           const char *config_tbl );

void
buzz_data_pgsql_set_datatable(buzzDataPgSQL *bzpgsql, char* tbl_name);

#endif
