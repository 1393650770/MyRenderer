#include "Property.h"
#include "ClassInfo.h"





MXRender::Property::Property(ClassInfo& owner, const std::string& name, unsigned int element_offset, unsigned int flag):
owner(&owner),name(name),element_offset(element_offset),flag(flag)
{

}

MXRender::Property::~Property()
{

}

bool MXRender::Property::clone(Property* p)
{
	if ((get_owner() && p->get_owner() == get_owner()))
	{
		name= p->name;
		flag=p->flag;
		element_offset=p->element_offset;
		return true;
	}
	return false;
}

void MXRender::Property::set_owner(ClassInfo& new_owner)
{
	owner=&new_owner;
}


MXRender::ClassInfo* MXRender::Property::get_owner() const
{
	return owner;
}

const std::string& MXRender::Property::get_name() const
{
	return name;
}

void* MXRender::Property::get_address(void* obj) const 
{
	return (void*)(((unsigned char*)obj) + element_offset);
}

unsigned int MXRender::Property::get_flag() const
{
	return flag;
}

void MXRender::Property::set_flag(unsigned int new_flag)
{
	flag=new_flag;
}

