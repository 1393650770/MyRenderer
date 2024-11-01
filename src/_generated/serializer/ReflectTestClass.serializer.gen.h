#pragma once
#include "..\..\Sample\4-Reflect\ReflectTestClass.h"

namespace Piccolo{
    template<>
    Json Serializer::write(const ReflectTest& instance);
    template<>
    ReflectTest& Serializer::read(const Json& json_context, ReflectTest& instance);
}//namespace

