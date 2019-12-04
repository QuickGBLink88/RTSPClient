// MainFrm.cpp : CMainFrame 类的实现
//
/**
** Blog:  https://blog.csdn.net/quickgblink
** Email: quickgblink@126.com
**/

#include "stdafx.h"
#include "StreamRecv.h"

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#include <stdio.h>
#include <mmsystem.h>
#include "MainFrm.h"
#include "StatisticsViewDlg.h"
#include "PProfile.h"
#include "StatisticsViewDlg.h"
#include <math.h>
#include "rtsp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



#define  WM_START_STREAM         WM_USER + 104
#define  WM_STOP_STREAM          WM_USER + 105


#define  TIMER_CLIENT       20


#define  WM_SET_BORDER_STYLE      WM_USER + 21

#define  WM_COMM_MESSAGE         WM_USER + 111


time_t m_tStarted = 0;
UINT   m_uBytesWritten = 0;
ULONG  gChannelTotalLength;
ULONG  StartTime;
BOOL   g_border = 1;


CMainFrame         *  gpMainFrame = NULL;

CStatisticsViewDlg  *  m_pViewStatisDlg = NULL;


LRESULT CALLBACK OnDisplayVideo(int devNum,PBYTE pRgb, int dwSize); //图像RGB24数据回调


DWORD WINAPI NewYUV420(ULONG SrcAddr, int Slot, int Port, int Linesize, int Width, int Height, ULONGLONG Pts, BYTE *pBuf); //YUV420回调

DWORD WINAPI RtspStreamCallback(BYTE *pBuf, long Buflen, ULONG lTimeStamp, int nFormat, int nFrameType, LPVOID lpContext); //RTSP数据回调函数


// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()
	//ON_WM_SETFOCUS()
	ON_WM_TIMER()
	ON_WM_DESTROY()
    ON_COMMAND_RANGE(4700,4730, OnClickMenu)
    ON_UPDATE_COMMAND_UI_RANGE(4700, 4730, OnUpdateMenuUI)
	ON_MESSAGE(WM_COMM_MESSAGE, OnProcessCommnMessage)
	ON_MESSAGE(WM_START_STREAM, OnStartReceiveStream)
	ON_MESSAGE(WM_STOP_STREAM, OnStopReceiveStream)
	ON_WM_SIZE()
	ON_WM_MOVE()
	ON_WM_MOVING()
END_MESSAGE_MAP()


BOOL GetHostInfo(char * outIP, char * outName)
{
	char   name[256];
	if (gethostname(name, 256) == 0)
	{
		if (outName)
		{
			strcpy(outName,  name);
		}

		PHOSTENT  hostinfo;
		if ((hostinfo = gethostbyname(name)) != NULL)
		{
			LPCSTR pIP = inet_ntoa (*(struct in_addr *)*hostinfo->h_addr_list);
			strcpy(outIP,  pIP);
			return TRUE;
		}
	}
	return FALSE;
}

// CMainFrame 构造/析构

CMainFrame::CMainFrame()
{
	gpMainFrame = this;
	m_pViewStatisDlg = NULL;
   
	m_bRecording  = 0;
    m_nFPS = 0;
	m_dwBPS = 0;
		                 
	m_fp = INVALID_HANDLE_VALUE;
	m_bRecording = FALSE; //录制文件

	m_pAudioOutBuf = new BYTE[AVCODEC_MAX_AUDIO_FRAME_SIZE];
	m_width = 0;  
	m_height = 0; 
	m_bStretch = FALSE;
	m_DrawDib = NULL;
}

CMainFrame::~CMainFrame()
{
  if(m_pAudioOutBuf)
  {
	  delete m_pAudioOutBuf;
	  m_pAudioOutBuf = NULL;
  }
}


