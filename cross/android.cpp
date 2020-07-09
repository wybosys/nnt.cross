#include "cross.hpp"
#include "android.hpp"
#include "sys.hpp"
#include "str.hpp"
#define __CROSS_PRIVATE__
#include "android-prv.hpp"

#include <ajni++/ajni++.hpp>
#include <ajni++/jre.hpp>
#include <ajni++/kotlin.hpp>

USE_AJNI;

CROSS_BEGIN

class UUID
    : public jre::Object
{
public:
    static JClassPath const CLASSPATH;

    UUID(JClassPath const& = CLASSPATH);

    JStaticMethod randomUUID;
};

JClassPath const UUID::CLASSPATH = "java/util/UUID";

UUID::UUID(JClassPath const& cp)
    : jre::Object(cp),
      randomUUID(*this)
{
    randomUUID.name = "randomUUID";
    randomUUID.sreturn = cp;
}

string uuid()
{
    JClassEntry<UUID> e;
    JEntry<UUID> u(e->randomUUID());
    auto str = u->toString(u)->toString();
    // 格式化为标准形式
    return replace(str, "-", "");
}

CROSS_END
