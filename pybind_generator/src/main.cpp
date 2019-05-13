#include <iostream>
#include <string>
#include <map>
#include <memory>
#include <algorithm>

#include "show_contents.hpp"



int main(int argc, char *argv[])
{
	auto nsdata = Visitor(argc, argv);

	ShowContents<NameSpaceData> d{ nsdata.get(), "    " };
	std::string code = d.Print(0);

	std::cout << code << std::endl;


	return 0;
}