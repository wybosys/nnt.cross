#include "cross.hpp"
#include "json.hpp"
#include <sstream>
#include <json/json.h>

CROSS_BEGIN

string json_encode(JsonObject const& obj, JsonEncodeOption const& opt) {
    ostringstream oss;
    Json::StreamWriterBuilder builder;
    if (!opt.pretty)
        builder.settings_["indentation"] = "";
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

shared_ptr<JsonObject> tojsonobj(Property const& po) {
    shared_ptr<JsonObject> r;
    switch (po.vt) {
    default:
    case Property::VT::NIL:
        r = make_shared<JsonObject>(Json::nullValue);
        break;
    case Property::VT::INTEGER:
        r = make_shared<JsonObject>(po.toInteger());
        break;
    case Property::VT::NUMBER:
        r = make_shared<JsonObject>(po.toNumber());
        break;
    case Property::VT::BOOLEAN:
        r = make_shared<JsonObject>(po.toBool());
        break;
    case Property::VT::STRING:
        r = make_shared<JsonObject>(po.toString());
        break;
    case Property::VT::ARRAY:
        r = make_shared<JsonObject>(Json::arrayValue);
        for (auto &e : po.array()) {
            r->append(*tojsonobj(*e));
        }
        break;
    case Property::VT::MAP:
        r = make_shared<JsonObject>(Json::objectValue);
        for (auto &e : po.map()) {
            (*r)[(string const&)e.first] = *tojsonobj(*e.second);
        }
        break;
    }
    return r;
}

shared_ptr<Property> toproperty(JsonObject const& jo) {
    shared_ptr<Property> r;
    switch (jo.type()) {
    default:
    case Json::nullValue:
        r = make_shared<Property>();
        break;
    case Json::intValue:
        r = make_shared<Property>((Property::integer)jo.asInt());
        break;
    case Json::uintValue:
        r = make_shared<Property>((Property::integer)jo.asUInt());
        break;
    case Json::realValue:
        r = make_shared<Property>((Property::number)jo.asDouble());
        break;
    case Json::stringValue:
        r = make_shared<Property>(jo.asString());
        break;
    case Json::booleanValue:
        r = make_shared<Property>(jo.asBool());
        break;
    case Json::arrayValue:
        r = make_shared<Property>();
        r->array();
        for (auto iter = jo.begin(); iter != jo.end(); ++iter) {
            r->array().emplace_back(toproperty(*iter));
        }
        break;
    case Json::objectValue:
        r = make_shared<Property>();        
        r->map();
        for (auto iter = jo.begin(); iter != jo.end(); ++iter) {
            auto n = toproperty(*iter);
            n->name = iter.name();
            r->map()[n->name] = n;
        }
        break;
    }
    return r;
}

shared_ptr<JsonObject> tojsonobj(shared_ptr<Property> const& obj) {
    return obj ? tojsonobj(*obj) : nullptr;
}

shared_ptr<Property> toproperty(shared_ptr<JsonObject> const& obj) {
    return obj ? toproperty(*obj) : nullptr;
}

CROSS_END