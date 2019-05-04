#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <clang-c\Index.h>

#include "xxdata.h"

class NameSpaceVisitor;
class ClassVisitor;
class FunctionVisitor;
class EnumVisitor;
class VariableVisitor;

static std::string cx2string(const CXString &cx_str)
{
	std::string str(clang_getCString(cx_str));
	clang_disposeString(cx_str);
	return str;
}

class NameSpaceVisitor
{
public:
	NameSpaceVisitor() {}
	~NameSpaceVisitor() {}
	static CXChildVisitResult travelAST(CXCursor cursor, CXCursor parent, CXClientData client_data);

private:
	NameSpaceData *data;
};

class ClassVisitor
{
public:
	ClassVisitor() {}
	~ClassVisitor() {}
	static CXChildVisitResult travelAST(CXCursor cursor, CXCursor parent, CXClientData client_data);
private:
	ClassData *data;
};

class FunctionVisitor
{
public:
	FunctionVisitor() {}
	~FunctionVisitor() {}
	static CXChildVisitResult travelAST(CXCursor cursor, CXCursor parent, CXClientData client_data);
private:
	FunctionData *data;
};

class EnumVisitor
{
public:
	EnumVisitor() {}
	~EnumVisitor() {}
	static CXChildVisitResult travelAST(CXCursor cursor, CXCursor parent, CXClientData client_data);
	static CXChildVisitResult travelEnumValue(CXCursor cursor, CXCursor parent, CXClientData client_data);
private:
	EnumData *data;
};

class VariableVisitor
{
public:
	VariableVisitor() {}
	~VariableVisitor() {}
	static CXChildVisitResult travelAST(CXCursor cursor, CXCursor parent, CXClientData client_data);
private:
	VariableData *data;
};