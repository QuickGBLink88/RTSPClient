// StreamRecv.h : NSPlayer 应用程序的主头文件
//
#pragma once

#ifndef __AFXWIN_H__
	#error 在包含用于 PCH 的此文件之前包含“stdafx.h” 
#endif

#include "resource.h"       // 主符号

#define DM_SPEC      WM_APP+3
#define XCM_SPEC     WM_APP+4
#define NS_SPEC      WM_APP+5
#define GETSPEC      0
#define SETSPEC      1　　　
#define SETNBOD      2
#define SETNSPEC     3     
#define GETNSPEC     4


typedef struct sNSPARAM
{
char  ipAddr[32];
int   input;
float contrast;
float brightness;
float hue;
float saturation;
}NSPARAM;


//startup parameters
typedef struct sAppParam
{

  LONG DmanID;
  RECT geom; //窗口位置
  BOOL border;
  char  ipAddr[256];//服务器IP
  int   input;//通道号
}AppParam;

// CNSPlayerApp:
// 有关此类的实现，请参阅 StreamRecv.cpp
//

class CNSPlayerApp : public CWinApp
{
public:
	CNSPlayerApp();

	//void  processParams(int argc, char* argv[], AppParam *param);

// 重写
public:
	virtual BOOL InitInstance();

// 实现

public:
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
	virtual int ExitInstance();
};

extern CNSPlayerApp theApp;
