#pragma once
#include "Core/Reflection.h"
#include "Core/ReflectionRegister.h"
#include "ReflectTestClass.Reflection.Gen.h"

namespace MXRender{
namespace Reflection{
    void TypeMetaRegister::metaRegister(){
        TypeWrappersRegister::ReflectTestClassRegister();
    }
}
}

