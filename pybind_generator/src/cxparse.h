#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <clang-c\Index.h>
#include "xxdata.h"

enum class DataType
{
	Data_Class = 0,
	Data_Struct,
	Data_Function,
	Data_Enum,
	Data_Variable
};


//struct VariableInfo;
//struct FunctionInfo;
//struct EnumInfo;
//struct ClassInfo;
//struct NameSpaceInfo;



class CXParseData
{
public:
	CXParseData(const std::string &global_name) 
	{
		global_namespace = new NameSpaceInfo(global_name);
	}
	~CXParseData() 
	{
		delete global_namespace;
	}

	
private:
	NameSpaceInfo *global_namespace;
};

