#include "stdafx.h"
#include "PProfile.h"
void GetConfigPathName(LPTSTR lpReturnedString,DWORD nSize)
{
	ZeroMemory(lpReturnedString,nSize);
	HMODULE hModule = GetModuleHandle(NULL);
	GetModuleFileName(hModule,lpReturnedString,nSize);

	TCHAR* p = strrchr(lpReturnedString,'\\');
	if(p != NULL)
		memcpy(p,"\\config.ini",sizeof("\\config.ini"));
}
DWORD P_GetProfileString(LPCTSTR lpAppName,LPCTSTR lpKeyName,LPTSTR lpReturnedString,DWORD nSize)
{
	TCHAR szFileName[MAX_PATH];
	GetConfigPathName(szFileName,MAX_PATH);
	return GetPrivateProfileString(lpAppName,lpKeyName,"",lpReturnedString,nSize,szFileName);
}

BOOL P_WriteProfileString(LPCTSTR lpAppName,LPCTSTR lpKeyName,LPCTSTR lpString)
{
	TCHAR szFileName[MAX_PATH];
	GetConfigPathName(szFileName,MAX_PATH);
	return WritePrivateProfileString(lpAppName,lpKeyName,lpString,szFileName);
}

UINT P_GetProfileInt(LPCTSTR lpAppName,LPCTSTR lpKeyName,INT nDefault)
{
	TCHAR szFileName[MAX_PATH];
	GetConfigPathName(szFileName,MAX_PATH);
	return GetPrivateProfileInt(lpAppName,lpKeyName,nDefault,szFileName);
}

BOOL P_WriteProfileInt(LPCTSTR lpAppName,LPCTSTR lpKeyName,INT iValue)
{
	TCHAR szFileName[MAX_PATH];
	GetConfigPathName(szFileName,MAX_PATH);
	TCHAR szNum[32];
	ZeroMemory(szNum,sizeof szNum);
	sprintf(szNum,"%d",iValue);    
	return WritePrivateProfileString(lpAppName,lpKeyName,szNum,szFileName);
}