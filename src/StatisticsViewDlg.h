#pragma once


// CStatisticsViewDlg 对话框

class CStatisticsViewDlg : public CDialog
{
	DECLARE_DYNAMIC(CStatisticsViewDlg)

public:
	CStatisticsViewDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CStatisticsViewDlg();

// 对话框数据
	enum { IDD = IDD_DIALOG_STATIS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	void  StartTimer();
	void  SetChannelNumber(int nChannel) { m_nChannel = nChannel; }
	//void  SetFrameSize(CSize imgSize) { m_nImgWidth = imgSize.cx; m_nImgHeight = imgSize.cy; }

	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT nIDEvent);

protected:
	CWnd * m_pwndParent;
	int    m_nChannel;
	UINT_PTR m_nTimer;
	//int m_nImgWidth;
	//int m_nImgHeight;
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnNcDestroy();
};
