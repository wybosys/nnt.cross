#ifndef __NNTCROSS_XMLOBJ_H_INCLUDED
#define __NNTCROSS_XMLOBJ_H_INCLUDED

#include "property.hpp"

CROSS_BEGIN

#ifdef TINYXML2_INCLUDED
typedef TINYXML_NS::XMLDocument XmlObject;
#endif

#ifdef NNT_DARWIN
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

extern NNT_API string xml_encode(XmlObject const&);

extern NNT_API shared_ptr<XmlObject> xml_decode(string const&);

extern NNT_API shared_ptr<XmlObject> toxmlobj(Property const&);

extern NNT_API shared_ptr<XmlObject> toxmlobj(shared_ptr<Property> const&);

extern NNT_API shared_ptr<Property> toproperty(XmlObject const&);

extern NNT_API shared_ptr<Property> toproperty(shared_ptr<XmlObject> const&);

CROSS_END

#endif
