#pragma once
#ifndef _PROPERTY_
#define _PROPERTY_
#include <vector>
#include <string>

#include <glm/glm.hpp>
#include "ClassInfo.h"
#include <utility>
#include <map>

namespace MXRender { class ClassInfo; }

namespace MXRender
{

	class Property
	{
	private:
	protected:
		ClassInfo* owner;
		std::string name;
		unsigned int element_offset;
		unsigned int flag;
	public:
		enum ENUM_PROPERTY_FLAG
		{
			ENUM_SAVE_LOAD=0x01,
			ENUM_CLONE=0x02,
			ENUM_COPY =0x04,
			ENUM_SAVE_LOAD_CLONE = 0x03,
			ENUM_SAVE_LOAD_COPY=0x05,
			ENUM_REFLECT_NAME=0x08,
			ENUM_NO_USE_GC=0x10,
			ENUM_COUNT
		};
		enum ENUM_PROPERTY_TYPE
		{
			ENUM_PT_VALUE,
			ENUM_PT_ENUM,
			ENUM_PT_DATA,
			ENUM_PT_ARRAY,
			ENUM_PT_MAP,
			ENUM_PT_MAX
		};

		Property(ClassInfo& owner,const std::string& name,unsigned int element_offset,unsigned int flag);
		virtual ~Property();

		//virtual bool save()=0;
		virtual unsigned int get_property_type() const=0;
		virtual Property* get_instance()=0;
		virtual void clone_data(void* src,void* des)=0;
		virtual void copy_data(void* src,void* des)=0;

		virtual bool clone(Property* p);
		void set_owner(ClassInfo& new_owner);
		ClassInfo* get_owner() const;
		const std::string& get_name() const ;
		virtual void* get_address(void* obj) const;

		unsigned int get_flag() const;
		void set_flag(unsigned int new_flag);
	};

	template<typename T>
	class EnumProperty :public Property
	{
	private:
	protected:
		std::string enum_name;
	public:
		EnumProperty()
		{
			assert(sizeof(T)==4);
		}
		EnumProperty(ClassInfo& owner, const std::string& name, const std::string& enum_name, unsigned int element_offset, unsigned int flag) :Property(owner, name, element_offset, flag)
		{
			assert(sizeof(T) == 4);
			this->enum_name= enum_name;
		};
		virtual ~EnumProperty()
		{

		};

		virtual bool clone(Property* p)
		{
			EnumProperty * temp = dynamic_cast<EnumProperty*>(p);
			if(!Property::clone(temp))
				return false;
			enum_name=temp->enum_name;
			return true;
		}

		virtual bool set_value(void* obj, unsigned int data_src) const
		{
			*(unsigned int*)(((unsigned char *)obj)+ element_offset) = data_src;
			return true;
		}

		virtual bool get_value(void* obj, unsigned int& data_src) const
		{
			 data_src = *(unsigned int*)(((unsigned char*)obj) + element_offset) ;
			 return true;
		}

		virtual bool get_value(const void* obj, unsigned int& data_src) const
		{
			data_src = *(const unsigned int*)(((const char*)obj) + element_offset);
			return true;
		}

		virtual unsigned int& value(void* obj) const
		{
			return *(unsigned int*)(((const char*)obj) + element_offset);
		}


		virtual void clone_data(void* src, void* des)
		{
			value(des)=value(src);
		}

		virtual void copy_data(void* src, void* des)
		{
			value(des)=value(src);
		}

		virtual Property* get_instance()
		{
			return new EnumProperty();
		}
		std::string& get_enum_name()
		{
			return enum_name;
		}

		virtual unsigned int get_property_type() const
		{
			return ENUM_PT_ENUM;
		}
	};

	/// <summary>
	/// 定长数组或不定长数组
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <typeparam name="NumType"></typeparam>
	template<typename T,typename NumType>
	class DataProperty :public Property
	{
	private:
	protected:

		bool is_dynamic_create;
		unsigned int data_num;
		unsigned int num_element_offset;
	public:
		DataProperty()
		{

		}
		DataProperty(ClassInfo& owner, const std::string& name, unsigned int element_offset, unsigned int data_num, bool is_dynamic_create)
			:Property(owner, name, element_offset, ENUM_NO_USE_GC | ENUM_SAVE_LOAD_CLONE)
		{
			this->data_num = data_num;
			this->is_dynamic_create = is_dynamic_create;
		};
		DataProperty(ClassInfo& owner, const std::string& name, unsigned int element_offset, unsigned int num_element_offset)
			:Property(owner, name, element_offset, ENUM_NO_USE_GC | ENUM_SAVE_LOAD_CLONE)
		{
			this->data_num = 0;
			this->is_dynamic_create=true;
			this->num_element_offset = num_element_offset;
		};
		virtual ~DataProperty()
		{

		};

