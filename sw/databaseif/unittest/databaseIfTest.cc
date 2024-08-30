#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include "databaseIf.h"

using namespace DbEngine::DatabaseIf::V1;

int main()
{
	const std::string key1 { "/isFeatureXyzEnabled" };
	if(const auto& it = IDatabase::getInstance().autoGet<uint8_t>(key1); it.has_value())
	{
		std::cout << "Reading DB key (" << key1 << "): " << +it.value() << std::endl;
	}

	const std::string key2 { "/initSequence" };
	if(const auto& it = IDatabase::getInstance().autoGetVec<uint8_t>(key2); it.has_value() && it.value().size())
	{
		std::cout << "Reading DB key (" << key2 << "):";
		for(const auto& v : it.value())
		{
			std::cout << " " << +v;
		}
		std::cout << std::endl;
	}

	const std::string key3 { "/supportedCapabilities" };
	if(const auto& it = IDatabase::getInstance().autoGet<uint16_t>(key3); it.has_value())
	{
		std::cout << "Reading DB key (" << key3 << "): " << it.value() << std::endl;
	}

	const std::string key4 { "/driverName" };
	if(const auto& it = IDatabase::getInstance().autoGet<std::string>(key4); it.has_value())
	{
		std::cout << "Reading DB key (" << key4 << "): " << it.value() << std::endl;
	}

	const std::string key5 { "/temperatureRanges" };
	if(const auto& it = IDatabase::getInstance().autoGetVec<int16_t>(key5); it.has_value() && it.value().size())
	{
		std::cout << "Reading DB key (" << key5 << "):";
		for(const auto& v : it.value())
		{
			std::cout << " " << v;
		}
		std::cout << std::endl;
	}

	const std::string key6 { "/supportedProtocols" };
	if(const auto& it = IDatabase::getInstance().autoGet<std::string>(key6); it.has_value())
	{
		std::cout << "Reading DB key (" << key6 << "), entire string: " << it.value() << std::endl;
	}

	if(const auto& it = IDatabase::getInstance().autoGetVec<std::string>(key6); it.has_value() && it.value().size())
	{
		std::cout << "Reading DB key (" << key6 << "), split string:";
		for(const auto& v : it.value())
		{
			std::cout << " - " << v;
		}
		std::cout << std::endl;
	}

	std::vector<uint8_t> vkey1 {0};
	if(IDatabase::getInstance().update(key1, vkey1, false).getRawEnum() == ReturnCodeRaw::OK)
	{
		std::cout << "Soft writing DB key (" << key1 << ") successfully!\n";
	}

	if(const auto& it = IDatabase::getInstance().autoGet<uint8_t>(key1); it.has_value())
	{
		std::cout << "Reading DB key (" << key1 << "): " << +it.value() << std::endl;
	}

	// std::vector<uint16_t> vkey3 {3};
	// if(IDatabase::getInstance().update(key3, vkey3, true).getRawEnum() == ReturnCodeRaw::OK)
	// {
	// 	std::cout << "Hard writing DB key (" << key3 << ") successfully!\n";
	// }

	// if(const auto& it = IDatabase::getInstance().autoGet<uint16_t>(key3); it.has_value())
	// {
	// 	std::cout << "Reading DB key (" << key3 << "): " << it.value() << std::endl;
	// }

	return 0;
}