/*
* ________________     __________              _____             
* ___  __ \__  __ )    ___  ____/_____________ ___(_)___________ 
* __  / / /_  __  |    __  __/  __  __ \_  __ `/_  /__  __ \  _ \
* _  /_/ /_  /_/ /     _  /___  _  / / /  /_/ /_  / _  / / /  __/
* /_____/ /_____/      /_____/  /_/ /_/_\__, / /_/  /_/ /_/\___/ 
*                                      /____/                    
*/

#pragma once

#include <vector>
#include <string>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <any>
#include <type_traits>

#include "databaseIf.h"

#include <enumUtils.h>
#include <stringUtils.h>
#include <traceIf.h>
#include "dbengine_tpt_provider.h"

using namespace CommonUtils::V1::StringUtils;
using namespace CommonUtils::V1::EnumUtils;

namespace DbEngine
{
namespace DatabaseIf
{
namespace V1
{

enum class DbTypeEnumRaw
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

class DbTypeEnum : public EnumType<DbTypeEnumRaw>
{
public:
	explicit DbTypeEnum(const DbTypeEnumRaw& raw) : EnumType<DbTypeEnumRaw>(raw) {}
	explicit DbTypeEnum() : EnumType<DbTypeEnumRaw>(DbTypeEnumRaw::TYPE_OF_ENTRY_UNDEFINED) {}

	std::string toString() const override
	{
		switch (getRawEnum())
		{
		case DbTypeEnumRaw::TYPE_OF_ENTRY_UNDEFINED:
			return "TYPE_OF_ENTRY_UNDEFINED";

		case DbTypeEnumRaw::TYPE_OF_ENTRY_U8:
			return "TYPE_OF_ENTRY_U8";

		case DbTypeEnumRaw::TYPE_OF_ENTRY_S8:
			return "TYPE_OF_ENTRY_S8";

		case DbTypeEnumRaw::TYPE_OF_ENTRY_U16:
			return "TYPE_OF_ENTRY_U16";

		case DbTypeEnumRaw::TYPE_OF_ENTRY_S16:
			return "TYPE_OF_ENTRY_S16";

		case DbTypeEnumRaw::TYPE_OF_ENTRY_U32:
			return "TYPE_OF_ENTRY_U32";

		case DbTypeEnumRaw::TYPE_OF_ENTRY_S32:
			return "TYPE_OF_ENTRY_S32";

		case DbTypeEnumRaw::TYPE_OF_ENTRY_U64:
			return "TYPE_OF_ENTRY_U64";

		case DbTypeEnumRaw::TYPE_OF_ENTRY_S64:
			return "TYPE_OF_ENTRY_S64";

		case DbTypeEnumRaw::TYPE_OF_ENTRY_CHAR:
			return "TYPE_OF_ENTRY_CHAR";

		default:
			return "Unknown EnumType: " + std::to_string(toS32());
		}
	}
};

class DbLoader
{
public:
	static DbLoader& getInstance();

	DbLoader(const DbLoader& other) = delete;
	DbLoader(DbLoader&& other) = delete;
	DbLoader& operator=(const DbLoader& other) = delete;
	DbLoader& operator=(DbLoader&& other) = delete;

	template<typename T>
	std::vector<T> retrieveValueByIndex(const std::string& key, ReturnCodeEnum& rc)
	{
		rc.set(ReturnCodeRaw::OK);

		auto indices = findMatchingKeys(key);
		if(indices.empty())
		{
			TPT_TRACE(TRACE_ABN, SSTR("DB key ", key, " could not be found!"));
			rc.set(ReturnCodeRaw::KEY_NOT_FOUND);
			return {};
		}
		else if(indices.size() > 1)
		{
			TPT_TRACE(TRACE_ABN, SSTR("Found ", indices.size(), " matching keys (\"", key,"\"): only the first key will be returned which might not be expected!"));
		}

		std::vector<T> values;
		values.reserve(indices.size());

		DbTypeEnum requestedType;
		if(std::is_same<T, uint8_t>::value) requestedType.set(DbTypeEnumRaw::TYPE_OF_ENTRY_U8);
		else if(std::is_same<T, int8_t>::value) requestedType.set(DbTypeEnumRaw::TYPE_OF_ENTRY_S8);
		else if(std::is_same<T, uint16_t>::value) requestedType.set(DbTypeEnumRaw::TYPE_OF_ENTRY_U16);
		else if(std::is_same<T, int16_t>::value) requestedType.set(DbTypeEnumRaw::TYPE_OF_ENTRY_S16);
		else if(std::is_same<T, uint32_t>::value) requestedType.set(DbTypeEnumRaw::TYPE_OF_ENTRY_U32);
		else if(std::is_same<T, int32_t>::value) requestedType.set(DbTypeEnumRaw::TYPE_OF_ENTRY_S32);
		else if(std::is_same<T, uint64_t>::value) requestedType.set(DbTypeEnumRaw::TYPE_OF_ENTRY_U64);
		else if(std::is_same<T, int64_t>::value) requestedType.set(DbTypeEnumRaw::TYPE_OF_ENTRY_S64);
		else if(std::is_same<T, std::string>::value) requestedType.set(DbTypeEnumRaw::TYPE_OF_ENTRY_CHAR);

		if(m_dbStorage.at(indices.front()).type != requestedType)
		{
			TPT_TRACE(TRACE_ABN, SSTR("Requested type ", requestedType.toString(), " did not match with DB entry type ", m_dbStorage.at(indices.front()).type.toString()));
			rc.set(ReturnCodeRaw::TYPE_MISMATCH);
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
					TPT_TRACE(TRACE_ABN, SSTR("Could not any_cast DB entry for key ", key, ", type ", requestedType.toString()));
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

	struct DbEntry
	{
		std::string key;
		DbTypeEnum type;
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