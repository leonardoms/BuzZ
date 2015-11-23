
#include <buzzData_pgsql.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libpq-fe.h>

struct _pgsql_private {
    char    *host;
    char    *port;
    char    *dbname;
    char    *user;
    char    *passwd;
    PGconn  *con;
    PGresult   *res;
    char    *config_tbl;
    char    *data_tbl;
};

struct _buzzDataPgSQL {
    buzzData  data;
    struct _pgsql_private *private;
};

char
buzz_data_connect_pgsql(buzzData *self);

void
buzz_data_disconnect_pgsql(buzzData *self);

buzzValue_t*
buzz_data_read_config_pgsql(buzzData *self, const char *key);

void
buzz_data_write_config_pgsql(buzzData *self, buzzValue_t  *buzzValue);

void
buzz_data_write_pgsql(buzzData *self,  buzzValueList_t  *bvList);

buzzDataPgSQL*
buzz_data_create_pgsql() {
    buzzDataPgSQL *bzpgsql = NULL;
    buzzData      *bzdata;

    bzpgsql = BUZZ_DATA_PGSQL( malloc(sizeof(buzzDataPgSQL)) );
    if( bzpgsql == NULL )
        return NULL;

    bzdata  = buzz_data_create_default();
    if( bzdata == NULL ) {
        free( bzpgsql );
        return NULL;
    }

    memcpy( (void*)bzpgsql, (void*)bzdata, sizeof(buzzData) );

    bzpgsql->private = (struct _pgsql_private*)malloc(sizeof(struct _pgsql_private));
    if( bzpgsql->private == NULL ) {
      free(bzpgsql);
      return NULL;
    }
    memset(bzpgsql->private, 0, sizeof(struct _pgsql_private));

    BUZZ_DATA(bzpgsql)->name = "PostgreSQL";
    BUZZ_DATA(bzpgsql)->connect = buzz_data_connect_pgsql;
    BUZZ_DATA(bzpgsql)->disconnect = buzz_data_disconnect_pgsql;
    BUZZ_DATA(bzpgsql)->write_config = buzz_data_write_config_pgsql;
    BUZZ_DATA(bzpgsql)->read_config = buzz_data_read_config_pgsql;
    BUZZ_DATA(bzpgsql)->write = buzz_data_write_pgsql;
    return bzpgsql;
}

void
buzz_data_pgsql_set_datatable(buzzDataPgSQL *bzpgsql, char* tbl_name) {
    bzpgsql->private->data_tbl = tbl_name;
}

void
buzz_data_pgsql_configure(buzzDataPgSQL *bzpgsql, const char *host, const char *port,
                          const char *database, const char *user, const char* passwd,
                          const char *config_tbl) {
    if( bzpgsql == NULL )
        return;

    if( host == NULL )
      bzpgsql->private->host = NULL;
    else {
      bzpgsql->private->host = (char*)malloc(strlen(host));
      strcpy(bzpgsql->private->host, host);
    }

    if( port == NULL )
      bzpgsql->private->port = NULL;
    else {
      bzpgsql->private->port = (char*)malloc(strlen(port));
      strcpy(bzpgsql->private->port, port);
    }

    if( database == NULL )
      bzpgsql->private->dbname = NULL;
    else {
      bzpgsql->private->dbname = (char*)malloc(strlen(database));
      strcpy(bzpgsql->private->dbname, database);
    }

    if( user == NULL )
      bzpgsql->private->user = NULL;
    else {
      bzpgsql->private->user = (char*)malloc(strlen(user));
      strcpy(bzpgsql->private->user, user);
    }

    if( passwd == NULL )
      bzpgsql->private->passwd = NULL;
    else {
      bzpgsql->private->passwd = (char*)malloc(strlen(passwd));
      strcpy(bzpgsql->private->passwd, passwd);
    }

    if( config_tbl == NULL )
      bzpgsql->private->config_tbl = NULL;
    else {
      bzpgsql->private->config_tbl = (char*)malloc(strlen(config_tbl));
      strcpy(bzpgsql->private->config_tbl, config_tbl);
    }

}

