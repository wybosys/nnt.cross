#include "cross.h"
#include "xml.h"
#include <tinyxml2/tinyxml2.h>

CROSS_BEGIN

string xml_encode(XmlObject const& xo) 
{
    tinyxml2::XMLPrinter pr;
    xo.Print(&pr);
    return string(pr.CStr(), pr.CStrSize());
}

shared_ptr<XmlObject> xml_decode(string const& str)
{
    auto xo = make_shared<XmlObject>();
    xo->Parse(str.c_str(), str.length());
    if (xo->Error())
        return nullptr;
    return xo;
}

void toxmlobj(Property const& po, tinyxml2::XMLElement& cur)
{
    cur.SetAttribute("vt", (int)po.vt);

#define CUR_SET_VALUE(toval) \
        auto v = po.##toval(); \
    cur.SetAttribute("value", v); \
    cur.SetText(v);

    switch (po.vt) {
    default:
    case Property::VT::NIL: {
    } break;
    case Property::VT::INTEGER: {
        CUR_SET_VALUE(toInteger);
    } break;
    case Property::VT::NUMBER: {
        CUR_SET_VALUE(toNumber);
    } break;
    case Property::VT::BOOLEAN: {
        CUR_SET_VALUE(toBool);
    } break;
    case Property::VT::STRING: {
        auto &v = po.toString();
        cur.SetText(v.c_str());
    } break;
    case Property::VT::ARRAY: {
        for (auto &e : po.array()) {
            auto ele = cur.InsertNewChildElement("element");
            toxmlobj(*e, *ele);
        }
    } break;
    case Property::VT::MAP: {
        for (auto &e : po.map()) {
            auto nm = (string const&)e.first;
            auto ele = cur.InsertNewChildElement("element");
            ele->SetAttribute("key", nm.c_str());
            toxmlobj(*e.second, *ele);
        }
    } break;
    }
}

shared_ptr<XmlObject> toxmlobj(Property const& po)
{
    auto r = make_shared<XmlObject>();
    auto rootName = po.name.empty() ? "root" : po.name;
    auto root = r->NewElement(rootName.c_str());
    r->InsertFirstChild(root);

    toxmlobj(po, *root);
    return r;
}

void toproperty(shared_ptr<Property>& p, tinyxml2::XMLElement const& ele)
{
    auto vt = (Property::VT)ele.IntAttribute("vt");
    switch (vt)
    {
    default:
    case Property::VT::NIL:
        p = make_shared<Property>();
        break;
    case Property::VT::INTEGER:
        p = make_shared<Property>(ele.IntAttribute("value"));
        break;
    case Property::VT::NUMBER:
        p = make_shared<Property>(ele.DoubleAttribute("value"));
        break;
    case Property::VT::BOOLEAN:
        p = make_shared<Property>(ele.BoolAttribute("value"));
        break;
    case Property::VT::STRING:
        p = make_shared<Property>(ele.GetText());
        break;
    case Property::VT::ARRAY: {
        p = make_shared<Property>();
        p->array();
        auto each = ele.FirstChildElement("element");
        while (each) {
            shared_ptr<Property> t;
            toproperty(t, *each);
            p->array().emplace_back(t);
            each = each->NextSiblingElement();
        }
    } break;
    case Property::VT::MAP: {
        p = make_shared<Property>();
        p->map();
        auto each = ele.FirstChildElement("element");
        while (each) {
            shared_ptr<Property> t;
            toproperty(t, *each);
            t->name = each->Attribute("key");
            p->map()[t->name] = t;
            each = each->NextSiblingElement();
        }
    } break;
    }
}

shared_ptr<Property> toproperty(XmlObject const& xo)
{
    shared_ptr<Property> r;
    toproperty(r, *xo.RootElement());
    return r;
}

CROSS_END