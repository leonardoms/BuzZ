#ifndef BUZZ_DATA_STDIO_H
#define BUZZ_DATA_STDIO_H

#include "buzzDataInterface.h"
#include "buzzValue.h"

typedef struct _buzzDataStdIO   buzzDataStdIO;

#define BUZZ_DATA_STDIO(obj)  ( (buzzDataStdIO*)obj )

buzzDataStdIO*
buzz_data_create_stdio();

#endif
