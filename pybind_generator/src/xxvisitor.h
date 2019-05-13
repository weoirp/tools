#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <clang-c\Index.h>

#include "xxdata.h"


template<typename Data, typename std::enable_if<std::is_base_of<XXData, Data>::value, std::nullptr_t>::type = nullptr>
struct ClientData
{
	CXTranslationUnit *_TU;
	Data *_data;
};

CXChildVisitResult TravelNamespace(CXCursor cursor, CXCursor parent, CXClientData client_data);
CXChildVisitResult TravelClass(CXCursor cursor, CXCursor parent, CXClientData client_data);
CXChildVisitResult TravelFunction(CXCursor cursor, CXCursor parent, CXClientData client_data);
CXChildVisitResult TravelEnum(CXCursor cursor, CXCursor parent, CXClientData client_data);
CXChildVisitResult TravelVariable(CXCursor cursor, CXCursor parent, CXClientData client_data);
CXChildVisitResult TravelParam(CXCursor cursor, CXCursor parent, CXClientData client_data);
std::unique_ptr<NameSpaceData> Visitor(int argc, char *argv[]);