		virtual bool clone(Property* p)
		{
			DataProperty<T,NumType>* temp = dynamic_cast<DataProperty<T, NumType>*>(p);
			if (!Property::clone(temp))
				return false;
			is_dynamic_create = temp->is_dynamic_create;
			data_num = temp->data_num;
			num_element_offset = temp->num_element_offset;
			return true;
		}



		virtual void clone_data(void* src, void* des)
		{
			T* src_value_address = *(T**)get_address(src);

			if (data_num > 0)
			{
				T** temp = (T**)get_address(des);
				if (is_dynamic_create)
				{
					*temp = new T[data_num];
					memcpy_s((void*)(*temp), data_num * sizeof(T), (void*)src_value_address, data_num * sizeof(T));
				}
				else
				{
					memcpy_s((void*)(*temp), data_num * sizeof(T), (void*)src_value_address, data_num * sizeof(T));
				}
			}
			else
			{
				T** temp = (T**)get_address(des);


				void* src_num_offSet = (void*)(((unsigned char*)src) + num_element_offset);
				void* dest_num_offSet = (void*)(((unsigned char*)des) + num_element_offset);
				*(NumType*)dest_num_offSet = *(NumType*)src_num_offSet;
				NumType num = *(NumType*)src_num_offSet;

				*temp = new T[num];
				memcpy_s((void*)(*temp), num * sizeof(T), (void*)src_value_address, num * sizeof(T));

			}
		}

		virtual void copy_data(void* src, void* des)
		{
			assert(0);
		}

		virtual Property* get_instance()
		{
			return new DataProperty<T,NumType>();
		}

		virtual unsigned int get_property_type() const
		{
			return ENUM_PT_DATA;
		}
	};

	//template<typename T>
	//class ValueBaseProperty : public Property
	//{
	//protected:
	//	T low_value;
	//	T hight_value;
	//	T step;
	//	bool is_range;
	//public:
	//	ValueBaseProperty()
	//	{

	//	}
	//	ValueBaseProperty(ClassInfo& owner, const std::string& name, unsigned int element_offset, unsigned int flag	, bool is_range = false, T high_value = T(), T low_value = T(), T step = T())
	//		:Property(owner, name, element_offset, flag)
	//	{
	//		this->low_value = low_value;
	//		this->hight_value = hight_value;
	//		this->step = step;
	//		this->is_range = is_range;
	//	}

	//	virtual ~ValueBaseProperty()
	//	{

	//	}
	//	virtual bool clone(Property* p)
	//	{
	//		ValueBaseProperty<T>* temp = dynamic_cast<ValueBaseProperty<T> *>(p);
	//		if (!Property::clone(temp))
	//			return false;
	//		low_value = temp->low_value;
	//		hight_value = temp->hight_value;
	//		return true;
	//	}
	//};



	/// <summary>
		/// 容器类内的数据
		/// </summary>
		/// <typeparam name="T"></typeparam>
	//template<typename T>
	//class ValueProperty : public ValueBaseProperty<T>
	//{
	//public:
	//	ValueProperty()
	//	{

	//	}
	//	ValueProperty(ClassInfo& owner, const std::string& name, unsigned int element_offset, unsigned int flag, bool is_range = false, T high_value = T(), T low_value = T(), T step = T())
	//		:ValueBaseProperty(owner, name, element_offset, flag, is_range, high_value, low_value, step)
	//	{

	//	}

	//	virtual ~ValueProperty()
	//	{

	//	}
	//	virtual unsigned int get_property_type()const
	//	{
	//		return ENUM_PT_VALUE;
	//	}
	//	virtual bool set_value(void* obj, T& data_src) const
	//	{

	//		if (data_src > hight_value || data_src < low_value)
	//		{
	//			return false;
	//		}

	//		*(T*)(((unsigned char*)obj) + element_offset) = data_src;
	//		return true;

	//	}
	//	virtual bool get_value(void* obj, T& data_des) const
	//	{
	//		data_des = *(T*)(((unsigned char*)obj) + element_offset);
	//		return true;
	//	}
	//	virtual bool get_value(const void* obj, T& data_des) const
	//	{
	//		data_des = *(const T*)(((const char*)obj) + element_offset);
	//		return true;
	//	}
	//	virtual T& value(void* obj)const
	//	{
	//		return *(T*)(((const char*)obj) + element_offset);
	//	}

	//	virtual void clone_data(void* src, void* des)
	//	{
	//		value(des) = value(src);
	//	}
	//	virtual void copy_data(void* src, void* des)
	//	{
	//		value(des) = value(src);
	//	}

	//	virtual Property* get_instance()
	//	{
	//		return new ValueProperty<T>();
	//	}
	//};

	//template<typename ArrayType, typename T>
	//class ArrayProperty : public ValueBaseProperty<T>
	//{
	//public:
	//	ArrayProperty()
	//	{

	//	}
	//	ArrayProperty(ClassInfo& owner, const std::string& name, unsigned int element_offset, unsigned int flag, bool is_range = false, T high_value = T(), T low_value = T(), T step = T())
	//		:ValueBaseProperty(owner, name, element_offset, flag, is_range, high_value, low_value, step)
	//	{
	//	}

