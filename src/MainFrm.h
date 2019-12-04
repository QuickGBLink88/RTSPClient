// MainFrm.h : CMainFrame 类的接口
//
/**
** Blog: https://blog.csdn.net/quickgblink
** Email: quickgblink@126.com
**/

#pragma once

#include "ChildView.h"
#include <afxmt.h>
#include "VideoDisplayWnd.h"
#include "H264.h"
#include "Rtsp.h"
#include "AVDecoder.h"


#include <vfw.h>
#pragma comment(lib,"vfw32.lib")



class CMainFrame : public CFrameWnd
{
	
public:
	CMainFrame();
    virtual ~CMainFrame();

public:
	DWORD  GetBPS();

   void        OnVideoFrame(PBYTE pData, DWORD dwSize, INT64 lTimeStamp, VideoFormat vFormat, int nFrameType);
   void        OnAudioFrame(PBYTE pData, DWORD dwSize, INT64 lTimeStamp, AudioFormat aFormat);

   void        OnDisplayFrame(PBYTE pData, DWORD dwSize);

    void       CalculateFPS();

   BOOL        GetDecodeStatis(DECODE_STATIS & statis);


protected: 
	DECLARE_DYNAMIC(CMainFrame)

	LRESULT  OnVideoWinMsg(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);

   	void     SetServerIP(long  inServerIP);
	
	LRESULT  OnStartReceiveStream(WPARAM wParam, LPARAM lParam);
	LRESULT  OnStopReceiveStream(WPARAM wParam, LPARAM lParam);

	 void OnBnConnect();
	 void OnBnDisconnect();

	 void OnTimer(UINT nIDEvent);
	 void OnDestroy();

	 void OnStatisticsView();

	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	void   HandleMenuID(UINT  nOSDMenuID, POINT pt);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	//CChildView    m_wndView;

	CAVDecoder           m_Decoder; //解码音频和视频，该对象的类继承于CWnd，所以它也是一个窗口

	RtspClient           m_RtspClient;

	POINT              m_ptMenuClickPoint;

protected:

    DWORD             m_dwBPS; //接收码率

	UINT              m_frmCount;
    UINT              m_nFPS;

	HANDLE            m_fp;
	BOOL              m_bRecording; //录制文件

	BYTE *            m_pAudioOutBuf; //音频解码后存放PCM音频的缓冲区

	//VFW播放要用
	int m_width;  
	int m_height; 
	HDRAWDIB m_DrawDib;
	BITMAPINFOHEADER  bmiHeader; 

	BOOL m_bStretch; //是否缩放图像显示（暂时该变量没有起作用，留作扩展）

	void init_bm_head(int cx, int cy);  
	void display_pic(unsigned char* data, int width, int height);  

protected:
	DECLARE_MESSAGE_MAP()

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSetFocus(CWnd *pOldWnd);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnClickMenu(UINT id);
	afx_msg void OnUpdateMenuUI(CCmdUI* pCmdUI);

	virtual BOOL PreTranslateMessage(MSG* pMsg);

	afx_msg void OnMove(int x, int y);
	afx_msg void OnMoving(UINT fwSide, LPRECT pRect);

	LRESULT OnProcessCommnMessage(WPARAM wParam, LPARAM lParam);

};


