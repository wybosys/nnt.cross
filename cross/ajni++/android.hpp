#ifndef __AJNI_ANDROID_H
#define __AJNI_ANDROID_H

AJNI_BEGIN_NS(android)

namespace TypeSignature
{
    extern const JTypeSignature CONTEXT;
    extern const JTypeSignature ATTRIBUTESET;
    extern const JTypeSignature VIEW;
    extern const JTypeSignature VIEWGROUP;
    extern const JTypeSignature VIEWGROUP_LAYOUTPARAMS;
} // namespace TypeSignature

class Context : public JClass
{
public:
    static JClassPath const CLASSPATH;

    Context(JClassPath const& = CLASSPATH);

    JMemberMethod getClassLoader;
};

class Activity : public JClass
{
public:
    Activity(const JClassPath & = "androidx/appcompat/app/AppCompatActivity");

    JMethod findViewById;
};

class View : public JClass
{
public:
    View(const JClassPath & = "android/view/View");

    JMethod setBackgroundColor;
};

class ViewGroup : public View
{
public:
    class LayoutParams : public JClass
    {
    public:
        LayoutParams(const JClassPath & = "android/view/ViewGroup$LayoutParams");

        enum
        {
            FILL_PARENT = -1,
            MATCH_PARENT = -1,
            WRAP_CONTENT = -2
        };
    };

    ViewGroup(const JClassPath & = "android/view/ViewGroup");

    JMethod addView;
};

class ConstraintLayout : public ViewGroup
{
public:
    ConstraintLayout(const JClassPath & = "androidx/constraintlayout/widget/ConstraintLayout");
};

AJNI_END_NS

#endif