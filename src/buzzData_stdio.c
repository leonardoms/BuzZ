
#include <buzzData_stdio.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct _buzzDataStdIO_private {
  buzzValueList_t* config;
};

struct _buzzDataStdIO {
    buzzData   bzData;
    struct _buzzDataStdIO_private *private;
};

void
buzz_data_write_config_stdio(buzzData *self, buzzValue_t  *buzzValue) {

    //TODO: se key ja existe, apenas subistiuir value
    BUZZ_DATA_STDIO(self)->private->config = buzz_value_list_add(
                                    BUZZ_DATA_STDIO(self)->private->config,
                                    buzzValue );
}

buzzValue_t  *
buzz_data_read_config_stdio(buzzData *self, const char* key) {

    return buzz_value_list_get(   BUZZ_DATA_STDIO(self)->private->config,
                                  key );
}

void write_stdio_callback(buzzValue_t* bzvalue, void  *user_data) {
    switch (bzvalue->type) {
      case BUZZ_TYPE_INTEGER:
          printf("%d\t", buzz_value_integer(bzvalue) );
          break;
      case BUZZ_TYPE_NUMBER:
          printf("%f\t", buzz_value_number(bzvalue) );
          break;
      case BUZZ_TYPE_STRING:
          printf("%s\t", buzz_value_string(bzvalue) );
          break;
      case BUZZ_TYPE_VOID:
      default:
          break;
    };
}

void
buzz_data_write_stdio(buzzData *self, buzzValueList_t  *bvList) {
    buzz_value_list_foreach(bvList, write_stdio_callback, NULL);
    printf("\n");
}

buzzDataStdIO*
buzz_data_create_stdio() {
    buzzDataStdIO *bzstdio = NULL;
    buzzData      *bzdata = NULL;

    bzstdio = BUZZ_DATA_STDIO( malloc(sizeof(buzzDataStdIO)) );
    if(bzstdio == NULL)
        return NULL;

    bzdata = buzz_data_create_default();
    if( bzdata == NULL ) {
        free( bzstdio );
        return NULL;
    }

    memcpy( (void*)bzstdio, (void*)bzdata, sizeof(buzzData) );

    bzstdio->private = (struct _buzzDataStdIO_private*)malloc(sizeof(struct _buzzDataStdIO_private));

    memset((void*)bzstdio->private, 0, sizeof(struct _buzzDataStdIO_private));

    BUZZ_DATA(bzstdio)->name = "STDIO";
    BUZZ_DATA(bzstdio)->write_config = buzz_data_write_config_stdio;
    BUZZ_DATA(bzstdio)->read_config = buzz_data_read_config_stdio;
    BUZZ_DATA(bzstdio)->write = buzz_data_write_stdio;

    return bzstdio;
}
