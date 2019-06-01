#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <clang-c\Index.h>

#include "xxdata.h"


std::string cx2string(const CXString &cx_str)
{
	std::string str(clang_getCString(cx_str));
	clang_disposeString(cx_str);
	return str;
}

template<typename Data>
struct ClientData;

template<typename Data>
class DataVisitor
{
public:
	DataVisitor(ClientData<Data> *client)
		:_client(client)
	{ }

	CXChildVisitResult Travel(CXCursor cursor, CXCursor parent)
	{
		return CXChildVisit_Continue;
	}
private:
	ClientData<Data> *_client;
};

template<typename Data, typename = std::enable_if<std::is_base_of<XXInfo<Data>, Data>::value>::type>
CXChildVisitResult CursorVisitor(CXCursor cursor, CXCursor parent, CXClientData client_data)
{
	ClientData<Data> *client = reinterpret_cast<ClientData<Data> *>(client_data);
	return client->_visitor.Travel(cursor, parent);
}

template<typename Data>
struct ClientData
{
	static_assert(std::is_base_of<XXInfo<Data>, Data>::value, "Data must inherit from XXInfo");
	CXTranslationUnit _TU;
	Data *_data;

	DataVisitor<Data> _visitor{ this };
	void Travel(CXCursor cursor)
	{
		clang_visitChildren(cursor, CursorVisitor<Data>, this);
	}
};

template<>
CXChildVisitResult DataVisitor<FunctionInfo>::Travel(CXCursor cursor, CXCursor parent)
{
	CXSourceLocation location = clang_getCursorLocation(cursor);

	if (clang_Location_isInSystemHeader(location))
		return CXChildVisit_Continue;

	CXCursorKind cursorKind = clang_getCursorKind(cursor);
	CXString cursorSpelling = clang_getCursorSpelling(cursor);
	auto name = cx2string(cursorSpelling);
	auto c = this->_client;
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
		child.Travel(cursor);
		break;
	}
	default:
		break;
	}

	return CXChildVisit_Continue;
}

template<>
CXChildVisitResult DataVisitor<CXXMethodInfo>::Travel(CXCursor cursor, CXCursor parent)
{
	CXSourceLocation location = clang_getCursorLocation(cursor);

	if (clang_Location_isInSystemHeader(location))
		return CXChildVisit_Continue;

	CXCursorKind cursorKind = clang_getCursorKind(cursor);
	CXString cursorSpelling = clang_getCursorSpelling(cursor);
	auto name = cx2string(cursorSpelling);
	auto c = this->_client;
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
		child.Travel(cursor);
		break;
	}
	default:
		break;
	}

	return CXChildVisit_Continue;
}

template<>
CXChildVisitResult DataVisitor<EnumInfo>::Travel(CXCursor cursor, CXCursor parent)
{
	CXSourceLocation location = clang_getCursorLocation(cursor);

	if (clang_Location_isInSystemHeader(location))
		return CXChildVisit_Continue;

	CXCursorKind cursorKind = clang_getCursorKind(cursor);
	CXString cursorSpelling = clang_getCursorSpelling(cursor);
	auto name = cx2string(cursorSpelling);

	auto c = this->_client;
	auto data = c->_data;

	if (cursorKind == CXCursor_EnumConstantDecl)
	{
		data->tags.emplace_back(name);
	}

	return CXChildVisit_Continue;
}