int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	//if (!m_wndView.Create(NULL, NULL, AFX_WS_DEFAULT_VIEW,
	//	CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST, NULL))
	//{
	//	TRACE0("未能创建视图窗口\n");
	//	return -1;
	//}

	SetWindowText("RTSP Player (https://blog.csdn.net/quickgblink)");

	RECT rc;
	GetClientRect(&rc);

	RECT rcVideoWindow = rc;
	if(!m_Decoder.Create(NULL, NULL, WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN, rcVideoWindow, this, IDC_VIDEO_SHOW_WINDOW))
	{
        return -1;
	}

	m_Decoder.SetupVideoWindow(m_Decoder.GetSafeHwnd());


    SetWindowPos(NULL, 0, 0, 400, 350, SWP_NOZORDER|SWP_NOMOVE);
	
  //  OnBnConnect();//连接服务器
	
	return 0;
}

void CMainFrame::OnDestroy()
{
	KillTimer(TIMER_CLIENT);

    OnStopReceiveStream(0, 0);

	if(m_pViewStatisDlg)
		delete m_pViewStatisDlg;

	Sleep(100);
	
	CFrameWnd::OnDestroy();

	TRACE("OnDestroy end.\n");
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;

	// TODO: 在此处通过修改 CREATESTRUCT cs 来修改窗口类或样式

	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	cs.lpszClass = AfxRegisterWndClass(0);
	cs.style |= WS_CLIPCHILDREN;

	return TRUE;
}


// CMainFrame 诊断

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG



void CMainFrame::OnSetFocus(CWnd* /*pOldWnd*/)
{
	// 将焦点前移到视图窗口
	//m_wndView.SetFocus();
}

BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	//if (m_wndView.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
	//	return TRUE;

	return CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}



void CMainFrame::OnBnConnect()
{

	OnStopReceiveStream(0, 0);
	OnStartReceiveStream(0, 0);
}

void CMainFrame::OnBnDisconnect()
{

	OnStopReceiveStream(0, 0);

	m_Decoder.Invalidate(1);
}

void CMainFrame::OnStatisticsView()
{
	if(m_pViewStatisDlg == NULL)
	{
         m_pViewStatisDlg = new CStatisticsViewDlg(this);
		 m_pViewStatisDlg->Create(IDD_DIALOG_STATIS, this);
	}
	//m_pViewStatisDlg->SetChannelNumber(m_nChannelNum);
	m_pViewStatisDlg->StartTimer();
	m_pViewStatisDlg->ShowWindow(SW_SHOW);
}



