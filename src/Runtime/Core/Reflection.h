#pragma once
#ifndef __REFLECTION__
#define __REFLECTION__
namespace MXRender
{

#define MYRENDERER_REFLECTION_BODY(class_name) \
    friend class Type##class_name##Operator; \
    friend class Serializer;
    
#define MYRENDERER_REFLECTION_TYPE(class_name) \
    namespace MXRender \
    { \
            class Type##class_name##Operator; \
    };

	
}

#endif

