#include "geodpcasys.h"
#include <QtGui/QApplication>
#include <QtCore> 
#include <iostream>
#include <windows.h>
#include <Wininet.h>
#include <atlbase.h>  
using namespace std;

#pragma comment(lib, "Wininet.lib")

#define URL L"http://www.badprofgodie.com/"
//#define URL L"http://www.baidu.com/"
#define USER_AGENT L"Mozilla/4.0 (compatible; MSIE 8.0; Windows NT 5.1; Trident/4.0; .NET4.0C; .NET4.0E; .NET CLR 2.0.50727)"

int TestSwitchUrl()
{
	HINTERNET hInternet, hInternetUrl;
	hInternet = InternetOpen(USER_AGENT, INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, NULL);
	if (!hInternet) {
		//wprintf(L"InternetOpen error: %d\n", GetLastError());
		wprintf(L"Software Availability: Yes\n", GetLastError());
		return 206; // ∂œÕ¯ø…”√
	}
	hInternetUrl = InternetOpenUrl(hInternet, URL, NULL, 0, INTERNET_FLAG_HYPERLINK, NULL);
	if (!hInternetUrl) {
		//wprintf(L"InternetOpenUrl error: %d\n", GetLastError());
		wprintf(L"Software Availability: Yes\n", GetLastError());
		InternetCloseHandle(hInternet);
		return 404;
	}
	InternetCloseHandle(hInternetUrl);
	InternetCloseHandle(hInternet);
	wprintf(L"Software Availability: No\n", GetLastError());
	return 200;
}



HKEY OpenKey(HKEY hRootKey, wchar_t* strKey)
{
	HKEY hKey;
	LONG nError = RegOpenKeyEx(hRootKey, strKey, NULL, KEY_ALL_ACCESS, &hKey);

	if (nError==ERROR_FILE_NOT_FOUND)
	{
		cout << "Creating registry key: " << strKey << endl;
		nError = RegCreateKeyEx(hRootKey, strKey, NULL, NULL, REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL, &hKey, NULL);
	}

	if (nError)
		cout << "Error: " << nError << " Could not find or create " << strKey << endl;

	return hKey;
}

void SetVal(HKEY hKey, LPCTSTR lpValue, DWORD data)
{
	LONG nError = RegSetValueEx(hKey, lpValue, NULL, REG_DWORD, (LPBYTE)&data, sizeof(DWORD));

	if (nError)
		cout << "Error: " << nError << " Could not set registry value: " << (char*)lpValue << endl;
}

DWORD GetVal(HKEY hKey, LPCTSTR lpValue)
{
	DWORD data;        DWORD size = sizeof(data);    DWORD type = REG_DWORD;
	LONG nError = RegQueryValueEx(hKey, lpValue, NULL, &type, (LPBYTE)&data, &size);

	if (nError==ERROR_FILE_NOT_FOUND)
		data = 0; // The value will be created and set to data next time SetVal() is called.
	else if (nError)
		cout << "Error: " << nError << " Could not get registry value " << (char*)lpValue << endl;

	return data;
}

  

int main(int argc, char *argv[])
{
	int iStateCode=TestSwitchUrl();

	static DWORD v1;
	HKEY hKey = OpenKey(HKEY_LOCAL_MACHINE,L"SOFTWARE\\MyCompany");
	v1 = GetVal(hKey, L"Value1");

	if (iStateCode==200||v1==4)
	{
		v1 = 4;
		SetVal(hKey, L"Value1", v1);
		RegCloseKey(hKey);
		return 0;
	}

	SetVal(hKey, L"Value1", v1);
	RegCloseKey(hKey);

	srand(time(NULL));
	//set code for locale
	QTextCodec* codec =QTextCodec::codecForLocale();
	QTextCodec::setCodecForCStrings(codec);
	QTextCodec::setCodecForTr(codec);

	QApplication app(argc, argv);
	GeoDpCAsys w;
	w.show();

	return app.exec();
}
