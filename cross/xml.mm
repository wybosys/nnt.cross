#import "cross.hpp"
#import "xml.hpp"
#import "logger.hpp"
#import "stringbuilder.hpp"

CROSS_BEGIN

string xml_encode(XmlObject const &xo)
{
    auto d = [(NSXMLDocument*)xo.obj XMLDataWithOptions:NSXMLNodePrettyPrint];
    auto s = [[NSString alloc] initWithData:d encoding:NSUTF8StringEncoding];
    return s.UTF8String;
}

shared_ptr<XmlObject> xml_decode(string const &str)
{
    auto s = [NSString stringWithUTF8String:str.c_str()];
    NSError *err;
    auto doc = [[NSXMLDocument alloc] initWithXMLString:s
                                                options:NSXMLNodeOptionsNone
                                                  error:&err];
    if (err) {
        Logger::Warn(stringbuilder().space(" ") << "XML:" << err.code << err.debugDescription.UTF8String);
        return nullptr;
    }
    return make_shared<XmlObject>(doc);
}

void toxmlobj(Property const& po, NSXMLElement *cur)
{
    [cur addAttribute:[NSXMLNode attributeWithName:@"vt" stringValue:[NSNumber numberWithInteger:(NSInteger)po.vt].stringValue]];
    
    switch (po.vt)
    {
        default:
        case Property::VT::NIL: {
            // pass
        } break;
        case Property::VT::INTEGER: {
            auto v = [NSNumber numberWithInteger:(Property::integer)po].stringValue;
            [cur addAttribute:[NSXMLNode attributeWithName:@"value" stringValue:v]];
            [cur setStringValue:v];
        } break;
        case Property::VT::NUMBER: {
            auto v = [NSNumber numberWithDouble:(double)po].stringValue;
            [cur addAttribute:[NSXMLNode attributeWithName:@"value" stringValue:v]];
            [cur setStringValue:v];
        } break;
        case Property::VT::BOOLEAN: {
            auto v = [NSNumber numberWithBool:(bool)po].stringValue;
            [cur addAttribute:[NSXMLNode attributeWithName:@"value" stringValue:v]];
            [cur setStringValue:v];
        } break;
        case Property::VT::STRING: {
            auto str = po.toString();
            auto v = [NSString stringWithUTF8String:str.c_str()];
            [cur setStringValue:v];
        } break;
        case Property::VT::ARRAY: {
            for (auto &e: po.array()) {
                NSXMLElement* n = [NSXMLElement elementWithName:@"element"];
                toxmlobj(*e, n);
                [cur addChild:n];
            }
        } break;
        case Property::VT::MAP: {
            for (auto &e: po.map()) {
                auto nm = (string const&)e.first;
                NSXMLElement* n = [NSXMLElement elementWithName:@"element"];
                [n addAttribute:[NSXMLNode attributeWithName:@"key" stringValue:[NSString stringWithUTF8String:nm.c_str()]]];
                toxmlobj(*e.second, n);
                [cur addChild:n];
            }
        } break;
    }
}

shared_ptr<XmlObject> toxmlobj(Property const &po)
{
    auto doc = [NSXMLDocument new];
    auto r = make_shared<XmlObject>(doc);
    doc.rootElement = [NSXMLElement elementWithName:po.name.empty() ? @"root" : [NSString stringWithUTF8String:po.name.c_str()]];
    toxmlobj(po, doc.rootElement);
    return r;
}

void toproperty(shared_ptr<Property> &p, NSXMLElement *ele)
{
    auto vt = (Property::VT)[ele attributeForName:@"vt"].stringValue.intValue;
    switch (vt)
    {
        default:
        case Property::VT::NIL:
            p = make_shared<Property>();
            break;
        case Property::VT::INTEGER:
            p = make_shared<Property>((Property::integer)[ele attributeForName:@"value"].stringValue.integerValue);
            break;
        case Property::VT::NUMBER:
            p = make_shared<Property>([ele attributeForName:@"value"].stringValue.doubleValue);
            break;
        case Property::VT::BOOLEAN:
            p = make_shared<Property>((bool)[ele attributeForName:@"value"].stringValue.boolValue);
            break;
        case Property::VT::STRING: {
            auto v = ele.stringValue;
            p = make_shared<Property>(string(v.UTF8String));
        } break;
        case Property::VT::ARRAY: {
            p = make_shared<Property>();
            p->array();
            for (id child in ele.children) {
                shared_ptr<Property> t;
                toproperty(t, child);
                p->array().emplace_back(t);
            }
        } break;
        case Property::VT::MAP: {
            p = make_shared<Property>();
            p->map();
            for (id child in ele.children) {
                shared_ptr<Property> t;
                toproperty(t, child);
                t->name = [child attributeForName:@"key"].stringValue.UTF8String;
                p->map()[t->name] = t;
            }
        } break;
    }
}

shared_ptr<Property> toproperty(XmlObject const &xo)
{
    shared_ptr<Property> r;
    auto doc = (NSXMLDocument*)xo.obj;
    toproperty(r, doc.rootElement);
    return r;
}

CROSS_END
