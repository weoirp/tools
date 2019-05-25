#include <iostream>
#include <string>
#include <map>
#include <memory>
#include <algorithm>

#include "xxvisitor.hpp"
#include "show_contents.hpp"
#include "pybind_generator.hpp"

int func11(int i)
{
	return 11;
}

int func11()
{
	return 12;
}

//template<class T, class R>
//auto resolve(R(T::*m)(void)) -> decltype(m)
//{
//	return m;
//}


template<typename ...Args>
struct custom_cast_impl
{
	template<typename Return>
	auto operator()(Return (*p)(Args...)) -> decltype(p)
	{
		return p;
	}

};

template<typename ...Args>
static custom_cast_impl<Args...> custom_cast = {};


int main(int argc, char *argv[])
{
	auto nsdata = Visitor(argc, argv);

	//auto code = show_contents(nsdata.get());
	//std::cout << code << std::endl;

	auto code = printExtensionCode(nsdata.get(), "pymod");
	std::cout << code << std::endl;

}
