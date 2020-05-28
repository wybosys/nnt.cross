#ifndef __NNTCROSS_JSON_H_INCLUDED
#define __NNTCROSS_JSON_H_INCLUDED

#include <json/json.h>

CROSS_BEGIN

typedef Json::Value JsonObject;

struct JsonEncodeOption {
    bool pretty = false;
};

extern NNT_API string json_encode(JsonObject const&, JsonEncodeOption const& opt = JsonEncodeOption());
extern NNT_API shared_ptr<JsonObject> json_decode(string const&);

CROSS_END

#endif