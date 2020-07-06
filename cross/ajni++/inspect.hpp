#ifndef _AJNI_INSPECT_H
#define _AJNI_INSPECT_H

AJNI_BEGIN

NNT_CLASS_PREPARE(JInspect);

class JInspect
{
    NNT_CLASS_DECL(JInspect);

public:

    JInspect(const JClass&);
    ~JInspect();

private:
    JClass const& _clz;
};

AJNI_END

#endif

