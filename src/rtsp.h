/**
** Blog: https://blog.csdn.net/quickgblink
** Email: quickgblink@126.com
**/
//
// Rtsp.h
//

#ifndef _RTSP_CLIENT_H
#define _RTSP_CLIENT_H

#pragma once

//#include "config_unix.h"
#include "config_win32.h"
#include "H264.h"
#include "SpsDecode.h"
#include <vector>

#define MAX_READBUFSIZE 20000
#define MAX_PACKET_SIZE 10000

typedef		DWORD (WINAPI * RtspDataCallback)(BYTE *pBuf, long Buflen, ULONG lTimeStamp, int nFormat, int nFrameType, LPVOID lpContext);

struct MediaAttribute
{
	unsigned fVideoFrequency; //视频1秒的时间表示，一般是90000
	unsigned char fVideoPayloadFormat;
	char fConfigAsc[200];
	unsigned char fConfigHex[100];
	int fConfigHexLen;
	unsigned fVideoFrameRate; //视频帧率
	unsigned fTimeIncBits;
	unsigned fixed_vop_rate;
	unsigned fVideoWidth;
	unsigned fVideoHeight; 
	unsigned fAudioFrequency; //音频1秒的时间表示，一般等于音频采样率
	unsigned char fAudioPayloadFormat; 
	unsigned fTrackNum;
	unsigned fAudioChannels; // 声道数
	VideoFormat fVideoFormat; //视频编码格式
	AudioFormat fAudioFormat; //音频编码格式
};

struct ResultData
{
	int streamChannelId; //0--RTP Stream Data, 1--RTCP
	unsigned char * buffer;
	int  len; 
	int  cache_size; //缓存下一帧（跟当前帧的PTS不相同）的数据长度
	unsigned char fRTPPayloadFormat;
	unsigned long frtpTimestamp;
	int frtpMarkerBit;
	unsigned char cFrameType; //帧的类型, 'I'、 'P'、 'B'
};

struct MediaSubsession 
{
	char * fMediumName;
	char * fCodecName;
	char * fControlPath;// "a=control:<control-path>" 
	char * fConfig;
	unsigned short fClientPortNum;
	unsigned fNumChannels;
	unsigned frtpTimestampFrequency;
	unsigned char fRTPPayloadFormat;
	char * fSessionId;
	char *fFileName;
	FILE* fFid;
	int fWidth; //视频宽度
	int fHeight; //视频高度
	int  rtpChannelID;
	int  rtcpChannelID;
	struct MediaSubsession *fNext;
};

typedef struct
{
	BYTE	RC:5;				// 包内块计数
	BYTE	P:1;				// 填充位，设置1表示包尾带填充数据，最后一个字节表示填充数据的长度
	BYTE	Version:2;			// RTP版本号，2
	BYTE	PT;					// 包类型，200-RTCP-SR包
	USHORT	Len;				// 长度
	UINT	SSRC;				// SSRC段标识同步源

	UINT	TimeNtpH;			// NTP时间高位
	UINT	TimeNtpL;			// NTP时间低位
	UINT	TimeRtp;			// 与Rtp对应的时间
	UINT	RtpCounts;			// Rtp包发送数据计数
	UINT	RtpByteCounts;		// Rtp包发送的字节数
}RTCP_HEAD,*PRTCP_HEAD;


enum PACKET_TYPE
{
	RTP_PACKET = 0,
	RTCP_PACKET,
	UNKNOWN_PACKET
};

//一个音频帧在缓冲区的位置和大小
struct AudioFrameSlice
{
  int    pos;
  int   framelen;
};


class RtspClient
{

public:
	RtspClient();
	~RtspClient();

	BOOL  OpenStream(const char * szURL);
	void  CloseStream();

	int parseRTSPURL(char* url,char* address,int* portNum); 

    void SetRtspCallback(RtspDataCallback lpFunc, LPVOID lpContext); //设置回调函数获取媒体数据

	VideoFormat  GetVideoFormat();
	AudioFormat  GetAudioFormat();

	const MediaAttribute * GetMediaAttribute(); 

