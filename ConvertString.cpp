#include "ConvertString.h"

#include <Windows.h>


std::wstring ConvertString(const std::string &str) {
	if (str.empty()) {
		return std::wstring();
	}

	auto stringSize = MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), NULL, 0);
	if (stringSize == 0) {
		return std::wstring();
	}
	std::wstring result(stringSize, 0);
	MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), result.data(), stringSize);
	return result;
}


std::string ConvertString(const std::wstring &str) {
	if (str.empty()) {
		return std::string();
	}

	auto stringSize = WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), NULL, 0, NULL, NULL);
	if (stringSize == 0) {
		return std::string();
	}
	std::string result(stringSize, 0);
	WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), result.data(), stringSize, NULL, NULL);
	return result;
}