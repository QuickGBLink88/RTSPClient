// StatisticsViewDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "StreamRecv.h"
#include "Resource.h"
#include "StatisticsViewDlg.h"
#include "MainFrm.h"
#include "H264.h"



#define  TIMER_UPDATE_STATIS  3


// CStatisticsViewDlg 对话框

IMPLEMENT_DYNAMIC(CStatisticsViewDlg, CDialog)
CStatisticsViewDlg::CStatisticsViewDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CStatisticsViewDlg::IDD, pParent)
{
	m_nChannel = 0;
	m_pwndParent = pParent;
}

CStatisticsViewDlg::~CStatisticsViewDlg()
{
}

void CStatisticsViewDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CStatisticsViewDlg, CDialog)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDOK, OnBnClickedOk)

END_MESSAGE_MAP()


// CStatisticsViewDlg 消息处理程序

BOOL CStatisticsViewDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_nTimer = -1;

	return TRUE;  

}

void CStatisticsViewDlg::OnTimer(UINT nIDEvent)
{
	CDialog::OnTimer(nIDEvent);

	if(!IsWindowVisible())
		return;

	//if(m_nChannel < 0)
	//	return;


	DECODE_STATIS  statis;

	if(!((CMainFrame*)m_pwndParent)->GetDecodeStatis(statis))
	{
         return;
	}


	CString str;

	switch(statis.nVideoFormat)
	{
	case VFMT_MPEG2:
		str.Format("Video Format: MPEG2");
		break;
	case VFMT_MPEG4:
		str.Format("Video Format: MPEG4");
		break;
	case VFMT_H264:
		str.Format("Video Format: H264");
		break;
	case VFMT_H265:
		str.Format("Video Format: H265");
		break;
	default:
		str.Format("Video Format: Unknown");
		break;
	}

	GetDlgItem(IDC_STATIC_VF)->SetWindowText(str);

	switch(statis.nAudioFormat)
	{
	case AFMT_MP2:
		str.Format("Audio Format: MP2");
		break;
	case AFMT_MP3:
		str.Format("Audio Format: MP3");
		break;
	case AFMT_PCM_ULAW:
		str.Format("Audio Format: G711-u");
		break;
	case AFMT_PCM_ALAW:
		str.Format("Audio Format: G711-a");
		break;
	case AFMT_AMR:
		str.Format("Audio Format: AMR");
		break;
	case AFMT_AAC:
		str.Format("Audio Format: AAC");
		break;
	default:
		str.Format("Audio Format: Unknown");
		break;
	}

	GetDlgItem(IDC_STATIC_AUDIO_FORMAT)->SetWindowText(str);

	str.Format("Picture Size: %d x %d", statis.nWidth, statis.nHeight);

	GetDlgItem(IDC_STATIC_WH)->SetWindowText(str);

		
	str.Format("Bitrate: %ld kb/s", statis.dwBitrate);
	GetDlgItem(IDC_STATIC_BPS)->SetWindowText(str);


	str.Format("Framerate: %d", statis.nFramerate);
	GetDlgItem(IDC_STATIC_FPS)->SetWindowText(str);


	//TRACE("CStatisticsViewDlg::OnTimer()...\n");

}

void  CStatisticsViewDlg::StartTimer()
{
	if(m_nTimer > 0)
	{
		KillTimer(TIMER_UPDATE_STATIS);
		m_nTimer = -1;
	}

    m_nTimer = SetTimer(TIMER_UPDATE_STATIS, 2000, NULL);

   	CString str;

	str.Format("Framerate: %d", 0);
	GetDlgItem(IDC_STATIC_FPS)->SetWindowText(str);

	str.Format("Picture size: %d * %d",0, 0);
	GetDlgItem(IDC_STATIC_WH)->SetWindowText(str);

	str.Format("Bitrate: %ld kb/s", 0);
	GetDlgItem(IDC_STATIC_BPS)->SetWindowText(str);

	GetDlgItem(IDC_STATIC_VF)->SetWindowText("Video Format: ");
	GetDlgItem(IDC_STATIC_AUDIO_FORMAT)->SetWindowText("Audio Format: ");
}

void CStatisticsViewDlg::OnBnClickedOk()
{
	KillTimer(TIMER_UPDATE_STATIS);
    m_nTimer = -1;

    ShowWindow(SW_HIDE);
	//OnOK();
}


