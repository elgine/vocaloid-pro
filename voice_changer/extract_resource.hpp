#pragma once
#include <windows.h>
#include "../utility/path.hpp"
#include "../utility/string_wstring.hpp"
#include <fstream>
#include <map>
using namespace std;

#define NO_SUCH_RESOURCE L"Not exist resource"

static wstring temp_path;
static map<int, wstring> resources;


bool ExtractResource(LPCTSTR strDstFile, LPCTSTR strResType, LPCTSTR strResName){
    HANDLE hFile = ::CreateFile(strDstFile, GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
        return false;
    HRSRC hRes = ::FindResource(NULL, strResName, strResType);
	HGLOBAL hMem = ::LoadResource(NULL, hRes);
	DWORD dwSize = ::SizeofResource(NULL, hRes);

    DWORD dwWrite = 0;
	::WriteFile(hFile, hMem, dwSize, &dwWrite, NULL);
	::CloseHandle(hFile);
    return true;
}

void SetTempPath(const char* path) {
	temp_path = StringToWString(path);
}

string ExtractResource(int res_id, WCHAR* res_type) {
	wstring result;
	if (resources.find(res_id) != resources.end()) {
		result = resources[res_id];
	}
	else {
		wstring res_name = MAKEINTRESOURCE(LPCTSTR(res_id));
		wstring file_path = temp_path + res_name;
		if (ExtractResource(file_path.data(), res_type, res_name.data())) {
			resources[res_id] = file_path;
			result = file_path;
		}else
			result = NO_SUCH_RESOURCE;
	}
	return WStringToString(result.data());
}

void DisposeAllResources() {
	for (auto iter : resources) {
		char* path = WStringToString(iter.second.data());
		if (IsFileExists(path)) {
			remove(path);
		}
	}
	resources.clear();
}