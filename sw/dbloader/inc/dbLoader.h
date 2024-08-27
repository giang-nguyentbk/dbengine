/*
* ________________     __________              _____             
* ___  __ \__  __ )    ___  ____/_____________ ___(_)___________ 
* __  / / /_  __  |    __  __/  __  __ \_  __ `/_  /__  __ \  _ \
* _  /_/ /_  /_/ /     _  /___  _  / / /  /_/ /_  / _  / / /  __/
* /_____/ /_____/      /_____/  /_/ /_/_\__, / /_/  /_/ /_/\___/ 
*                                      /____/                    
*/

#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <any>
#include <type_traits>

#include "databaseIf.h"

namespace DbEngine
{
namespace DatabaseIf
{
namespace V1
{



class DbLoader
{
public:
	static DbLoader& getInstance();

	DbLoader(const DbLoader& other) = delete;
	DbLoader(DbLoader&& other) = delete;
	DbLoader& operator=(const DbLoader& other) = delete;
	DbLoader& operator=(DbLoader&& other) = delete;

	template<typename T>
	std::vector<T> retrieveValueByIndex(const std::string& key, IDatabase::ReturnCode& rc)
	{
		rc = IDatabase::ReturnCode::OK;

		auto indices = findMatchingKeys(key);
		if(indices.empty())
		{
			rc = IDatabase::ReturnCode::KEY_NOT_FOUND;
			return {};
		}
		else if(indices.size() > 1)
		{
			// ABN TRACE, found two matching keys, only the first found will be returned
			std::cout << "ABN: Found two matching keys, only the first found will be returned!" << std::endl;
		}

		// std::cout << "DEBUG: findMatchingKeys has value " << indices.size() << std::endl;

		std::vector<T> values;
		values.reserve(indices.size());

		DbTypeEnum requestedType { DbTypeEnum::TYPE_OF_ENTRY_UNDEFINED };
		if(std::is_same<T, uint8_t>::value) requestedType = DbTypeEnum::TYPE_OF_ENTRY_U8;
		else if(std::is_same<T, int8_t>::value) requestedType = DbTypeEnum::TYPE_OF_ENTRY_S8;
		else if(std::is_same<T, uint16_t>::value) requestedType = DbTypeEnum::TYPE_OF_ENTRY_U16;
		else if(std::is_same<T, int16_t>::value) requestedType = DbTypeEnum::TYPE_OF_ENTRY_S16;
		else if(std::is_same<T, uint32_t>::value) requestedType = DbTypeEnum::TYPE_OF_ENTRY_U32;
		else if(std::is_same<T, int32_t>::value) requestedType = DbTypeEnum::TYPE_OF_ENTRY_S32;
		else if(std::is_same<T, uint64_t>::value) requestedType = DbTypeEnum::TYPE_OF_ENTRY_U64;
		else if(std::is_same<T, int64_t>::value) requestedType = DbTypeEnum::TYPE_OF_ENTRY_S64;
		else if(std::is_same<T, std::string>::value) requestedType = DbTypeEnum::TYPE_OF_ENTRY_CHAR;

		// std::cout << "DEBUG: key = " << m_dbStorage.at(indices.front()).key << std::endl;
		if(m_dbStorage.at(indices.front()).type != requestedType)
		{
			// ERROR TRACE, requested type mismatch
			std::cout << "DEBUG: TYPE_MISMATCH!\n";
			rc = IDatabase::ReturnCode::TYPE_MISMATCH;
			return { };
		}

		for(const std::any& v : m_dbStorage.at(indices.front()).values)
		{
			if(v.has_value())
			{
				try
				{
					values.emplace_back(std::any_cast<T>(v));
				}
				catch(const std::exception& e)
				{
					// ERROR TRACE
					// std::cerr << e.what() << '\n';
					std::cout << "DEBUG: exception what: " << e.what() << "\n";
				}
			}
			else
			{
				values.emplace_back(T());
			}
		}

		return values;
	}

	

private:
	explicit DbLoader(const std::string& binFilePath);
	virtual ~DbLoader() = default;

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
		DbTypeEnum type { DbTypeEnum::TYPE_OF_ENTRY_UNDEFINED };
		std::vector<std::any> values;
	};

	using DatabaseStorage = std::vector<DbEntry>;
	using DatabaseDictionary = std::unordered_map<std::string, std::unordered_set<std::size_t>>;

	std::mutex m_storageMutex;
	DatabaseStorage m_dbStorage;
	std::mutex m_dictionaryMutex;
	DatabaseDictionary m_dbDictionary;

private:
	bool loadDb(const std::string& binFilePath);
	std::vector<std::string> tokenize(const std::string& key, const std::string& delimiter);
	std::vector<std::size_t> findMatchingKeys(const std::string& input);
	bool isFitIntegralType(const int64_t& valueToCheck, const DbTypeEnum& type);

	

}; // class DbLoader

} // namespace V1

} // namespace DatabaseIf

} // namespace DbEngine