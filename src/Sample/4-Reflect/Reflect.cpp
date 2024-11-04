#include<iostream>
#include <fstream>
#include "Core/ReflectionRegister.h"
#include "rttr/type.h"
#include "ReflectTestClass.h"
using namespace std;
int main()
{
	MXRender::Reflection::TypeMetaRegister::metaRegister();
	rttr::type t = rttr::type::get_by_name("ReflectTest");
	rttr::variant v = t.create();
	for (auto& prop : t.get_properties())
	{
		cout << "properties name: " << prop.get_name() << endl;
	}
	t.get_property("test3").set_value(v, std::string("1111"));
	cout << t.get_property("test3").get_value(v).to_string() << endl;
	for (auto& meth : t.get_methods())
	{
		rttr::variant ret = meth.invoke(v, 222);
		cout << "call function result:" << ret.to_int() << endl;
		cout << "function name: " << meth.get_name() << endl;
	}
	system("pause");
	return 0;
}
