#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <clang-c\Index.h>

#include "xxdata.h"

class NameSpaceVisitor;
class ClassVisitor;
class FunctionVisitor;
class EnumVisitor;
class VariableVisitor;


template<typename Data, typename std::enable_if<std::is_base_of<XXData, Data>::value, std::nullptr_t>::type = nullptr>
struct ClientData
{
	CXTranslationUnit *_TU;
	Data *_data;
};

class NameSpaceVisitor
{
public:
	NameSpaceVisitor() {}
	~NameSpaceVisitor() {}
	static CXChildVisitResult travelAST(CXCursor cursor, CXCursor parent, CXClientData client_data);

};

class ClassVisitor
{
public:
	ClassVisitor() {}
	~ClassVisitor() {}
	static CXChildVisitResult travelAST(CXCursor cursor, CXCursor parent, CXClientData client_data);
};

class FunctionVisitor
{
public:
	FunctionVisitor() {}
	~FunctionVisitor() {}
	static CXChildVisitResult travelAST(CXCursor cursor, CXCursor parent, CXClientData client_data);
};

class EnumVisitor
{
public:
	EnumVisitor() {}
	~EnumVisitor() {}
	static CXChildVisitResult travelAST(CXCursor cursor, CXCursor parent, CXClientData client_data);

};

class VariableVisitor
{
public:
	VariableVisitor() {}
	~VariableVisitor() {}
	static CXChildVisitResult travelAST(CXCursor cursor, CXCursor parent, CXClientData client_data);

};

class ParamVisitor
{
public:
	ParamVisitor() {}
	~ParamVisitor() {}
	static CXChildVisitResult travelAST(CXCursor cursor, CXCursor parent, CXClientData client_data);
};

void toString(VariableData *data);
void toString(ParamData *data);
void toString(EnumData *data);
void toString(FunctionData *data);
void toString(ClassData *data);
void toString(NameSpaceData *data);
