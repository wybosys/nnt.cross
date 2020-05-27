#ifndef __NNTCROSS_JSON_H_INCLUDED
#define __NNTCROSS_JSON_H_INCLUDED

#include "jsoncpp/json/json.h"

CROSS_BEGIN

typedef Json::Value JsonObject;

extern NNT_API string json_encode(JsonObject const&);
extern NNT_API shared_ptr<JsonObject> json_decode(string const&);

CROSS_END

#endif