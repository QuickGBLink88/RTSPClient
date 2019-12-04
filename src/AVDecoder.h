#if !defined(AVDECODER_H_INCLUDED_)
#define AVDECODER_H_INCLUDED_



#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AVDecoder.h : header file
#include <windows.h>
#include "WaveSound.h"
#include "H264.h"

#ifdef __cplusplus
extern "C" {
#endif 

#ifdef HAVE_AV_CONFIG_H
#undef HAVE_AV_CONFIG_H
#endif

#include "./include/libavcodec/avcodec.h"
#include "./include/libavutil/mathematics.h"
#include "./include/libswresample/swresample.h"  
#include "libswscale/swscale.h"

#ifdef __cplusplus
}
#endif 

//#pragma comment( lib, "libgcc.a")
//#pragma comment( lib, "libmingwex.a")

//#pragma comment( lib, "libavcodec.a")
//#pragma comment( lib, "libavutil.a")
//#pragma comment(lib,  "libswscale.a")


#pragma comment( lib, "avcodec.lib")
#pragma comment( lib, "avutil.lib")
#pragma comment(lib,  "swscale.lib")
#pragma comment(lib, "swresample.lib")


typedef LRESULT (CALLBACK* OutputVideoCB)(int devNum, PBYTE pRgb, int  dwSize);



 //将AudioFormat转成FFMpeg的AVCodecID
extern AVCodecID  GetAudioCodecID(AudioFormat audio_fmt);

//将VideoFormat转成FFMpeg的AVCodecID
extern AVCodecID  GetVideoCodecID(VideoFormat video_fmt);


enum PLAYSTATE {
	STOPPED, PAUSED, RUNNING, INIT };


/////////////////////////////////////////////////////////////////////////////
// CAVDecoder window

class CAVDecoder : public CWnd
{
// Construction
public:
	CAVDecoder();

	void SetNum(int num) { m_Num = num; }

// Attributes
public:

	HRESULT   StartDecode(); //开始解码，只是把内部状态设成"Running"，初始化解码器是在OnDecodeVideo函数
	void      StopDecode(); //销毁解码器

	void      SetVideoSize(int width, int height);
	BOOL      GetVideoSize(CSize & size); //获取图像分辨率

	PLAYSTATE GetState() { return m_psCurrent; }

	void      SetupVideoWindow(HWND hVideoWnd); //设置显示图像的窗口句柄
	void      ResizeVideoWindow();

	DWORD     GetCurrentBitrate(); 


	//设置显示图像回调函数
	void       SetDisplayCallback(OutputVideoCB captureCB) { m_OutputCB = captureCB; }

	//解码一帧图像
	BOOL       OnDecodeVideo(PBYTE pBuffer, long inLen, VideoFormat vFormat, int nFrameType); 

    
	//解码音频
	BOOL       OnDecodeAudio(PBYTE inbuf, int inLen, AudioFormat aFormat, AVSampleFormat out_sample_fmt, BYTE * pBufOut, int * pOutLen);

    BOOL       OpenDecoder(AVCodecID vCodec); //打开解码器
	void       CloseDecoder(); //关闭解码器


	BOOL       IsInited() { return m_bInited; }

	void       SetAudioTimeBase(unsigned int audioTimebase) { m_audio_rtp_timebase = audioTimebase; } //音频的时间基线（一般等于采样率）

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAVDecoder)
	//}}AFX_VIRTUAL

// Implementation
public:

	virtual ~CAVDecoder();

	// Generated message map functions
protected:
	//{{AFX_MSG(CAVDecoder)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()

protected:

	void     DisplayMesg(TCHAR* szFormat, ...);

	AVFrame * alloc_picture(enum PixelFormat pix_fmt, int width, int height);

private:

    UINT           m_Num;
	HWND           m_hApp;
	OutputVideoCB   m_OutputCB;

	PLAYSTATE m_psCurrent; 

	int   m_nWidth, m_nHeight;

	int      m_nFrameNumber1;  //输入解码器的帧数

	//视频解码器变量
	AVCodec * m_pVideoCodec;
	AVCodecContext * m_pVideoCodecCtx;
			
	AVFrame *m_picture; 
	SwsContext *m_pImgCtx;

	//AVFrame *m_pFrameRGB;
	//uint8_t *m_PicBuf;
	//int m_PicBytes;

	PBYTE          m_pRgb24;
    UINT           m_nRgbSize;

	AVFrame *out_picture;
	//uint8_t *picture_buf;

	BOOL   m_bInited;


	//音频解码器变量
	AVCodecContext *m_pAudioCodecCtx;
	AVCodec *m_pAudioCodec;

	int16_t *m_pSamples; //音频解码后的临时缓冲区

	 struct SwrContext *au_convert_ctx; 

	//回放音频的对象指针
	CWaveSound  *m_pAudioPlay;

	AVFrame *m_pAudioFrame;
	
	unsigned int m_audio_rtp_timebase;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AVDECODER_H_INCLUDED_)
