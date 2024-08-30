/*
* ________________     __________              _____             
* ___  __ \__  __ )    ___  ____/_____________ ___(_)___________ 
* __  / / /_  __  |    __  __/  __  __ \_  __ `/_  /__  __ \  _ \
* _  /_/ /_  /_/ /     _  /___  _  / / /  /_/ /_  / _  / / /  __/
* /_____/ /_____/      /_____/  /_/ /_/_\__, / /_/  /_/ /_/\___/ 
*                                      /____/                    
*/

#pragma once

#include <cstdio>
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

enum class DbPermissionEnumRaw
{
	PERM_UNDEFINED = 0,
	PERM_READ_ONLY = 1,
	PERM_READ_WRITE = 2
};

class DbPermissionEnum : public EnumType<DbPermissionEnumRaw>
{
public:
	explicit DbPermissionEnum(const DbPermissionEnumRaw& raw) : EnumType<DbPermissionEnumRaw>(raw) {}
	explicit DbPermissionEnum() : EnumType<DbPermissionEnumRaw>(DbPermissionEnumRaw::PERM_UNDEFINED) {}

	std::string toString() const override
	{
		switch (getRawEnum())
		{
		case DbPermissionEnumRaw::PERM_UNDEFINED:
			return "PERM_UNDEFINED";

		case DbPermissionEnumRaw::PERM_READ_ONLY:
			return "PERM_READ_ONLY";

		case DbPermissionEnumRaw::PERM_READ_WRITE:
			return "PERM_READ_WRITE";

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
	std::vector<T> retrieve(const std::string& key, ReturnCodeEnum& rc)
	{
		rc.set(ReturnCodeRaw::OK);

		const auto& it = findMatchingIndices(key);
		if(!it.has_value())
		{
			rc.set(ReturnCodeRaw::KEY_NOT_FOUND);
			return {};
		}

		const auto& [index, isFoundInModDb] = it.value();

		DbTypeEnum requestedType;
		if(checkIfCorrectType<T>(index, isFoundInModDb, requestedType) && !checkIfErased(index, isFoundInModDb))
		{
			return getEntryValues<T>(index, isFoundInModDb, requestedType);
		}

		return {};
	}

	template<typename T>
	ReturnCodeEnum update(const std::string& key, std::vector<T>& values, bool isHardWrite)
	{
		const auto& it = findMatchingIndices(key);
		if(!it.has_value()) return ReturnCodeEnum(ReturnCodeRaw::KEY_NOT_FOUND);

		const auto& [index, isFoundInModDb] = it.value();

		if(!checkIfWritable(index, isFoundInModDb)) return ReturnCodeEnum(ReturnCodeRaw::NOT_WRITABLE);

		DbTypeEnum requestedType;
		if(!checkIfCorrectType<T>(index, isFoundInModDb, requestedType)) return ReturnCodeEnum(ReturnCodeRaw::TYPE_MISMATCH);
		else if(checkIfErased(index, isFoundInModDb)) return ReturnCodeEnum(ReturnCodeRaw::KEY_NOT_FOUND);

		auto indexInModDb = updateDbEntry<T>(index, isFoundInModDb, values, requestedType);

		if(isHardWrite)
		{
			// Edit and overwrite swdb-hardsave.bin file with new entry's value to keep it persistent over restarts.
			// Note that: updateDbEntry() must be called before updateHardSavedDb()
			updateHardSavedDb(indexInModDb);
		}

		return ReturnCodeEnum(ReturnCodeRaw::OK);
	}
	
	ReturnCodeEnum restore(const std::string& key);
	ReturnCodeEnum resetToDefault();
	ReturnCodeEnum erase(const std::string& key);

private:
	explicit DbLoader();
	virtual ~DbLoader() = default;
	
	struct EntryStatus
	{
		bool isErased {false};
	};

	struct DbEntry
	{
		std::string key;
		DbPermissionEnum permission;
		DbTypeEnum type;
		std::vector<std::any> values;

		EntryStatus status;
	};

	using DatabaseStorage = std::vector<DbEntry>;
	using DatabaseDictionary = std::unordered_map<std::string, std::unordered_set<std::size_t>>;

	// Original Database
	std::mutex m_storageMutex;
	DatabaseStorage m_dbStorage;
	std::mutex m_dictionaryMutex;
	DatabaseDictionary m_dbDictionary;

	// Modified Database (prefer searching in this database first, if not found then try on Original Database)
	std::mutex m_modStorageMutex;
	DatabaseStorage m_modDbStorage;
	std::mutex m_modDictionaryMutex;
	DatabaseDictionary m_modDbDictionary;

	bool isHardSavedDbFileInit {false};
	const std::string m_binDbPath { "/home/giangnguyentbk/workspace/dbengine/sw/texttobin/swdb" }; // currently hardcoded
	uint32_t m_crc16Table[256] = 
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

private:
	bool loadDb(const std::string& binFilePath);
	bool loadHardSavedDb(const std::string& binFilePath);
	std::vector<std::string> tokenize(const std::string& key, const std::string& delimiter);
	std::vector<std::size_t> findMatchingKeys(const std::string& input, const DatabaseDictionary& dbDictionary, std::mutex& mtx);
	bool isFitIntegralType(const int64_t& valueToCheck, const DbTypeEnum& type);
	std::optional<std::pair<std::size_t, bool>> findMatchingIndices(const std::string& input);
	bool checkIfWritable(const std::size_t& index, const bool& isFoundInModDb);
	bool checkIfErased(const std::size_t& index, const bool& isFoundInModDb);
	void updateHardSavedDb(const std::size_t& index);
	void initHardSavedDbFile();
	uint16_t getCRC16(uint8_t *startAddr, uint32_t numberBytes);
	uint32_t lookupCRC16Table(uint32_t initCRC, uint8_t data);
	std::size_t eraseDbEntry(const std::size_t& index, const bool& isFoundInModDb);
	void restoreHardSavedDb(const std::size_t& index);

	template<typename T>
	bool checkIfCorrectType(const std::size_t& index, const bool& isFoundInModDb, DbTypeEnum& requestedType)
	{
		DatabaseStorage& dbStorage = isFoundInModDb ? m_modDbStorage : m_dbStorage;
		std::mutex& mtx = isFoundInModDb ? m_modStorageMutex : m_storageMutex;

		if(std::is_same<T, uint8_t>::value) requestedType.set(DbTypeEnumRaw::TYPE_OF_ENTRY_U8);
		else if(std::is_same<T, int8_t>::value) requestedType.set(DbTypeEnumRaw::TYPE_OF_ENTRY_S8);
		else if(std::is_same<T, uint16_t>::value) requestedType.set(DbTypeEnumRaw::TYPE_OF_ENTRY_U16);
		else if(std::is_same<T, int16_t>::value) requestedType.set(DbTypeEnumRaw::TYPE_OF_ENTRY_S16);
		else if(std::is_same<T, uint32_t>::value) requestedType.set(DbTypeEnumRaw::TYPE_OF_ENTRY_U32);
		else if(std::is_same<T, int32_t>::value) requestedType.set(DbTypeEnumRaw::TYPE_OF_ENTRY_S32);
		else if(std::is_same<T, uint64_t>::value) requestedType.set(DbTypeEnumRaw::TYPE_OF_ENTRY_U64);
		else if(std::is_same<T, int64_t>::value) requestedType.set(DbTypeEnumRaw::TYPE_OF_ENTRY_S64);
		else if(std::is_same<T, std::string>::value) requestedType.set(DbTypeEnumRaw::TYPE_OF_ENTRY_CHAR);

		std::scoped_lock<std::mutex> lockStorage(mtx);
		if(dbStorage.at(index).type != requestedType)
		{
			TPT_TRACE(TRACE_ABN, SSTR("Requested type ", requestedType.toString(), " did not match with DB entry type ", dbStorage.at(index).type.toString()));
			return false;
		}

		return true;
	}

	template<typename T>
	std::vector<T> getEntryValues(const std::size_t& index, const bool& isFoundInModDb, const DbTypeEnum& requestedType)
	{
		std::mutex& mtx = isFoundInModDb ? m_modStorageMutex : m_storageMutex;
		DatabaseStorage& dbStorage = isFoundInModDb ? m_modDbStorage : m_dbStorage;

		std::vector<T> values;
		values.reserve(64); // Currently hardcoded
		std::scoped_lock<std::mutex> lockStorage(mtx);
		for(const std::any& v : dbStorage.at(index).values)
		{
			if(v.has_value())
			{
				try
				{
					values.emplace_back(std::any_cast<T>(v));
				}
				catch(const std::exception& e)
				{
					TPT_TRACE(TRACE_ABN, SSTR("Could not any_cast DB entry for key ", dbStorage.at(index).key, " to type ", requestedType.toString()));
				}
			}
			else
			{
				values.emplace_back(T());
			}
		}

		return values;
	}

	template<typename T>
	std::size_t updateDbEntry(const std::size_t& index, const bool& isFoundInModDb, std::vector<T>& values, const DbTypeEnum& requestedType)
	{
		std::scoped_lock<std::mutex> lockStorage(m_modStorageMutex);
		if(isFoundInModDb)
		{
			// Modify value in the found entry in Modified DB
			m_modDbStorage.at(index).values.clear();
			if(requestedType.getRawEnum() == DbTypeEnumRaw::TYPE_OF_ENTRY_CHAR)
			{
				std::string concatStr;
				for(const auto& v : values)
				{
					concatStr += (v + " "); // Try to make it as much similar as with a complete string "value" with at least 1 space between each substring
					m_modDbStorage.at(index).values.emplace_back(v);
				}

				concatStr.pop_back(); // Remove the last redundant space " "
				m_modDbStorage.at(index).values.emplace_back(concatStr);
			}
			else
			{
				for(const auto& v : values)
				{
					m_modDbStorage.at(index).values.emplace_back(v);
				}
			}

			TPT_TRACE(TRACE_INFO, SSTR("Modified entry ", m_modDbStorage.at(index).key, " in Modified DB successfully!"));
			return index;
		}
		else
		{
			// Add new entry with updated value into Modified DB. Do not change anything in Original DB
			std::scoped_lock<std::mutex> lockStorage(m_storageMutex);
			std::scoped_lock<std::mutex> lockDictionary(m_modDictionaryMutex);
			auto copiedEntry = m_dbStorage.at(index);

			copiedEntry.values.clear();
			if(requestedType.getRawEnum() == DbTypeEnumRaw::TYPE_OF_ENTRY_CHAR)
			{
				std::string concatStr;
				for(const auto& v : values)
				{
					concatStr += (v + " "); // Try to make it as much similar as with a complete string "value" with at least 1 space between each substring
					copiedEntry.values.emplace_back(v);
				}

				concatStr.pop_back(); // Remove the last redundant space " "
				copiedEntry.values.emplace_back(concatStr);
			}
			else
			{
				for(const auto& v : values)
				{
					copiedEntry.values.emplace_back(v);
				}
			}

			m_modDbStorage.emplace_back(copiedEntry);

			std::vector<std::string> subKeys = tokenize(copiedEntry.key, "/");
			for(const auto& sk : subKeys)
			{
				m_modDbDictionary[sk].insert(m_modDbStorage.size() - 1); // Storing the index of entry in m_dbStorage vector
			}

			TPT_TRACE(TRACE_INFO, SSTR("Added entry ", copiedEntry.key, " into Modified DB successfully!"));
			return m_modDbStorage.size() - 1;
		}
	}

}; // class DbLoader

} // namespace V1

} // namespace DatabaseIf

} // namespace DbEngine