
#include "stdafx.h"
#include <string.h>
BOOL DisplayPair(LPTSTR, DWORD, LPBYTE, DWORD, LPBOOL);
DWORD Filter(LPEXCEPTION_POINTERS, LPDWORD);

int _tmain(int argc, LPTSTR argv[])

{
	HKEY hKey;
	LONG result;
	DWORD valueType, index;
	DWORD numSubKeys, maxSubKeyLen, numValues, maxValueNameLen, maxValueLen;
	DWORD subKeyNameLen, valueNameLen, valueLen;
	FILETIME lastWriteTime;
	LPTSTR valueName;
	LPBYTE value;
	DWORD cbData;
	DWORD eCategory;


	RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment"), 0, KEY_READ | KEY_WRITE, &hKey);

	RegQueryInfoKey(hKey, NULL, NULL, NULL, &numSubKeys, &maxSubKeyLen, NULL, &numValues, &maxValueNameLen, &maxValueLen, NULL, &lastWriteTime);

	_tprintf(_T("\nSubKey Count: %d, Values Count: %d\n"), numSubKeys, numValues); //by me for info

	valueName = (LPTSTR)malloc(TSIZE * (maxValueNameLen + 1));
	value = (LPBYTE)malloc(maxValueLen);      /* size in bytes */



	for (index = 0; index < numValues; index++) {
		valueNameLen = maxValueNameLen + 1; /* A very common bug is to forget to set */
		valueLen = maxValueLen + 1; /* these values; both are in/out params */
		//enumerates values for the specified key
		result = RegEnumValue(hKey, index,
			valueName, &valueNameLen, //value name buffer and its length in characters
			NULL, &valueType,
			value, &valueLen);//value buffer and its length in bytes
		if (result == ERROR_SUCCESS && _tcscmp(_T("Path"), valueName) == 0) {
			_tprintf(_T("\n===================================\n"));
			DisplayPair(valueName, valueType, value, valueLen, 0);
			_tprintf(_T("\n===================================\n"));

			LPTSTR arg = (LPTSTR)_T("Lab_Task_01");
			wchar_t string[80];
			wcscpy(string, _T(";"));
			wcscat(string, arg);
			LPTSTR path_to_add = wcscat(string, L"");
			LPTSTR new_value = (LPTSTR)value;


			LPCWSTR v_Name = (LPCWSTR)_T("tester");


			if (lstrcat(new_value, (LPTSTR)path_to_add) != NULL)
			{
				_tprintf(_T("%s"), v_Name);
			}


			LPBYTE new_value_bytes = (LPBYTE)new_value;

			_tprintf(_T("\n\n%s"), (LPTSTR)new_value_bytes);
			cbData = _tcsclen(new_value) + 1;
			__try {
				result = RegSetValueEx(
					hKey,
					v_Name,
					0,
					REG_SZ,
					new_value_bytes,
					cbData
				);

			}
			__except (Filter(GetExceptionInformation(), &eCategory)) {
				ReportError((LPCTSTR)_T("Nothing happended here"), 0, TRUE);
			}

			if (result == ERROR_SUCCESS)
			{
				_tprintf(_T("IT WORKED"));
			}


		}
	}
	free(valueName);
	free(value);
	getchar();
	return 0;
}


BOOL DisplayPair(LPTSTR valueName, DWORD valueType,
	LPBYTE value, DWORD valueLen,
	LPBOOL flags)

	/* Function to display key-value pairs. */

