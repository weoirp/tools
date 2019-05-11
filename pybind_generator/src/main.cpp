#include <iostream>
#include <string>
#include <map>
#include <memory>
#include <algorithm>

#include "cxparse.h"

bool printKindSpelling(CXCursor cursor)
{
	enum CXCursorKind curKind = clang_getCursorKind(cursor);
	const char *curkindSpelling = clang_getCString(
		clang_getCursorKindSpelling(curKind));
	printf("The AST node kind spelling is:%s\n", curkindSpelling);
	return true;
}

bool printSpelling(CXCursor cursor)
{
	const char *astSpelling = clang_getCString(clang_getCursorSpelling(cursor));
	printf("The AST node spelling is:%s\n", astSpelling);
	return true;
}

bool printLocation(CXCursor cursor)
{
	CXSourceRange range = clang_getCursorExtent(cursor);
	CXSourceLocation startLocation = clang_getRangeStart(range);
	CXSourceLocation endLocation = clang_getRangeEnd(range);

	CXFile file;
	unsigned int line, column, offset;
	clang_getInstantiationLocation(startLocation, &file, &line, &column, &offset);
	printf("Start: Line: %u Column: %u Offset: %u\n", line, column, offset);
	clang_getInstantiationLocation(endLocation, &file, &line, &column, &offset);
	printf("End: Line: %u Column: %u Offset: %u\n", line, column, offset);

	return true;
}

std::ostream &operator<<(std::ostream &stream, const CXString &str)
{
	stream << clang_getCString(str);
	clang_disposeString(str);
	return stream;
}

static std::string cx2string(const CXString &cx_str)
{
	std::string str(clang_getCString(cx_str));
	clang_disposeString(cx_str);
	return str;
}

CXTranslationUnit unit = nullptr;

CXChildVisitResult function_visitor(CXCursor cursor, CXCursor parent, CXClientData client_data)
{
	CXSourceLocation location = clang_getCursorLocation(cursor);

	if (clang_Location_isInSystemHeader(location))
		return CXChildVisit_Continue;

	//if (clang_Location_isFromMainFile(location) == 0)
	//	return CXChildVisit_Continue;

	CXCursorKind cursorKind = clang_getCursorKind(cursor);
	CXString cursorSpelling = clang_getCursorSpelling(cursor);
	auto name = cx2string(cursorSpelling);

	NameSpaceData *data = reinterpret_cast<NameSpaceData*>(client_data);

	unsigned int curLevel = *(reinterpret_cast<unsigned int*>(client_data));
	unsigned int nextLevel = curLevel + 1;

	std::cout << curLevel << " " << std::string(curLevel * 4, ' ');

	switch (cursorKind)
	{
	case CXCursor_ParmDecl:
	{
		auto cxrange = clang_getCursorExtent(cursor);

		CXToken *tokens = nullptr;
		unsigned int num_tokens = 0;
		clang_tokenize(unit, cxrange, &tokens, &num_tokens);
		auto equal_sign = std::find_if(tokens, tokens + num_tokens, [](const CXToken &iter)
		{
			if (cx2string(clang_getTokenSpelling(unit, iter)) == std::string("="))
			{
				return true;
			}
			return false;
		});
		if (equal_sign != tokens + num_tokens)
		{
			std::cout << "default_value >>>: " << cx2string(clang_getTokenSpelling(unit, tokens[num_tokens - 1])) << " ";
		}
		clang_disposeTokens(unit, tokens, num_tokens);
		break;
	}
	default:
		break;
	}

	return CXChildVisit_Continue;
}


