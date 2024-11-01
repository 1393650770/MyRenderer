#pragma once
#include "..\..\Sample\4-Reflect\ReflectTestClass.h"

namespace MXRender{
    class ReflectTest;
namespace Reflection{
namespace TypeFieldReflectionOparator{
    class TypeReflectTestOperator{
    public:
        static const char* getClassName(){ return "ReflectTest";}
        //static void* constructorWithJson(const Json& json_context){
        //    ReflectTest* ret_instance= new ReflectTest;
        //    Serializer::read(json_context, *ret_instance);
        //    return ret_instance;
        //}
        //static Json writeByName(void* instance){
        //    return Serializer::write(*(ReflectTest*)instance);
        //}
    };
}//namespace TypeFieldReflectionOparator


    void TypeWrapperRegister_ReflectTest(){
        registration::class_<ReflectTest>("ReflectTest")
		 .constructor<>()
		 .property("test1", &ReflectTest::test1)
		 .property("test2", &ReflectTest::test2)
		 .property("test3", &ReflectTest::test3)
		 
		 ;
    }
namespace TypeWrappersRegister{
    void ReflectTestClass()
    {
        TypeWrapperRegister_ReflectTest();
    }
}//namespace TypeWrappersRegister

}//namespace Reflection
}//namespace MXRender

