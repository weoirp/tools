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

std::string MethodCode(const std::string &cls_defined,
	const std::string &py_method,
	const std::string &params_str,
	const std::string &cls_method)
{
	const char *code = R"(%s.def("%s", overload_cast2<%s>::instance(&%s));)";
	
	return string_format(code, cls_defined.c_str(), py_method.c_str(), params_str.c_str(), cls_method.c_str());
}

std::string ConstructCode(const std::string &cls_defined, const std::string &params_str)
{
	const char *code = R"(%s.def(py::init<%s>());)";

	return string_format(code, cls_defined.c_str(), params_str.c_str());
}

std::string StaticMethodCode(const std::string &cls_defined,
	const std::string &py_method,
	const std::string &params_str,
	const std::string &cls_method)
{
	const char *code = R"(%s.def_static("%s", overload_cast2<%s>::instance(&%s));)";

	return string_format(code, cls_defined.c_str(), py_method.c_str(), params_str.c_str(), cls_method.c_str());
}

std::string FieldCode(const std::string &cls_defined,
	const std::string &py_var,
	const std::string &cls_var)
{
	const char *code = R"(def_field(%s, &%s, "%s");)";
	return string_format(code, cls_defined.c_str(), cls_var.c_str(), py_var.c_str());
}

std::string StaticFieldCode(const std::string &cls_defined,
	const std::string &py_var,
	const std::string &cls_var)
{
	//bug: ODR
	const char *code = R"(def_static_field(%s, &%s, "%s");)";
	return string_format(code, cls_defined.c_str(), cls_var.c_str(), py_var.c_str());
}



/**
 * write to travel NameSpaceData
 */

template<typename Data>
struct AssistData
{
	static_assert(std::is_base_of<XXInfo<Data>, Data>::value, "Data must inherit from XXInfo");
	XXInfo<Data> *_data;
	std::string decl_var;
	int level;
};

class GenerateCode
{
public:
	GenerateCode(const std::string &indent = "\t")
		:indent(indent)
	{}
	friend std::string Travel(NameSpaceInfo *ns);
private:
	std::string Travel(AssistData<VariableInfo> &current, const AssistData<NameSpaceInfo> &parent);
	std::string Travel(AssistData<FunctionInfo> &current, const AssistData<NameSpaceInfo> &parent);
	std::string Travel(AssistData<ClassInfo> &current, const AssistData<NameSpaceInfo> &parent);
	std::string Travel(AssistData<NameSpaceInfo> &current, const AssistData<NameSpaceInfo> &parent);
	std::string Travel(AssistData<FieldInfo> &current, const AssistData<ClassInfo> &parent);
	std::string Travel(AssistData<CXXMethodInfo> &current, const AssistData<ClassInfo> &parent);

	std::string indent = "\t";
};


std::string GenerateCode::Travel(AssistData<VariableInfo> &current, const AssistData<NameSpaceInfo> &parent)
{
	return {};
}


std::string GenerateCode::Travel(AssistData<FunctionInfo> &current, const AssistData<NameSpaceInfo> &parent)
{
	std::ostringstream out;
	for (int i = 0; i < current.level; i++)
		out << indent;

	auto mod = parent.decl_var;
	auto &params = current._data->derived()->params;
	std::ostringstream params_str;
	if (params.size())
	{
		auto iter = params.begin();
		for (; iter != params.end() - 1; iter++)
		{
			params_str << iter->arg_type << ", ";
		}
		params_str << iter->arg_type;
	}
	if (current._data->derived()->is_static)
	{
		return {};
	}
	out << MethodCode(mod, current._data->name, params_str.str(), current._data->name);
	return out.str();
}


std::string GenerateCode::Travel(AssistData<ClassInfo> &current, const AssistData<NameSpaceInfo> &parent)
{
	std::ostringstream out;
	std::ostringstream indent_block;
	for (int i = 0; i < current.level; i++)
		indent_block << indent;
	
	auto &access = current._data->derived()->public_access;
	for (auto &iter: access.class_functions)
	{
		if (!(iter.f_type == FunctionKind::CXXMETHOD || iter.f_type == FunctionKind::NORMAL_CONSTRUCTOR))
			continue;
		AssistData<CXXMethodInfo> method{ &iter, "", current.level };
		out << Travel(method, current) << "\n";
	}

	for (auto &iter: access.class_variables)
	{
		AssistData<FieldInfo> field{ &iter, "", current.level };
		out << Travel(field, current) << "\n";
	}

	
	return out.str();
}


