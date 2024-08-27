#include <fstream>
#include <algorithm>

#include "dbLoader.h"

namespace DbEngine
{
namespace DatabaseIf
{
namespace V1
{

static const std::string m_binFilePath { "/home/giangnguyentbk/workspace/dbengine/sw/texttobin/swdb/swdb.bin" }; // currently hardcoded

DbLoader& DbLoader::getInstance()
{
	static DbLoader instance(m_binFilePath);
	return instance;
}

DbLoader::DbLoader(const std::string& binFilePath)
{
	if(!loadDb(binFilePath))
	{
		// ERROR TRACE
		return;
	}
}

bool DbLoader::loadDb(const std::string& binFilePath)
{
	std::ifstream dbFile(binFilePath);
	if(!dbFile.is_open())
	{
		// ERROR TRACE
		return false;
	}

	char c;

	// DB Header Tag check
	c = dbFile.get();
	if(c != 'H')
	{
		// ERROR TRACE
		dbFile.close();
		return false;
	}

	// DB Revision check, currently hardcoded = 10
	c = dbFile.get();
	if(c != 10)
	{
		// ERROR TRACE
		dbFile.close();
		return false;
	}

	// Ignore 4 reserved bytes for future uses of DB parameters
	for(int i = 0; i < 4; ++i) (void)dbFile.get();

	// Read 4 bytes of total number bytes of DB entries (payload)
	uint8_t buff[4];
	for(int i = 0; i < 4; ++i) buff[i] = dbFile.get();
	uint32_t totalPayloadBytes = be32toh(*(uint32_t *)buff); // When converting text-based DB file into binary file, we used Big Endian

	// Analyze DB entries
	for(auto i = 0u; i < totalPayloadBytes; c = dbFile.get(), ++i)
	{
		if(c == 'F')
		{
			// Start a new entry
			DbEntry newEntry;

			// Read the "key" in null-terminated string format
			while((c = dbFile.get()) != '\0')
			{
				newEntry.key += c;
				++i;
			}

			// Read 1 byte of "type"
			c = dbFile.get();
			++i;
			switch (c)
			{
			case static_cast<char>(DbTypeEnum::TYPE_OF_ENTRY_U8):
				newEntry.type = DbTypeEnum::TYPE_OF_ENTRY_U8;
				break;
			case static_cast<char>(DbTypeEnum::TYPE_OF_ENTRY_S8):
				newEntry.type = DbTypeEnum::TYPE_OF_ENTRY_S8;
				break;
			case static_cast<char>(DbTypeEnum::TYPE_OF_ENTRY_U16):
				newEntry.type = DbTypeEnum::TYPE_OF_ENTRY_U16;
				break;
			case static_cast<char>(DbTypeEnum::TYPE_OF_ENTRY_S16):
				newEntry.type = DbTypeEnum::TYPE_OF_ENTRY_S16;
				break;
			case static_cast<char>(DbTypeEnum::TYPE_OF_ENTRY_U32):
				newEntry.type = DbTypeEnum::TYPE_OF_ENTRY_U32;
				break;
			case static_cast<char>(DbTypeEnum::TYPE_OF_ENTRY_S32):
				newEntry.type = DbTypeEnum::TYPE_OF_ENTRY_S32;
				break;
			case static_cast<char>(DbTypeEnum::TYPE_OF_ENTRY_U64):
				newEntry.type = DbTypeEnum::TYPE_OF_ENTRY_U64;
				break;
			case static_cast<char>(DbTypeEnum::TYPE_OF_ENTRY_S64):
				newEntry.type = DbTypeEnum::TYPE_OF_ENTRY_S64;
				break;
			case static_cast<char>(DbTypeEnum::TYPE_OF_ENTRY_CHAR):
				newEntry.type = DbTypeEnum::TYPE_OF_ENTRY_CHAR;
				break;
			default:
				// ERROR TRACE
				dbFile.close();
				return false;
			}

			// Read the "value" in null-terminated string format
			std::string valueStr;
			while((c = dbFile.get()) != '\0')
			{
				valueStr += c;
				++i;
			}

			// Tokenize the "value" string
			if(newEntry.type == DbTypeEnum::TYPE_OF_ENTRY_CHAR)
			{
				// Sanity check, TYPE_OF_ENTRY_CHAR, "value" has two double-quote
				if(valueStr.length() < 2 || valueStr.front() != '\"' || valueStr.back() != '\"')
				{
					// ERROR TRACE
					dbFile.close();
					return false;
				}

				// Remove those two double quotes
				std::string trimmedValue(valueStr.begin() + 1, valueStr.begin() + valueStr.length() - 1);

				std::vector<std::string> values = tokenize(trimmedValue, " ");
				for(const auto& v : values)
				{
					newEntry.values.emplace_back(v);
				}
				newEntry.values.emplace_back(trimmedValue);
			}
			else
			{
				std::vector<std::string> values = tokenize(valueStr, ", ");
				for(const auto& v : values)
				{
					int base = 10;
					// Value is in hex format
					if(v.find("0x") != std::string::npos)
					{
						base = 16;
					}

					try
					{
						std::size_t pos {};
						auto numeric = std::stoll(v, &pos, base);
						if(pos < v.length())
						{
							// ERROR TRACE, failed to convert
						}
						else if(!isFitIntegralType(numeric, newEntry.type))
						{
							// ERROR TRACE, out of range
						}
						else
						{
							switch (newEntry.type)
							{
							case DbTypeEnum::TYPE_OF_ENTRY_U8:
								newEntry.values.emplace_back((uint8_t)numeric);
								break;
							case DbTypeEnum::TYPE_OF_ENTRY_S8:
								newEntry.values.emplace_back((int8_t)numeric);
								break;
							case DbTypeEnum::TYPE_OF_ENTRY_U16:
								newEntry.values.emplace_back((uint16_t)numeric);
								break;
							case DbTypeEnum::TYPE_OF_ENTRY_S16:
								newEntry.values.emplace_back((int16_t)numeric);
								break;
							case DbTypeEnum::TYPE_OF_ENTRY_U32:
								newEntry.values.emplace_back((uint32_t)numeric);
								break;
							case DbTypeEnum::TYPE_OF_ENTRY_S32:
								newEntry.values.emplace_back((int32_t)numeric);
								break;
							case DbTypeEnum::TYPE_OF_ENTRY_U64:
								newEntry.values.emplace_back((uint64_t)numeric);
								break;
							case DbTypeEnum::TYPE_OF_ENTRY_S64:
								newEntry.values.emplace_back((int64_t)numeric);
								break;
							default:
								break;
							}
						}
					}
					catch(const std::exception& e)
					{
						// ERROR TRACE
						// std::cerr << e.what() << '\n';
					}
				}
			}

			std::scoped_lock<std::mutex> lockStorage(m_storageMutex);
			std::scoped_lock<std::mutex> lockDictionary(m_dictionaryMutex);
			m_dbStorage.emplace_back(newEntry);

			// Tokenize the key into sub-keys, convenient for searching later (technique: Inverted Index - Hashing Dictionary)
			std::vector<std::string> subKeys = tokenize(newEntry.key, "/");
			for(const auto& sk : subKeys)
			{
				// std::cout << "m_dbDictionary insert: " << sk << std::endl;
				m_dbDictionary[sk].insert(m_dbStorage.size() - 1); // Storing the index of entry in m_dbStorage vector
			}
		}
	}

	dbFile.close();
	return true;
}

bool DbLoader::isFitIntegralType(const int64_t& valueToCheck, const DbTypeEnum& type)
{
	bool isFit = false;
	if(type == DbTypeEnum::TYPE_OF_ENTRY_U8)
	{
		if(valueToCheck <= std::numeric_limits<uint8_t>::max() && valueToCheck >= std::numeric_limits<uint8_t>::min())
		{
			isFit = true;
		}
	}
	else if(type == DbTypeEnum::TYPE_OF_ENTRY_S8)
	{
		if(valueToCheck <= std::numeric_limits<int8_t>::max() && valueToCheck >= std::numeric_limits<int8_t>::min())
		{
			isFit = true;
		}
	}
	if(type == DbTypeEnum::TYPE_OF_ENTRY_U16)
	{
		if(valueToCheck <= std::numeric_limits<uint16_t>::max() && valueToCheck >= std::numeric_limits<uint16_t>::min())
		{
			isFit = true;
		}
	}
	else if(type == DbTypeEnum::TYPE_OF_ENTRY_S16)
	{
		if(valueToCheck <= std::numeric_limits<int16_t>::max() && valueToCheck >= std::numeric_limits<int16_t>::min())
		{
			isFit = true;
		}
	}
	if(type == DbTypeEnum::TYPE_OF_ENTRY_U32)
	{
		if(valueToCheck <= std::numeric_limits<uint32_t>::max() && valueToCheck >= std::numeric_limits<uint32_t>::min())
		{
			isFit = true;
		}
	}
	else if(type == DbTypeEnum::TYPE_OF_ENTRY_S32)
	{
		if(valueToCheck <= std::numeric_limits<int32_t>::max() && valueToCheck >= std::numeric_limits<int32_t>::min())
		{
			isFit = true;
		}
	}

	// No need to check int64_t or uint64_t, because it's meaningless

	return isFit;
}

std::vector<std::string> DbLoader::tokenize(const std::string& key, const std::string& delimiter)
{
	std::vector<std::string> tokens;
	tokens.reserve(10);

	std::size_t startIndex = 0;
	std::size_t i;
	while((i = key.find(delimiter, startIndex)) != std::string::npos)
	{
		if(i > startIndex)
		{
		tokens.emplace_back(key.substr(startIndex, i - startIndex));
		}

		startIndex = i + delimiter.length();
	}

	if(startIndex < key.length())
	{
		tokens.emplace_back(key.substr(startIndex));
	}

	return tokens;
}

std::vector<std::size_t> DbLoader::findMatchingKeys(const std::string& input)
{
	auto tokens = tokenize(input, "/");
	if(tokens.empty())
	{
		// ERROR TRACE
		std::cout << "ERROR: Input " << input << " could not be tokenized!\n";
		return {};
	}
	
	std::unordered_set<std::size_t> intersect;
	std::unordered_set<std::size_t> tmp;
	std::scoped_lock<std::mutex> lockDictionary(m_dictionaryMutex);
	for(const auto& token : tokens)
	{
		auto it = m_dbDictionary.find(token);
		if(it == m_dbDictionary.end())
		{
			// ERROR TRACE
			std::cout << "ERROR: Token " << token << " could not be found in m_dbDictionary!\n";
			return {};
		}
		
		if(intersect.empty())
		{
			intersect = it->second;
		}
		else
		{
			tmp.clear();
			std::set_intersection(intersect.begin(), intersect.end(), (it->second).begin(), (it->second).end(), std::inserter(tmp, tmp.begin()));
			intersect = tmp;
		}
	}
	
	return {intersect.begin(), intersect.end()};
}

} // namespace V1

} // namespace DatabaseIf

} // namespace DbEngine