void CMainFrame::OnClickMenu(UINT uMenuId)
{
	// Parse the menu command
	switch (uMenuId)
	{
		case IDM_CONNECT: //连接
		{
			 OnBnConnect();
		}
		break;
		
		case IDM_DISCONNECT: //断开连接
		{
            OnBnDisconnect();  
		}
		break;
		
		case IDM_RECORD_VIDEO: //录像
		{
			m_bRecording = !m_bRecording;
			if(!m_bRecording)
			{
				::CloseHandle(m_fp);
				m_fp = INVALID_HANDLE_VALUE;
			}
			else
			{				 
				 m_fp = CreateFile("D:\\ipc_192.168.0.64.h264", GENERIC_WRITE, 0, /*&saAttr*/NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

				 if(m_fp == INVALID_HANDLE_VALUE)  //无法创建文件
				 {
					 m_bRecording = FALSE;
				 }
			}
		}
        break;

		case IDM_SET_NOBORDER:
		{
			
		}
		break;

		case IDM_VIEW_STATIS:
		{
            OnStatisticsView();
		}
		break;


		//case IDM_OSD1:
		//case IDM_OSD2:
		//case IDM_OSD3:
		//	{
		//		HandleMenuID(uMenuId, m_ptMenuClickPoint);
		//	}
		//	break;

		case IDM_OPEN_AUDIO: //是否打开音频
			{

			}
			break;


		case IDM_STRETCH_VIDEO: // 是否缩放图像
			{
				m_bStretch = !m_bStretch;
			
			}
			break;

	}//switch	 
}


void CMainFrame:: OnUpdateMenuUI(CCmdUI* pCmdUI)
{
  switch(pCmdUI->m_nID)
 {
  case IDM_CONNECT:
	  {	 
		//pCmdUI->Enable(1); 
	  }
     break;
  case IDM_DISCONNECT:
	  {
		//pCmdUI->Enable(1);	 
	  }
	 break;
  case IDM_SET_NOBORDER:
	  {
        pCmdUI->SetCheck(!g_border);
	  }
	  break;

  case IDM_RECORD_VIDEO: //是否录像
	 {
		 pCmdUI->SetCheck(m_bRecording); 
	 }
	 break;

  case IDM_OPEN_AUDIO: //是否打开音频
		{

		}
		break;
  case IDM_STRETCH_VIDEO: // 是否缩放图像
	  {
		  pCmdUI->SetCheck(m_bStretch);
	  }
	  break;
 }//switch

}

static  BOOL g_bCaptureCursor = FALSE;
POINT   g_pCapturePt;


// We use  "Esc" key to restore from fullscreen mode
BOOL CMainFrame::PreTranslateMessage(MSG* pMsg) 
{
	
	switch(pMsg->message)
	{
 
		case WM_LBUTTONDBLCLK:
		{
			//CRect rcVideo;
			//::GetWindowRect(m_Decoder.m_hWnd, rcVideo);
			////ScreenToClient(rcVideo);

			//POINT pt;
			//::GetCursorPos(&pt);
			//if (rcVideo.PtInRect(pt))
			//{
			//  ::SendMessage(m_hWnd, WM_SHOW_MAXIMIZED, 0, 0);
			//  m_bFullScreen = TRUE;
			//}	
			//    
			//return TRUE;
		}
		break;
		
		case WM_RBUTTONDOWN:
			{
				LONG style = GetWindowLong(m_hWnd, GWL_STYLE);
				if(style & (WS_DLGFRAME | WS_THICKFRAME | WS_BORDER))
				{
					CMenu menu;
					menu.CreatePopupMenu();
					menu.AppendMenu(MF_STRING, IDM_OSD1, "Test1");
					menu.AppendMenu(MF_STRING, IDM_OSD2, "Test2");
					menu.AppendMenu(MF_STRING, IDM_OSD3, "Test3");

					POINT pt;
					::GetCursorPos(&pt);
					menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, this);

	                ScreenToClient(&pt);
					m_ptMenuClickPoint = pt;
				}

			}
			break;
			
		case WM_LBUTTONDOWN:
		{
			LONG style = GetWindowLong(m_hWnd, GWL_STYLE);
			if(style & (WS_DLGFRAME | WS_THICKFRAME | WS_BORDER))
			{
				return TRUE;
			}

			SetCapture();
			g_bCaptureCursor = TRUE;
			//TRACE("SetCapture() \n");
				 
			POINT pt;
			pt = pMsg->pt;
			ScreenToClient(&pt);

			int nCaptionHeight = 0;
			if(g_border)
				nCaptionHeight = GetSystemMetrics(SM_CYSMCAPTION);

			g_pCapturePt.x   = pt.x;
			g_pCapturePt.y   = pt.y + nCaptionHeight;

		}
		break;

		case WM_LBUTTONUP:
		{
			ReleaseCapture();
			g_bCaptureCursor = FALSE;
			//TRACE("ReleaseCapture() \n");
		}
			break;

		case WM_MOUSEMOVE:
		{
			if(!g_bCaptureCursor)
				return TRUE;

				SetWindowPos(NULL, 
					pMsg->pt.x - g_pCapturePt.x,
					pMsg->pt.y - g_pCapturePt.y, 
					0, 0, SWP_NOZORDER|SWP_NOSIZE);
			  
			return TRUE;
		}
			break;

		case WM_KEYDOWN:
		{
			if(::GetAsyncKeyState(VK_F5))
			{

			}
	       
		}
		  break;

	}//switch

	return CFrameWnd::PreTranslateMessage(pMsg);
}



void CMainFrame::OnSize(UINT nType, int cx, int cy)
{
	CFrameWnd::OnSize(nType, cx, cy);

	CRect rc;
    GetClientRect(rc);
	m_Decoder.MoveWindow(0, 0, rc.Width(), rc.Height());
}