char
buzz_data_connect_pgsql(buzzData *self) {
    char  *str = NULL;
    unsigned  str_sz = 0;
    char  dbconfig[] = "host=%s port=%s dbname=%s user=%s password=%s";

    if( BUZZ_DATA_PGSQL(self)->private->dbname == NULL ||
        BUZZ_DATA_PGSQL(self)->private->user == NULL ||
        BUZZ_DATA_PGSQL(self)->private->passwd == NULL ) {
            fprintf(stderr, "error [%s]: unconfigured buzzDataManager.\n", BUZZ_DATA(self)->name );
            return FALSE;
        }

    if( BUZZ_DATA_PGSQL(self)->private->host == NULL )
        BUZZ_DATA_PGSQL(self)->private->host = "localhost";

    if( BUZZ_DATA_PGSQL(self)->private->port == NULL )
        BUZZ_DATA_PGSQL(self)->private->port = "5432";

    str_sz =  strlen(dbconfig) +
              strlen(BUZZ_DATA_PGSQL(self)->private->host) +
              strlen(BUZZ_DATA_PGSQL(self)->private->port) +
              strlen(BUZZ_DATA_PGSQL(self)->private->dbname) +
              strlen(BUZZ_DATA_PGSQL(self)->private->user) +
              strlen(BUZZ_DATA_PGSQL(self)->private->passwd) + 1;

    str = (char*)malloc(str_sz);
    if( str == NULL ) {
        fprintf(stderr, "error [%s]: connect failed. (memory not allocated)\n", BUZZ_DATA(self)->name );
        return FALSE;
    }
    sprintf(str, dbconfig,  BUZZ_DATA_PGSQL(self)->private->host,
                            BUZZ_DATA_PGSQL(self)->private->port,
                            BUZZ_DATA_PGSQL(self)->private->dbname,
                            BUZZ_DATA_PGSQL(self)->private->user,
                            BUZZ_DATA_PGSQL(self)->private->passwd
          );

    BUZZ_DATA_PGSQL(self)->private->con = PQconnectdb( str );
    free( str );

    if( PQstatus(BUZZ_DATA_PGSQL(self)->private->con) != CONNECTION_OK ) {
        fprintf(stderr, "error [%s]: connection failed. (%s)\n", BUZZ_DATA(self)->name, PQerrorMessage(BUZZ_DATA_PGSQL(self)->private->con) );
        return FALSE;
    }

    return TRUE;
}

void
buzz_data_disconnect_pgsql(buzzData *self) {
    if( BUZZ_DATA_PGSQL(self)->private->con != NULL ) {
        PQfinish( BUZZ_DATA_PGSQL(self)->private->con );
        BUZZ_DATA_PGSQL(self)->private->con = NULL;
    }
}

