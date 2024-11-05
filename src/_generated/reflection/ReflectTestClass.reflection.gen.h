#pragma once
#include "rttr/registration.h"
#include "..\..\Sample\4-Reflect\ReflectTestClass.h"

namespace MXRender{
    class MXRender::ReflectTest;
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

        static void TypeWrapperRegister_ReflectTest(){
        rttr::registration::class_<MXRender::ReflectTest>("ReflectTest")
		 .constructor<>()
		 .property("test1", &MXRender::ReflectTest::test1)
		 .property("test2", &MXRender::ReflectTest::test2)
		 .property("test3", &MXRender::ReflectTest::test3)
		 .method("testfunc", &MXRender::ReflectTest::testfunc)
		 ;
    }        
    };
namespace Reflection{
namespace TypeWrappersRegister{
    void ReflectTestClassRegister()
    {
        TypeReflectTestOperator::TypeWrapperRegister_ReflectTest();
    }
}//namespace TypeWrappersRegister

}//namespace Reflection
}//namespace MXRender

