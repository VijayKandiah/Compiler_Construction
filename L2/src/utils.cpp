#include "L2.h"
#include "utils.h"

namespace L2{
	void replace_string(std::string & input, std::string find, std::string replace) //just a find & replace function for a string
	{
		size_t pos = input.find(find);
		while( pos != std::string::npos)
		{
			input.replace(pos, find.size(), replace);
			pos =input.find(find, pos + replace.size());
		}
	}
}