void CMainFrame::OnMove(int x, int y)
{
	CFrameWnd::OnMove(x, y);
	//TRACE("OnMove() \n");
}

void CMainFrame::OnMoving(UINT fwSide, LPRECT pRect)
{
	CFrameWnd::OnMoving(fwSide, pRect);
	//TRACE("OnMoveing() \n");
}


LRESULT CMainFrame::OnProcessCommnMessage(WPARAM wParam, LPARAM lParam)
{
	if(wParam == 0)
	{
		//AfxMessageBox("OnProcessCommnMessage() is called.");
		return 0;
	}
	else if(wParam == 1)
	{

	}
	return 0;
}



void CMainFrame::OnTimer(UINT nIDEvent)
{
	CFrameWnd::OnTimer(nIDEvent);

}

void   CMainFrame::HandleMenuID(UINT  nOSDMenuID, POINT pt)
{
	switch(nOSDMenuID)
	{
	case IDM_OSD1:
		break;
	case IDM_OSD2:
		break;
	case IDM_OSD3:
		break;
	}
}
  

DWORD WINAPI NewYUV420(ULONG SrcAddr, int Slot, int Port, int Linesize, int Width, int Height, ULONGLONG Pts, BYTE *pBuf)
{
	return 0;
}


void CMainFrame::OnVideoFrame(PBYTE pData, DWORD dwSize, INT64 lTimeStamp, VideoFormat vFormat, int nFrameType)
{
	//CAutoLock lock(&m_csDecoder);

	CalculateFPS();//计算帧率

    m_Decoder.OnDecodeVideo(pData, dwSize, vFormat, nFrameType); //解码视频

}


void  CMainFrame:: OnAudioFrame(PBYTE pData, DWORD dwSize, INT64 lTimeStamp, AudioFormat aFormat)
{
	int nBufLen = 0;
	AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;  

   m_Decoder.OnDecodeAudio(pData, dwSize, aFormat, out_sample_fmt, m_pAudioOutBuf, &nBufLen); //解码音频
}

LRESULT CALLBACK OnDisplayVideo(int devNum, PBYTE pRgb, int dwSize)
{
	gpMainFrame->OnDisplayFrame(pRgb, dwSize);
	return 0;
}

   
void  CMainFrame:: OnDisplayFrame(PBYTE pRgb, DWORD dwSize)
{
	//TRACE("OnDisplayFrame: %d \n", dwSize);

	if(m_width == 0 || m_height == 0)
	{
	    CSize PicSize;
		if(!m_Decoder.GetVideoSize(PicSize))
		{
			return;
		}
		int nPicWidth, nPicHeight;
		nPicWidth  = PicSize.cx;
		nPicHeight = PicSize.cy;
	
		m_width  = nPicWidth;
		m_height = nPicHeight;

	    m_DrawDib = DrawDibOpen();
	}

	if(m_DrawDib != NULL && m_width > 0 && m_height > 0)
	{
	    display_pic(pRgb, m_width, m_height);
	}
}

//函数： init_bm_head
//作用： 初始化RGB图像的位图信息头部
void CMainFrame::init_bm_head(int cx, int cy)  
{  
	bmiHeader.biSize = sizeof(BITMAPINFOHEADER);  
	bmiHeader.biWidth  = cx;  
	bmiHeader.biHeight =  cy;   
	bmiHeader.biPlanes       = 1;  
	bmiHeader.biBitCount     = 24;  
	bmiHeader.biCompression  = BI_RGB;  
	bmiHeader.biSizeImage    = 0;  
	bmiHeader.biClrUsed      = 0;  
	bmiHeader.biClrImportant = 0;  
}  


