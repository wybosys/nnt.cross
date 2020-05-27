#include "cross.h"
#include "json.h"
#include <sstream>

CROSS_BEGIN

string json_encode(JsonObject const& obj) {
    ostringstream oss;
    Json::StreamWriterBuilder builder;
    unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    writer->write(obj, &oss);
    return oss.str();
}

shared_ptr<JsonObject> json_decode(string const& str) {
    auto r = make_shared<JsonObject>();
    Json::CharReaderBuilder builder;
    unique_ptr<Json::CharReader> const reader(builder.newCharReader());
    if (!reader->parse(str.c_str(), str.c_str() + str.length(), &*r, nullptr))
        return nullptr;
    return r;
}

CROSS_END