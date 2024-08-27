#include <vector>
#include <string>
#include <optional>

#include "databaseImpl.h"

namespace DbEngine
{
namespace DatabaseIf
{
namespace V1
{

IDatabase& IDatabase::getInstance()
{
	return DatabaseImpl::getInstance();
}

DatabaseImpl& DatabaseImpl::getInstance()
{
	static DatabaseImpl instance;
	return instance;
}


IDatabase::ReturnCode DatabaseImpl::get(const std::string& key, std::vector<uint8_t>& values) const
{
	IDatabase::ReturnCode rc;
	values = DbLoader::getInstance().retrieveValueByIndex<uint8_t>(key, rc);
	return rc;
}

IDatabase::ReturnCode DatabaseImpl::get(const std::string& key, std::vector<int8_t>& values) const
{
	IDatabase::ReturnCode rc;
	values = DbLoader::getInstance().retrieveValueByIndex<int8_t>(key, rc);
	return rc;
}

IDatabase::ReturnCode DatabaseImpl::get(const std::string& key, std::vector<uint16_t>& values) const
{
	IDatabase::ReturnCode rc;
	values = DbLoader::getInstance().retrieveValueByIndex<uint16_t>(key, rc);
	return rc;
}

IDatabase::ReturnCode DatabaseImpl::get(const std::string& key, std::vector<int16_t>& values) const
{
	IDatabase::ReturnCode rc;
	values = DbLoader::getInstance().retrieveValueByIndex<int16_t>(key, rc);
	return rc;
}

IDatabase::ReturnCode DatabaseImpl::get(const std::string& key, std::vector<uint32_t>& values) const
{
	IDatabase::ReturnCode rc;
	values = DbLoader::getInstance().retrieveValueByIndex<uint32_t>(key, rc);
	return rc;
}

IDatabase::ReturnCode DatabaseImpl::get(const std::string& key, std::vector<int32_t>& values) const
{
	IDatabase::ReturnCode rc;
	values = DbLoader::getInstance().retrieveValueByIndex<int32_t>(key, rc);
	return rc;
}

IDatabase::ReturnCode DatabaseImpl::get(const std::string& key, std::vector<uint64_t>& values) const
{
	IDatabase::ReturnCode rc;
	values = DbLoader::getInstance().retrieveValueByIndex<uint64_t>(key, rc);
	return rc;
}

IDatabase::ReturnCode DatabaseImpl::get(const std::string& key, std::vector<int64_t>& values) const
{
	IDatabase::ReturnCode rc;
	values = DbLoader::getInstance().retrieveValueByIndex<int64_t>(key, rc);
	return rc;
}

IDatabase::ReturnCode DatabaseImpl::get(const std::string& key, std::vector<std::string>& values) const
{
	IDatabase::ReturnCode rc;
	values = DbLoader::getInstance().retrieveValueByIndex<std::string>(key, rc);
	return rc;
}


} // namespace V1

} // namespace DatabaseIf

} // namespace DbEngine