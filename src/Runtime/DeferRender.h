#pragma once
#ifndef _DEFERRENDER_
#define _DEFERRENDER_
#include "Render.h"

namespace MXRender
{
    class DeferRender :
        public Render
    {
    public:
        DeferRender();
        virtual ~DeferRender();
        void run() override;
    private:

    };
}

#endif // !_DEFERRENDER_


