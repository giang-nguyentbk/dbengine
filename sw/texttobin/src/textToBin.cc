#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include <cstring>
#include <unistd.h>

enum class DbTypeEnum
{
	TYPE_OF_ENTRY_UNDEFINED	= 0,
	TYPE_OF_ENTRY_U8	= 1,
	TYPE_OF_ENTRY_S8	= 2,
	TYPE_OF_ENTRY_U16	= 3,
	TYPE_OF_ENTRY_S16	= 4,
	TYPE_OF_ENTRY_U32	= 5,
	TYPE_OF_ENTRY_S32	= 6,
	TYPE_OF_ENTRY_U64	= 7,
	TYPE_OF_ENTRY_S64	= 8,
	TYPE_OF_ENTRY_CHAR	= 9,
};

struct DbEntry
{
	std::string key;
	std::string type;
	std::string value;
};

bool isBigEndian();
bool tokenize(const char*& p, std::string& token, uint8_t index);
std::vector<char> generatePayload(const std::vector<std::string>& entries);
std::vector<std::string> convertDBEntries(std::ifstream& txtFile);


/* Format: ./textToBin -i <abs_path_to_txt_DB_file> -o <abs_path_to_bin_DB_file> -e 		*/
/* Options:											*/
/* 	+ i: absolute path to the text-based database file					*/
/* 	+ o: absolute path to the converted binary database file				*/
/*	+ e: is binary database file encrypted?							*/ 
int main(int argc, char* argv[])
{
	int opt = 0;
	std::string txtFilePath {""};
	std::string binFilePath {""};
	bool isEncrypted = false;

	while((opt = getopt(argc, argv, "i:o:e")) != -1)
	{
		switch (opt)
		{
		case 'i':
			txtFilePath = std::string(optarg);
			break;

		case 'o':
			binFilePath = std::string(optarg);
			break;
		
		case 'e':
			isEncrypted = true;
			break;
		default:
			std::cout << "ERROR:\n";
			std::cout << "\tUsage:  " << argv[0] << "-i <abs_path_to_txt_DB_file> -o <abs_path_to_bin_DB_file> -e\n";
			std::cout << "\tOption:\n";
			std::cout << "\t\t -i : absolute path to the text-based database file.\n";
			std::cout << "\t\t -o : absolute path to the converted binary database file.\n";
			std::cout << "\t\t -e : is the converted binary database file's content encrypted?\n";
			exit(EXIT_FAILURE);
			break;
		}
	}

	if(txtFilePath.empty() || binFilePath.empty())
	{
		std::cout << "ERROR: Empty file path given, double check execute command!" << std::endl;
		exit(EXIT_FAILURE);
	}

	std::ifstream txtFile(txtFilePath.c_str());
	if(!txtFile.is_open())
	{
		// ERROR TRACE
		std::cout << "ERROR: Failed to open file: " << txtFilePath << std::endl;
		exit(EXIT_FAILURE);
	}

	std::ofstream binFile(binFilePath, std::ios_base::binary);
	if(!binFile.is_open())
	{
		// ERROR TRACE
		std::cout << "ERROR: Failed to open file: " << binFilePath << std::endl;
		exit(EXIT_FAILURE);
	}

	
	std::vector<std::string> entries = convertDBEntries(txtFile);

	std::vector<char> payload = generatePayload(entries);

	char c = 'H';
	binFile.write(&c, 1); // DB Header
	c = 10;
	binFile.write(&c, 1); // DB revision, currently hardcoded
	c = 0;
	for(int i = 0; i < 4; ++i) binFile.write(&c, 1); // Reserved 4 bytes for additional DB parameters
	uint32_t totalPayloadBytes = payload.size();
	std::cout << "totalPayloadBytes = " << totalPayloadBytes << std::endl;
	if(!isBigEndian())
	{
		uint32_t highhigh = (totalPayloadBytes & 0xFF000000) >> 24;
		uint32_t lowlow = (totalPayloadBytes & 0xFF) << 24;
		uint32_t high = (totalPayloadBytes & 0x00FF0000) >> 8;
		uint32_t low = (totalPayloadBytes & 0xFF00) << 8;
		totalPayloadBytes = highhigh | high | low | lowlow;
	}
	for(int i = 0; i < 4; ++i) binFile.write((char *)&totalPayloadBytes + i, 1); // Total bytes of payload (all DB entries)
	for(const auto& ch : payload) binFile.write(&ch, 1); // Write DB payload (converted DB entries)
	c = 'E';
	binFile.write(&c, 1); // DB End Indication
	// TODO: Add CRC Checksum

	txtFile.close();
	binFile.close();

	exit(EXIT_SUCCESS);
}

