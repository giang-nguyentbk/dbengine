#include <fstream>
#include <algorithm>

#include "dbLoader.h"

namespace DbEngine
{
namespace DatabaseIf
{
namespace V1
{

DbLoader& DbLoader::getInstance()
{
	static DbLoader instance;
	return instance;
}

DbLoader::DbLoader()
{
	// Load Original Database
	if(!loadDb(m_binDbPath + "/swdb.bin"))
	{
		TPT_TRACE(TRACE_ERROR, SSTR("Failed to load DB binary file ", m_binDbPath, "/swdb.bin"));
	}

	// Load Hard-Saved Database into Modified Data structures
	if(!loadHardSavedDb(m_binDbPath + "/swdb-hardsave.bin"))
	{
		TPT_TRACE(TRACE_ABN, SSTR("Failed to load DB binary file ", m_binDbPath, "/swdb-hardsave.bin"));
	}
}

bool DbLoader::loadDb(const std::string& binFilePath)
{
	std::ifstream dbFile(binFilePath, std::ifstream::binary);
	if(!dbFile.is_open())
	{
		TPT_TRACE(TRACE_ABN, SSTR("Could not open DB binary file ", binFilePath));
		return false;
	}

	char c;

	// DB Header Tag check
	c = dbFile.get();
	if(c != 'H')
	{
		TPT_TRACE(TRACE_ERROR, SSTR("The DB Header Tag 'H' was not correct ", std::string(1, c)));
		dbFile.close();
		return false;
	}

	// DB Revision check, currently hardcoded = 10
	c = dbFile.get();
	if(c != 10)
	{
		TPT_TRACE(TRACE_ERROR, SSTR("The DB Revision '10' was not correct ", (int)c));
		dbFile.close();
		return false;
	}

	// Ignore 4 reserved bytes for future uses of DB parameters
	for(int i = 0; i < 4; ++i) (void)dbFile.get();

	// Read 4 bytes of total number bytes of DB entries (payload)
	uint8_t buff[4];
	for(int i = 0; i < 4; ++i) buff[i] = dbFile.get();
	uint32_t totalPayloadBytes = be32toh(*(uint32_t *)buff); // When converting text-based DB file into binary file, we used Big Endian
	TPT_TRACE(TRACE_INFO, SSTR("Total DB entry's payload size: ", totalPayloadBytes, " bytes!"));

	std::vector<uint8_t> payload;
	payload.reserve(totalPayloadBytes);
	// Analyze DB entries
	for(auto i = 0u; i < totalPayloadBytes && c != EOF; ++i)
	{
		c = dbFile.get();
		payload.push_back(c);
		if(c == 'F')
		{
			// Start a new entry
			DbEntry newEntry;

			// Read the "key" in null-terminated string format
			while((c = dbFile.get()) != '\0')
			{
				payload.push_back(c);
				newEntry.key += c;
				++i;
			}
			++i;
			payload.push_back(c);

			// Read 1 byte of "permission"
			c = dbFile.get();
			payload.push_back(c);
			++i;
			switch (c)
			{
			case static_cast<char>(toUnderlyingType(DbPermissionEnumRaw::PERM_READ_ONLY)):
				newEntry.permission.set(DbPermissionEnumRaw::PERM_READ_ONLY);
				break;

			case static_cast<char>(toUnderlyingType(DbPermissionEnumRaw::PERM_READ_WRITE)):
				newEntry.permission.set(DbPermissionEnumRaw::PERM_READ_WRITE);
				break;
			
			default:
				TPT_TRACE(TRACE_ERROR, SSTR("EnumPermission of this DB Entry was not recognized, ", (int)c));
				dbFile.close();
				return false;
			}

			// Read 1 byte of "type"
			c = dbFile.get();
			payload.push_back(c);
			++i;
			switch (c)
			{
			case static_cast<char>(toUnderlyingType(DbTypeEnumRaw::TYPE_OF_ENTRY_U8)):
				newEntry.type.set(DbTypeEnumRaw::TYPE_OF_ENTRY_U8);
				break;
			case static_cast<char>(toUnderlyingType(DbTypeEnumRaw::TYPE_OF_ENTRY_S8)):
				newEntry.type.set(DbTypeEnumRaw::TYPE_OF_ENTRY_S8);
				break;
			case static_cast<char>(toUnderlyingType(DbTypeEnumRaw::TYPE_OF_ENTRY_U16)):
				newEntry.type.set(DbTypeEnumRaw::TYPE_OF_ENTRY_U16);
				break;
			case static_cast<char>(toUnderlyingType(DbTypeEnumRaw::TYPE_OF_ENTRY_S16)):
				newEntry.type.set(DbTypeEnumRaw::TYPE_OF_ENTRY_S16);
				break;
			case static_cast<char>(toUnderlyingType(DbTypeEnumRaw::TYPE_OF_ENTRY_U32)):
				newEntry.type.set(DbTypeEnumRaw::TYPE_OF_ENTRY_U32);
				break;
			case static_cast<char>(toUnderlyingType(DbTypeEnumRaw::TYPE_OF_ENTRY_S32)):
				newEntry.type.set(DbTypeEnumRaw::TYPE_OF_ENTRY_S32);
				break;
			case static_cast<char>(toUnderlyingType(DbTypeEnumRaw::TYPE_OF_ENTRY_U64)):
				newEntry.type.set(DbTypeEnumRaw::TYPE_OF_ENTRY_U64);
				break;
			case static_cast<char>(toUnderlyingType(DbTypeEnumRaw::TYPE_OF_ENTRY_S64)):
				newEntry.type.set(DbTypeEnumRaw::TYPE_OF_ENTRY_S64);
				break;
			case static_cast<char>(toUnderlyingType(DbTypeEnumRaw::TYPE_OF_ENTRY_CHAR)):
				newEntry.type.set(DbTypeEnumRaw::TYPE_OF_ENTRY_CHAR);
				break;
			default:
				TPT_TRACE(TRACE_ERROR, SSTR("EnumType of this DB Entry was not recognized, ", (int)c));
				dbFile.close();
				return false;
			}

			// Read the "value" in null-terminated string format
			std::string valueStr;
			while((c = dbFile.get()) != '\0')
			{
				payload.push_back(c);
				valueStr += c;
				++i;
			}
			++i;
			payload.push_back(c);

			// Tokenize the "value" string
			if(newEntry.type.getRawEnum() == DbTypeEnumRaw::TYPE_OF_ENTRY_CHAR)
			{
				// Sanity check, "value" will have two double-quote
				if(valueStr.length() < 2 || valueStr.front() != '\"' || valueStr.back() != '\"')
				{
					TPT_TRACE(TRACE_ERROR, SSTR("Value field of this DB Entry too short or not in correct format: ", valueStr));
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
				std::vector<std::string> values = tokenize(valueStr, ",");
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
							TPT_TRACE(TRACE_ERROR, SSTR("Failed to convert DB value into numeric: ", v));
						}
						else if(!isFitIntegralType(numeric, newEntry.type))
						{
							TPT_TRACE(TRACE_ERROR, SSTR("DB Value is out of range: ", v, ", compared to type ", newEntry.type.toString()));
						}
						else
						{
							switch (newEntry.type.getRawEnum())
							{
							case DbTypeEnumRaw::TYPE_OF_ENTRY_U8:
								newEntry.values.emplace_back((uint8_t)numeric);
								break;
							case DbTypeEnumRaw::TYPE_OF_ENTRY_S8:
								newEntry.values.emplace_back((int8_t)numeric);
								break;
							case DbTypeEnumRaw::TYPE_OF_ENTRY_U16:
								newEntry.values.emplace_back((uint16_t)numeric);
								break;
							case DbTypeEnumRaw::TYPE_OF_ENTRY_S16:
								newEntry.values.emplace_back((int16_t)numeric);
								break;
							case DbTypeEnumRaw::TYPE_OF_ENTRY_U32:
								newEntry.values.emplace_back((uint32_t)numeric);
								break;
							case DbTypeEnumRaw::TYPE_OF_ENTRY_S32:
								newEntry.values.emplace_back((int32_t)numeric);
								break;
							case DbTypeEnumRaw::TYPE_OF_ENTRY_U64:
								newEntry.values.emplace_back((uint64_t)numeric);
								break;
							case DbTypeEnumRaw::TYPE_OF_ENTRY_S64:
								newEntry.values.emplace_back((int64_t)numeric);
								break;
							default:
								break;
							}
						}
					}
					catch(const std::exception& e)
					{
						TPT_TRACE(TRACE_ERROR, SSTR("Raised an exception: ", e.what()));
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
				m_dbDictionary[sk].insert(m_dbStorage.size() - 1); // Storing the index of entry in m_dbStorage vector
			}
		}
	}

	// Check DB End tag
	c = dbFile.get();
	if(c != 'E')
	{
		TPT_TRACE(TRACE_ERROR, SSTR("The DB End Tag 'E' was not correct, char = ", (int)c));
		dbFile.close();
		return false;
	}

	// CRC16 checksum
	for(int i = 0; i < 2; ++i) buff[i] = dbFile.get();
	uint16_t crc16 = be16toh(*(uint16_t *)buff);
	uint16_t calculatedCrc16 = getCRC16((uint8_t *)payload.data(), payload.size());
	if(crc16 != calculatedCrc16)
	{
		TPT_TRACE(TRACE_ERROR, SSTR("The DB CRC16 checksum was not correct, origin crc16 = ", crc16, ", calculated crc16 = ", calculatedCrc16));
		dbFile.close();
		return false;
	}

	dbFile.close();
	return true;
}

bool DbLoader::loadHardSavedDb(const std::string& binFilePath)
{
	std::ifstream dbFile(binFilePath, std::ifstream::binary);
	if(!dbFile.is_open())
	{
		TPT_TRACE(TRACE_ABN, SSTR("Could not open DB binary file ", binFilePath));
		return false;
	}

	// Read 4 bytes of total number of entries
	uint8_t buff[4];
	for(int i = 0; i < 4; ++i) buff[i] = dbFile.get();
	uint32_t totalEntries = be32toh(*(uint32_t *)buff); // When converting text-based DB file into binary file, we used Big Endian
	TPT_TRACE(TRACE_INFO, SSTR("Total number of entries in Hard Saved DB: ", totalEntries, " entries!"));

	char c;
	// Analyze DB entries
	for(auto i = 0u; i < totalEntries; ++i)
	{
		c = dbFile.get();
		if(c == 'F')
		{
			// Start a new entry
			DbEntry newEntry;

			// Read the "key" in null-terminated string format
			while((c = dbFile.get()) != '\0')
			{
				newEntry.key += c;
			}

			// Read 1 byte of "permission"
			c = dbFile.get();
			switch (c)
			{
			case static_cast<char>(toUnderlyingType(DbPermissionEnumRaw::PERM_READ_ONLY)):
				newEntry.permission.set(DbPermissionEnumRaw::PERM_READ_ONLY);
				break;

			case static_cast<char>(toUnderlyingType(DbPermissionEnumRaw::PERM_READ_WRITE)):
				newEntry.permission.set(DbPermissionEnumRaw::PERM_READ_WRITE);
				break;
			
			default:
				TPT_TRACE(TRACE_ERROR, SSTR("EnumPermission of this DB Entry was not recognized, ", (int)c));
				dbFile.close();
				return false;
			}

			// Read 1 byte of "type"
			c = dbFile.get();
			switch (c)
			{
			case static_cast<char>(toUnderlyingType(DbTypeEnumRaw::TYPE_OF_ENTRY_U8)):
				newEntry.type.set(DbTypeEnumRaw::TYPE_OF_ENTRY_U8);
				break;
			case static_cast<char>(toUnderlyingType(DbTypeEnumRaw::TYPE_OF_ENTRY_S8)):
				newEntry.type.set(DbTypeEnumRaw::TYPE_OF_ENTRY_S8);
				break;
			case static_cast<char>(toUnderlyingType(DbTypeEnumRaw::TYPE_OF_ENTRY_U16)):
				newEntry.type.set(DbTypeEnumRaw::TYPE_OF_ENTRY_U16);
				break;
			case static_cast<char>(toUnderlyingType(DbTypeEnumRaw::TYPE_OF_ENTRY_S16)):
				newEntry.type.set(DbTypeEnumRaw::TYPE_OF_ENTRY_S16);
				break;
			case static_cast<char>(toUnderlyingType(DbTypeEnumRaw::TYPE_OF_ENTRY_U32)):
				newEntry.type.set(DbTypeEnumRaw::TYPE_OF_ENTRY_U32);
				break;
			case static_cast<char>(toUnderlyingType(DbTypeEnumRaw::TYPE_OF_ENTRY_S32)):
				newEntry.type.set(DbTypeEnumRaw::TYPE_OF_ENTRY_S32);
				break;
			case static_cast<char>(toUnderlyingType(DbTypeEnumRaw::TYPE_OF_ENTRY_U64)):
				newEntry.type.set(DbTypeEnumRaw::TYPE_OF_ENTRY_U64);
				break;
			case static_cast<char>(toUnderlyingType(DbTypeEnumRaw::TYPE_OF_ENTRY_S64)):
				newEntry.type.set(DbTypeEnumRaw::TYPE_OF_ENTRY_S64);
				break;
			case static_cast<char>(toUnderlyingType(DbTypeEnumRaw::TYPE_OF_ENTRY_CHAR)):
				newEntry.type.set(DbTypeEnumRaw::TYPE_OF_ENTRY_CHAR);
				break;
			default:
				TPT_TRACE(TRACE_ERROR, SSTR("EnumType of this DB Entry was not recognized, ", (int)c));
				dbFile.close();
				return false;
			}

			// Read the "value" in null-terminated string format
			std::string valueStr;
			while((c = dbFile.get()) != '\0')
			{
				valueStr += c;
			}

			// Tokenize the "value" string
			if(newEntry.type.getRawEnum() == DbTypeEnumRaw::TYPE_OF_ENTRY_CHAR)
			{
				// Sanity check, "value" will have two double-quote
				if(valueStr.length() < 2 || valueStr.front() != '\"' || valueStr.back() != '\"')
				{
					TPT_TRACE(TRACE_ERROR, SSTR("Value field of this DB Entry too short or not in correct format: ", valueStr));
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
				std::vector<std::string> values = tokenize(valueStr, ",");
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
							TPT_TRACE(TRACE_ERROR, SSTR("Failed to convert DB value into numeric: ", v));
						}
						else if(!isFitIntegralType(numeric, newEntry.type))
						{
							TPT_TRACE(TRACE_ERROR, SSTR("DB Value is out of range: ", v, ", compared to type ", newEntry.type.toString()));
						}
						else
						{
							switch (newEntry.type.getRawEnum())
							{
							case DbTypeEnumRaw::TYPE_OF_ENTRY_U8:
								newEntry.values.emplace_back((uint8_t)numeric);
								break;
							case DbTypeEnumRaw::TYPE_OF_ENTRY_S8:
								newEntry.values.emplace_back((int8_t)numeric);
								break;
							case DbTypeEnumRaw::TYPE_OF_ENTRY_U16:
								newEntry.values.emplace_back((uint16_t)numeric);
								break;
							case DbTypeEnumRaw::TYPE_OF_ENTRY_S16:
								newEntry.values.emplace_back((int16_t)numeric);
								break;
							case DbTypeEnumRaw::TYPE_OF_ENTRY_U32:
								newEntry.values.emplace_back((uint32_t)numeric);
								break;
							case DbTypeEnumRaw::TYPE_OF_ENTRY_S32:
								newEntry.values.emplace_back((int32_t)numeric);
								break;
							case DbTypeEnumRaw::TYPE_OF_ENTRY_U64:
								newEntry.values.emplace_back((uint64_t)numeric);
								break;
							case DbTypeEnumRaw::TYPE_OF_ENTRY_S64:
								newEntry.values.emplace_back((int64_t)numeric);
								break;
							default:
								break;
							}
						}
					}
					catch(const std::exception& e)
					{
						TPT_TRACE(TRACE_ERROR, SSTR("Raised an exception: Failed to convert value: \'", v, "\', e.what(): ", e.what()));
					}
				}
			}

			std::scoped_lock<std::mutex> lockStorage(m_modStorageMutex);
			std::scoped_lock<std::mutex> lockDictionary(m_modDictionaryMutex);
			m_modDbStorage.emplace_back(newEntry);

			// Tokenize the key into sub-keys, convenient for searching later (technique: Inverted Index - Hashing Dictionary)
			std::vector<std::string> subKeys = tokenize(newEntry.key, "/");
			for(const auto& sk : subKeys)
			{
				m_modDbDictionary[sk].insert(m_modDbStorage.size() - 1); // Storing the index of entry in m_dbStorage vector
			}
		}
	}

