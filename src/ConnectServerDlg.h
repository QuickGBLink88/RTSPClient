#pragma once


// CConnectServerDlg 对话框

class CConnectServerDlg : public CDialog
{
	DECLARE_DYNAMIC(CConnectServerDlg)

public:
	CConnectServerDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CConnectServerDlg();

	CString  m_strIP;
	UINT     m_nChan;

// 对话框数据
	enum { IDD = IDD_DIALOG_CONNECT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
};
