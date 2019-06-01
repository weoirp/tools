#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

template<typename Derived>
struct XXInfo
{
	XXInfo(const std::string &name)
		:name(name)
	{}

	XXInfo(const std::string &&name)
		:name(std::move(name))
	{}

	Derived *derived()
	{
		return static_cast<Derived*>(this);
	}

	std::string name{ };
};

struct VariableInfo: XXInfo<VariableInfo>
{
	using XXInfo::XXInfo;
};

struct FieldInfo: XXInfo<FieldInfo>
{
	using XXInfo::XXInfo;

	std::string class_name{ };
	bool is_static{ false };
	bool is_const{ false };
};

struct ParamInfo: XXInfo<ParamInfo>
{
	using XXInfo::XXInfo;

	bool has_default{ false };

	std::string arg_type{ };
	std::string arg_value{ };
};

enum class FunctionKind
{
	CXXMETHOD = 0,
	NORMAL_CONSTRUCTOR,
	COPY_CONSTRUCTOR,
	MOVE_CONSTRUCTOR,
	DESTRUCTOR
};

struct FunctionInfo: XXInfo<FunctionInfo>
{
	FunctionInfo(const std::string &name)
		:XXInfo(name)
	{}

	FunctionInfo(const std::string &&name)
		:XXInfo(name)
	{}

	std::vector<ParamInfo> params{ };
	bool is_static{ false };
};

struct CXXMethodInfo : XXInfo<CXXMethodInfo>
{
	CXXMethodInfo(const std::string &name, FunctionKind type = FunctionKind::CXXMETHOD)
		:XXInfo(name), f_type(type)
	{}
	
	CXXMethodInfo(const std::string &&name, FunctionKind type = FunctionKind::CXXMETHOD)
		:XXInfo(name), f_type(type)
	{}

	std::vector<ParamInfo> params{ };
	std::string class_name{};
	FunctionKind f_type{ FunctionKind::CXXMETHOD };
	bool is_static{ false };
	bool is_const{ false };
};


struct EnumInfo: XXInfo<EnumInfo>
{
	using XXInfo::XXInfo;

	std::vector<std::string> tags{ };
};

struct AccessInfo
{
	std::vector<FieldInfo> class_variables{ };
	std::vector<CXXMethodInfo> class_functions{ };
};

struct ClassInfo: XXInfo<ClassInfo>
{
	using XXInfo::XXInfo;

	AccessInfo private_access{ };
	AccessInfo protect_access{ };
	AccessInfo public_access{ };
};

struct NameSpaceInfo: XXInfo<NameSpaceInfo>
{
	using XXInfo::XXInfo;
	using VariablePtr = std::unique_ptr<VariableInfo>;
	using EnumInfoPtr = std::unique_ptr<EnumInfo>;
	using ClassInfoPtr = std::unique_ptr<ClassInfo>;
	using NameSpaceInfoPtr = std::unique_ptr<NameSpaceInfo>;

	std::unordered_map<std::string, VariablePtr> variables{ };
	std::vector<FunctionInfo> functions{ };
	std::unordered_map<std::string, EnumInfoPtr> enumerates{ };
	std::unordered_map<std::string, ClassInfoPtr> classes{ };
	std::unordered_map<std::string, NameSpaceInfoPtr> inner_namespaces{ };
};