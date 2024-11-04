#pragma once
#include <string>
#include "Core/ConstDefine.h"
/*
* simple use
* metaparser.exe "" E:/GameEngine/MyRenderer/src/Reflect/parser_header.h E:/GameEngine/MyRenderer/src * MXRender 0
*/

namespace MXRender {
    MYRENDERER_REFLECTION_TYPE(ReflectTest)
    MYRENDERER_BEGIN_CLASS(ReflectTest,WhiteListFields, WhiteListMethods)
    MYRENDERER_REFLECTION_BODY(ReflectTest)


    public:
        META(Enable)
            float test1{ 3.f };
        META(Enable)
            float      test2{ 2.5f };
    private:
        META(Enable)
            std::string      test3 = "";
        META(Enable)
            int testfunc(int i) { return i; }

    };
}