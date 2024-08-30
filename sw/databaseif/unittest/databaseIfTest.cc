#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include "databaseIf.h"

using namespace DbEngine::DatabaseIf::V1;

int main()
{
	const std::string key1 { "/isFeatureXyzEnabled" };
	std::cout << "[DEBUG]: Reading uint8_t DB key " << key1 << std::endl;
	if(const auto& it = IDatabase::getInstance().autoGet<uint8_t>(key1); it.has_value())
	{
		std::cout << "[DEBUG]: Reading DB key (" << key1 << "): " << +it.value() << std::endl;
	}

	const std::string key2 { "/initSequence" };
	std::cout << "[DEBUG]: Reading uint8_t DB key " << key2 << std::endl;
	if(const auto& it = IDatabase::getInstance().autoGetVec<uint8_t>(key2); it.has_value() && it.value().size())
	{
		std::cout << "[DEBUG]: Reading DB key (" << key2 << "):";
		for(const auto& v : it.value())
		{
			std::cout << " " << +v;
		}
		std::cout << std::endl;
	}

	const std::string key3 { "/supportedCapabilities" };
	std::cout << "[DEBUG]: Reading uint16_t DB key " << key3 << std::endl;
	if(const auto& it = IDatabase::getInstance().autoGet<uint16_t>(key3); it.has_value())
	{
		std::cout << "[DEBUG]: Reading DB key (" << key3 << "): " << it.value() << std::endl;
	}

	const std::string key4 { "/driverName" };
	std::cout << "[DEBUG]: Reading duplicated string DB key " << key4 << std::endl;
	if(const auto& it = IDatabase::getInstance().autoGet<std::string>(key4); it.has_value())
	{
		std::cout << "[DEBUG]: Reading DB key (" << key4 << "): " << it.value() << std::endl;
	}

	const std::string key5 { "/temperatureRanges" };
	std::cout << "[DEBUG]: Reading int16_t DB key " << key5 << std::endl;
	if(const auto& it = IDatabase::getInstance().autoGetVec<int16_t>(key5); it.has_value() && it.value().size())
	{
		std::cout << "[DEBUG]: Reading DB key (" << key5 << "):";
		for(const auto& v : it.value())
		{
			std::cout << " " << v;
		}
		std::cout << std::endl;
	}

	const std::string key6 { "/supportedProtocols" };
	std::cout << "[DEBUG]: Reading string DB key " << key6 << std::endl;
	if(const auto& it = IDatabase::getInstance().autoGet<std::string>(key6); it.has_value())
	{
		std::cout << "[DEBUG]: Reading DB key (" << key6 << "), entire string: " << it.value() << std::endl;
	}

	std::cout << "[DEBUG]: Reading string DB key " << key6 << std::endl;
	if(const auto& it = IDatabase::getInstance().autoGetVec<std::string>(key6); it.has_value() && it.value().size())
	{
		std::cout << "[DEBUG]: Reading DB key (" << key6 << "), split string:";
		for(const auto& v : it.value())
		{
			std::cout << " - " << v;
		}
		std::cout << std::endl;
	}

	std::vector<int16_t> vkey5 {-1, 1, 1, -1};
	std::cout << "[DEBUG]: Soft writing int16_t DB key " << key5 << std::endl;
	if(IDatabase::getInstance().update(key5, vkey5, false).getRawEnum() == ReturnCodeRaw::OK)
	{
		std::cout << "[DEBUG]: Soft writing DB key (" << key5 << ") successfully!\n";
	}

	std::vector<uint8_t> vkey1 {0};
	std::cout << "[DEBUG]: Soft writing uint8_t DB key " << key1 << std::endl;
	if(IDatabase::getInstance().update(key1, vkey1, false).getRawEnum() == ReturnCodeRaw::OK)
	{
		std::cout << "[DEBUG]: Soft writing DB key (" << key1 << ") successfully!\n";
	}

	std::cout << "[DEBUG]: Reading uint8_t DB key " << key1 << std::endl;
	if(const auto& it = IDatabase::getInstance().autoGet<uint8_t>(key1); it.has_value())
	{
		std::cout << "[DEBUG]: Reading DB key (" << key1 << "): " << +it.value() << std::endl;
	}

	std::cout << "[DEBUG]: Erasing uint8_t DB key " << key1 << std::endl;
	IDatabase::getInstance().erase(key1);

	std::cout << "[DEBUG]: Reading uint8_t DB key " << key1 << std::endl;
	if(const auto& it = IDatabase::getInstance().autoGet<uint8_t>(key1); it.has_value())
	{
		std::cout << "[DEBUG]: Reading DB key (" << key1 << "): " << +it.value() << std::endl;
	}

	std::cout << "[DEBUG]: Restoring uint8_t DB key " << key1 << std::endl;
	IDatabase::getInstance().restore(key1);

	std::cout << "[DEBUG]: Reading uint8_t DB key " << key1 << std::endl;
	if(const auto& it = IDatabase::getInstance().autoGet<uint8_t>(key1); it.has_value())
	{
		std::cout << "[DEBUG]: Reading DB key (" << key1 << "): " << +it.value() << std::endl;
	}

	std::cout << "[DEBUG]: Reset the whole DB!" << std::endl;
	IDatabase::getInstance().reset();

	std::cout << "[DEBUG]: Reading uint8_t DB key " << key1 << std::endl;
	if(const auto& it = IDatabase::getInstance().autoGet<uint8_t>(key1); it.has_value())
	{
		std::cout << "[DEBUG]: Reading DB key (" << key1 << "): " << +it.value() << std::endl;
	}

	std::vector<uint16_t> vkey3 {3};
	std::cout << "[DEBUG]: Hard writing uint16_t DB key " << key3 << std::endl;
	if(IDatabase::getInstance().update(key3, vkey3, true).getRawEnum() == ReturnCodeRaw::OK)
	{
		std::cout << "[DEBUG]: Hard writing DB key (" << key3 << ") successfully!\n";
	}

	std::cout << "[DEBUG]: Reading uint16_t DB key " << key3 << std::endl;
	if(const auto& it = IDatabase::getInstance().autoGet<uint16_t>(key3); it.has_value())
	{
		std::cout << "[DEBUG]: Reading DB key (" << key3 << "): " << it.value() << std::endl;
	}

	// std::cout << "[DEBUG]: Restoring uint16_t DB key " << key3 << std::endl;
	// IDatabase::getInstance().restore(key3);

	// std::cout << "[DEBUG]: Reading uint16_t DB key " << key3 << std::endl;
	// if(const auto& it = IDatabase::getInstance().autoGet<uint16_t>(key3); it.has_value())
	// {
	// 	std::cout << "[DEBUG]: Reading DB key (" << key3 << "): " << it.value() << std::endl;
	// }


	return 0;
}