	BOOL  GetVideoSize(int & nWidth, int & nHeight); //获取视频分辨率

	void  SetAuthInfo(const char * szUserName, const char * szPassword); //设置用户认证信息

	//int   GetAudioFrameSliceInfo(std::vector<AudioFrameSlice> & vSlices); //获取所有音频帧在缓冲区的位置信息
	const std::vector<AudioFrameSlice> & GetAudioFrameSliceInfo();

	__int64      GetVideoNtpTime(unsigned int rtpTimestamp);  //convert the rtp time into ntp time
	__int64      GetAudioNtpTime(unsigned int rtpTimestamp); //convert the rtp time into ntp time

	unsigned int  GetVideoTimeFrequency() { return VTimestampFrequency; }
	unsigned int  GetAudioTimeFrequency() { return ATimestampFrequency; }

protected:
	void    SetUrl(const char * szURL);

	int     netRTSPConnect(SOCKET fd, uint32_t ip, uint32_t port, uint32_t nTimeOut);
	int     init_rtsp(const char *url, struct MediaAttribute *Attribute);

	int     setupMediaSubsession(struct MediaSubsession* subsession,int subsessionNum);

	int     getSDPDescriptionFromURL(char* url,char* Description);
	int     openConnectionFromURL(char* url);

	struct   MediaSubsession * initializeWithSDP(char* sdpDescription,int *SubsessionNum); 
	struct   MediaAttribute *  SetMediaAttrbute(struct MediaAttribute *Attribute,struct MediaSubsession * subsession,int subsessionNum);

	int     setupStreams(struct MediaSubsession *subsession,int subsessionNum);
	int     startPlayingStreams(struct MediaSubsession *subsession,int subsessionNum); 

	int     teardownMediaSession();
	void    resumeStreams();
	int     pauseMediaSession();
	int     playMediaSession(int start,int end);
	int     SetParametersMediaSession(char *parameters);
	int     GetParametersMediaSession(char *parameters, bool bNotWaitForResponse = false); 

	int     SendOptionsCmd(bool bNotWaitForResponse);

	//接收RTP和RTCP包的函数
	 int    RTP_OR_RTCP_ReadHandler(PACKET_TYPE * packet_type, struct ResultData** rtp_data, RTCP_HEAD * rtcp_data);

	int     rtpHandler(unsigned char * buffer, unsigned int bytesRead,  struct ResultData** data);
	int     handleRead(unsigned char* buffer,unsigned bufferMaxSize,unsigned* bytesRead,unsigned* NextTCPReadSize);

	//从RTP包中拆出H264数据
	int     rtp_unpackage_H264(unsigned char *inbuf, int len, BOOL & marker, int & nIDR, unsigned char * outbuf, int & total_bytes, int nSeqNo);


	//从RTP包中拆出AAC数据
	int      rtp_unpackage_AAC(unsigned char * bufIn, int recvLen, BOOL  marker, int audioSamprate, unsigned char* pBufOut,  int* pOutLen);

	int      rtp_unpackage_AMR(unsigned char * bufIn, int recvLen,  unsigned char* pBufOut,  int* pOutLen);
	int      rtp_unpackage_MP3(unsigned char * bufIn, int recvLen,  unsigned char* pBufOut,  int* pOutLen);

	int     blockUntilwriteable(int socket);
	int     blockUntilReadable(int socket);

	char*   parseSDPLine(char* inputLine); //分析SDP信息，从中提取视频和音频信息
					   
	int     parseResponseCode(char* line, unsigned int * responseCode);
	int     getResponse(char* responseBuffer,unsigned responseBufferSize) ;

	char*   getLine(char* startOfLine);

	char * parseSDPAttribute_rtpmap(char* sdpLine,unsigned* rtpTimestampFrequency,unsigned *fnumChannels) ;//Check for a "a=rtpmap:<fmt> <codec>/<freq>line
	char * parseSDPAttribute_control(char* sdpLine) ;//Check for a "a=control:<control-path>" line
	int    parseSDPAttribute_range(char* sdpLine);//Check for a "a=range:npt=<startTime>-<endTime>" line
	char * parseSDPAttribute_fmtp(char* sdpLine) ;//Check for a "a=fmtp:" line

