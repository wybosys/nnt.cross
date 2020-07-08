#ifndef __NNTCROSS_XMLOBJ_H_INCLUDED
#define __NNTCROSS_XMLOBJ_H_INCLUDED

#include "property.hpp"

CROSS_BEGIN

#ifdef TINYXML2_INCLUDED
#define HAS_XMLOBJECT
typedef tinyxml2::XMLDocument XmlObject;
#endif

#if defined(NNT_DARWIN) && !defined(HAS_XMLOBJECT)
#define HAS_XMLOBJECT
class XmlObject {
public:
#ifdef NNT_OBJC
    typedef NSObject* obj_type;
#else
    typedef void* obj_type;
#endif
    explicit XmlObject(obj_type o=nullptr) : obj(o) {}
    obj_type obj;
};
#endif

#ifdef HAS_XMLOBJECT

// 编码xml
extern NNT_API string xml_encode(XmlObject const&);

// 解码xml
extern NNT_API shared_ptr<XmlObject> xml_decode(string const&);

// property对象转换为xml对象
extern NNT_API shared_ptr<XmlObject> toxmlobj(Property const&);

extern NNT_API shared_ptr<XmlObject> toxmlobj(shared_ptr<Property> const&);

// xml对象转换成标准property对象
extern NNT_API shared_ptr<Property> toproperty(XmlObject const&);

extern NNT_API shared_ptr<Property> toproperty(shared_ptr<XmlObject> const&);

#endif

// property对象编码为字符串
extern NNT_API string property_toxml(Property const&);

// xml字符串解码为property对象
extern NNT_API shared_ptr<Property> xml_toproperty(string const&);

CROSS_END

#endif
