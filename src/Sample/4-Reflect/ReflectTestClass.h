#pragma once
#include <string>

/*
* simple use
* metaparser.exe "" E:/GameEngine/MyRenderer/src/Reflect/parser_header.h E:/GameEngine/MyRenderer/src * MXRender 0
*/

#define MYRENDERER_REFLECTION_BODY(class_name) \
    friend class Reflection::TypeFieldReflectionOparator::Type##class_name##Operator; \
    friend class Serializer;
#define MYRENDERER_REFLECTION_TYPE(class_name) \
    namespace Reflection \
    { \
        namespace TypeFieldReflectionOparator \
        { \
            class Type##class_name##Operator; \
        } \
    };


#if defined(__REFLECTION_PARSER__)
#define META(...) __attribute__((annotate(#__VA_ARGS__)))
#define MYRENDERER_BEGIN_STRUCT(Name, ...) struct __attribute__((annotate(#__VA_ARGS__))) Name  \
                                    {
#include <string>
#define MYRENDERER_BEGIN_CLASS(Name, ...) class __attribute__((annotate(#__VA_ARGS__))) Name  \
{
#define MYRENDERER_BEGIN_CLASS_WITH_DERIVE(Name, ...) class __attribute__((annotate(#__VA_ARGS__))) Name : __VA_ARGS__ \
                                        {
#else
#define META(...)
#define MYRENDERER_BEGIN_STRUCT(Name, ...) struct Name  \
                                    {
#define MYRENDERER_BEGIN_CLASS(Name, ...) class Name  \
                                    { 
#endif

namespace MXRender {
    namespace Reflection {
        namespace TypeFieldReflectionOparator {
            class TypeReflectTestOperator;
        }
    }; 
    MYRENDERER_BEGIN_CLASS(ReflectTest,WhiteListFields) 
        friend class Reflection::TypeFieldReflectionOparator::TypeReflectTestOperator; friend class Serializer;


    public:
        META(Enable)
            float test1{ 3.f };
        META(Enable)
            float      test2{ 2.5f };
    private:
        META(Enable)
            std::string      test3 = "";

    };
}