buzzValue_t*
buzz_data_read_config_pgsql(buzzData *self, const char *key) {
    buzzValue_t *val;
    const char  sql[] = "SELECT key,value FROM %s WHERE key=%s;";
    char  *str = NULL, *str_escaped = NULL;
    unsigned  str_sz = 0;

    if( key == NULL )
        return NULL;

    str_escaped = PQescapeLiteral(BUZZ_DATA_PGSQL(self)->private->con, key, strlen(key));
    if( str_escaped == NULL ) {
        fprintf(stderr, "error [%s]: read config failed. (%s)\n", BUZZ_DATA(self)->name, PQerrorMessage(BUZZ_DATA_PGSQL(self)->private->con) );
        return NULL;
    }

    str_sz =  strlen(BUZZ_DATA_PGSQL(self)->private->config_tbl) + strlen(str_escaped) + strlen(sql) + 1;
    str = (char*)malloc(str_sz);
    if( str == NULL ) {
        fprintf(stderr, "error [%s]: read config failed. (memory not allocated)\n", BUZZ_DATA(self)->name );
        return NULL;
    }

    sprintf(str, sql, BUZZ_DATA_PGSQL(self)->private->config_tbl, str_escaped);

    BUZZ_DATA_PGSQL(self)->private->res = PQexec(BUZZ_DATA_PGSQL(self)->private->con,
                                                 str );
    PQfreemem(str_escaped);
    free(str);

    if( PQresultStatus(BUZZ_DATA_PGSQL(self)->private->res) != PGRES_TUPLES_OK ) {
        fprintf(stderr, "error [%s]: read config failed. (%s)\n", BUZZ_DATA(self)->name, PQerrorMessage(BUZZ_DATA_PGSQL(self)->private->con) );
        return NULL;
    }

    if( PQntuples(BUZZ_DATA_PGSQL(self)->private->res) < 1 )
        return NULL;

    val = buzz_value_string_new(  key,
                                  PQgetvalue(BUZZ_DATA_PGSQL(self)->private->res, 0, 1)
                                );
    PQclear( BUZZ_DATA_PGSQL(self)->private->res );

    return val;
}

void
buzz_data_write_config_pgsql(buzzData *self, buzzValue_t  *buzzValue) {
    const char  sql_insert[] = "INSERT INTO %s (key,value) VALUES(%s,%s);";
    const char  sql_update[] = "UPDATE %s SET key=%s, value=%s WHERE key=%s;";
    const char  sql[] = "SELECT key,value FROM %s WHERE key=%s;";
    char  *str = NULL, *str_escaped = NULL, *str_escaped2 = NULL;
    unsigned  str_sz = 0;

    if( buzzValue == NULL )
        return;

    if( buzzValue->key == NULL )
        return;

    if( buzzValue->type != BUZZ_TYPE_STRING ) {
        fprintf(stderr, "error [%s]: write config failed. (value must be a String)\n", BUZZ_DATA(self)->name );
        return;
    }


    str_escaped = PQescapeLiteral(BUZZ_DATA_PGSQL(self)->private->con, buzzValue->key, strlen(buzzValue->key));
    if( str_escaped == NULL ) {
        fprintf(stderr, "error [%s]: write config failed. (%s)\n", BUZZ_DATA(self)->name, PQerrorMessage(BUZZ_DATA_PGSQL(self)->private->con) );
        return;
    }

    str_sz =  strlen(str_escaped) + strlen(sql) +
              strlen(BUZZ_DATA_PGSQL(self)->private->config_tbl) + 1;

    str = (char*)malloc(str_sz);
    if( str == NULL ) {
        fprintf(stderr, "error [%s]: write config failed. (memory not allocated)\n", BUZZ_DATA(self)->name );
        return;
    }
    sprintf(str, sql, BUZZ_DATA_PGSQL(self)->private->config_tbl, str_escaped);


    BUZZ_DATA_PGSQL(self)->private->res = PQexec(BUZZ_DATA_PGSQL(self)->private->con,
                                                 str );
    PQfreemem(str_escaped);
    free( str );

    if( PQresultStatus(BUZZ_DATA_PGSQL(self)->private->res) != PGRES_TUPLES_OK ) {
        fprintf(stderr, "error [%s]: write config failed. (%s)\n", BUZZ_DATA(self)->name, PQerrorMessage(BUZZ_DATA_PGSQL(self)->private->con) );
        return;
    }

    str_escaped = PQescapeLiteral(BUZZ_DATA_PGSQL(self)->private->con, buzzValue->key, strlen(buzzValue->key));
    if( str_escaped == NULL ) {
        fprintf(stderr, "error [%s]: write config failed. (%s)\n", BUZZ_DATA(self)->name, PQerrorMessage(BUZZ_DATA_PGSQL(self)->private->con) );
        return;
    }

    str_escaped2 = PQescapeLiteral(BUZZ_DATA_PGSQL(self)->private->con, (char*)buzzValue->value, strlen((char*)buzzValue->value));
    if( str_escaped2 == NULL ) {
        fprintf(stderr, "error [%s]: write config failed. (%s)\n", BUZZ_DATA(self)->name, PQerrorMessage(BUZZ_DATA_PGSQL(self)->private->con) );
        return;
    }

    if( PQntuples(BUZZ_DATA_PGSQL(self)->private->res) == 0 ) {
        // INSERT
        PQclear( BUZZ_DATA_PGSQL(self)->private->res );

        str_sz =  strlen(str_escaped) + strlen(sql_insert) +
                  strlen(BUZZ_DATA_PGSQL(self)->private->config_tbl) +
                  strlen(str_escaped2) + 1;
        str = (char*)malloc(str_sz);
        sprintf(str, sql_insert, BUZZ_DATA_PGSQL(self)->private->config_tbl,
                str_escaped, (char*)buzzValue->value);
    } else {
        // UPDATE
        PQclear( BUZZ_DATA_PGSQL(self)->private->res );

        str_sz =  strlen(str_escaped) * 2 + strlen(sql_update) +
                  strlen(BUZZ_DATA_PGSQL(self)->private->config_tbl) +
                  strlen(str_escaped2) + 1;
        str = (char*)malloc(str_sz);
        sprintf(str, sql_update, BUZZ_DATA_PGSQL(self)->private->config_tbl,
                str_escaped, str_escaped2, str_escaped);
    }

    PQfreemem(str_escaped);
    PQfreemem(str_escaped2);

    BUZZ_DATA_PGSQL(self)->private->res = PQexec(BUZZ_DATA_PGSQL(self)->private->con,
                                                 str );
    free( str );

    if( PQresultStatus(BUZZ_DATA_PGSQL(self)->private->res) != PGRES_COMMAND_OK ) {
       fprintf(stderr, "error [%s]: write config failed. (%s)\n", BUZZ_DATA(self)->name, PQerrorMessage(BUZZ_DATA_PGSQL(self)->private->con) );
       return;
    }
    PQclear( BUZZ_DATA_PGSQL(self)->private->res );

}

