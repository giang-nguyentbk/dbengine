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


ReturnCodeEnum DatabaseImpl::get(const std::string& key, std::vector<uint8_t>& values) const
{
	ReturnCodeEnum rc;
	values = DbLoader::getInstance().retrieve<uint8_t>(key, rc);
	return rc;
}

ReturnCodeEnum DatabaseImpl::get(const std::string& key, std::vector<int8_t>& values) const
{
	ReturnCodeEnum rc;
	values = DbLoader::getInstance().retrieve<int8_t>(key, rc);
	return rc;
}

ReturnCodeEnum DatabaseImpl::get(const std::string& key, std::vector<uint16_t>& values) const
{
	ReturnCodeEnum rc;
	values = DbLoader::getInstance().retrieve<uint16_t>(key, rc);
	return rc;
}

ReturnCodeEnum DatabaseImpl::get(const std::string& key, std::vector<int16_t>& values) const
{
	ReturnCodeEnum rc;
	values = DbLoader::getInstance().retrieve<int16_t>(key, rc);
	return rc;
}

ReturnCodeEnum DatabaseImpl::get(const std::string& key, std::vector<uint32_t>& values) const
{
	ReturnCodeEnum rc;
	values = DbLoader::getInstance().retrieve<uint32_t>(key, rc);
	return rc;
}

ReturnCodeEnum DatabaseImpl::get(const std::string& key, std::vector<int32_t>& values) const
{
	ReturnCodeEnum rc;
	values = DbLoader::getInstance().retrieve<int32_t>(key, rc);
	return rc;
}

ReturnCodeEnum DatabaseImpl::get(const std::string& key, std::vector<uint64_t>& values) const
{
	ReturnCodeEnum rc;
	values = DbLoader::getInstance().retrieve<uint64_t>(key, rc);
	return rc;
}

ReturnCodeEnum DatabaseImpl::get(const std::string& key, std::vector<int64_t>& values) const
{
	ReturnCodeEnum rc;
	values = DbLoader::getInstance().retrieve<int64_t>(key, rc);
	return rc;
}

ReturnCodeEnum DatabaseImpl::get(const std::string& key, std::vector<std::string>& values) const
{
	ReturnCodeEnum rc;
	values = DbLoader::getInstance().retrieve<std::string>(key, rc);
	return rc;
}

ReturnCodeEnum DatabaseImpl::update(const std::string& key, std::vector<uint8_t>& values, bool isHardWrite) const
{
	return DbLoader::getInstance().update<uint8_t>(key, values, isHardWrite);
}

ReturnCodeEnum DatabaseImpl::update(const std::string& key, std::vector<int8_t>& values, bool isHardWrite) const
{
	return DbLoader::getInstance().update<int8_t>(key, values, isHardWrite);
}

ReturnCodeEnum DatabaseImpl::update(const std::string& key, std::vector<uint16_t>& values, bool isHardWrite) const
{
	return DbLoader::getInstance().update<uint16_t>(key, values, isHardWrite);
}

ReturnCodeEnum DatabaseImpl::update(const std::string& key, std::vector<int16_t>& values, bool isHardWrite) const
{
	return DbLoader::getInstance().update<int16_t>(key, values, isHardWrite);
}

ReturnCodeEnum DatabaseImpl::update(const std::string& key, std::vector<uint32_t>& values, bool isHardWrite) const
{
	return DbLoader::getInstance().update<uint32_t>(key, values, isHardWrite);
}

ReturnCodeEnum DatabaseImpl::update(const std::string& key, std::vector<int32_t>& values, bool isHardWrite) const
{
	return DbLoader::getInstance().update<int32_t>(key, values, isHardWrite);
}

ReturnCodeEnum DatabaseImpl::update(const std::string& key, std::vector<uint64_t>& values, bool isHardWrite) const
{
	return DbLoader::getInstance().update<uint64_t>(key, values, isHardWrite);
}

ReturnCodeEnum DatabaseImpl::update(const std::string& key, std::vector<int64_t>& values, bool isHardWrite) const
{
	return DbLoader::getInstance().update<int64_t>(key, values, isHardWrite);
}

ReturnCodeEnum DatabaseImpl::update(const std::string& key, std::vector<std::string>& values, bool isHardWrite) const
{
	return DbLoader::getInstance().update<std::string>(key, values, isHardWrite);
}


} // namespace V1

} // namespace DatabaseIf

} // namespace DbEngine