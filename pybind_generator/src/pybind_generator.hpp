#pragma once

#include "xxdata.h"
#include <sstream>

template<typename... Args>
std::string string_format(const char *fmt, const Args&... args)
{
	int len = std::snprintf(nullptr, 0, fmt, args...) + 1;

	std::string result;
	result.resize(len);
	std::snprintf(const_cast<char *>(result.data()), len, fmt, args...);
	return result;
}

template<typename Class, typename C, typename D>
void def_field(Class &c, D C::*pm, const std::string &py_var)
{
	c.def_readwrite(py_var.c_str(), pm);
}

template<typename Class, typename C, typename D>
void def_field(Class &c, const D C::*pm, const std::string &py_var)
{
	c.def_readonly(py_var.c_str(), pm);
}

template<typename Class, typename C, typename D>
void def_static_field(Class &c, D C::*pm, const std::string &py_var)
{
	c.def_readwrite_static(py_var.c_str(), pm);
}

template<typename Class, typename C, typename D>
void def_static_field(Class &c, const D C::*pm, const std::string &py_var)
{
	c.def_readonly_static(py_var.c_str(), pm);
}


std::string MethodCode(const std::string &py_cls,
	const std::string &py_method,
	const std::string &params_str,
	const std::string &cls_method)
{
	const char *code = R"(%s.def("%s", overload_cast<%s>(&%s));)";
	
	return string_format(code, py_cls.c_str(), py_method.c_str(), params_str.c_str(), cls_method.c_str());
}

std::string ConstructCode(const std::string &py_cls, const std::string &params_str)
{
	const char *code = R"(%s.def(py::init<%s>());)";

	return string_format(code, py_cls.c_str(), params_str.c_str());
}

std::string StaticMethodCode(const std::string &py_cls,
	const std::string &py_method,
	const std::string &params_str,
	const std::string &cls_method)
{
	const char *code = R"(%s.def_static("%s", overload_cast<%s>(&%s));)";

	return string_format(code, py_cls.c_str(), py_method.c_str(), params_str.c_str(), cls_method.c_str());
}

std::string FieldCode(const std::string &py_cls,
	const std::string &py_var,
	const std::string &cls_var)
{
	const char *code = R"(def_field(%s, &%s, "%s");)";
	return string_format(code, py_cls.c_str(), cls_var.c_str(), py_var.c_str());
}

std::string StaticFieldCode(const std::string &py_cls,
	const std::string &py_var,
	const std::string &cls_var)
{
	const char *code = R"(def_static_field(%s, &%s, "%s");)";
	return string_format(code, py_cls.c_str(), cls_var.c_str(), py_var.c_str());
}


//struct GenerateConstruct {};
//struct GenerateMethod {};
//struct GenerateStaticMethod {};
//struct GenerateVariable {};

/**
 * write to travel NameSpaceData
 */

template<typename Data>
struct AssistData
{
	static_cast(std::is_base_of<XXInfo, Data>::value, "Data must inherit from XXInfo");
	Data *_data;
};

class GenerateCode
{
public:
	template<typename Data>
	std::string Travel(const AssistData<Data> &current, const AssistData<Data> &parent)
	{
		return std::string{};
	}
};

template<>
std::string GenerateCode::Travel(const AssistData<ClassInfo> &current, const AssistData<ClassInfo> &parent)
{
	return "";
}



