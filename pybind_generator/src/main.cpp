#include <iostream>
#include <string>
#include <map>
#include <memory>

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
	case CXCursor_FieldDecl:
		break;
	case CXCursor_VarDecl:
		break;
	case CXCursor_FunctionDecl:
		break;
	case CXCursor_ParmDecl:
	{
		auto ctype = clang_getCursorType(cursor);
		auto cxstr = clang_getTypeSpelling(ctype);
		std::cout << cx2string(cxstr) << " ";
		break;
	}
	default:
		break;
	}
	

	std::cout << clang_getCursorSpelling(cursor) 
		<< " (" << clang_getCursorKindSpelling(cursorKind) << ") ";

	//std::cout << clang_getTypeSpelling(clang_getCursorType(cursor));

	std::cout << std::endl;

	clang_visitChildren(cursor,
		visitor,
		&nextLevel);

	return CXChildVisit_Continue;
}

int main(int argc, char *argv[])
{
	CXIndex index = clang_createIndex(0, 0);

	//CXTranslationUnit_DetailedPreprocessingRecord  //����ص�

	auto flag = CXTranslationUnit_SkipFunctionBodies | 
		//CXTranslationUnit_DetailedPreprocessingRecord |
		CXTranslationUnit_IncludeBriefCommentsInCodeCompletion |
		//CXTranslationUnit_SingleFileParse |
		CXTranslationUnit_VisitImplicitAttributes |
		CXTranslationUnit_KeepGoing;

	CXTranslationUnit unit = clang_parseTranslationUnit(index, nullptr, argv, argc, nullptr, 0, flag);

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