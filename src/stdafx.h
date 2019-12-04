// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 项目特定的包含文件

#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// 从 Windows 标头中排除不常使用的资料
#endif

#define  WINVER 0x0501


#ifndef WINVER				// 允许使用 Windows 95 和 Windows NT 4 或更高版本的特定功能。
#define WINVER 0x0501		//为 Windows98 和 Windows 2000 及更新版本改变为适当的值。
#endif

#ifndef _WIN32_WINNT		// 允许使用 Windows NT 4 或更高版本的特定功能。
#define _WIN32_WINNT 0x0501		//为 Windows98 和 Windows 2000 及更新版本改变为适当的值。
#endif						

#ifndef _WIN32_WINDOWS		// 允许使用 Windows 98 或更高版本的特定功能。
#define _WIN32_WINDOWS 0x0501 //为 Windows Me 及更新版本改变为适当的值。
#endif

#ifndef _WIN32_IE			// 允许使用 IE 4.0 或更高版本的特定功能。
#define _WIN32_IE 0x0501	//为 IE 5.0 及更新版本改变为适当的值。
#endif


#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// 某些 CString 构造函数将是显式的

// 关闭 MFC 对某些常见但经常被安全忽略的警告消息的隐藏
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC 核心和标准组件
#include <afxext.h>         // MFC 扩展

#include <afxdtctl.h>		// Internet Explorer 4 公共控件的 MFC 支持
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// Windows 公共控件的 MFC 支持
#endif // _AFX_NO_AFXCMN_SUPPORT


#define AVCODEC_MAX_AUDIO_FRAME_SIZE 192000


//视音频流的信息
typedef struct _decode_statis_
{
	UINT  nVideoFormat; //视频编码格式（定义见H264.h - VideoFormat）
    UINT  nAudioFormat; //音频编码格式（定义见H264.h - AudioFormat）
    UINT  nFramerate; //帧率
	int   nWidth; //图像宽度
	int   nHeight; //图像高度
    DWORD dwBitrate;  //码率，单位Bps
}DECODE_STATIS;




//#include <afxsock.h>		// MFC socket extensions
#include <Winsock2.h>
#include "./include/stdint.h"

//#include <streams.h>

#ifndef CodecID
#define CodecID AVCodecID
#endif



//#define _WRITE_FILE_

#define NTP_TIME_FREQUENCY  (90000)