void CMainFrame::display_pic(unsigned char* data, int width, int height)  
{  
	CRect  rc;  
	CDC * pDC =  m_Decoder.GetDC();  //获取视频预览窗口的DC
	m_Decoder.GetClientRect(&rc);  

	//if(m_height != height || m_width != width)
	//{  
	//	m_height = height;  
	//	m_width  = width;  
	//	MoveWindow(0, 0, width, height, 0);  
	//	Invalidate();  
	//}  
	init_bm_head(width, height);  

	//利用VFW来显示
	DrawDibDraw(m_DrawDib,  
		pDC->GetSafeHdc(),  
		rc.left,  
		rc.top,  
		-1, // don't stretch  
		-1,  
		&bmiHeader,   
		(void*)data,   
		0,   
		0,   
		width,   
		height,   
		0); 

	m_Decoder.ReleaseDC(pDC);
}

//nFormat = 0，不能识别的格式；1-10, 视频格式，值的定义见枚举类型VideoFormat；等于或大于11为音频格式，值的定义见枚举类型AudioFormat
//nFrameType=1，I帧，其他值表示非关键帧（P，B）
DWORD WINAPI RtspStreamCallback(BYTE *pBuf, long Buflen, ULONG lTimeStamp, int nFormat, int nFrameType, LPVOID lpContext)
{
	if(nFormat > 0 && nFormat <= 10)//video 
	{
		//static DWORD dwVideoTick = timeGetTime();
		//TRACE("video frame interval: %ld \n", timeGetTime() - dwVideoTick);
		//dwVideoTick = timeGetTime();

		gpMainFrame->OnVideoFrame(pBuf, Buflen, lTimeStamp, (VideoFormat)nFormat, nFrameType);
	}
	else if(nFormat > 10) //Audio
	{
		//static DWORD dwAudioTick = timeGetTime();
		//TRACE("audio frame interval: %ld \n", timeGetTime() - dwAudioTick);
		//dwAudioTick = timeGetTime();

		gpMainFrame->OnAudioFrame(pBuf, Buflen, lTimeStamp, (AudioFormat)nFormat);
	}
	return 0;
}


BOOL  GetRtspParams(const char * lpszUrl, char * cRtspAddr, char * cUserName, char * cPassWord, char * cStreamName, char * cIP, int & nPort)
{
    char cPrefix[10] = {0};
    char cRtpPort[32] = {0};
    nPort = 554;

    strncpy(cPrefix,  lpszUrl, strlen("RTSP://"));

    if(stricmp(cPrefix, "RTSP://") == 0)
    {
        char *pu8 = (char *)lpszUrl;
        char *pu82 = pu8;

        pu82 = pu8 + strlen("RTSP://");

        strcpy(cRtspAddr, pu82);  //192.168.60.106/mpeg4

        pu82 = strchr(pu82, ':');
        if(pu82 != NULL)
        {
            pu8 = pu8 + strlen("RTSP://");
            strncpy(cUserName, pu8, pu82 - pu8);
            pu8 = pu82 + 1;

            pu82 = strchr(pu8,'@');
            if(pu82 != NULL)
            {
                strncpy(cPassWord,pu8,pu82 - pu8);
                pu8 = pu82 + 1;
                strcpy(cRtspAddr, pu8);  //192.168.60.106/mpeg4
            }
            else
            {
                TRACE("can not find the @\n");
            }
        }
        else
        {
            TRACE("can not search the username and password\n");
        }

        pu8 =  strchr(cRtspAddr, '/');
        if(pu8)
        {
            strcpy(cStreamName, pu8+1);
            strncpy(cIP, cRtspAddr, pu8 - cRtspAddr);
        }
        else
        {
            strcpy(cIP, cRtspAddr);
        }

        pu8 = cIP;
        pu82 =  strchr(pu8, ':');//如果IP后跟着端口号,比如192.168.60.241:554，分离出IP地址和端口号
        if(pu82)
        {
            pu8 = pu82 + 1;
            strcpy(cRtpPort, pu8);
            *(pu82) = '\0';

            nPort = atoi(cRtpPort);
        }

        return TRUE;
    }

    return FALSE;

}

