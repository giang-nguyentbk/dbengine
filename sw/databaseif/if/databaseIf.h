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

namespace DbEngine
{
namespace DatabaseIf
{
namespace V1
{

class IDatabase
{
public:
	static IDatabase& getInstance();

	IDatabase(const IDatabase& other) = delete;
	IDatabase(IDatabase&& other) = delete;
	IDatabase& operator=(const IDatabase& other) = delete;
	IDatabase& operator=(IDatabase&& other) = delete;

	enum class ReturnCode
	{
		OK,
		KEY_NOT_FOUND,
		TYPE_MISMATCH
	};

	virtual ReturnCode get(const std::string& key, std::vector<uint8_t>& values) const = 0;
	virtual ReturnCode get(const std::string& key, std::vector<int8_t>& values) const = 0;
	virtual ReturnCode get(const std::string& key, std::vector<uint16_t>& values) const = 0;
	virtual ReturnCode get(const std::string& key, std::vector<int16_t>& values) const = 0;
	virtual ReturnCode get(const std::string& key, std::vector<uint32_t>& values) const = 0;
	virtual ReturnCode get(const std::string& key, std::vector<int32_t>& values) const = 0;
	virtual ReturnCode get(const std::string& key, std::vector<uint64_t>& values) const = 0;
	virtual ReturnCode get(const std::string& key, std::vector<int64_t>& values) const = 0;
	virtual ReturnCode get(const std::string& key, std::vector<std::string>& values) const = 0;
	// virtual ReturnCode get(const std::string& key, std::string& value) const = 0;

	template<typename T>
	std::optional<std::vector<T>> autoGetVec(const std::string& key) noexcept
	{
		std::vector<T> values;
		if(get(key, values) == ReturnCode::OK)
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
			if(get(key, values) == ReturnCode::OK)
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