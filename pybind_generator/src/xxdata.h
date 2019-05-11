#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <memory>

struct XXData
{
	XXData(const std::string &name)
		:name(name)
	{}

	XXData(const std::string &&name)
		:name(std::move(name))
	{}

	std::string name{ };
};

struct VariableData: XXData
{
	using XXData::XXData;
};

struct ParamData: XXData
{
	using XXData::XXData;

	bool has_default{ false };

	std::string arg_type{ };
	std::string arg_value{ };
};

enum class FunctionKind
{
	NORMAL = 0,
	CXXMETHOD,
	CONSTRUCTOR,
	DESTRUCTOR
};

struct FunctionData: XXData
{
	FunctionData(const std::string &name, FunctionKind type = FunctionKind::NORMAL)
		:XXData(name), f_type(type)
	{}

	FunctionData(const std::string &&name, FunctionKind type = FunctionKind::NORMAL)
		:XXData(name), f_type(type)
	{}

	std::vector<ParamData> params{ };
	bool is_static{ false };
	FunctionKind f_type{ FunctionKind::NORMAL };
};


struct EnumData: XXData
{
	using XXData::XXData;

	std::vector<std::string> tags{ };
};

struct AccessData
{
	std::vector<VariableData> class_variables{ };
	std::vector<FunctionData> class_functions{ };
};

struct ClassData: XXData
{
	using XXData::XXData;

	AccessData private_access{ };
	AccessData protect_access{ };
	AccessData public_access{ };
};

struct NameSpaceData: XXData
{
	using XXData::XXData;
	using NameSpaceDataPtr = std::unique_ptr<NameSpaceData>;

	std::vector<VariableData> variables{ };
	std::vector<FunctionData> functions{ };
	std::vector<EnumData> enumerates{ };
	std::vector<ClassData> classes{ };
	std::map<std::string, NameSpaceDataPtr> inner_namespaces{ };
};