std::vector<std::string> convertDBEntries(std::ifstream& txtFile)
{
	std::vector<std::string> entryVec;
	entryVec.reserve(512); // currently hardcoded
	std::string entry;
	char prevprev = -1;
	char prev = -1;
	char current;
	bool isCommentStarted = false;

	while((current = txtFile.get()) != EOF)
	{
		if(prev == '/' && current == '*')
		{
			isCommentStarted = true;
			// Pop out last inserted character '/' from entry which was wrongly inserted
			entry.pop_back(); 
		}
		else if(prev == '*' && current == '/')
		{
			isCommentStarted = false;
		}

		if(isCommentStarted || (prev == '*' && current == '/') || current == '\n')
		{
			prevprev = prev;
			prev = current;
			continue;
		}

		if(prevprev == '\n' && prev == '/' && current != '*')
		{
			// Start new DB entry
			if(entry.length() > 1)
			{
				entry.pop_back(); // Remove redundant '/' from our old entry
				std::cout << "Entry: " << entry << std::endl;
				entryVec.push_back(entry);
			}
			entry.clear();

			entry += '/'; // Add redundant '/' back again
		}

		entry += current;
		prevprev = prev;
		prev = current;
	}

	return entryVec;
}

std::vector<char> generatePayload(const std::vector<std::string>& entries)
{
	std::vector<char> payload;

	for(const auto& e : entries)
	{
		DbEntry entry;
		std::string token;
		const char *p = e.c_str();
		uint8_t index = 0;
		while(tokenize(p, token, index))
		{
			if(index == 0) entry.key = token;
			else if(index == 1) entry.type = token;
			else if(index == 2) entry.value = token;

			if(++index > 2) break;
		}

		payload.push_back('F');
		for(const auto& c : entry.key) payload.push_back(c);
		payload.push_back('\0');
		if(entry.type == "U8") payload.push_back((char)DbTypeEnum::TYPE_OF_ENTRY_U8);
		else if(entry.type == "S8") payload.push_back((char)DbTypeEnum::TYPE_OF_ENTRY_S8);
		else if(entry.type == "U16") payload.push_back((char)DbTypeEnum::TYPE_OF_ENTRY_U16);
		else if(entry.type == "S16") payload.push_back((char)DbTypeEnum::TYPE_OF_ENTRY_S16);
		else if(entry.type == "U32") payload.push_back((char)DbTypeEnum::TYPE_OF_ENTRY_U32);
		else if(entry.type == "S32") payload.push_back((char)DbTypeEnum::TYPE_OF_ENTRY_S32);
		else if(entry.type == "U64") payload.push_back((char)DbTypeEnum::TYPE_OF_ENTRY_U64);
		else if(entry.type == "S64") payload.push_back((char)DbTypeEnum::TYPE_OF_ENTRY_S64);
		else if(entry.type == "CHAR") payload.push_back((char)DbTypeEnum::TYPE_OF_ENTRY_CHAR);
		else
		{
			std::cout << "ERROR: Unknown DB entry type = " << entry.type << std::endl;
			break;
		}
		std::cout << "entry.value: " << entry.value << ", length: " << entry.value.length() << std::endl;
		for(const auto& c : entry.value) payload.push_back(c);
		payload.push_back('\0');
	}

	return payload;
}

bool tokenize(const char*& p, std::string& token, uint8_t index)
{
	// Remove " \t" from front
	while(*p && strchr(" \t", *p))
	{
		p++;
	}

	/* Or token is an actual argument */
	token.clear(); // Clear the previous token
	if(index == 2) // Special handle for value in each DB entry
	{
		while(*p)
		{
			token += *p;
			if(!*(p + 1))
			{
				// Next byte will be NULL-termination
				// Remove " \t" from back
				while(*p == ' ' || *p == '\t')
				{
					token.pop_back();
					--p;
				}
				break;
			}
			else
			{
				++p;
			}
		}
	}
	else
	{
		while(*p && !strchr(" \t", *p))
		{
			token += *p++;
		}
	}

	return (token.length() > 0);
}

bool isBigEndian()
{
	int n = 1;
	if(*((char *)&n) == 1) return false;
	return true;
}
