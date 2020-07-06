#include <jni.h>
#define __CROSS_PRIVATE__
#include "android-prv.hpp"

USE_AJNI;

#define CROSS_FUNC(cls, name) AJNI_FUNCNAME(com_nnt_cross, cls, name)
