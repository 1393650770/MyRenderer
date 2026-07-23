#pragma once
#include "Core/Reflection.h"
#include "Core/ReflectionRegister.h"
#include "RmlUIDemo.Reflection.Gen.h"

namespace MXRender{
namespace Reflection{
    void TypeMetaRegister::metaRegister(){
        TypeWrappersRegister::RmlUIDemoRegister();
    }
}
}

