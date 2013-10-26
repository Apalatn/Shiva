#ifndef __FILE__UTILS__H
#define __FILE__UTILS__H

// file reader grabbed from http://stackoverflow.com/questions/2602013/read-whole-ascii-file-into-c-stdstring
// (Comments mine)

#include <fstream>
#include <string>
#include <cerrno>

std::string get_file_contents(const char *filename)
{
	// attempt to open file input stream
	std::ifstream in(filename, std::ios::in | std::ios::binary);
	if (in)
	{
		// create string to store output
		std::string contents;		

		// seek to end of file to determine size
		in.seekg(0, std::ios::end);	
		contents.resize(in.tellg());

		// seek to beginning and read file into string
		in.seekg(0, std::ios::beg);
		in.read(&contents[0], contents.size());

		// close the stream and return
		in.close();
		return(contents);
	}
	throw(errno);
}

#endif // !__FILE__UTILS__H
