#include<iostream>
#include "Runtime/Mesh/Mesh.h"
#include "Runtime/Mesh/Model.h"
#include "Runtime/Render/Window.h"
#include "Runtime/Render/DeferRender.h"


class A
{
public:
	A();
	~A();
    virtual void test1() {};
private:

};

A::A()
{
}

A::~A()
{
}
class B : public A
{
public:
	B ();
	~B ();
    void test1() override {};
private:

};

B::B()
{
}

B::~B()
{
}

int main()
{
    A* a = new A();
    B* b = new B();
    B* b1 = new B();
    std::shared_ptr<MXRender::DeferRender> defer_render = std::make_shared<MXRender::DeferRender>();

    MXRender::Window my_window;

    my_window.run(defer_render);

    delete a;
    delete b;
    delete b1;
    return 0;
}