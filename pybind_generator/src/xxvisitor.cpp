#include "xxvisitor.h"

static std::string cx2string(const CXString &cx_str)
{
	std::string str(clang_getCString(cx_str));
	clang_disposeString(cx_str);
	return str;
}

CXChildVisitResult TravelNamespace(CXCursor cursor, CXCursor parent, CXClientData client_data)
{
	CXSourceLocation location = clang_getCursorLocation(cursor);

	if (clang_Location_isInSystemHeader(location))
		return CXChildVisit_Continue;

	CXCursorKind cursorKind = clang_getCursorKind(cursor);
	CXString cursorSpelling = clang_getCursorSpelling(cursor);
	auto name = cx2string(cursorSpelling);

	auto c = reinterpret_cast<ClientData<NameSpaceData> *>(client_data);
	auto data = c->_data;

	switch (cursorKind)
	{
	case CXCursor_Namespace:
	{
		if (data->inner_namespaces.find(name) == data->inner_namespaces.end())
		{
			data->inner_namespaces.emplace(
				name, std::unique_ptr<NameSpaceData>(new NameSpaceData(name))
			);
		}
		auto child = ClientData<NameSpaceData>{ c->_TU, data->inner_namespaces[name].get() };
		clang_visitChildren(cursor, TravelNamespace, &child);
		break;
	}
	case CXCursor_StructDecl:
	case CXCursor_ClassDecl:
	{
		data->classes.emplace_back(ClassData(name));
		auto size = data->classes.size();
		auto child = ClientData<ClassData>{ c->_TU, &(data->classes[size - 1]) };
		clang_visitChildren(cursor, TravelClass, &child);
		break;
	}
	case CXCursor_EnumDecl:
	case CXCursor_EnumConstantDecl:
	{
		data->enumerates.emplace_back(EnumData(name));
		auto size = data->enumerates.size();
		auto child = ClientData<EnumData>{ c->_TU, &(data->enumerates[size - 1]) };
		clang_visitChildren(cursor, TravelEnum, &child);
		break;
	}
	case CXCursor_FieldDecl:
		break;
	case CXCursor_VarDecl:
	{
		data->variables.emplace_back(VariableData(name));
		auto size = data->variables.size();
		auto child = ClientData<VariableData>{ c->_TU, &(data->variables[size - 1]) };
		clang_visitChildren(cursor, TravelVariable, &child);
		break;
	}
	case CXCursor_FunctionDecl:
	{
		data->functions.emplace_back(FunctionData(name));
		auto size = data->functions.size();
		auto child = ClientData<FunctionData>{ c->_TU, &(data->functions[size - 1]) };
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

	auto c = reinterpret_cast<ClientData<ClassData> *>(client_data);
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

		access_data->class_functions.emplace_back(FunctionData(name, ftype));
		auto size = access_data->class_functions.size();
		auto storage_sc = clang_Cursor_getStorageClass(cursor);
		if (storage_sc == CX_SC_Static)
		{
			access_data->class_functions[size - 1].is_static = true;
		}

		auto child = ClientData<FunctionData>{ c->_TU, &access_data->class_functions[size - 1] };
		clang_visitChildren(cursor, TravelFunction, &child);
		break;
	}
	case CXCursor_FieldDecl:
	{
		access_data->class_variables.emplace_back(VariableData(name));
		auto size = access_data->class_variables.size();
		auto child = ClientData<VariableData>{ c->_TU, &access_data->class_variables[size - 1] };
		clang_visitChildren(cursor, TravelVariable, &child);
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

	auto c = reinterpret_cast<ClientData<EnumData> *>(client_data);
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

	auto c = reinterpret_cast<ClientData<FunctionData> *>(client_data);
	auto data = c->_data;

	switch (cursorKind)
	{
	case CXCursor_ParmDecl:
	{
		ParamData param(name);
		param.arg_type = cx2string(clang_getTypeSpelling(clang_getCursorType(cursor)));
		auto cxrange = clang_getCursorExtent(cursor);
		CXToken *tokens = nullptr;
		unsigned int num_tokens = 0;
		clang_tokenize(*(c->_TU), cxrange, &tokens, &num_tokens);
		auto equal_sign = std::find_if(tokens, tokens + num_tokens, [TU = *(c->_TU)](const CXToken &iter)
		{
			if (cx2string(clang_getTokenSpelling(TU, iter)) == std::string("="))
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
				param.arg_value.append(cx2string(clang_getTokenSpelling(*(c->_TU), *iter)));
				iter++;
			}
		}
		clang_disposeTokens(*(c->_TU), tokens, num_tokens);

		data->params.emplace_back(std::move(param));
		auto size = data->params.size();
		auto child = ClientData<ParamData>{ c->_TU, &(data->params[size - 1]) };
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

CXChildVisitResult TravelParam(CXCursor cursor, CXCursor parent, CXClientData client_data)
{
	return CXChildVisit_Continue;
}

std::unique_ptr<NameSpaceData> Visitor(int argc, char *argv[])
{
	CXIndex index = clang_createIndex(0, 0);

	//CXTranslationUnit_DetailedPreprocessingRecord  //����ص�

	auto flag = CXTranslationUnit_SkipFunctionBodies |
		CXTranslationUnit_DetailedPreprocessingRecord |
		CXTranslationUnit_IncludeBriefCommentsInCodeCompletion |
		CXTranslationUnit_VisitImplicitAttributes |
		//CXTranslationUnit_IncludeAttributedTypes |
		CXTranslationUnit_KeepGoing;

	auto unit = clang_parseTranslationUnit(index, nullptr, argv, argc, nullptr, 0, flag);

	if (unit == nullptr)
	{
		printf("Unable to parse translation unit!\n");
		std::exit(-1);
	}

	CXCursor root_cursor = clang_getTranslationUnitCursor(unit);

	auto ns = new NameSpaceData{ "global" };
	ClientData<NameSpaceData> data{ &unit, ns };

	clang_visitChildren(root_cursor, TravelNamespace, &data);

	clang_disposeTranslationUnit(unit);
	clang_disposeIndex(index);

	return std::unique_ptr<NameSpaceData>{ ns };
}