	//
	//	virtual ~ArrayProperty()
	//	{

	//	}
	//	virtual unsigned int get_property_type()const
	//	{
	//		return ENUM_PT_ARRAY;
	//	}
	//	ArrayType& get_containe(void* obj) const
	//	{
	//		return (*(ArrayType*)(((unsigned char*)obj) + element_offset));
	//	}
	//	bool add_element(void* obj, unsigned int index, T& data_src)
	//	{
	//		get_containe(obj).add_element(data_src);
	//		return true;
	//	}
	//	bool erase(void* obj, unsigned int i)
	//	{
	//		get_containe(obj).erase(i);
	//	}
	//	virtual bool set_value(void* obj, unsigned int index, T& data_src) const
	//	{

	//		if (data_src > hight_value || data_src < low_value)
	//		{
	//			return false;
	//		}
	//		(get_containe(obj)[index]) = data_src;
	//		return true;

	//	}
	//	virtual bool get_value(void* obj, unsigned int index, T& data_des) const
	//	{
	//		data_des = (get_containe(obj)[index]);
	//		return true;
	//	}

	//	virtual void clone_data(void* src, void* des)
	//	{
	//		get_containe(des) = get_containe(src);
	//	}
	//	virtual void copy_data(void* src, void* des)
	//	{
	//		get_containe(des) = get_containe(src);
	//	}

	//	virtual Property* get_instance()
	//	{
	//		return new ArrayProperty<ArrayType, T>();
	//	}
	//};

	//template<typename MapType, typename KEY, typename VALUE>
	//class MapProperty : public ValueBaseProperty<VALUE>
	//{
	//public:
	//	MapProperty(ClassInfo& owner, const std::string& name, unsigned int element_offset, unsigned int flag, bool is_range = false, VALUE high_value = VALUE(), VALUE low_value = VALUE(), VALUE step = VALUE())
	//		:ValueBaseProperty(owner, name, element_offset, flag, is_range, high_value, low_value, step)
	//	{

	//	}

	//	MapProperty()
	//	{

	//	}
	//	virtual ~MapProperty()
	//	{

	//	}
	//	virtual unsigned int get_property_type()const
	//	{
	//		return ENUM_PT_MAP;
	//	}
	//	MapType& get_containe(void* obj)const
	//	{
	//		return (*(MapType*)(((unsigned char*)obj) + element_offset));
	//	}
	//	bool add_element(void* obj, unsigned int index, std::pair<KEY, VALUE>& data_src)
	//	{
	//		get_containe(obj).insert(data_src);
	//		return true;
	//	}
	//	bool erase(void* obj, unsigned int i)
	//	{
	//		get_containe(obj).erase(i);
	//	}
	//	virtual bool set_value(void* obj, unsigned int index, KEY& new_key, VALUE& new_value) const
	//	{

	//		if (new_value > hight_value || new_value < low_value)
	//		{
	//			return false;
	//		}

	//		std::pair<KEY, VALUE> temp(new_key, new_value);
	//		(get_containe(obj)[index]) = temp;
	//		return true;

	//	}
	//	virtual bool get_value(void* obj, unsigned int index, std::pair<KEY, VALUE>& data_src) const
	//	{
	//		des = (get_containe(obj)[index]);
	//		return true;
	//	}

	//	virtual void clone_data(void* src, void* des)
	//	{
	//		get_containe(des) = get_containe(src);
	//	}
	//	virtual void copy_data(void* src, void* des)
	//	{
	//		get_containe(des) = get_containe(src);
	//	}

	//	virtual Property* get_instance()
	//	{
	//		return new MapProperty<MapType, KEY, VALUE>();
	//	}
	//};

	//template<class T, class NumType>
	//struct DataPropertyCreator
	//{
	//	Property* CreateProperty(const std::string& Name, ClassInfo& Owner, unsigned int Offset, unsigned int NumOffset)
	//	{

	//		return new DataProperty<T, NumType>(Owner, Name, Offset, NumOffset);
	//	}

	//	Property* CreateProperty(const std::string& Name, ClassInfo& Owner, unsigned int Offset, unsigned int uiDataNum, bool bDynamicCreate)
	//	{

	//		return new DataProperty<T, NumType>(Owner, Name, Offset, uiDataNum, bDynamicCreate);
	//	}
	//};
	//template<class T>
	//struct AutoPropertyCreator
	//{
	//	Property* CreateProperty(const std::string& Name, ClassInfo& Owner, unsigned int Offset, unsigned int uiFlag)
	//	{

	//		return new ValueProperty<T>(Owner, Name, Offset, uiFlag);


	//	}

	//	Property* CreateProperty(const std::string& Name, ClassInfo& Owner, unsigned int Offset, T HighValue, T LowValue, float fStep, unsigned int uiFlag)
	//	{
	//		return new ValueProperty<T>(Owner, Name, Offset, uiFlag, true, HighValue, LowValue, fStep);
	//	}
	//};



}
#endif 
