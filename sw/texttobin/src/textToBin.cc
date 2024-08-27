#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include <cstring>
#include <unistd.h>

static uint32_t crc16Table[256] = 
{
	0x0000, 0x1189, 0x2312, 0x329B, 0x4624, 0x57AD, 0x6536, 0x74BF,
	0x8C48, 0x9DC1, 0xAF5A, 0xBED3, 0xCA6C, 0xDBE5, 0xE97E, 0xF8F7,
	0x0919, 0x1890, 0x2A0B, 0x3B82, 0x4F3D, 0x5EB4, 0x6C2F, 0x7DA6,
	0x8551, 0x94D8, 0xA643, 0xB7CA, 0xC375, 0xD2FC, 0xE067, 0xF1EE,
	0x1232, 0x03BB, 0x3120, 0x20A9, 0x5416, 0x459F, 0x7704, 0x668D,
	0x9E7A, 0x8FF3, 0xBD68, 0xACE1, 0xD85E, 0xC9D7, 0xFB4C, 0xEAC5,
	0x1B2B, 0x0AA2, 0x3839, 0x29B0, 0x5D0F, 0x4C86, 0x7E1D, 0x6F94,
	0x9763, 0x86EA, 0xB471, 0xA5F8, 0xD147, 0xC0CE, 0xF255, 0xE3DC,
	0x2464, 0x35ED, 0x0776, 0x16FF, 0x6240, 0x73C9, 0x4152, 0x50DB,
	0xA82C, 0xB9A5, 0x8B3E, 0x9AB7, 0xEE08, 0xFF81, 0xCD1A, 0xDC93,
	0x2D7D, 0x3CF4, 0x0E6F, 0x1FE6, 0x6B59, 0x7AD0, 0x484B, 0x59C2,
	0xA135, 0xB0BC, 0x8227, 0x93AE, 0xE711, 0xF698, 0xC403, 0xD58A,
	0x3656, 0x27DF, 0x1544, 0x04CD, 0x7072, 0x61FB, 0x5360, 0x42E9,
	0xBA1E, 0xAB97, 0x990C, 0x8885, 0xFC3A, 0xEDB3, 0xDF28, 0xCEA1,
	0x3F4F, 0x2EC6, 0x1C5D, 0x0DD4, 0x796B, 0x68E2, 0x5A79, 0x4BF0,
	0xB307, 0xA28E, 0x9015, 0x819C, 0xF523, 0xE4AA, 0xD631, 0xC7B8,
	0x48C8, 0x5941, 0x6BDA, 0x7A53, 0x0EEC, 0x1F65, 0x2DFE, 0x3C77,
	0xC480, 0xD509, 0xE792, 0xF61B, 0x82A4, 0x932D, 0xA1B6, 0xB03F,
	0x41D1, 0x5058, 0x62C3, 0x734A, 0x07F5, 0x167C, 0x24E7, 0x356E,
	0xCD99, 0xDC10, 0xEE8B, 0xFF02, 0x8BBD, 0x9A34, 0xA8AF, 0xB926,
	0x5AFA, 0x4B73, 0x79E8, 0x6861, 0x1CDE, 0x0D57, 0x3FCC, 0x2E45,
	0xD6B2, 0xC73B, 0xF5A0, 0xE429, 0x9096, 0x811F, 0xB384, 0xA20D,
	0x53E3, 0x426A, 0x70F1, 0x6178, 0x15C7, 0x044E, 0x36D5, 0x275C,
	0xDFAB, 0xCE22, 0xFCB9, 0xED30, 0x998F, 0x8806, 0xBA9D, 0xAB14,
	0x6CAC, 0x7D25, 0x4FBE, 0x5E37, 0x2A88, 0x3B01, 0x099A, 0x1813,
	0xE0E4, 0xF16D, 0xC3F6, 0xD27F, 0xA6C0, 0xB749, 0x85D2, 0x945B,
	0x65B5, 0x743C, 0x46A7, 0x572E, 0x2391, 0x3218, 0x0083, 0x110A,
	0xE9FD, 0xF874, 0xCAEF, 0xDB66, 0xAFD9, 0xBE50, 0x8CCB, 0x9D42,
	0x7E9E, 0x6F17, 0x5D8C, 0x4C05, 0x38BA, 0x2933, 0x1BA8, 0x0A21,
	0xF2D6, 0xE35F, 0xD1C4, 0xC04D, 0xB4F2, 0xA57B, 0x97E0, 0x8669,
	0x7787, 0x660E, 0x5495, 0x451C, 0x31A3, 0x202A, 0x12B1, 0x0338,
	0xFBCF, 0xEA46, 0xD8DD, 0xC954, 0xBDEB, 0xAC62, 0x9EF9, 0x8F70
};

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

void constructBinaryFile(const std::vector<char>& payload, std::ofstream& binFile);
uint32_t lookupCRC16Table(uint32_t initCRC, uint8_t data);
uint16_t getCRC16(uint8_t *startAddr, uint32_t numberBytes);
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

	(void)isEncrypted;

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

	constructBinaryFile(payload, binFile);

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
				// std::cout << "Entry: " << entry << std::endl;
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
		// std::cout << "entry.value: " << entry.value << ", length: " << entry.value.length() << std::endl;
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
			token += *p++;
		}
		--p;
		// Remove " \t" from back
		while(*p == ' ' || *p == '\t')
		{
			token.pop_back();
			--p;
		}
	}
	else
	{
		while(*p && !strchr(" \t", *p)) token += *p++;
	}

	return (token.length() > 0);
}

bool isBigEndian()
{
	int n = 1;
	if(*((char *)&n) == 1) return false;
	return true;
}

uint32_t lookupCRC16Table(uint32_t initCRC, uint8_t data)
{
	// std::cout << "(initCRC ^ data) & 0xFF = " << uint32_t((initCRC ^ data) & 0xFF) << std::endl;
	initCRC = (initCRC >> 8) ^ crc16Table[(initCRC ^ data) & 0xFF];
	return initCRC;
}

uint16_t getCRC16(uint8_t *startAddr, uint32_t numberBytes)
{
	uint32_t tmp = 0xFFFF;
	uint8_t *currentAddr;

	for(currentAddr = startAddr; currentAddr < startAddr + numberBytes; ++currentAddr)
	{
		tmp = lookupCRC16Table(tmp, *currentAddr);
	}

	return (tmp ^ 0xFFFF) & 0xFFFF;
}

void constructBinaryFile(const std::vector<char>& payload, std::ofstream& binFile)
{
	char c = 'H';
	binFile.write(&c, 1); // DB Header
	c = 10;
	binFile.write(&c, 1); // DB revision, currently hardcoded
	c = 0;
	for(int i = 0; i < 4; ++i) binFile.write(&c, 1); // Reserved 4 bytes for additional DB parameters
	uint32_t totalPayloadBytes = payload.size();
	// std::cout << "totalPayloadBytes = " << totalPayloadBytes << std::endl;
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
	uint16_t crc16 = getCRC16((uint8_t *)payload.data(), payload.size());
	// std::cout << "crc16 = " << crc16 << std::endl;
	if(!isBigEndian())
	{
		uint16_t high = (crc16 & 0xFF00) >> 8;
		crc16 = (crc16 << 8) | high;
	}
	for(int i = 0; i < 2; ++i) binFile.write((char *)&crc16 + i, 1); // CRC16 Checksum
}