// StreamRecv.cpp : 定义应用程序的类行为。
//

#include "stdafx.h"
#include "StreamRecv.h"
#include "MainFrm.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CNSPlayerApp

BEGIN_MESSAGE_MAP(CNSPlayerApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
END_MESSAGE_MAP()


// CNSPlayerApp 构造

CNSPlayerApp::CNSPlayerApp()
{

}


// 唯一的一个 CNSPlayerApp 对象

CNSPlayerApp theApp;

// CNSPlayerApp 初始化

BOOL CNSPlayerApp::InitInstance()
{

	InitCommonControls();

	CWinApp::InitInstance();

	// Init WinSock
	WSADATA   data;
	int ret = WSAStartup(0x0202, &data);
	if (ret != 0)
	{
		WSACleanup();
		return FALSE;
	}

	int argc = __argc;
	char **argv = __argv;
	//processParams(argc, argv, &g_appParam);

	SetRegistryKey(_T("应用程序向导生成的本地应用程序"));

	CMainFrame* pFrame = new CMainFrame;
	if (!pFrame)
		return FALSE;
	m_pMainWnd = pFrame;

	BOOL bSucceed = pFrame->LoadFrame(IDR_MAINFRAME,
		WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, NULL,
		NULL);

	if(!bSucceed)
		return FALSE;

	pFrame->ShowWindow(SW_SHOW);
	pFrame->UpdateWindow();

	return TRUE;
}


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// 用于运行对话框的应用程序命令
void CNSPlayerApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}


// CNSPlayerApp 消息处理程序


int CNSPlayerApp::ExitInstance()
{
	WSACleanup();

	return CWinApp::ExitInstance();
}
