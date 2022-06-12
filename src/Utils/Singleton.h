#pragma once
#ifndef _SINGLETON_
#define _SINGLETON_

template<typename T>
class Singleton
{
private:
	Singleton() = default;
	~Singleton() = default;
	Singleton(const Singleton&) = delete;
	Singleton(Singleton&&) = delete;
	Singleton& operator=(const Singleton&) = delete;
	Singleton& operator=(Singleton&&) = delete;

public:
	static T& get_instance()
	{
		static T _instance;
		return _instance;
	}
};
#endif //_SINGLETON_
