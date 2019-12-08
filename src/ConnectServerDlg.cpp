// ConnectServerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "StreamRecv.h"
#include "ConnectServerDlg.h"

// CConnectServerDlg 对话框

IMPLEMENT_DYNAMIC(CConnectServerDlg, CDialog)
CConnectServerDlg::CConnectServerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CConnectServerDlg::IDD, pParent)
{
	m_strURL = "";
}

CConnectServerDlg::~CConnectServerDlg()
{
}

void CConnectServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CConnectServerDlg, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()


// CConnectServerDlg 消息处理程序
BOOL CConnectServerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	GetDlgItem(IDC_EDIT_CONNECT_URL)->SetWindowText(m_strURL);

	return TRUE;  

}

void CConnectServerDlg::OnBnClickedOk()
{
	GetDlgItem(IDC_EDIT_CONNECT_URL)->GetWindowText(m_strURL);

	if(m_strURL.IsEmpty() || (m_strURL.GetLength() < 7) || _strnicmp(m_strURL, "rtsp://", 7) != 0)
	{
         MessageBox("URL地址无效", "提示", MB_OK|MB_ICONWARNING);
		 return;
	}
	
	OnOK();
}


