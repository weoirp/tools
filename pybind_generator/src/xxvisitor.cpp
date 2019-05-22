#include "xxvisitor.h"

static std::string cx2string(const CXString &cx_str)
{
	std::string str(clang_getCString(cx_str));
	clang_disposeString(cx_str);
	return str;
}

CXChildVisitResult TravelNamespace(CXCursor cursor, CXCursor parent, CXClientData client_data);
CXChildVisitResult TravelClass(CXCursor cursor, CXCursor parent, CXClientData client_data);
CXChildVisitResult TravelFunction(CXCursor cursor, CXCursor parent, CXClientData client_data);
CXChildVisitResult TravelCXXMethod(CXCursor cursor, CXCursor parent, CXClientData client_data);
CXChildVisitResult TravelEnum(CXCursor cursor, CXCursor parent, CXClientData client_data);
CXChildVisitResult TravelVariable(CXCursor cursor, CXCursor parent, CXClientData client_data);
CXChildVisitResult TravelField(CXCursor cursor, CXCursor parent, CXClientData client_data);
CXChildVisitResult TravelParam(CXCursor cursor, CXCursor parent, CXClientData client_data);

CXChildVisitResult TravelNamespace(CXCursor cursor, CXCursor parent, CXClientData client_data)
{
	CXSourceLocation location = clang_getCursorLocation(cursor);

	if (clang_Location_isInSystemHeader(location))
		return CXChildVisit_Continue;

	CXCursorKind cursorKind = clang_getCursorKind(cursor);
	CXString cursorSpelling = clang_getCursorSpelling(cursor);
	auto name = cx2string(cursorSpelling);

	auto c = reinterpret_cast<ClientData<NameSpaceInfo> *>(client_data);
	auto data = c->_data;

	switch (cursorKind)
	{
	case CXCursor_Namespace:
	{
		if (data->inner_namespaces.find(name) == data->inner_namespaces.end())
		{
			data->inner_namespaces.emplace(
				name, std::unique_ptr<NameSpaceInfo>(new NameSpaceInfo(name))
			);
		}
		auto child = ClientData<NameSpaceInfo>{ c->_TU, data->inner_namespaces[name].get() };
		clang_visitChildren(cursor, TravelNamespace, &child);
		break;
	}
	case CXCursor_StructDecl:
	case CXCursor_ClassDecl:
	{
		data->classes.emplace_back(ClassInfo(name));
		auto size = data->classes.size();
		auto child = ClientData<ClassInfo>{ c->_TU, &(data->classes[size - 1]) };
		clang_visitChildren(cursor, TravelClass, &child);
		break;
	}
	case CXCursor_EnumDecl:
	case CXCursor_EnumConstantDecl:
	{
		data->enumerates.emplace_back(EnumInfo(name));
		auto size = data->enumerates.size();
		auto child = ClientData<EnumInfo>{ c->_TU, &(data->enumerates[size - 1]) };
		clang_visitChildren(cursor, TravelEnum, &child);
		break;
	}
	case CXCursor_FieldDecl:
		break;
	case CXCursor_VarDecl:
	{
		data->variables.emplace_back(VariableInfo(name));
		auto size = data->variables.size();
		auto child = ClientData<VariableInfo>{ c->_TU, &(data->variables[size - 1]) };
		clang_visitChildren(cursor, TravelVariable, &child);
		break;
	}
	case CXCursor_FunctionDecl:
	{
		data->functions.emplace_back(FunctionInfo(name));
		auto size = data->functions.size();
		auto storage_sc = clang_Cursor_getStorageClass(cursor);
		if (storage_sc == CX_SC_Static)
		{
			data->functions[size - 1].is_static = true;
		}
		auto child = ClientData<FunctionInfo>{ c->_TU, &(data->functions[size - 1]) };
		clang_visitChildren(cursor, TravelFunction, &child);
		break;
	}
	default:
		break;
	}
	return CXChildVisit_Continue;
}