	dbFile.close();
	return true;
}

bool DbLoader::isFitIntegralType(const int64_t& valueToCheck, const DbTypeEnum& type)
{
	bool isFit = false;
	if(type.getRawEnum() == DbTypeEnumRaw::TYPE_OF_ENTRY_U8)
	{
		if(valueToCheck <= std::numeric_limits<uint8_t>::max() && valueToCheck >= std::numeric_limits<uint8_t>::min())
		{
			isFit = true;
		}
	}
	else if(type.getRawEnum() == DbTypeEnumRaw::TYPE_OF_ENTRY_S8)
	{
		if(valueToCheck <= std::numeric_limits<int8_t>::max() && valueToCheck >= std::numeric_limits<int8_t>::min())
		{
			isFit = true;
		}
	}
	if(type.getRawEnum() == DbTypeEnumRaw::TYPE_OF_ENTRY_U16)
	{
		if(valueToCheck <= std::numeric_limits<uint16_t>::max() && valueToCheck >= std::numeric_limits<uint16_t>::min())
		{
			isFit = true;
		}
	}
	else if(type.getRawEnum() == DbTypeEnumRaw::TYPE_OF_ENTRY_S16)
	{
		if(valueToCheck <= std::numeric_limits<int16_t>::max() && valueToCheck >= std::numeric_limits<int16_t>::min())
		{
			isFit = true;
		}
	}
	if(type.getRawEnum() == DbTypeEnumRaw::TYPE_OF_ENTRY_U32)
	{
		if(valueToCheck <= std::numeric_limits<uint32_t>::max() && valueToCheck >= std::numeric_limits<uint32_t>::min())
		{
			isFit = true;
		}
	}
	else if(type.getRawEnum() == DbTypeEnumRaw::TYPE_OF_ENTRY_S32)
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
			std::string newToken = key.substr(startIndex, i - startIndex);
			newToken.erase(std::remove_if(newToken.begin(), newToken.end(), [](auto& c){
				return c == ' ' || c == '\t';
			}), newToken.end());

			if(newToken.length()) tokens.emplace_back(newToken);
		}