void
buzz_data_write_callback(buzzValue_t *bzvalue, void *udata) {
    // udata[0] must contains char* to the columns query string
    // udata[1] must contains char* to the value query string
    char  *new_keys = NULL, *new_values = NULL,
          *key_scaped = NULL, *value_scaped = NULL;
    char* tmp;

    if( udata == NULL )
        return;

    if( bzvalue->key == NULL )
        return;

    key_scaped = (char*)malloc(strlen(bzvalue->key) * 2 + 1); // max theorical size
    if( key_scaped == NULL ) {
       fprintf(stderr, "error: buzz_data_write_callback failed. (out of memory)\n" );
       return;
    }
    if( PQescapeString( key_scaped, bzvalue->key, strlen(bzvalue->key) ) == 0 ) {
        free(key_scaped);
        fprintf(stderr, "error: buzz_data_write_callback failed. (out of memory)\n" );
        return;
    }

    if( ((char**)udata)[0] == NULL ) {
        new_keys = (char*)malloc( strlen(key_scaped) + 1 );
        sprintf(new_keys, "%s", key_scaped);
    } else {
        new_keys = (char*)malloc( strlen(((char**)udata)[0]) + strlen(key_scaped) + 2 );
        sprintf(new_keys, "%s,%s", (char*)((char**)udata)[0], key_scaped);
    }

    free(key_scaped);

    switch( bzvalue->type ) {
        case BUZZ_TYPE_INTEGER:
            if( ((char**)udata)[1] == NULL ) {
                new_values = (char*)malloc( 32 ); // FIXME: arbitrary value
                sprintf(new_values, "%d", buzz_value_integer(bzvalue));
            } else {
                new_values = (char*)malloc( strlen(((char**)udata)[1]) + 32 + 2 );
                sprintf(new_values, "%s,%d", (char*)((char**)udata)[1], buzz_value_integer(bzvalue));
            }
            break;
        case BUZZ_TYPE_NUMBER:
            if( ((char**)udata)[1] == NULL ) {
                new_values = (char*)malloc( 32 ); // FIXME: arbitrary value
                sprintf(new_values, "%g", buzz_value_number(bzvalue));
            } else {
                new_values = (char*)malloc( strlen(((char**)udata)[1]) + 32 + 2 );
                sprintf(new_values, "%s,%g", (char*)((char**)udata)[1], buzz_value_number(bzvalue));
            }
            break;
        case BUZZ_TYPE_STRING:
            tmp = buzz_value_string(bzvalue);

            value_scaped = (char*)malloc(strlen(tmp) * 2 + 1); // max theorical size
            if( value_scaped == NULL ) {
               fprintf(stderr, "error: buzz_data_write_callback failed. (out of memory)\n" );
               return;
            }
            if( PQescapeString( value_scaped, tmp, strlen(tmp) ) == 0 ) {
                free(value_scaped);
                fprintf(stderr, "error: buzz_data_write_callback failed. (out of memory)\n" );
                return;
            }

            if( ((char**)udata)[1] == NULL ) {
                new_values = (char*)malloc( strlen( value_scaped ) + 4 );
                sprintf(new_values, "'%s'", value_scaped);
            } else {
                new_values = (char*)malloc( strlen(((char**)udata)[1]) + strlen( value_scaped ) + 5 );
                sprintf(new_values, "%s,'%s'", (char*)((char**)udata)[1], value_scaped);
            }
            break;
        default:
            break;
    }

    if( value_scaped != NULL )
        free(value_scaped);

    if( new_keys != NULL && new_values != NULL ) {
        ((char**)udata)[0] = new_keys;
        ((char**)udata)[1] = new_values;
    }

}