template<>
CXChildVisitResult DataVisitor<ClassInfo>::Travel(CXCursor cursor, CXCursor parent)
{
	CXSourceLocation location = clang_getCursorLocation(cursor);

	if (clang_Location_isInSystemHeader(location))
		return CXChildVisit_Continue;

	CXCursorKind cursorKind = clang_getCursorKind(cursor);
	CXString cursorSpelling = clang_getCursorSpelling(cursor);
	auto name = cx2string(cursorSpelling);
	auto c = this->_client;
	auto data = c->_data;
	AccessInfo *access_data = clang_getCursorKind(parent) == CXCursor_StructDecl ?
		&(data->public_access) : &(data->private_access);
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
		{
			if (clang_CXXConstructor_isCopyConstructor(cursor))
				ftype = FunctionKind::COPY_CONSTRUCTOR;
			else if (clang_CXXConstructor_isMoveConstructor(cursor))
				ftype = FunctionKind::MOVE_CONSTRUCTOR;
			else
				ftype = FunctionKind::NORMAL_CONSTRUCTOR;
		}
		else if (cursorKind == CXCursor_Destructor)
			ftype = FunctionKind::DESTRUCTOR;


		access_data->class_functions.emplace_back(CXXMethodInfo(name, ftype));
		auto size = access_data->class_functions.size();

		access_data->class_functions[size - 1].class_name = cx2string(clang_getCursorSpelling(parent));
		if (clang_CXXMethod_isConst(cursor))
		{
			access_data->class_functions[size - 1].is_const = true;
		}

		auto storage_sc = clang_Cursor_getStorageClass(cursor);
		if (storage_sc == CX_SC_Static)
		{
			access_data->class_functions[size - 1].is_static = true;
		}

		auto child = ClientData<CXXMethodInfo>{ c->_TU, &access_data->class_functions[size - 1] };
		child.Travel(cursor);
		break;
	}
	case CXCursor_VarDecl:
	case CXCursor_FieldDecl:
	{
		access_data->class_variables.emplace_back(FieldInfo(name));
		auto size = access_data->class_variables.size();
		
		auto storage_sc = clang_Cursor_getStorageClass(cursor);
		if (storage_sc == CX_SC_Static)
		{
			access_data->class_variables[size - 1].is_static = true;
		}

		access_data->class_variables[size - 1].class_name = cx2string(clang_getCursorSpelling(parent));
		auto child = ClientData<FieldInfo>{ c->_TU, &access_data->class_variables[size - 1] };
		child.Travel(cursor);
		break;
	}
	default:
		break;
	}

	return CXChildVisit_Continue;
}

template<>
CXChildVisitResult DataVisitor<NameSpaceInfo>::Travel(CXCursor cursor, CXCursor parent)
{
	CXSourceLocation location = clang_getCursorLocation(cursor);

	if (clang_Location_isInSystemHeader(location))
		return CXChildVisit_Continue;

	CXCursorKind cursorKind = clang_getCursorKind(cursor);
	CXString cursorSpelling = clang_getCursorSpelling(cursor);
	auto name = cx2string(cursorSpelling);

	auto c = this->_client;
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
		child.Travel(cursor);
		break;
	}
	case CXCursor_StructDecl:
	case CXCursor_ClassDecl:
	{
		if (data->classes.find(name) == data->classes.end())
		{
			data->classes.emplace(
				name, std::unique_ptr<ClassInfo>(new ClassInfo(name))
			);
		}
		auto child = ClientData<ClassInfo>{ c->_TU, data->classes[name].get() };
		child.Travel(cursor);
		break;
	}
	case CXCursor_EnumDecl:
	case CXCursor_EnumConstantDecl:
	{
		if (data->enumerates.find(name) == data->enumerates.end())
		{
			data->enumerates.emplace(
				name, std::unique_ptr<EnumInfo>(new EnumInfo(name))
			);
		}
		auto child = ClientData<EnumInfo>{ c->_TU, data->enumerates[name].get() };
		child.Travel(cursor);
		break;
	}
	case CXCursor_FieldDecl:
		break;
	case CXCursor_VarDecl:
	{
		if (data->variables.find(name) == data->variables.end())
		{
			data->variables.emplace(
				name, std::unique_ptr<VariableInfo>(new VariableInfo(name))
			);
		}
		auto child = ClientData<VariableInfo>{ c->_TU, data->variables[name].get() };
		child.Travel(cursor);
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
		child.Travel(cursor);
		break;
	}
	default:
		break;
	}
	return CXChildVisit_Continue;
}


std::unique_ptr<NameSpaceInfo> Visitor(int argc, const char *argv[])
{
	auto index = std::unique_ptr<void, decltype(&clang_disposeIndex)>{
		clang_createIndex(0, 0),
		clang_disposeIndex
	};

	//std::vector<const char *> arguments(&argv[0], &argv[argc]);

	//arguments.push_back("temp.hpp");
	//arguments.push_back("-xc++");
	//arguments.push_back("-I../../src");


	auto flag = CXTranslationUnit_SkipFunctionBodies |
		CXTranslationUnit_DetailedPreprocessingRecord |
		CXTranslationUnit_IncludeBriefCommentsInCodeCompletion |
		CXTranslationUnit_VisitImplicitAttributes |
		//CXTranslationUnit_IncludeAttributedTypes |
		CXTranslationUnit_KeepGoing;

	auto unit = std::unique_ptr<std::remove_pointer<CXTranslationUnit>::type,
		decltype(&clang_disposeTranslationUnit)>{
		clang_parseTranslationUnit(index.get(), nullptr, 
		argv,
		argc,
		nullptr, 0, flag),
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
	data.Travel(root_cursor);

	return std::unique_ptr<NameSpaceInfo>{ ns };
}