	char*   lookupPayloadFormat(unsigned char rtpPayloadType, unsigned& freq, unsigned& nCh);

	unsigned char * AllocNetBuffer(); //分配接收网络数据的Buffer
	void           ReleaseBuffer();

	void          FreeSubSessions(struct MediaSubsession *subsession,int subsessionNum);
	void          clearup();

	static DWORD WINAPI RtspThrd(void * pParam); //RTSP连接和接收线程
	int         RtspThreadProc(); //接收RTP或RTCP包的线程函数

	static DWORD WINAPI SendThrd(void * pParam); //向服务器发送数据包的线程
	int         SendThreadProc(); //发送心跳或RTCP包的线程函数

	int          GetFormatType(unsigned char fRTPPayloadFormat);

	void         SetThreadActive(BOOL bActive);

	void       HostOrderToNetOrder(RTCP_HEAD * pHead);

private:
	unsigned long LVrtpTimestamp;//last video timestamp
	unsigned long LArtpTimestamp;//last audio timestamp

	unsigned long MaxFrameNum;
	unsigned long aloopNum;
	unsigned long vloopNum;
	unsigned long audioFirstTimestamp;
	unsigned long videoFirstTimestamp;
	unsigned subTimeFlag;
     
	unsigned char LVFrameType; //上次的视频帧类型('I','P','B')
	BOOL    m_bFrameContainSPS; //当前帧是否包含H264 SPS（IDR = 0x7）

protected:
    int m_socketNum;
	unsigned fCSeq;

	char fBaseURL[128]; //RTSP URL地址
	char fLastSessionId[64];
	unsigned long fMaxPlayEndTime;

	unsigned VTimestampFrequency;
	unsigned ATimestampFrequency;

	unsigned char VPayloadType;
	unsigned char APayloadType;
	unsigned long VFrameRate;


	//int  m_AudioSampleRate; //音频采样率(44100, 48000, 9600)
    struct MediaAttribute m_Attribute;//音频、视频属性

	int   m_rtpNumber;
	int   m_rtcpNumber;

	int PotPos;

	unsigned char *   m_netbuf;

	struct ResultData  m_VFrame; //视频数据(完整的一帧)
	struct ResultData  m_AFrame; //音频数据

	HANDLE      m_RtspThread;
	HANDLE      m_SendThread;
	BOOL        m_bStopThread;
	BOOL        m_bThreadActive;

	RtspDataCallback  m_lpFunc;
	LPVOID            m_lpUserContext;

	unsigned char m_buffer2[1500]; 

	BOOL  m_bUserAuth; //服务器是否需要用户认证

	char m_username[64];
	char m_password[64];

	char m_realm[64];//记录用户认证信息，Digest认证时需要使用到
	char m_nonce[64];//记录用户认证信息，Digest认证时需要使用到
	int  m_authType;//用户认证类型,1为BASE64认证，2为Digest认证

	unsigned char   m_sprop_parameter[4096];//H264 SPS的内容
	int    m_sprop_len;//上面数组的实际内容的长度
 
	int  m_video_rtp_id, m_audio_rtp_id; //视频和音频流的RTP通道ID
	int  m_video_rtcp_id, m_audio_rtcp_id; //视频和音频流的RTCP通道ID

	std::vector<AudioFrameSlice> m_vAudioFrameSlices;

	RTCP_HEAD  m_video_rtcp_packet;
	RTCP_HEAD  m_audio_rtcp_packet;

	//time_t  m_ntp_msw_video; //单位：秒，可换算成当前的日期-时间
	//time_t  m_ntp_msw_audio; //单位：秒
	//UINT    m_ntp_lsw_video; //单位：232 picoseconds, 1 second = 1,000,000,000,000 picoseconds
	//UINT    m_ntp_lsw_audio; //单位：232 picoseconds, 1 second = 1,000,000,000,000 picoseconds

};

#endif
