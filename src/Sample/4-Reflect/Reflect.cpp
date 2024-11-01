#include<iostream>
#include <fstream>
#include "Core/ReflectionRegister.h"

int main()
{
	MXRender::Reflection::TypeMetaRegister::metaRegister();
	system("pause");
	return 0;
}
