#ifndef __NNTCROSS_JSON_H_INCLUDED
#define __NNTCROSS_JSON_H_INCLUDED

#include "property.hpp"

namespace Json
{
    class Value;
}

CROSS_BEGIN

typedef Json::Value JsonObject;

struct JsonEncodeOption {
    bool pretty = false;
};

extern NNT_API string json_encode(JsonObject const&, JsonEncodeOption const& opt = JsonEncodeOption());

extern NNT_API shared_ptr<JsonObject> json_decode(string const&);

extern NNT_API shared_ptr<JsonObject> tojsonobj(Property const&);

extern NNT_API shared_ptr<Property> toproperty(JsonObject const&);

CROSS_END

#endif