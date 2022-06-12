#include<iostream>
#include"Render/Mesh.h"
#include"Render/Model.h"
#include"Runtime/Window.h"
#include"Runtime/DeferRender.h"

int main()
{
    std::shared_ptr<MXRender::DeferRender> defer_render = std::make_shared<MXRender::DeferRender>();
    MXRender::Window my_window;

    my_window.run(defer_render);

    return 0;
}