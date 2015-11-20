#ifndef BUZZ_TWITTER_H
#define BUZZ_TWITTER_H

#include "buzzInterface.h"
#include "buzzValue.h"

#define TWITTER_STREAM  0
#define TWITTER_SEARCH  1

#define TWITTER_STREAM_URL  "https://stream.twitter.com/1.1/statuses/filter.json?track=%s&language=%s"
#define TWITTER_SEARCH_URL  "https://api.twitter.com/1.1/search/tweets.json?q=%s&language=%s"

typedef struct _buzzTwitter   buzzTwitter;

#define BUZZ_TWITTER(obj)  ( (buzzTwitter*)obj )

buzzTwitter*
buzz_create_twitter();

#endif
