#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <memory>

struct VariableData
{
	std::string name;

	VariableData(const std::string &name)
		:name(std::move(name))
	{}
};

struct FunctionData
{
	std::string name;
	std::vector<std::string> params;

	FunctionData(const std::string &name)
		:name(std::move(name))
	{}
};

struct EnumData
{
	std::string name;
	std::vector<std::pair<std::string, long long>> tags;

	EnumData(const std::string &name)
		:name(std::move(name))
	{}
};

struct AccessData
{
	std::vector<VariableData> class_variables;
	std::vector<FunctionData> class_functions;
};

struct ClassData
{
	std::string name;
	AccessData private_access;
	AccessData protect_access;
	AccessData public_access;

	ClassData(const std::string &name)
		:name(std::move(name))
	{}
};

struct NameSpaceData
{
	using NameSpaceDataPtr = std::unique_ptr<NameSpaceData>;

	std::string name;
	std::vector<VariableData> variables;
	std::vector<FunctionData> functions;
	std::vector<EnumData> enumerates;
	std::vector<ClassData> classes;
	std::map<std::string, NameSpaceDataPtr> inner_namespaces;

	NameSpaceData(const std::string &name)
		:name(std::move(name))
	{}
};