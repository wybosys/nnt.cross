#ifndef __NNTCROSS_JSONOBJ_H_INCLUDED
#define __NNTCROSS_JSONOBJ_H_INCLUDED

#include "property.hpp"

CROSS_BEGIN

#ifdef JSON_JSON_H_INCLUDED
#define HAS_JSONOBJECT
typedef Json::Value JsonObject;
#endif

#ifdef NNT_DARWIN
#define HAS_JSONOBJECT
class JsonObject {
public:
#ifdef NNT_OBJC
    typedef NSObject * obj_type;
#else
    typedef void * obj_type;
#endif
    explicit JsonObject(obj_type o=nullptr) : obj(o) {}
    obj_type obj;
};
#endif

struct JsonEncodeOption {
    bool pretty = false;
};

#ifdef HAS_JSONOBJECT

// json对象转换为字符串
extern NNT_API string json_encode(JsonObject const&, JsonEncodeOption const& opt = JsonEncodeOption());

// 字符串解析为对象
extern NNT_API shared_ptr<JsonObject> json_decode(string const&);

// 属性对象转换为json对象
extern NNT_API shared_ptr<JsonObject> tojsonobj(Property const&);

// 属性对象转换为json对象
extern NNT_API shared_ptr<JsonObject> tojsonobj(shared_ptr<Property> const&);

// json对象转换为属性对象
extern NNT_API shared_ptr<Property> toproperty(JsonObject const&);

// json对象转换为属性对象
extern NNT_API shared_ptr<Property> toproperty(shared_ptr<JsonObject> const&);

#endif

// property对象编码为字符串
extern NNT_API string property_tojson(Property const&);

// json字符串解码为property对象
extern NNT_API shared_ptr<Property> json_toproperty(string const&);

CROSS_END

#endif
