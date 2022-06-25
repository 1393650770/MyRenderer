#include<iostream>
#include "Runtime/Mesh/Mesh.h"
#include "Runtime/Mesh/Model.h"
#include "Runtime/Render/Window.h"
#include "Runtime/Render/DeferRender.h"

int main()
{
    std::shared_ptr<MXRender::DeferRender> defer_render = std::make_shared<MXRender::DeferRender>();

    MXRender::Window my_window;

    my_window.run(defer_render);

    return 0;
}