{

	LPBYTE pV = value;
	DWORD i;

	_tprintf(_T("\n%s = "), valueName);
	switch (valueType) {
	case REG_FULL_RESOURCE_DESCRIPTOR: /* 9: Resource list in the hardware description */
	case REG_BINARY: /*  3: Binary data in any form. */
		for (i = 0; i < valueLen; i++, pV++)
			_tprintf(_T(" %x"), *pV);
		break;

	case REG_DWORD: /* 4: A 32-bit number. */
		_tprintf(_T("%x"), (DWORD)*value);
		break;

	case REG_EXPAND_SZ: /* 2: null-terminated string with unexpanded references to environment variables (for example, �%PATH%�). */
	case REG_SZ: /* 1: A null-terminated string. */
		_tprintf(_T("%s"), (LPTSTR)value);
		break;
	case REG_MULTI_SZ: /* 7: An array of null-terminated strings, terminated by two null characters. E.g. string01\0string02\0..\0\0 */
		_tprintf(_T("Begin Multiple String:\n"));
		while (*pV) {
			_tprintf(_T("\t\t%s"), (LPTSTR)pV);
			pV = pV + _tcslen((LPTSTR)pV) + 1;//+1 for null char

		}
		_tprintf(_T("End Multiple String:\n"));
		break;
	case REG_DWORD_BIG_ENDIAN: /* 5:  A 32-bit number in big-endian format. */
		for (i = valueLen - 1; i >= 0; i--) //MSB ...LSB
			_tprintf(_T(" %x"), pV[i]);//*(pV+i)
		break;
	case REG_LINK: /* 6: A Unicode symbolic link. */
		_tprintf(_T("%s"), (LPTSTR)value);
		break;
	case REG_NONE: /* 0: No defined value type. */
		_tprintf(_T("No Defined Type"));
		break;
	case REG_RESOURCE_LIST: /* 8: A device-driver resource list. */
	default: _tprintf(_T(" ** Cannot display value of type: %d. Exercise for reader\n"), valueType);
		break;
	}

	return TRUE;
}


static DWORD Filter(LPEXCEPTION_POINTERS pExP, LPDWORD eCategory)

/*	Categorize the exception and decide whether to continue execution or
	execute the handler or to continue the search for a handler that
	can process this exception type. The exception category is only used
	by the exception handler. */
{
	DWORD exCode;
	DWORD_PTR readWrite, virtAddr;
	exCode = pExP->ExceptionRecord->ExceptionCode;

	_tprintf(_T("Filter. exCode: %x\n"), exCode);

	if ((0x20000000 & exCode) != 0) { //bit 29 is set for user generated exception
				/* User Exception. */
		*eCategory = 10;
		return EXCEPTION_EXECUTE_HANDLER;
	}

	switch (exCode) {
	case EXCEPTION_ACCESS_VIOLATION:
		/* Determine whether it was a read, write, or execute
			and give the virtual address. */
			//0 means read, 1 means write, 8 means user-mode dataexecution prevention (DEP) violation.
		readWrite =
			(DWORD)(pExP->ExceptionRecord->ExceptionInformation[0]);
		//virtual address that caused the exception
		virtAddr =
			(DWORD)(pExP->ExceptionRecord->ExceptionInformation[1]);
		_tprintf
		(_T("Access Violation. Read/Write/Execute: %d. Address: %x\n"),
			readWrite, virtAddr);
		*eCategory = 1;
		return EXCEPTION_EXECUTE_HANDLER;
	case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
		*eCategory = 1;
		return EXCEPTION_EXECUTE_HANDLER;
		/* Integer arithmetic exception. Halt execution. */
	case EXCEPTION_INT_DIVIDE_BY_ZERO:
	case EXCEPTION_INT_OVERFLOW:
		*eCategory = 2;
		return EXCEPTION_EXECUTE_HANDLER;

	case EXCEPTION_DATATYPE_MISALIGNMENT:
		*eCategory = 4;
		return EXCEPTION_CONTINUE_SEARCH;
	case STATUS_NONCONTINUABLE_EXCEPTION:
		*eCategory = 5;
		return EXCEPTION_EXECUTE_HANDLER;
	case EXCEPTION_ILLEGAL_INSTRUCTION:
	case EXCEPTION_PRIV_INSTRUCTION:
		*eCategory = 6;
		return EXCEPTION_EXECUTE_HANDLER;
	case STATUS_NO_MEMORY:
		*eCategory = 7;
		return EXCEPTION_EXECUTE_HANDLER;
	default:
		*eCategory = 0;
		return EXCEPTION_CONTINUE_SEARCH;
	}
}