CXChildVisitResult TravelClass(CXCursor cursor, CXCursor parent, CXClientData client_data)
{
	CXSourceLocation location = clang_getCursorLocation(cursor);

	if (clang_Location_isInSystemHeader(location))
		return CXChildVisit_Continue;

	CXCursorKind cursorKind = clang_getCursorKind(cursor);
	CXString cursorSpelling = clang_getCursorSpelling(cursor);
	auto name = cx2string(cursorSpelling);

	auto c = reinterpret_cast<ClientData<ClassInfo> *>(client_data);
	auto data = c->_data;
	auto *access_data = clang_getCursorKind(parent) == CXCursor_StructDecl ?
		&data->public_access : &data->private_access;
	auto cxxasp = clang_getCXXAccessSpecifier(cursor);
	if (cxxasp == CX_CXXPublic)
	{
		access_data = &data->public_access;
	}
	else if (cxxasp == CX_CXXProtected)
	{
		access_data = &data->protect_access;
	}
	else if (cxxasp == CX_CXXPrivate)
	{
		access_data = &data->private_access;
	}

	switch (cursorKind)
	{
	case CXCursor_CXXAccessSpecifier:
		break;
	case CXCursor_Constructor:
	case CXCursor_Destructor:
	case CXCursor_CXXMethod:
	{
		auto ftype = FunctionKind::CXXMETHOD;
		if (cursorKind == CXCursor_Constructor)
			ftype = FunctionKind::CONSTRUCTOR;
		else if (cursorKind == CXCursor_Destructor)
			ftype = FunctionKind::DESTRUCTOR;

		access_data->class_functions.emplace_back(CXXMethodInfo(name, ftype));
		auto size = access_data->class_functions.size();

		access_data->class_functions[size - 1].cls_name = cx2string(clang_getCursorSpelling(parent));

		auto storage_sc = clang_Cursor_getStorageClass(cursor);
		if (storage_sc == CX_SC_Static)
		{
			access_data->class_functions[size - 1].is_static = true;
		}

		auto child = ClientData<CXXMethodInfo>{ c->_TU, &access_data->class_functions[size - 1] };
		clang_visitChildren(cursor, TravelCXXMethod, &child);
		break;
	}
	case CXCursor_FieldDecl:
	{
		access_data->class_variables.emplace_back(FieldInfo(name));
		auto size = access_data->class_variables.size();
		access_data->class_variables[size - 1].class_name = cx2string(clang_getCursorSpelling(parent));
		auto child = ClientData<FieldInfo>{ c->_TU, &access_data->class_variables[size - 1] };
		clang_visitChildren(cursor, TravelField, &child);
		break;
	}
	default:
		break;
	}

	return CXChildVisit_Continue;
}

CXChildVisitResult TravelEnum(CXCursor cursor, CXCursor parent, CXClientData client_data)
{
	CXSourceLocation location = clang_getCursorLocation(cursor);

	if (clang_Location_isInSystemHeader(location))
		return CXChildVisit_Continue;

	CXCursorKind cursorKind = clang_getCursorKind(cursor);
	CXString cursorSpelling = clang_getCursorSpelling(cursor);
	auto name = cx2string(cursorSpelling);

	auto c = reinterpret_cast<ClientData<EnumInfo> *>(client_data);
	auto data = c->_data;

	if (cursorKind == CXCursor_EnumConstantDecl)
	{
		data->tags.emplace_back(name);
	}

	return CXChildVisit_Continue;
}

CXChildVisitResult TravelFunction(CXCursor cursor, CXCursor parent, CXClientData client_data)
{
	CXSourceLocation location = clang_getCursorLocation(cursor);

	if (clang_Location_isInSystemHeader(location))
		return CXChildVisit_Continue;

	CXCursorKind cursorKind = clang_getCursorKind(cursor);
	CXString cursorSpelling = clang_getCursorSpelling(cursor);
	auto name = cx2string(cursorSpelling);
	auto c = reinterpret_cast<ClientData<FunctionInfo> *>(client_data);
	auto data = c->_data;

	switch (cursorKind)
	{
	case CXCursor_ParmDecl:
	{
		ParamInfo param(name);
		param.arg_type = cx2string(clang_getTypeSpelling(clang_getCursorType(cursor)));
		auto cxrange = clang_getCursorExtent(cursor);
		CXToken *tokens = nullptr;
		unsigned int num_tokens = 0;
		clang_tokenize(c->_TU, cxrange, &tokens, &num_tokens);
		auto equal_sign = std::find_if(tokens, tokens + num_tokens, [TU = c->_TU](const CXToken &iter)
		{
			auto _token = cx2string(clang_getTokenSpelling(TU, iter));
			if (_token == std::string("="))
			{
				return true;
			}
			return false;
		});
		if (equal_sign != tokens + num_tokens)
		{
			param.has_default = true;
			auto iter = equal_sign + 1;
			while (iter != tokens + num_tokens)
			{
				param.arg_value.append(cx2string(clang_getTokenSpelling(c->_TU, *iter)));
				iter++;
			}
		}
		clang_disposeTokens(c->_TU, tokens, num_tokens);

		data->params.emplace_back(std::move(param));
		auto size = data->params.size();
		auto child = ClientData<ParamInfo>{ c->_TU, &(data->params[size - 1]) };
		clang_visitChildren(cursor, TravelParam, &child);
		break;
	}
	default:
		break;
	}

	return CXChildVisit_Continue;
}

