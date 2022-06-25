#pragma once
#ifndef _DEFERRENDER_
#define _DEFERRENDER_
#include "MyRender.h"

namespace MXRender
{
    class DeferRender :
        public MyRender
    {
    public:
        DeferRender();
        virtual ~DeferRender();
        void run() override;
    private:

    };
}

#endif // !_DEFERRENDER_


