
//////////////////////////////////////////////////////////////////////////
//≈‰÷√SectionName
DWORD P_GetProfileString(LPCTSTR lpAppName,LPCTSTR lpKeyName,LPTSTR lpReturnedString,DWORD nSize);
BOOL  P_WriteProfileString(LPCTSTR lpAppName,LPCTSTR lpKeyName,LPCTSTR lpString);
UINT  P_GetProfileInt(LPCTSTR lpAppName,LPCTSTR lpKeyName,INT nDefault);
BOOL  P_WriteProfileInt(LPCTSTR lpAppName,LPCTSTR lpKeyName,INT iValue);
