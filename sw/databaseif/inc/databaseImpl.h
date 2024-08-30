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

#include "databaseIf.h"
#include "dbLoader.h"

namespace DbEngine
{
namespace DatabaseIf
{
namespace V1
{

class DatabaseImpl : public IDatabase
{
public:
	static DatabaseImpl& getInstance();

	DatabaseImpl(const DatabaseImpl& other) = delete;
	DatabaseImpl(DatabaseImpl&& other) = delete;
	DatabaseImpl& operator=(const DatabaseImpl& other) = delete;
	DatabaseImpl& operator=(DatabaseImpl&& other) = delete;

	ReturnCodeEnum get(const std::string& key, std::vector<uint8_t>& values) const override;
	ReturnCodeEnum get(const std::string& key, std::vector<int8_t>& values) const override;
	ReturnCodeEnum get(const std::string& key, std::vector<uint16_t>& values) const override;
	ReturnCodeEnum get(const std::string& key, std::vector<int16_t>& values) const override;
	ReturnCodeEnum get(const std::string& key, std::vector<uint32_t>& values) const override;
	ReturnCodeEnum get(const std::string& key, std::vector<int32_t>& values) const override;
	ReturnCodeEnum get(const std::string& key, std::vector<uint64_t>& values) const override;
	ReturnCodeEnum get(const std::string& key, std::vector<int64_t>& values) const override;
	ReturnCodeEnum get(const std::string& key, std::vector<std::string>& values) const override;

	ReturnCodeEnum update(const std::string& key, std::vector<uint8_t>& values, bool isHardWrite) const override;
	ReturnCodeEnum update(const std::string& key, std::vector<int8_t>& values, bool isHardWrite) const override;
	ReturnCodeEnum update(const std::string& key, std::vector<uint16_t>& values, bool isHardWrite) const override;
	ReturnCodeEnum update(const std::string& key, std::vector<int16_t>& values, bool isHardWrite) const override;
	ReturnCodeEnum update(const std::string& key, std::vector<uint32_t>& values, bool isHardWrite) const override;
	ReturnCodeEnum update(const std::string& key, std::vector<int32_t>& values, bool isHardWrite) const override;
	ReturnCodeEnum update(const std::string& key, std::vector<uint64_t>& values, bool isHardWrite) const override;
	ReturnCodeEnum update(const std::string& key, std::vector<int64_t>& values, bool isHardWrite) const override;
	ReturnCodeEnum update(const std::string& key, std::vector<std::string>& values, bool isHardWrite) const override;

	ReturnCodeEnum restore(const std::string& key) const override;

	ReturnCodeEnum reset() const override;

	ReturnCodeEnum erase(const std::string& key) const override;


private:
	DatabaseImpl() = default;
	~DatabaseImpl() = default;

}; // class DatabaseImpl

} // namespace V1

} // namespace DatabaseIf

} // namespace DbEngine