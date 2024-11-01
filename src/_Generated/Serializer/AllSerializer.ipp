#pragma once
#include "_Generated\Serializer\ReflectTestClass.serializer.gen.h"
namespace Piccolo{
    template<>
    Json Serializer::write(const ReflectTest& instance){
        Json::object  ret_context;
        
        ret_context.insert_or_assign("test1", Serializer::write(instance.test1));
        ret_context.insert_or_assign("test2", Serializer::write(instance.test2));
        ret_context.insert_or_assign("test3", Serializer::write(instance.test3));
        return  Json(ret_context);
    }
    template<>
    ReflectTest& Serializer::read(const Json& json_context, ReflectTest& instance){
        assert(json_context.is_object());
        
        if(!json_context["test1"].is_null()){
            Serializer::read(json_context["test1"], instance.test1);
        }
        if(!json_context["test2"].is_null()){
            Serializer::read(json_context["test2"], instance.test2);
        }
        if(!json_context["test3"].is_null()){
            Serializer::read(json_context["test3"], instance.test3);
        }
        return instance;
    }

}