CXChildVisitResult visitor(CXCursor cursor, CXCursor parent, CXClientData client_data)
{
	CXSourceLocation location = clang_getCursorLocation(cursor);

	if (clang_Location_isInSystemHeader(location))
		return CXChildVisit_Continue;

	//if (clang_Location_isFromMainFile(location) == 0)
	//	return CXChildVisit_Continue;

	CXCursorKind cursorKind = clang_getCursorKind(cursor);
	CXString cursorSpelling = clang_getCursorSpelling(cursor);
	auto name = cx2string(cursorSpelling);

	NameSpaceData *data = reinterpret_cast<NameSpaceData*>(client_data);

	unsigned int curLevel = *(reinterpret_cast<unsigned int*>(client_data));
	unsigned int nextLevel = curLevel + 1;

	std::cout << curLevel << " " << std::string(curLevel * 4, ' ');

	switch (cursorKind)
	{
	case CXCursor_Namespace:
		break;
	case CXCursor_StructDecl:
	case CXCursor_ClassDecl:
		break;
	case CXCursor_EnumDecl:
	case CXCursor_EnumConstantDecl:
		break;
	case CXCursor_Constructor:
	case CXCursor_Destructor:
	case CXCursor_CXXMethod:
	case CXCursor_FieldDecl:
	{
		//auto access = clang_getCXXAccessSpecifier(cursor);
		//std::string access_str = (access == CX_CXXPublic) ? "public:" :
		//	(access == CX_CXXProtected) ? "protected:" :
		//	(access == CX_CXXPrivate) ? "private:" : "Invalid";
		//std::cout << access_str << " ";
		//
		//auto storage_sc = clang_Cursor_getStorageClass(cursor);
		//if (storage_sc == CX_SC_Extern)
		//{
		//	std::cout << "extern ";
		//}
		//else if (storage_sc == CX_SC_Static)
		//{
		//	std::cout << "static ";
		//}
		//int arg_nums = clang_Cursor_getNumArguments(cursor);
		//for (int i = 0; i < arg_nums; i++)
		//{
		//	auto arg_cur = clang_Cursor_getArgument(cursor, i);
		//	std::cout << clang_getTypeSpelling(clang_getCursorType(arg_cur)) << " # ";
		//}
		break;


	}
	case CXCursor_VarDecl:
	{
		auto type = clang_getCursorType(cursor);
		auto spelling = clang_getTypeSpelling(type);
		auto type_str = cx2string(spelling);
		std::cout << type_str << " ";
		break;
	}
	case CXCursor_FunctionDecl:
	{
		//auto storage_sc = clang_Cursor_getStorageClass(cursor);
		//if (storage_sc == CX_SC_Extern)
		//{
		//	std::cout << "extern ";
		//}
		//else if (storage_sc == CX_SC_Static)
		//{
		//	std::cout << "static ";
		//}
		//int arg_nums = clang_Cursor_getNumArguments(cursor);
		//for (int i = 0; i < arg_nums; i++)
		//{
		//	auto arg_cur = clang_Cursor_getArgument(cursor, i);
		//	std::cout << clang_getTypeSpelling(clang_getCursorType(arg_cur)) << " $ ";
		//}		
		//break;

		clang_visitChildren(cursor, function_visitor, client_data);
		break;
	}
	case CXCursor_ParmDecl:
	{
		//auto type = clang_getCursorType(cursor);
		//auto spelling = clang_getTypeSpelling(type);
		//auto type_str = cx2string(spelling);
		//std::cout << "$$$ "<< type_str << " ";

		//auto cxrange = clang_getCursorExtent(cursor);

		//CXToken *tokens = nullptr;
		//unsigned int num_tokens = 0;
		//clang_tokenize(unit, cxrange, &tokens, &num_tokens);
		//auto equal_sign = std::find_if(tokens, tokens + num_tokens, [](const CXToken &iter)
		//{
		//	if (cx2string(clang_getTokenSpelling(unit, iter)) == std::string("="))
		//	{
		//		return true;
		//	}
		//	return false;
		//});
		//if (equal_sign != tokens + num_tokens)
		//{
		//	std::cout << "default_value: " << cx2string(clang_getTokenSpelling(unit, tokens[num_tokens - 1])) << " ";
		//}
		//clang_disposeTokens(unit, tokens, num_tokens);
		break;

	}
	default:
		break;
	}

	std::cout << name << " (" << clang_getCursorKindSpelling(cursorKind) << ")";

	std::cout << std::endl;

	clang_visitChildren(cursor,
		visitor,
		&nextLevel);

	return CXChildVisit_Continue;
}

int main(int argc, char *argv[])
{
	CXIndex index = clang_createIndex(0, 0);

	//CXTranslationUnit_DetailedPreprocessingRecord  //ºêÏà¹ØµÄ

	auto flag = CXTranslationUnit_SkipFunctionBodies |
		CXTranslationUnit_DetailedPreprocessingRecord |
		CXTranslationUnit_IncludeBriefCommentsInCodeCompletion |
		CXTranslationUnit_VisitImplicitAttributes |
		//CXTranslationUnit_IncludeAttributedTypes |
		CXTranslationUnit_KeepGoing;

	unit = clang_parseTranslationUnit(index, nullptr, argv, argc, nullptr, 0, flag);

	if (unit == nullptr)
	{
		printf("Unable to parse translation unit!\n");
		std::exit(-1);
	}

	printf("success!\n");
	CXCursor root_cursor = clang_getTranslationUnitCursor(unit);

	//CXParseData client_data;

	unsigned int client_data = 0;

	clang_visitChildren(
		root_cursor,
		visitor,
		&client_data);

	clang_disposeTranslationUnit(unit);
	clang_disposeIndex(index);


	return 0;
}