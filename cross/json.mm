#import "cross.hpp"
#import "json.hpp"
#import "logger.hpp"
#import "stringbuilder.hpp"

CROSS_BEGIN

string json_encode(JsonObject const &obj, JsonEncodeOption const &opt) {
    NSJSONWritingOptions o = kNilOptions;
    if (opt.pretty)
        o |= NSJSONWritingPrettyPrinted;
    NSError *err;
    auto d = [NSJSONSerialization dataWithJSONObject:obj.obj
                                             options:o
                                               error:&err];
    if (err) {
        Logger::Warn(stringbuilder().space(" ") << "JSON:" << err.code << err.debugDescription.UTF8String);
        return "";
    }
    
    auto s = [[NSString alloc] initWithData:d encoding:NSUTF8StringEncoding];
    return s.UTF8String;
}

shared_ptr<JsonObject> json_decode(string const &str) {
    NSJSONReadingOptions o = NSJSONReadingMutableContainers;
    
    NSError *err;
    NSObject* obj = [NSJSONSerialization JSONObjectWithData:[[NSString stringWithUTF8String:str.c_str()] dataUsingEncoding:NSUTF8StringEncoding]
                                               options:o
                                                 error:&err];
    if (err) {
        Logger::Warn(stringbuilder().space(" ") << "JSON:" << err.code << err.debugDescription.UTF8String);
        return nullptr;
    }
    
    auto r = make_shared<JsonObject>();
    r->obj = obj;
    return r;
}

shared_ptr<JsonObject> tojsonobj(Property const &po) {
    shared_ptr<JsonObject> r;
    switch (po.vt) {
        default:
        case Property::VT::NIL:
            r = make_shared<JsonObject>([NSNull null]);
            break;
        case Property::VT::INTEGER:
            r = make_shared<JsonObject>([NSNumber numberWithInteger:po.toInteger()]);
            break;
        case Property::VT::NUMBER:
            r = make_shared<JsonObject>([NSNumber numberWithDouble:po.toNumber()]);
            break;
        case Property::VT::BOOLEAN:
            r = make_shared<JsonObject>([NSNumber numberWithBool:po.toBool()]);
            break;
        case Property::VT::STRING: {
            auto str = po.toString();
            r = make_shared<JsonObject>([NSString stringWithUTF8String:str.c_str()]);
        } break;
        case Property::VT::ARRAY: {
            r = make_shared<JsonObject>([NSMutableArray arrayWithCapacity:po.array().size()]);
            for (auto &e : po.array()) {
                [(NSMutableArray*)r->obj addObject:tojsonobj(*e)->obj];
            }
        } break;
        case Property::VT::MAP: {
            r = make_shared<JsonObject>([NSMutableDictionary dictionaryWithCapacity:po.map().size()]);
            for (auto &e : po.map()) {
                string const& key = e.first;
                [(NSMutableDictionary*)r->obj setObject:tojsonobj(*e.second)->obj forKey:[NSString stringWithUTF8String:key.c_str()]];
            }
        } break;
    }
    return r;
}

shared_ptr<Property> toproperty(JsonObject const &jo) {
    shared_ptr<Property> r;
    
    if ([jo.obj isKindOfClass:NSNull.class]) {
        r = make_shared<Property>();
    }
    else if ([jo.obj isKindOfClass:NSNumber.class]) {
        auto v = (NSNumber*)jo.obj;
        auto ctyp = v.objCType;
        switch (ctyp[0]) {
            case 'c': case 'B':
                r = make_shared<Property>((bool)v.boolValue);
                break;
            case 'd':
                r = make_shared<Property>((Property::number)v.doubleValue);
                break;
            case 'f':
                r = make_shared<Property>((Property::number)v.floatValue);
                break;
            default:
                r = make_shared<Property>((Property::integer)v.integerValue);
                break;
        }
    }
    else if ([jo.obj isKindOfClass:NSString.class]) {
        auto s = (NSString*)jo.obj;
        r = make_shared<Property>(string(s.UTF8String));
    }
    else if ([jo.obj isKindOfClass:NSArray.class]) {
        auto v = (NSArray*)jo.obj;
        r = make_shared<Property>();
        r->array();
        for (id e in v) {
            JsonObject eo(e);
            r->array().emplace_back(toproperty(eo));
        }
    }
    else if ([jo.obj isKindOfClass:NSDictionary.class]) {
        auto v = (NSDictionary*)jo.obj;
        r = make_shared<Property>();
        r->map();
        for (NSString* k in v) {
            JsonObject eo([v objectForKey:k]);
            auto n = toproperty(eo);
            n->name = k.UTF8String;
            r->map()[n->name] = n;
        }
    }
    else {
        Logger::Critical(stringbuilder() << "处理Json遇到不支持的类型 " << jo.obj.className.UTF8String);
    }
    
    return r;
}

CROSS_END
