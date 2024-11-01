#pragma once
#include "Core/Reflection.h"
#include "Core/ReflectionRegister.h"
//#include "_generated/serializer/all_serializer.h"
#include "ReflectTestClass.Reflection.Gen.h"

namespace MXRender{
namespace Reflection{
    void TypeMetaRegister::metaRegister(){
        TypeWrappersRegister::ReflectTestClass();
    }
}
}