		startIndex = i + delimiter.length();
	}

	if(startIndex < key.length())
	{
		std::string newToken = key.substr(startIndex, i - startIndex);
		newToken.erase(std::remove_if(newToken.begin(), newToken.end(), [](auto& c){
			return c == ' ' || c == '\t';
		}), newToken.end());

		if(newToken.length()) tokens.emplace_back(newToken);
	}

	return tokens;
}

std::vector<std::size_t> DbLoader::findMatchingKeys(const std::string& input, const DatabaseDictionary& dbDictionary, std::mutex& mtx)
{
	if(dbDictionary.empty())
	{
		// TPT_TRACE(TRACE_ABN, SSTR("The DB Dictionary is empty!"));
		return {};
	}

	auto tokens = tokenize(input, "/");
	if(tokens.empty())
	{
		TPT_TRACE(TRACE_ABN, SSTR("Input key ", input, " cannot be tokenized!"));
		return {};
	}

	std::unordered_set<std::size_t> intersect;
	std::unordered_set<std::size_t> tmp;
	std::scoped_lock<std::mutex> lockDictionary(mtx);
	for(const auto& token : tokens)
	{
		auto it = dbDictionary.find(token);
		if(it == dbDictionary.end())
		{
			// TPT_TRACE(TRACE_ABN, SSTR("Token ", token, " could not be found in dbDictionary!"));
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

std::optional<std::pair<std::size_t, bool>> DbLoader::findMatchingIndices(const std::string& input)
{
	// First try seeking on Modified Database
	auto indices = findMatchingKeys(input, m_modDbDictionary, m_modDictionaryMutex);
	if(indices.empty())
	{
		TPT_TRACE(TRACE_INFO, SSTR("DB key ", input, " could not be found in Modified DB, try on Original DB!"));
		indices = findMatchingKeys(input, m_dbDictionary, m_dictionaryMutex);
		if(indices.empty())
		{
			TPT_TRACE(TRACE_ABN, SSTR("DB key ", input, " could not be found even in Original DB!"));
			return std::nullopt;
		}
		else if(indices.size() > 1)
		{
			TPT_TRACE(TRACE_ABN, SSTR("Found ", indices.size(), " matching keys (\"", input,"\"): only the first key will be returned which might not be expected!"));
		}

		return std::make_pair(indices.front(), false);
	}
	else if(indices.size() > 1)
	{
		TPT_TRACE(TRACE_ABN, SSTR("Found ", indices.size(), " matching keys (\"", input,"\"): only the first key will be returned which might not be expected!"));
	}

	// Only the first found entry will be returned
	return std::make_pair(indices.front(), true);
}

void DbLoader::updateHardSavedDb(const std::size_t& index, const bool& isFoundInModDb)
{
	static bool firstTime = false;
	if(!firstTime)
	{
		firstTime = true;
		initHardSavedDbFile();
	}

	if(!isFoundInModDb)
	{
		TPT_TRACE(TRACE_ABN, SSTR("The updated DB entry should be already available in Modified DB!"));
	}

	std::mutex& mtx = isFoundInModDb ? m_modStorageMutex : m_storageMutex;
	DatabaseStorage& dbStorage = isFoundInModDb ? m_modDbStorage : m_dbStorage;

	std::scoped_lock<std::mutex> lockStorage(mtx);
	const auto& updatedEntry = dbStorage.at(index);

	// TODO: in case cannot open input hard save file (need to initially construct it for the first time)
	// TODO: implement permission when update()
	// TODO: implement restore() and erase()
	std::ifstream inputFile(m_binDbPath + "/swdb-hardsave.bin", std::ifstream::binary);
	if(!inputFile.is_open())
	{
		TPT_TRACE(TRACE_ABN, SSTR("Could not open DB binary file ", m_binDbPath, "/swdb-hardsave.bin"));
		return;
	}

	std::ofstream outputFile(m_binDbPath + "/swdb-hardsave.tmp.bin", std::ios_base::binary);
	if(!outputFile.is_open())
	{
		TPT_TRACE(TRACE_ABN, SSTR("Could not open DB binary file ", m_binDbPath, "/swdb-hardsave.tmp.bin"));
		inputFile.close();
		return;
	}

	std::vector<char> newContent;
	newContent.reserve(2048); // Currently hardcoded

	// Read 4 bytes of total number bytes of DB entries (payload)
	uint8_t buff[4];
	for(int i = 0; i < 4; ++i)
	{
		buff[i] = inputFile.get();
		newContent.emplace_back((char)buff[i]);
	}
	uint32_t totalEntries = be32toh(*(uint32_t *)buff);

	// Analyze DB entries
	char c;
	bool found = false;
	for(auto i = 0u; i < totalEntries; ++i)
	{
		c = inputFile.get();
		newContent.emplace_back(c);
		if(c == 'F')
		{
			std::string key;
			// Read the "key" in null-terminated string format
			while((c = inputFile.get()) != '\0')
			{
				newContent.emplace_back(c);
				key += c;
			}
			newContent.emplace_back(c);

			// Ignore the next 2 bytes for permission and type
			for(int j = 0; j < 2; ++j)
			{
				newContent.emplace_back(inputFile.get());
			}

			if(key == updatedEntry.key)
			{
				// Found the entry which should be updated
				// By pass "value" in inputFile, write our own one
				while((c = inputFile.get()) != '\0')
				{
				}

				if(updatedEntry.type.getRawEnum() == DbTypeEnumRaw::TYPE_OF_ENTRY_CHAR)
				{
					for(const auto& ch : std::any_cast<std::string>(updatedEntry.values.back()))
					{
						newContent.emplace_back(ch);
					}
				}
				else
				{
					for(const auto& v : updatedEntry.values)
					{
						std::string str;
						switch (updatedEntry.type.getRawEnum())
						{
						case DbTypeEnumRaw::TYPE_OF_ENTRY_U8:
							str = std::to_string((int)std::any_cast<uint8_t>(v));
							break;
						case DbTypeEnumRaw::TYPE_OF_ENTRY_S8:
							str = std::to_string((int)std::any_cast<int8_t>(v));
							break;
						case DbTypeEnumRaw::TYPE_OF_ENTRY_U16:
							str = std::to_string((int)std::any_cast<uint16_t>(v));
							break;
						case DbTypeEnumRaw::TYPE_OF_ENTRY_S16:
							str = std::to_string((int)std::any_cast<int16_t>(v));
							break;
						case DbTypeEnumRaw::TYPE_OF_ENTRY_U32:
							str = std::to_string((int)std::any_cast<uint32_t>(v));
							break;
						case DbTypeEnumRaw::TYPE_OF_ENTRY_S32:
							str = std::to_string((int)std::any_cast<int32_t>(v));
							break;
						case DbTypeEnumRaw::TYPE_OF_ENTRY_U64:
							str = std::to_string((int)std::any_cast<uint64_t>(v));
							break;
						case DbTypeEnumRaw::TYPE_OF_ENTRY_S64:
							str = std::to_string((int)std::any_cast<int64_t>(v));
							break;
						default:
							break;
						}

						for(const auto& ch : str)
						{
							newContent.emplace_back(ch);
						}
						newContent.emplace_back(',');
						newContent.emplace_back(' ');
					}

					newContent.pop_back(); // Remove redundant of the last ", "
					newContent.pop_back();
				}
				found = true;
				break;
			}
			else
			{
				// Copy "value" in inputFile into outputFile
				while((c = inputFile.get()) != '\0')
				{
					newContent.emplace_back(c);
				}
				
			}
			newContent.emplace_back('\0');
		}
	}

	if(!found)
	{
		// Need to add new entry into hardsaved DB file
		newContent.emplace_back('F');
		for(const auto& c : updatedEntry.key) newContent.emplace_back(c);
		newContent.emplace_back('\0');

		if(updatedEntry.permission == DbPermissionEnumRaw::PERM_READ_ONLY) newContent.emplace_back(static_cast<char>(toUnderlyingType(DbPermissionEnumRaw::PERM_READ_ONLY)));
		else if(updatedEntry.permission == DbPermissionEnumRaw::PERM_READ_WRITE) newContent.emplace_back(static_cast<char>(toUnderlyingType(DbPermissionEnumRaw::PERM_READ_WRITE)));
		else newContent.emplace_back(static_cast<char>(toUnderlyingType(DbPermissionEnumRaw::PERM_UNDEFINED)));

		if(updatedEntry.type.getRawEnum() == DbTypeEnumRaw::TYPE_OF_ENTRY_U8) newContent.emplace_back(static_cast<char>(DbTypeEnumRaw::TYPE_OF_ENTRY_U8));
		else if(updatedEntry.type == DbTypeEnumRaw::TYPE_OF_ENTRY_S8) newContent.emplace_back(static_cast<char>(DbTypeEnumRaw::TYPE_OF_ENTRY_S8));
		else if(updatedEntry.type == DbTypeEnumRaw::TYPE_OF_ENTRY_U16) newContent.emplace_back(static_cast<char>(DbTypeEnumRaw::TYPE_OF_ENTRY_U16));
		else if(updatedEntry.type == DbTypeEnumRaw::TYPE_OF_ENTRY_S16) newContent.emplace_back(static_cast<char>(DbTypeEnumRaw::TYPE_OF_ENTRY_S16));
		else if(updatedEntry.type == DbTypeEnumRaw::TYPE_OF_ENTRY_U32) newContent.emplace_back(static_cast<char>(DbTypeEnumRaw::TYPE_OF_ENTRY_U32));
		else if(updatedEntry.type == DbTypeEnumRaw::TYPE_OF_ENTRY_S32) newContent.emplace_back(static_cast<char>(DbTypeEnumRaw::TYPE_OF_ENTRY_S32));
		else if(updatedEntry.type == DbTypeEnumRaw::TYPE_OF_ENTRY_U64) newContent.emplace_back(static_cast<char>(DbTypeEnumRaw::TYPE_OF_ENTRY_U64));
		else if(updatedEntry.type == DbTypeEnumRaw::TYPE_OF_ENTRY_S64) newContent.emplace_back(static_cast<char>(DbTypeEnumRaw::TYPE_OF_ENTRY_S64));
		else if(updatedEntry.type == DbTypeEnumRaw::TYPE_OF_ENTRY_CHAR) newContent.emplace_back(static_cast<char>(DbTypeEnumRaw::TYPE_OF_ENTRY_CHAR));
		else newContent.emplace_back(static_cast<char>(DbTypeEnumRaw::TYPE_OF_ENTRY_UNDEFINED));

		if(updatedEntry.type.getRawEnum() == DbTypeEnumRaw::TYPE_OF_ENTRY_CHAR)
		{
			for(const auto& ch : std::any_cast<std::string>(updatedEntry.values.back()))
			{
				newContent.emplace_back(ch);
			}
		}
		else
		{
			for(const auto& v : updatedEntry.values)
			{
				std::string str;
				switch (updatedEntry.type.getRawEnum())
				{
				case DbTypeEnumRaw::TYPE_OF_ENTRY_U8:
					str = std::to_string((int)std::any_cast<uint8_t>(v));
					break;
				case DbTypeEnumRaw::TYPE_OF_ENTRY_S8:
					str = std::to_string((int)std::any_cast<int8_t>(v));
					break;
				case DbTypeEnumRaw::TYPE_OF_ENTRY_U16:
					str = std::to_string((int)std::any_cast<uint16_t>(v));
					break;
				case DbTypeEnumRaw::TYPE_OF_ENTRY_S16:
					str = std::to_string((int)std::any_cast<int16_t>(v));
					break;
				case DbTypeEnumRaw::TYPE_OF_ENTRY_U32:
					str = std::to_string((int)std::any_cast<uint32_t>(v));
					break;
				case DbTypeEnumRaw::TYPE_OF_ENTRY_S32:
					str = std::to_string((int)std::any_cast<int32_t>(v));
					break;
				case DbTypeEnumRaw::TYPE_OF_ENTRY_U64:
					str = std::to_string((int)std::any_cast<uint64_t>(v));
					break;
				case DbTypeEnumRaw::TYPE_OF_ENTRY_S64:
					str = std::to_string((int)std::any_cast<int64_t>(v));
					break;
				default:
					break;
				}

				for(const auto& ch : str)
				{
					newContent.emplace_back(ch);
				}
				newContent.emplace_back(',');
				newContent.emplace_back(' ');
			}
		}
		newContent.emplace_back('\0');

		// Increase total number of entries by one
		totalEntries = htobe32(++totalEntries);
		for(int j = 0; j < 4; ++j)
		{
			newContent.at(j) = *((char *)(&totalEntries) + j);
		}
	}

	while((c = inputFile.get()) != EOF)
	{
		newContent.emplace_back(c);
	}

	outputFile.write(newContent.data(), newContent.size());

	inputFile.close();
	outputFile.close();

	std::remove(std::string(m_binDbPath + "/swdb-hardsave.bin").c_str());
	std::rename(std::string(m_binDbPath + "/swdb-hardsave.tmp.bin").c_str(), std::string(m_binDbPath + "/swdb-hardsave.bin").c_str());
}

void DbLoader::initHardSavedDbFile()
{
	std::ofstream binFile(m_binDbPath + "/swdb-hardsave.bin", std::ios_base::binary);
	if(!binFile.is_open())
	{
		TPT_TRACE(TRACE_ABN, SSTR("Could not init DB binary file ", m_binDbPath, "/swdb-hardsave.bin"));
		return;
	}

	// Hard Saved DB will be in form of 4 bytes that indicates total number of entries and then all entries (each start with byte 'F')
	uint32_t totalEntries = 0;
	for(int i = 0; i < 4; ++i) binFile.put((char)totalEntries);

	binFile.close();
}

uint32_t DbLoader::lookupCRC16Table(uint32_t initCRC, uint8_t data)
{
	// std::cout << "(initCRC ^ data) & 0xFF = " << uint32_t((initCRC ^ data) & 0xFF) << std::endl;
	initCRC = (initCRC >> 8) ^ m_crc16Table[(initCRC ^ data) & 0xFF];
	return initCRC;
}

uint16_t DbLoader::getCRC16(uint8_t *startAddr, uint32_t numberBytes)
{
	uint32_t tmp = 0xFFFF;
	uint8_t *currentAddr;

	for(currentAddr = startAddr; currentAddr < startAddr + numberBytes; ++currentAddr)
	{
		tmp = lookupCRC16Table(tmp, *currentAddr);
	}

	return (tmp ^ 0xFFFF) & 0xFFFF;
}

} // namespace V1

} // namespace DatabaseIf

} // namespace DbEngine