void
buzz_data_write_pgsql(buzzData *self,  buzzValueList_t  *bvlist) {
    char  *keys_values[2];
    char  *str = NULL;
    unsigned  str_sz = 0;
    const char  sql_insert[] = "INSERT INTO %s (%s) VALUES(%s);";

    keys_values[0] = NULL;
    keys_values[1] = NULL;

    if( BUZZ_DATA_PGSQL(self)->private->data_tbl == NULL )
        BUZZ_DATA_PGSQL(self)->private->data_tbl = PGSQL_DATA_TABLE_DEFAULT;

    buzz_value_list_foreach( bvlist, buzz_data_write_callback, (void*)keys_values );

    if( keys_values[0] == NULL || keys_values[1] == NULL )
        return;

    str_sz =  strlen(keys_values[0]) + strlen(sql_insert) +
              strlen(BUZZ_DATA_PGSQL(self)->private->data_tbl) +
              strlen(keys_values[1]) + 1;
    str = (char*)malloc(str_sz);
    sprintf(str, sql_insert, BUZZ_DATA_PGSQL(self)->private->data_tbl,
            keys_values[0], keys_values[1]);

    printf("%s\n----\n", str);
    BUZZ_DATA_PGSQL(self)->private->res = PQexec(BUZZ_DATA_PGSQL(self)->private->con,
                                                 str );
    free( str );

    if( PQresultStatus(BUZZ_DATA_PGSQL(self)->private->res) != PGRES_COMMAND_OK ) {
       fprintf(stderr, "error [%s]: write failed. (%s)\n", BUZZ_DATA(self)->name, PQerrorMessage(BUZZ_DATA_PGSQL(self)->private->con) );
       return;
    }
    PQclear( BUZZ_DATA_PGSQL(self)->private->res );

    printf("(%s)->(%s)\n\t---\n", (char*)keys_values[0],  (char*)keys_values[1]);
}
