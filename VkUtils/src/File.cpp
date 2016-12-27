#include "File.hpp"

static std::vector<uint8_t> readBinaryFile(const char* path)
{
	std::FILE* fp = std::fopen(filename.c_str(), "rb");
	if (fp)
	{
		std::vector<uint8_t> contents;
		std::fseek(fp, 0, SEEK_END);
		contents.resize(std::ftell(fp));
		std::rewind(fp);
		std::fread(&contents[0], 1, contents.size(), fp);
		std::fclose(fp);
		return (contents);
	}
	throw(errno);
}