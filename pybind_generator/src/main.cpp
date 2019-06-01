#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <memory>
#include <algorithm>

#include "xxvisitor.hpp"
#include "show_contents.hpp"
#include "pybind_generator.hpp"


std::string usage()
{
	auto usage = R"(
Usage:
	pybind_generator --input header_file --output output_file [ --embedded ]
)";
	return usage;
}

std::string pybind11_include()
{
	auto headers = R"(
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <pybind11/external.hpp>
)";
	// #include ....
	return headers;
}

struct ParamUnit 
{
	bool show_contents = false;
	bool is_embedded = false;
	std::string output_file = {};
	std::string module_name = {};
	std::vector<std::string> header_files = {};
	std::vector<std::string> clang_params = {};
};

ParamUnit parseArgs(int argc, char *argv[])
{
	ParamUnit unit;
	for (int i = 1; i < argc;)
	{
		auto param = std::string(argv[i]);
		if (param == "--input")
		{
			i++;
			while (i < argc && argv[i][0] != '-')
			{
				unit.header_files.push_back(std::string(argv[i]));
				i++;
			}
		}
		else if (param == "--output")
		{
			i++;
			if (i < argc)
			{
				unit.output_file = std::string(argv[i]);
				i++;
			}
		}
		else if (param == "--module")
		{
			i++;
			if (i < argc)
			{
				unit.module_name = std::string(argv[i]);
				i++;
			}
		}
		else if (param == "--embedded")
		{
			unit.is_embedded = true;
			i++;
		}
		else if (param == "--show")
		{
			unit.show_contents = true;
			i++;
		}
		else
		{
			unit.clang_params.push_back(std::string(argv[i]));
			i++;
		}
	}
	return unit;
}

int main(int argc, char *argv[])
{
	auto unit = parseArgs(argc, argv);

	{
		std::ofstream ofs("headers.hpp");
		for (auto &iter: unit.header_files)
		{
			ofs << "#include \"" << iter << "\"\n";
		}
		ofs << pybind11_include();
	}

	std::vector<const char *> arguments;
	arguments.push_back("headers.hpp");
	arguments.push_back("-xc++");


	auto nsdata = Visitor(
		static_cast<int>(arguments.size()),
		arguments.data()
	);


	//auto code = show_contents(nsdata.get());
	//std::cout << code << std::endl;

	std::string code = "#include \"headers.hpp\"\n\n";
	std::string mod_name;
	std::string out_file;
	
	if (unit.module_name != "")
	{
		mod_name = unit.module_name;
	}
	else
	{
		mod_name = "pymod";
	}
	
	if (unit.show_contents)
	{
		code.append(show_contents(nsdata.get()));
	}
	else if (unit.is_embedded)
	{
		code.append(printEmbeddedCode(nsdata.get(), mod_name.c_str()));
	}
	else
	{
		code.append(printExtensionCode(nsdata.get(), mod_name.c_str()));
	}

	if (unit.output_file != "")
	{
		std::ofstream out(unit.output_file.c_str(), std::ios::out);
		out << code << std::endl;
	}
	else
	{
		std::cout << code << std::endl;
	}

}
