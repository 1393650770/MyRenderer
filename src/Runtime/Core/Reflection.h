#pragma once
#ifndef __REFLECTION__
#define __REFLECTION__
namespace MXRender
{

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

	
}

#endif

