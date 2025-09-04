#include "stdafx.h"
#include <comdef.h>

CDxException::CDxException(HRESULT hr, const std::wstring& functionName, const std::wstring& fileName, int lineNumber)
	: error_code{hr}, function_name{functionName}, file_name{fileName}, line_number{lineNumber}
{
}

std::wstring CDxException::ToString() const
{
	// error code에 대한 설명을 얻음
	_com_error err{ error_code };
	std::wstring msg = err.ErrorMessage();

	return function_name + L" failed in " + file_name + L"; line " + std::to_wstring(line_number) + L"; error: " + msg;
}