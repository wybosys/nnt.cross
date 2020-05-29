#ifndef __NNTCROSS_XML_H_INCLUDED
#define __NNTCROSS_XML_H_INCLUDED

#include "property.h"

#define TINYXML_NS tinyxml2

namespace TINYXML_NS
{
    class XMLDocument;
}

CROSS_BEGIN

typedef TINYXML_NS::XMLDocument XmlObject;

extern NNT_API string xml_encode(XmlObject const&);

extern NNT_API shared_ptr<XmlObject> xml_decode(string const&);

extern NNT_API shared_ptr<XmlObject> toxmlobj(Property const&);

extern NNT_API shared_ptr<Property> toproperty(XmlObject const&);

CROSS_END

#endif