LRESULT  CMainFrame:: OnStartReceiveStream(WPARAM wParam, LPARAM lParam)
{
	char szURL[256] = {0};
	//char szFilePath[256] = {0};

	//从Config.INI文件中读取RTSP URL
	P_GetProfileString(_T("Client"), "URL", szURL, 256); //IPC的Rtsp地址
	//P_GetProfileString(_T("Client"), "file_path", szFilePath, 128); //保存的文件路径


    DWORD dwTick1;
	dwTick1 = GetTickCount();
	BOOL pass = FALSE;

	m_Decoder.SetDisplayCallback(OnDisplayVideo);
	m_Decoder.StartDecode();
	m_Decoder.SetupVideoWindow(m_Decoder.GetSafeHwnd());

	CString strURL = szURL;
	if(!(strURL.GetLength() > 7 && strURL.Left(7).CompareNoCase("rtsp://") == 0))
	{
		MessageBox("RTSP URL不正确", "错误", MB_OK||MB_ICONWARNING);
		return 1;
	}

	 char szSvrUrl[256]= {0};
    char cRtspAddr[256] = {0};
    char cUserName[64] = {0};
    char cPassWord[64] = {0};
    char cStreamName[256] = {0};
    char cIP[256] = {0};
    int nRtpPort = 554;

    if(GetRtspParams(strURL, cRtspAddr, cUserName, cPassWord, cStreamName, cIP, nRtpPort))
    {
        strcat(szSvrUrl, "rtsp://");
        strcat(szSvrUrl, cRtspAddr);
    }
    else
    {
        TRACE("RTSP url invalid!!!\n");
        assert(0);
        //return 1;
    }

	m_RtspClient.SetRtspCallback(RtspStreamCallback, this);
	m_RtspClient.SetAuthInfo(cUserName, cPassWord); //用户登录信息

	m_RtspClient.OpenStream(szSvrUrl); //连接RTSP服务器
 
  //////////////////////////////////////////////////////
	m_width = 0;  
	m_height = 0;
	StartTime = timeGetTime();
	gChannelTotalLength = 0;
	m_frmCount = 0;
	m_nFPS = 0;

	return (pass == TRUE)?0:1;
}

LRESULT  CMainFrame:: OnStopReceiveStream(WPARAM wParam, LPARAM lParam)
{
	m_RtspClient.CloseStream(); //关闭RTSP连接，停止收流

	Sleep(50);

	StartTime = 0;
	gChannelTotalLength = 0;

	m_bRecording = FALSE;
	if(m_fp != INVALID_HANDLE_VALUE)
	{
		::CloseHandle(m_fp);
		 m_fp = INVALID_HANDLE_VALUE;
	}

	m_Decoder.StopDecode();

	if (m_DrawDib)
	{
		DrawDibClose(m_DrawDib);
		m_DrawDib = NULL;
	}

	return 0;
} 


void CMainFrame::CalculateFPS()
{
	DWORD newtick;
	newtick = timeGetTime();

	m_frmCount++;
	int costtime = newtick - StartTime;
	if(costtime < 2000)
		return;
	{
		//TRACE("Frames input speed: %d\n", m_frmCount*1000/costtime);
		m_nFPS = m_frmCount*1000/costtime;
	}
	m_frmCount = 0;
	StartTime = newtick;
}

BOOL   CMainFrame:: GetDecodeStatis(DECODE_STATIS & statis)
{
	statis.nWidth = statis.nHeight = 0;

    CSize PicSize;
	if(m_RtspClient.GetVideoSize((int&)PicSize.cx, (int&)PicSize.cy))
	{
		statis.nWidth  = PicSize.cx;
		statis.nHeight = PicSize.cy;
	}
	else
	{
		m_Decoder.GetVideoSize(PicSize);
		statis.nWidth  = PicSize.cx;
		statis.nHeight = PicSize.cy;
	}
    statis.nFramerate = m_nFPS;
	statis.dwBitrate = 0; //暂时不统计码率信息
	
	statis.nVideoFormat = m_RtspClient.GetVideoFormat();
	statis.nAudioFormat = m_RtspClient.GetAudioFormat();
	
	return TRUE;
}