CXChildVisitResult TravelCXXMethod(CXCursor cursor, CXCursor parent, CXClientData client_data)
{
	CXSourceLocation location = clang_getCursorLocation(cursor);

	if (clang_Location_isInSystemHeader(location))
		return CXChildVisit_Continue;

	CXCursorKind cursorKind = clang_getCursorKind(cursor);
	CXString cursorSpelling = clang_getCursorSpelling(cursor);
	auto name = cx2string(cursorSpelling);
	auto c = reinterpret_cast<ClientData<CXXMethodInfo> *>(client_data);
	auto data = c->_data;

	switch (cursorKind)
	{
	case CXCursor_ParmDecl:
	{
		ParamInfo param(name);
		param.arg_type = cx2string(clang_getTypeSpelling(clang_getCursorType(cursor)));
		auto cxrange = clang_getCursorExtent(cursor);
		CXToken *tokens = nullptr;
		unsigned int num_tokens = 0;
		clang_tokenize(c->_TU, cxrange, &tokens, &num_tokens);
		auto equal_sign = std::find_if(tokens, tokens + num_tokens, [TU = c->_TU](const CXToken &iter)
		{
			auto _token = cx2string(clang_getTokenSpelling(TU, iter));
			if (_token == std::string("="))
			{
				return true;
			}
			return false;
		});
		if (equal_sign != tokens + num_tokens)
		{
			param.has_default = true;
			auto iter = equal_sign + 1;
			while (iter != tokens + num_tokens)
			{
				param.arg_value.append(cx2string(clang_getTokenSpelling(c->_TU, *iter)));
				iter++;
			}
		}
		clang_disposeTokens(c->_TU, tokens, num_tokens);

		data->params.emplace_back(std::move(param));
		auto size = data->params.size();
		auto child = ClientData<ParamInfo>{ c->_TU, &(data->params[size - 1]) };
		clang_visitChildren(cursor, TravelParam, &child);
		break;
	}
	default:
		break;
	}

	return CXChildVisit_Continue;
}

CXChildVisitResult TravelVariable(CXCursor cursor, CXCursor parent, CXClientData client_data)
{
	return CXChildVisit_Continue;
}

CXChildVisitResult TravelField(CXCursor cursor, CXCursor parent, CXClientData client_data)
{
	return CXChildVisit_Continue;
}

CXChildVisitResult TravelParam(CXCursor cursor, CXCursor parent, CXClientData client_data)
{
	return CXChildVisit_Continue;
}


std::unique_ptr<NameSpaceInfo> Visitor(int argc, char *argv[])
{
	auto index = std::unique_ptr<void, decltype(&clang_disposeIndex)> {
		clang_createIndex(0, 0), 
		clang_disposeIndex
	};


	//CXTranslationUnit_DetailedPreprocessingRecord  //ºêÏà¹ØµÄ

	auto flag = CXTranslationUnit_SkipFunctionBodies |
		CXTranslationUnit_DetailedPreprocessingRecord |
		CXTranslationUnit_IncludeBriefCommentsInCodeCompletion |
		CXTranslationUnit_VisitImplicitAttributes |
		//CXTranslationUnit_IncludeAttributedTypes |
		CXTranslationUnit_KeepGoing;

	auto unit = std::unique_ptr<std::remove_pointer<CXTranslationUnit>::type,
		decltype(&clang_disposeTranslationUnit)> { 
		clang_parseTranslationUnit(index.get(), nullptr, argv, argc, nullptr, 0, flag), 
		clang_disposeTranslationUnit 
	};

	if (unit == nullptr)
	{
		printf("Unable to parse translation unit!\n");
		std::exit(-1);
	}

	auto ns = new NameSpaceInfo{ "global" };
	ClientData<NameSpaceInfo> data{ unit.get(), ns };

	CXCursor root_cursor = clang_getTranslationUnitCursor(unit.get());
	clang_visitChildren(root_cursor, TravelNamespace, &data);

	return std::unique_ptr<NameSpaceInfo>{ ns };
}