std::string GenerateCode::Travel(AssistData<NameSpaceInfo> &current, const AssistData<NameSpaceInfo> &parent)
{
	auto mod = current.decl_var;

	std::ostringstream out;
	std::ostringstream indent_block;
	for (int i = 0; i < current.level; i++)
		indent_block << indent;

	{
		auto class_defined = R"(auto c = py::class_<%s>(%s, "%s");)";
		for (auto &iter: current._data->derived()->classes)
		{
			out << indent_block.str() << "{\n";
			out << indent_block.str() << indent << string_format(class_defined, iter.second->name.c_str(), mod.c_str(), iter.second->name.c_str()) << "\n";
			AssistData<ClassInfo> cls{ iter.second.get(), "c", current.level + 1 };
			out << Travel(cls, current);
			out << indent_block.str() << "}\n";
		}
	}

	{
		for (auto &iter: current._data->derived()->functions)
		{
			AssistData<FunctionInfo> func{ &iter, "", current.level };
			out << Travel(func, current) << "\n";
		}
	}
	
	{
		auto sub_mod_defined = R"(auto %s = %s.def_submodule("%s");)";
		for (auto &iter: current._data->derived()->inner_namespaces)
		{
			out << indent_block.str() << "{\n";
			auto sub_module = mod + "_sub";
			out << indent_block.str() << indent << string_format("using namespace %s;\n", iter.second->name.c_str());
			out << indent_block.str() << indent << string_format(sub_mod_defined, sub_module.c_str(), mod.c_str(), iter.second->name.c_str()) << "\n";
			AssistData<NameSpaceInfo> ns{ iter.second.get(), sub_module, current.level + 1 };
			out << Travel(ns, current);
			out << indent_block.str() << "}\n";
		}
	}

	return out.str();
}


std::string GenerateCode::Travel(AssistData<FieldInfo> &current, const AssistData<ClassInfo> &parent)
{
	std::ostringstream out;
	for (int i = 0; i < current.level; i++)
		out << indent;

	auto cls_defined = parent.decl_var;
	auto cls_var = current._data->derived()->class_name + "::" + current._data->name;
	auto py_var = current._data->name;
	if (current._data->derived()->is_static)
		out << StaticFieldCode(cls_defined, py_var, cls_var);
	else
		out << FieldCode(cls_defined, py_var, cls_var);
	return out.str();
}


std::string GenerateCode::Travel(AssistData<CXXMethodInfo> &current, const AssistData<ClassInfo> &parent)
{
	std::ostringstream out;
	for (int i = 0; i < current.level; i++)
		out << indent;

	auto cls_defined = parent.decl_var;
	auto &params = current._data->derived()->params;
	std::ostringstream params_str;
	if (params.size())
	{
		auto iter = params.begin();
		for (; iter != params.end() - 1; iter++)
		{
			params_str << iter->arg_type << ", ";
		}
		params_str << iter->arg_type;
	}
	auto f_type = current._data->derived()->f_type;
	if (f_type == FunctionKind::NORMAL_CONSTRUCTOR)
	{
		out << ConstructCode(cls_defined, params_str.str());
	}
	else if (f_type == FunctionKind::CXXMETHOD)
	{
		auto py_method = current._data->name;
		auto cls_method = current._data->derived()->class_name + "::" + current._data->name;
		if (current._data->derived()->is_static)
		{
			out << StaticMethodCode(cls_defined, py_method, params_str.str(), cls_method);
		}
		else
		{
			if (current._data->derived()->is_const)
			{
				cls_method.append(", py::const_");
			}
			out << MethodCode(cls_defined, py_method, params_str.str(), cls_method);
		}
	}
	else
	{
		return {};
	}
	return out.str();
}


static std::string Travel(NameSpaceInfo *ns)
{	
	std::string indent = "\t";
	GenerateCode generate_code(indent);

	std::ostringstream out;
	out << "{\n";
	out << indent << "namespace py = pybind11;\n";
	out << indent << "using py::overload_cast2;\n";
	out << indent << "using py::external::def_field;\n";
	out << indent << "using py::external::def_static_field;\n";
	AssistData<NameSpaceInfo> root{ ns, "m", 1 };
	AssistData<NameSpaceInfo> parent{ nullptr, "", 0 };
	out << generate_code.Travel(root, parent);
	out << "}\n";

	return out.str();
}

std::string printExtensionCode(NameSpaceInfo *ns, const std::string &py_mod)
{
	auto mod_defined = R"(PYBIND11_MODULE(%s, %s))";
	std::ostringstream out;
	out << string_format(mod_defined, py_mod.c_str(), "m").c_str() << "\n";
	out << Travel(ns);
	return out.str();
}

std::string printEmbeddedCode(NameSpaceInfo *ns, const std::string &py_mod)
{
	auto mod_defined = R"(PYBIND11_EMBEDDED_MODULE(%s, %s))";
	std::ostringstream out;
	out << string_format(mod_defined, py_mod.c_str(), "m") << "\n";
	out << Travel(ns);
	return out.str();
}