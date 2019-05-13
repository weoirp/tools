#pragma once

#include "xxvisitor.h"
#include <sstream>

template<class Data>
class CppContents
{
public:
	CppContents(const Data *data)
		:data(data)
	{
	}

	std::string Print(int level)
	{
		return std::string{};
	}

	const Data *data{ nullptr };
};

template<class ClassData>
class ShowContents : public CppContents<ClassData>
{
public:
	ShowContents(const ClassData *data, const std::string &indent = "\t")
		:CppContents<ClassData>(data), indent(indent)
	{}

	std::string Print(int level);
	std::string indent{ "\t" };
};

template<>
std::string ShowContents<NameSpaceData>::Print(int level)
{
	std::stringstream out;
	std::stringstream indent_block;
	for (int i = 0; i < level; i++)
		indent_block << indent;

	out << indent_block.str() << "namespace " << data->name << " {\n";

	for (const auto &iter: data->classes)
	{
		auto c = ShowContents<ClassData>{ &iter, indent };
		out << c.Print(level + 1) << "\n";
	}
	for (const auto &iter: data->functions)
	{
		auto f = ShowContents<FunctionData>{ &iter, indent };
		out << f.Print(level + 1) << "\n";
	}
	for (const auto &iter: data->enumerates)
	{
		auto e = ShowContents<EnumData>{ &iter, indent };
		out << e.Print(level + 1) << "\n";
	}
	for (const auto &iter: data->variables)
	{
		auto v = ShowContents<VariableData>{ &iter, indent };
		out << v.Print(level + 1) << "\n";
	}
	for (const auto &iter: data->inner_namespaces)
	{
		auto n = ShowContents<NameSpaceData>{ iter.second.get(), indent };
		out << n.Print(level + 1) << "\n";
	}

	out << indent_block.str() << "}\n";

	return out.str();
}

template<>
std::string ShowContents<ClassData>::Print(int level)
{
	std::stringstream out;
	std::stringstream indent_block;
	for (int i = 0; i < level; i++)
		indent_block << indent;

	out << indent_block.str() << "class " << data->name << " {\n";

	const AccessData * vec_access[] = { &data->public_access, &data->protect_access, &data->private_access };
	auto vec_acc_name = std::vector<std::string>{ "public:\n", "protected:\n", "private:\n" };

	for (int i = 0; i < 3; i++)
	{
		auto acc = vec_access[i];
		if (acc->class_functions.size() || acc->class_variables.size())
		{
			out << indent_block.str() << vec_acc_name[i];
			for (const auto &iter : acc->class_functions)
			{
				auto func = ShowContents<FunctionData>{ &iter, indent };
				out << func.Print(level + 1) << "\n";
			}
			for (const auto &iter : acc->class_variables)
			{
				auto variable = ShowContents<VariableData>{ &iter, indent };
				out << variable.Print(level + 1) << "\n";
			}
		}
	}

	out << indent_block.str() << "};";

	return out.str();
}

template<>
std::string ShowContents<EnumData>::Print(int level)
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
std::string ShowContents<FunctionData>::Print(int level)
{
	std::ostringstream out;
	for (int i = 0; i < level; i++)
		out << indent;
	if (data->is_static)
		out << "static ";
	if (data->f_type == FunctionKind::CONSTRUCTOR)
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

	out << data->name << " (";
	for (const auto &iter: data->params)
	{
		auto param = ShowContents<ParamData>{ &iter, indent };
		out << param.Print(level) << ", ";
	}
	out << ");";

	return out.str();
}

template<>
std::string ShowContents<VariableData>::Print(int level)
{
	std::ostringstream out;
	for (int i = 0; i < level; i++)
		out << indent;
	out << data->name << ";";
	return out.str();
}

template<>
std::string ShowContents<ParamData>::Print(int level)
{
	std::ostringstream out;
	out << data->arg_type << " " << data->name;
	if (data->has_default)
	{
		out << " = " << data->arg_value;
	}

	return out.str();
}
