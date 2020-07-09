#import "cross.hpp"
#import "objc.hpp"
#import <sstream>

CROSS_BEGIN

NSString *toOc(string const& str)
{
    return [NSString stringWithUTF8String:str.c_str()];
}

string fromOc(NSString *str)
{
    return str.UTF8String;
}

NSDictionary *toOc(::std::map<string, shared_ptr<Property> > const& args)
{
    NSMutableDictionary *r = [NSMutableDictionary dictionaryWithCapacity:args.size()];
    for (auto &e:args) {
        auto k = [NSString stringWithUTF8String:e.first.c_str()];
        ::std::ostringstream oss;
        oss << e.second;
        auto vstr = oss.str();
        auto v = [NSString stringWithUTF8String:vstr.c_str()];
        [r setValue:v forKey:k];
    }
    return r;
}

void fromOc(NSDictionary* dict, ::std::map<string, shared_ptr<Property> > &args)
{
    for (NSString *k in dict) {
        NSString *v = [dict valueForKey:k];
        string ck = k.UTF8String;
        string cv = v.UTF8String;
        args[ck] = make_property(cv);
    }
}

CROSS_END
