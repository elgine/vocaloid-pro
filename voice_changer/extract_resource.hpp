#pragma once
#include <fstream>
#include <map>
#include "../utility/module.hpp"
#include "../utility/path.hpp"
#include "../utility/string_wstring.hpp"
using namespace std;

#define NO_SUCH_RESOURCE L"Not exist resource"

static wstring temp_path;
static map<int, wstring> resources;

bool ExtractResource(LPCTSTR strDstFile, LPCTSTR strResType, LPCTSTR strResName){
	auto handler = GetSelfModuleHandle();
	if (handler == nullptr)return false;
    HANDLE hFile = ::CreateFile(strDstFile, GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
        return false;
    HRSRC hRes = ::FindResource(handler, strResName, strResType);
	HGLOBAL hMem = ::LoadResource(handler, hRes);
	DWORD dwSize = ::SizeofResource(handler, hRes);

    DWORD dwWrite = 0;
	::WriteFile(hFile, hMem, dwSize, &dwWrite, NULL);
	::CloseHandle(hFile);
    return true;
}

int SetTempPath(const char* path) {
	temp_path = StringToWString(path);
	if (CreateDirectoryRecursivly(WStringToString(temp_path.c_str())))return 0;
	return -510;
}

string ExtractResource(int res_id, WCHAR* res_type) {
	wstring result;
	if (resources.find(res_id) != resources.end()) {
		result = resources[res_id];
	}
	else {
		auto file_name = MAKEINTRESOURCE(res_id);
		wstring file_path = temp_path;
		wstring res_name;
		_itow(res_id, (wchar_t*)res_name.data(), 10);
		file_path = file_path + L"\\" + res_name.data() + L"." + res_type;
		if (ExtractResource(file_path.data(), res_type, file_name)) {
			resources[res_id] = file_path;
			result = file_path;
		}else
			result = NO_SUCH_RESOURCE;
	}
	return WStringToString(result.data());
}

int DisposeAllTempResources() {
	for (auto iter : resources) {
		char* path = WStringToString(iter.second.data());
		if (IsFileExists(path)) {
			remove(path);
		}
	}
	resources.clear();
	return 0;
}