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

#include <enumUtils.h>

using namespace CommonUtils::V1::EnumUtils;

namespace DbEngine
{
namespace DatabaseIf
{
namespace V1
{

enum class ReturnCodeRaw
{
	OK,
	KEY_NOT_FOUND,
	TYPE_MISMATCH,
	UNDEFINED
};

class ReturnCodeEnum : public EnumType<ReturnCodeRaw>
{
public:
	explicit ReturnCodeEnum(const ReturnCodeRaw& raw) : EnumType<ReturnCodeRaw>(raw) {}
	explicit ReturnCodeEnum() : EnumType<ReturnCodeRaw>(ReturnCodeRaw::UNDEFINED) {}

	std::string toString() const override
	{
		switch (getRawEnum())
		{
		case ReturnCodeRaw::OK:
			return "OK";

		case ReturnCodeRaw::KEY_NOT_FOUND:
			return "KEY_NOT_FOUND";

		case ReturnCodeRaw::TYPE_MISMATCH:
			return "TYPE_MISMATCH";

		case ReturnCodeRaw::UNDEFINED:
			return "UNDEFINED";

		default:
			return "Unknown EnumType: " + std::to_string(toS32());
		}
	}
};

class IDatabase
{
public:
	static IDatabase& getInstance();

	IDatabase(const IDatabase& other) = delete;
	IDatabase(IDatabase&& other) = delete;
	IDatabase& operator=(const IDatabase& other) = delete;
	IDatabase& operator=(IDatabase&& other) = delete;

	virtual ReturnCodeEnum get(const std::string& key, std::vector<uint8_t>& values) const = 0;
	virtual ReturnCodeEnum get(const std::string& key, std::vector<int8_t>& values) const = 0;
	virtual ReturnCodeEnum get(const std::string& key, std::vector<uint16_t>& values) const = 0;
	virtual ReturnCodeEnum get(const std::string& key, std::vector<int16_t>& values) const = 0;
	virtual ReturnCodeEnum get(const std::string& key, std::vector<uint32_t>& values) const = 0;
	virtual ReturnCodeEnum get(const std::string& key, std::vector<int32_t>& values) const = 0;
	virtual ReturnCodeEnum get(const std::string& key, std::vector<uint64_t>& values) const = 0;
	virtual ReturnCodeEnum get(const std::string& key, std::vector<int64_t>& values) const = 0;
	virtual ReturnCodeEnum get(const std::string& key, std::vector<std::string>& values) const = 0;

	// isHardWrite: a hard write into database meaning the modified value will be persistent even after software/hardware restarted
	// A hard write entry can be only restored by calling restore() overloads below
	// On another hand, a soft write entry can only exist in the current running session, when program or device restarted it's automatically restored.
	// A soft write entry can also be restored by calling restore()
	virtual ReturnCodeEnum update(const std::string& key, std::vector<uint8_t>& values, bool isHardWrite) const = 0;
	virtual ReturnCodeEnum update(const std::string& key, std::vector<int8_t>& values, bool isHardWrite) const = 0;
	virtual ReturnCodeEnum update(const std::string& key, std::vector<uint16_t>& values, bool isHardWrite) const = 0;
	virtual ReturnCodeEnum update(const std::string& key, std::vector<int16_t>& values, bool isHardWrite) const = 0;
	virtual ReturnCodeEnum update(const std::string& key, std::vector<uint32_t>& values, bool isHardWrite) const = 0;
	virtual ReturnCodeEnum update(const std::string& key, std::vector<int32_t>& values, bool isHardWrite) const = 0;
	virtual ReturnCodeEnum update(const std::string& key, std::vector<uint64_t>& values, bool isHardWrite) const = 0;
	virtual ReturnCodeEnum update(const std::string& key, std::vector<int64_t>& values, bool isHardWrite) const = 0;
	virtual ReturnCodeEnum update(const std::string& key, std::vector<std::string>& values, bool isHardWrite) const = 0;

	// Restore a specific key using back to original DB
	// virtual ReturnCodeEnum restore(const std::string& key) const = 0;

	// Make everything back to original DB
	// virtual ReturnCodeEnum default() const = 0;

	// Mark a specific key as deleted
	// virtual ReturnCodeEnum erase(const std::string& key, bool isHardWrite) const = 0;

	template<typename T>
	std::optional<std::vector<T>> autoGetVec(const std::string& key) noexcept
	{
		std::vector<T> values;
		if(get(key, values).getRawEnum() == ReturnCodeRaw::OK)
		{
			if(std::is_same<T, std::string>::value)
			{
				// Remove the full version of string at last index
				std::vector<T> ret(values.begin(), values.begin() + values.size() - 1);
				return ret;
			}
			
			return values;
		}

		return std::nullopt;
	}

	template<typename T>
	std::optional<T> autoGet(const std::string& key) noexcept
	{
		if(std::is_same<T, std::string>::value)
		{
			std::vector<T> values;
			if(get(key, values).getRawEnum() == ReturnCodeRaw::OK)
			{
				// The original string value is store at last
				return values.back();
			}
		}
		else
		{
			if(const auto& it = autoGetVec<T>(key); it.has_value() && it.value().size())
			{
				return it.value().front();
			}
		}

		

		return std::nullopt;
	}

protected:
	IDatabase() = default;
	virtual ~IDatabase() = default;

}; // class IDatabase

} // namespace V1

} // namespace DatabaseIf

} // namespace DbEngine