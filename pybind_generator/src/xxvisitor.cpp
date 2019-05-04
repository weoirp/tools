#include "xxvisitor.h"

CXChildVisitResult NameSpaceVisitor::travelAST(CXCursor cursor, CXCursor parent, CXClientData client_data)
{
	CXSourceLocation location = clang_getCursorLocation(cursor);

	if (clang_Location_isInSystemHeader(location))
		return CXChildVisit_Continue;

	CXCursorKind cursorKind = clang_getCursorKind(cursor);
	CXString cursorSpelling = clang_getCursorSpelling(cursor);
	auto name = cx2string(cursorSpelling);

	NameSpaceData *data = reinterpret_cast<NameSpaceData*>(client_data);

	switch (cursorKind)
	{
	case CXCursor_Namespace:
		if (data->inner_namespaces.find(name) == data->inner_namespaces.end())
		{
			data->inner_namespaces.insert(
				std::make_pair(
					name, 
					std::unique_ptr<NameSpaceData>(new NameSpaceData(name))
				)
			);
		}
		clang_visitChildren(cursor, NameSpaceVisitor::travelAST, data->inner_namespaces[name].get());
		break;
	case CXCursor_StructDecl:
	case CXCursor_ClassDecl:
	{
		data->classes.emplace_back(ClassData(name));
		auto size = data->classes.size();
		clang_visitChildren(cursor, ClassVisitor::travelAST, &(data->classes[size - 1]));
		break;
	}
	case CXCursor_EnumDecl:
	case CXCursor_EnumConstantDecl:
	{
		data->enumerates.emplace_back(EnumData(name));
		auto size = data->enumerates.size();
		clang_visitChildren(cursor, EnumVisitor::travelAST, &(data->enumerates[size - 1]));
		break;
	}
	case CXCursor_FieldDecl:
		break;
	case CXCursor_VarDecl:
	{
		data->variables.emplace_back(VariableData(name));
		auto size = data->variables.size();
		clang_visitChildren(cursor, VariableVisitor::travelAST, &(data->variables[size - 1]));
		break;
	}
	case CXCursor_FunctionDecl:
	{
		data->functions.emplace_back(FunctionData(name));
		auto size = data->functions.size();
		clang_visitChildren(cursor, FunctionVisitor::travelAST, &(data->functions[size - 1]));
		break;
	}
	default:
		break;
	}
	return CXChildVisit_Continue;
}

CXChildVisitResult ClassVisitor::travelAST(CXCursor cursor, CXCursor parent, CXClientData client_data)
{
	return CXChildVisit_Continue;
}

struct EnumStmtRight
{
	bool has_value;
	long long value;
};

CXChildVisitResult EnumVisitor::travelAST(CXCursor cursor, CXCursor parent, CXClientData client_data)
{
	CXSourceLocation location = clang_getCursorLocation(cursor);

	if (clang_Location_isInSystemHeader(location))
		return CXChildVisit_Continue;

	CXCursorKind cursorKind = clang_getCursorKind(cursor);
	CXString cursorSpelling = clang_getCursorSpelling(cursor);
	auto name = cx2string(cursorSpelling);

	EnumData *data = reinterpret_cast<EnumData *>(client_data);

	long long enum_value = 0;
	if (auto size = data->tags.size())
	{
		enum_value = data->tags[size - 1].second;
	}

	switch (cursorKind)
	{
	case CXCursor_EnumConstantDecl:
	{
		EnumStmtRight r = { false, 0 };
		clang_visitChildren(cursor, travelEnumValue, &r);
		if (r.has_value)
		{
			enum_value = r.value;
		}
		else // BUG: enum value is char ?
		{
			++enum_value;
		}
		data->tags.emplace_back(std::make_pair(name, enum_value));
		break;
	}
	default:
		break;
	}

	return CXChildVisit_Continue;
}

CXChildVisitResult EnumVisitor::travelEnumValue(CXCursor cursor, CXCursor parent, CXClientData client_data)
{
	CXCursorKind cursorKind = clang_getCursorKind(cursor);
	
	auto data = static_cast<EnumStmtRight *>(client_data);
	if (cursorKind == CXCursor_IntegerLiteral)
	{
		auto enum_value = clang_getEnumConstantDeclValue(cursor);
		data->has_value = true;
		data->value = enum_value;
	}

	return CXChildVisit_Break;
}

CXChildVisitResult FunctionVisitor::travelAST(CXCursor cursor, CXCursor parent, CXClientData client_data)
{
	CXSourceLocation location = clang_getCursorLocation(cursor);

	if (clang_Location_isInSystemHeader(location))
		return CXChildVisit_Continue;

	CXCursorKind cursorKind = clang_getCursorKind(cursor);
	CXString cursorSpelling = clang_getCursorSpelling(cursor);
	auto name = cx2string(cursorSpelling);

	FunctionData *data = reinterpret_cast<FunctionData *>(client_data);

	switch (cursorKind)
	{
	case CXCursor_ReturnStmt:
		break;
	case CXCursor_ParmDecl:
	{
		data->params.emplace_back(std::move(name));
		break;
	}
	default:
		break;
	}

	return CXChildVisit_Continue;
}

CXChildVisitResult VariableVisitor::travelAST(CXCursor cursor, CXCursor parent, CXClientData client_data)
{
	CXSourceLocation location = clang_getCursorLocation(cursor);

	if (clang_Location_isInSystemHeader(location))
		return CXChildVisit_Continue;

	CXCursorKind cursorKind = clang_getCursorKind(cursor);
	CXString cursorSpelling = clang_getCursorSpelling(cursor);
	auto name = cx2string(cursorSpelling);

	VariableData *data = reinterpret_cast<VariableData *>(client_data);

	switch (cursorKind)
	{
	default:
		break;
	}

	return CXChildVisit_Continue;
}