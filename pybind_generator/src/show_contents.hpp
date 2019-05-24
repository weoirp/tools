#pragma once

#include "xxvisitor.hpp"
#include <sstream>


template<class Data, typename std::enable_if<std::is_base_of<XXInfo<Data>, Data>::value, std::nullptr_t>::type = nullptr >
class ShowContents
{
public:
	ShowContents(const Data *data, const std::string &indent = "\t")
		:data(data), indent(indent)
	{}

	std::string Print(int level);
	std::string indent{ "\t" };
	const Data *data{ nullptr };
};

template<>
std::string ShowContents<NameSpaceInfo>::Print(int level)
{
	std::stringstream out;
	std::stringstream indent_block;
	for (int i = 0; i < level; i++)
		indent_block << indent;

	out << indent_block.str() << "namespace " << data->name << " {\n";

	for (const auto &iter: data->classes)
	{
		auto c = ShowContents<ClassInfo>{ &iter, indent };
		out << c.Print(level + 1) << "\n";
	}
	for (const auto &iter: data->functions)
	{
		auto f = ShowContents<FunctionInfo>{ &iter, indent };
		out << f.Print(level + 1) << "\n";
	}
	for (const auto &iter: data->enumerates)
	{
		auto e = ShowContents<EnumInfo>{ &iter, indent };
		out << e.Print(level + 1) << "\n";
	}
	for (const auto &iter: data->variables)
	{
		auto v = ShowContents<VariableInfo>{ &iter, indent };
		out << v.Print(level + 1) << "\n";
	}
	for (const auto &iter: data->inner_namespaces)
	{
		auto n = ShowContents<NameSpaceInfo>{ iter.second.get(), indent };
		out << n.Print(level + 1) << "\n";
	}

	out << indent_block.str() << "}\n";

	return out.str();
}

template<>
std::string ShowContents<ClassInfo>::Print(int level)
{
	std::stringstream out;
	std::stringstream indent_block;
	for (int i = 0; i < level; i++)
		indent_block << indent;

	out << indent_block.str() << "class " << data->name << " {\n";

	const AccessInfo * vec_access[] = { &data->public_access, &data->protect_access, &data->private_access };
	auto vec_acc_name = std::vector<std::string>{ "public:\n", "protected:\n", "private:\n" };

	for (int i = 0; i < 3; i++)
	{
		auto acc = vec_access[i];
		if (acc->class_functions.size() || acc->class_variables.size())
		{
			out << indent_block.str() << vec_acc_name[i];
			for (const auto &iter : acc->class_functions)
			{
				auto func = ShowContents<CXXMethodInfo>{ &iter, indent };
				out << func.Print(level + 1) << "\n";
			}
			for (const auto &iter : acc->class_variables)
			{
				auto variable = ShowContents<FieldInfo>{ &iter, indent };
				out << variable.Print(level + 1) << "\n";
			}
		}
	}

	out << indent_block.str() << "};";

	return out.str();
}

template<>
std::string ShowContents<EnumInfo>::Print(int level)
{
	std::stringstream out;
	std::stringstream indent_block;
	for (int i = 0; i < level; i++)
		indent_block << indent;

	out << indent_block.str() << "enum " << data->name << " {\n";
	for (const auto &iter: data->tags)
	{
		out << indent_block.str() << indent << iter.c_str() << ",\n";
	}
	out << indent_block.str() << "};";

	return out.str();
}

template<>
std::string ShowContents<FunctionInfo>::Print(int level)
{
	std::ostringstream out;
	for (int i = 0; i < level; i++)
		out << indent;
	if (data->is_static)
		out << "static ";

	out << data->name << " (";
	for (const auto &iter: data->params)
	{
		auto param = ShowContents<ParamInfo>{ &iter, indent };
		out << param.Print(level) << ", ";
	}
	out << ");";

	return out.str();
}

template<>
std::string ShowContents<CXXMethodInfo>::Print(int level)
{
	std::ostringstream out;
	for (int i = 0; i < level; i++)
		out << indent;
	if (data->is_static)
		out << "static ";
	if (data->f_type == FunctionKind::NORMAL_CONSTRUCTOR)
	{
		out << "(CONSTRUCTOR) ";
	}
	else if (data->f_type == FunctionKind::DESTRUCTOR)
	{
		out << "(DESTRUCTOR) ";
	}
	else if (data->f_type == FunctionKind::CXXMETHOD)
	{
		out << "(CXXMETHOD) ";
	}
	out << data->class_name << "::" << data->name << " (";
	for (const auto &iter : data->params)
	{
		auto param = ShowContents<ParamInfo>{ &iter, indent };
		out << param.Print(level) << ", ";
	}
	out << ")";
	if (data->is_const)
		out << " const";

	return out.str();
}


template<>
std::string ShowContents<VariableInfo>::Print(int level)
{
	std::ostringstream out;
	for (int i = 0; i < level; i++)
		out << indent;
	out << data->name << ";";
	return out.str();
}

template<>
std::string ShowContents<FieldInfo>::Print(int level)
{
	std::ostringstream out;
	for (int i = 0; i < level; i++)
		out << indent;
	if (data->is_static)
	{
		out << "(static) ";
	}
	if (data->is_const)
	{
		out << "(const) ";
	}
	out << data->class_name << "::" << data->name << ";";
	return out.str();
}

template<>
std::string ShowContents<ParamInfo>::Print(int level)
{
	std::ostringstream out;
	out << data->arg_type << " " << data->name;
	if (data->has_default)
	{
		out << " = " << data->arg_value;
	}

	return out.str();
}

std::string show_contents(const NameSpaceInfo *ns)
{
	ShowContents<NameSpaceInfo> d{ ns, "    " };
	return d.Print(0);
}