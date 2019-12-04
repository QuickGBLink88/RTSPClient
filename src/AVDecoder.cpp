// AVDecoder.cpp : implementation file
//
/**
** Blog: https://blog.csdn.net/quickgblink
** Email: quickgblink@126.com
**/

#include "stdafx.h"
#include "resource.h"
#include "AVDecoder.h"
//#include "MainFrm.h"
#include <mmsystem.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define _PLAY_AUDIO_ //是否播放音频

#define ONE_AUDIO_FRAME_SIZE (16*1024)


//static AVFrame *alloc_picture(enum PixelFormat pix_fmt, int width, int height);


static void setup_array(uint8_t* out[2], uint8_t* in, int format, int samples, int nPlanes)
{
	if (av_sample_fmt_is_planar((AVSampleFormat)format)) 
	{
		int i;
		int plane_size = av_get_bytes_per_sample((AVSampleFormat)(format & 0xFF)) * samples;
		format &= 0xFF;
		for (i = 0; i < nPlanes; i++) 
		{
		 out[i] = in + i*plane_size;
		}
	} 
	else
	{
	  out[0] = in;
	}
}


/////////////////////////////////////////////////////////////////////////////
// CAVDecoder

CAVDecoder::CAVDecoder()
{
	// Initialization 
	m_hApp=NULL;

	m_OutputCB = NULL;
	m_nWidth = m_nHeight = 0;
	m_pRgb24 = NULL;//存储RGB数据
   	m_nRgbSize = 0;

	m_pVideoCodec = NULL;
	m_pVideoCodecCtx = NULL;
	m_picture = NULL;
	m_pImgCtx = NULL;	
	out_picture = NULL;
	//picture_buf = NULL;

	m_pAudioPlay = NULL;
	m_pAudioCodecCtx = NULL;
	m_pAudioCodec = NULL;
	m_pSamples = NULL;

    m_pAudioFrame = NULL;
    m_bInited = FALSE;
	au_convert_ctx = NULL;
    
	m_nFrameNumber1 = -1;
    m_audio_rtp_timebase = 8000;

	//m_PicBuf = NULL;
	//m_PicBytes = 0;
	//m_pFrameRGB = NULL;

	//注册FFmpeg的Codec
	avcodec_register_all();
}

CAVDecoder::~CAVDecoder()
{
}


BOOL CAVDecoder:: GetVideoSize(CSize & size)
{
	if(m_nWidth == 0 || m_nHeight == 0)
	{
		return FALSE;
	}
	size.cx = m_nWidth;
	size.cy = m_nHeight;
    return TRUE;
}

void  CAVDecoder:: SetVideoSize(int width, int height) 
{ 
	m_nWidth = width; 
	m_nHeight = height; 
}

BEGIN_MESSAGE_MAP(CAVDecoder, CWnd)
	//{{AFX_MSG_MAP(CAVDecoder)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
	ON_WM_PAINT()
END_MESSAGE_MAP()


void CAVDecoder::OnPaint()
{
	//CWnd::OnPaint();

	CPaintDC dc(this);

	CRect rc;
	GetClientRect(&rc);

	CBrush br(RGB(0,0,0));

	dc.FillRect(rc, &br);
}

/////////////////////////////////////////////////////////////////////////////
// CAVDecoder message handlers

void CAVDecoder::SetupVideoWindow(HWND hVideoWnd)
{
	m_hApp = hVideoWnd; 
}


void CAVDecoder::ResizeVideoWindow()
{
    RECT rc; 
    ::GetClientRect(m_hApp,&rc);

}

HRESULT CAVDecoder::StartDecode()
{
	HRESULT hr;

    // Remember current state
    m_psCurrent = RUNNING;
        
    return S_OK;
}

void CAVDecoder::StopDecode()
{
	CloseDecoder();
	m_psCurrent = STOPPED;
}



void CAVDecoder::DisplayMesg(TCHAR *szFormat, ...)
{
	TCHAR szBuffer[1024];  // Large buffer for long filenames or URLs
    const size_t NUMCHARS = sizeof(szBuffer) / sizeof(szBuffer[0]);
    const int LASTCHAR = NUMCHARS - 1;

    // Format the input string
    va_list pArgs;
    va_start(pArgs, szFormat);

    // Use a bounded buffer size to prevent buffer overruns.  Limit count to
    // character size minus one to allow for a NULL terminating character.
    _vsntprintf(szBuffer, NUMCHARS - 1, szFormat, pArgs);
    va_end(pArgs);

    // Ensure that the formatted string is NULL-terminated
    szBuffer[LASTCHAR] = TEXT('\0');

	::MessageBox(NULL, szBuffer, TEXT("CAVDecoder Message"), MB_OK | MB_ICONERROR);

}


BOOL CAVDecoder:: OpenDecoder(AVCodecID vCodec)
{
	TRACE("OpenDecoder() \n");

	//if(m_nWidth <= 0 || m_nHeight <= 0)
	//	return FALSE;

	/* find the mpeg1 video decoder */
	m_pVideoCodec = avcodec_find_decoder(vCodec);
	if (!m_pVideoCodec) {
		TRACE("m_pVideoCodec not found\n");
		return FALSE;
	}

	m_pVideoCodecCtx = avcodec_alloc_context3(m_pVideoCodec);
	m_picture = avcodec_alloc_frame();


	/* For some codecs, such as msmpeg4 and mpeg4, width and height
	MUST be initialized there because this information is not
	available in the bitstream. */
	//m_pVideoCodecCtx->width = m_nWidth;
	//m_pVideoCodecCtx->height = m_nHeight;

	//out_picture = avcodec_alloc_frame();

	 
	/* open it */
	if (avcodec_open2(m_pVideoCodecCtx, m_pVideoCodec, NULL) < 0) {
		TRACE("could not open m_pVideoCodec\n");
		return FALSE;
	}

	m_nFrameNumber1 = -1;

    return TRUE;
}

void  CAVDecoder:: CloseDecoder()
{
	TRACE("CloseDecoder() \n");

	if(m_pVideoCodecCtx != NULL)
	{
		if(m_picture != NULL)
		{
		 // av_free(m_picture->data[0]); //  不能这样删除，因为m_picture的内存不是通过allocate_picture函数分配的
		  av_free(m_picture);
		  m_picture = NULL;
		}
		if(out_picture != NULL)
		{
			av_free(out_picture->data[0]);
		    av_free(out_picture);
			out_picture = NULL;
		}

		if(m_pVideoCodecCtx != NULL)
		{
			avcodec_close(m_pVideoCodecCtx);
			av_free(m_pVideoCodecCtx);
		}

		m_pVideoCodecCtx = NULL;
		m_pVideoCodec = NULL;
		m_picture = NULL;
        out_picture = NULL;
		m_pImgCtx = NULL;
	}

	if(m_pRgb24)
	{
		delete[] m_pRgb24;
		m_pRgb24 = NULL;
	}
		
	//if(m_PicBuf)
	//{
	//	delete[] m_PicBuf; 
	//	m_PicBuf = NULL;
	//}
    m_nWidth = 0;
	m_nHeight = 0;

	m_bInited = FALSE;

	if(m_pAudioPlay)
	{
		delete m_pAudioPlay;
		m_pAudioPlay = NULL;
	}

	if(m_pAudioCodecCtx != NULL)
	{
		avcodec_close(m_pAudioCodecCtx);
		av_free(m_pAudioCodecCtx);

		av_free(m_pSamples);
		m_pSamples = NULL;
        m_pAudioCodecCtx = NULL;
		m_pAudioCodec = NULL;
	}

	if(m_pAudioFrame != NULL)
	{
		av_free(m_pAudioFrame);
		m_pAudioFrame = NULL;
	}

	if(m_pSamples != NULL)
	{
		av_free(m_pSamples);
		m_pSamples = NULL;
	}

	if(m_pAudioPlay)
	{
		delete m_pAudioPlay;
		m_pAudioPlay = NULL;
	}

	if(au_convert_ctx != NULL)
	{
	  swr_free(&au_convert_ctx);
	  au_convert_ctx = NULL;
	}
}


static void pgm_save(unsigned char *buf, int wrap, int xsize, int ysize,
					 char *filename)
{
	FILE *f;
	int i;

	f=fopen(filename,"w");
	fprintf(f,"P5\n%d %d\n%d\n",xsize,ysize,255);
	for(i=0;i<ysize;i++)
		fwrite(buf + i * wrap,1,xsize,f);
	fclose(f);
}


AVFrame * CAVDecoder::alloc_picture(enum PixelFormat pix_fmt, int width, int height)
{
    AVFrame *m_picture;
    int size;
    uint8_t *picture_buf;

    m_picture = avcodec_alloc_frame();
    if (!m_picture)
        return NULL;
    size = avpicture_get_size(pix_fmt, width, height);
    picture_buf = (uint8_t *)av_malloc(size); //  解码器关闭时需要销毁picture_buf的内存
    if (!picture_buf) {
        av_free(m_picture);
        return NULL;
    }
    avpicture_fill((AVPicture *)m_picture, picture_buf,
                   pix_fmt, width, height);
    return m_picture;
}


//将AudioFormat转成FFMpeg的AVCodecID
AVCodecID  GetAudioCodecID(AudioFormat audio_fmt)
{
    AVCodecID nCodecID = CODEC_ID_NONE;

	switch(audio_fmt)
	{
	case AFMT_MP2:
		nCodecID = CODEC_ID_MP2;
		break;
	case AFMT_MP3:
		nCodecID = CODEC_ID_MP3; 
		break;
	case AFMT_AAC:
		nCodecID = CODEC_ID_AAC;
		break;
	case AFMT_AMR:
		nCodecID = CODEC_ID_AMR_NB;
		break;
	default:
		break;
	}
	return nCodecID;
}

//将VideoFormat转成FFMpeg的AVCodecID
AVCodecID  GetVideoCodecID(VideoFormat video_fmt)
{
    AVCodecID nCodecID = CODEC_ID_NONE;

	switch(video_fmt)
	{
	case VFMT_MPEG2:
		nCodecID = CODEC_ID_MPEG2VIDEO; 
		break;
	case VFMT_MPEG4:
		nCodecID = CODEC_ID_MPEG4; 
		break;
	case VFMT_H264:
		nCodecID = CODEC_ID_H264;
		break;
	case VFMT_H263:
		nCodecID = CODEC_ID_H263;
		break;
	case VFMT_H265:
		nCodecID = AV_CODEC_ID_HEVC;
		break;
	default:
		break;
	}
	return nCodecID;
}


BOOL  CAVDecoder:: OnDecodeAudio(PBYTE pData, int nDataLen, AudioFormat aFormat, AVSampleFormat out_sample_fmt, BYTE * pBufOut, int * pOutLen)
{
	AUDIO_OUT_PARAM AudioOutParam;
	AudioOutParam.uDeviceID = 0xff;

	if ( m_pAudioCodecCtx == NULL )
	{			
		//avcodec_register_all();
		
		m_pAudioCodecCtx = avcodec_alloc_context3(NULL);
		if(m_pAudioCodecCtx == NULL)
		{
			TRACE("Error: avcodec_alloc_context3() failed! \n");
			return false;
		}

		if ( m_pAudioCodec == NULL )
		{
			AVCodecID codecid = GetAudioCodecID(aFormat);
			m_pAudioCodec = avcodec_find_decoder(codecid);
			if ( !m_pAudioCodec )
			{
				TRACE("Error: avcodec_find_decoder() failed, Can not find this Audio codec! \n");
				ASSERT(0);
				av_free(m_pAudioCodecCtx);
			    m_pAudioCodecCtx = NULL;
				return false;
			}
		}

		if (aFormat == AFMT_AMR) 
		{
			m_pAudioCodecCtx->sample_rate = 8000;
			m_pAudioCodecCtx->channels = 1;
		}

		if ( avcodec_open2(m_pAudioCodecCtx, m_pAudioCodec, NULL) != 0 )
		{
			TRACE("Error: avcodec_open() failed! \n");
			av_free(m_pAudioCodecCtx);
			m_pAudioCodecCtx = NULL;
			m_pAudioCodec = NULL;
			return false;	
		}

		TRACE("Audio sample_fmt = %d \n", m_pAudioCodecCtx->sample_fmt);
	}

	if ( m_pAudioFrame == NULL )
		m_pAudioFrame = avcodec_alloc_frame();

	if ( m_pSamples == NULL )
	{
		m_pSamples = (short*)av_mallocz( FFMAX(8192,AVCODEC_MAX_AUDIO_FRAME_SIZE) );
	}

	int nLenOut = ( 8192 > AVCODEC_MAX_AUDIO_FRAME_SIZE ) ? 8192 : AVCODEC_MAX_AUDIO_FRAME_SIZE;

	AVPacket pkt;
	av_init_packet(&pkt);
	pkt.data = pData;
	pkt.size = nDataLen;

	int len = 0;
	int nTotalBytes = 0;
	int nPlanes = 1;
	int nSlices = 0;
	int nFrameDuration = 0;

	while(pkt.size > 0)
	{
		nLenOut = ( 8192 > AVCODEC_MAX_AUDIO_FRAME_SIZE ) ? 8192 : AVCODEC_MAX_AUDIO_FRAME_SIZE;

		memset(m_pSamples, 0, nLenOut);
		
#if 0
		len = avcodec_decode_audio3(m_pAudioCodecCtx, m_pSamples, &nLenOut, &pkt);
#else
		int nGot = 0;
        len = avcodec_decode_audio4(m_pAudioCodecCtx, m_pAudioFrame, &nGot, &pkt);
#endif
		if ( len < 0 )
		{
			TRACE("decoder audio failed \r\n");
			return false;
		}
		if ( len > 0 && !nGot )
		{
		   pkt.data += len;
		   pkt.size -= len;
		   continue;
		}

		pkt.data += len;
		pkt.size -= len;

		if(nFrameDuration == 0)
		{ 
			ASSERT(m_audio_rtp_timebase > 0);
			//计算单个音频帧的时间长度
			nFrameDuration = m_audio_rtp_timebase * m_pAudioFrame->nb_samples/m_pAudioCodecCtx->sample_rate;
		}

		//Out Audio Param  
		uint64_t out_channel_layout = (m_pAudioCodecCtx->channels==1) ? AV_CH_LAYOUT_MONO : AV_CH_LAYOUT_STEREO;  
		int out_nb_samples = m_pAudioFrame->nb_samples;  

		int out_sample_rate = m_pAudioCodecCtx->sample_rate;  //(aFormat == AFMT_AMR)?8000:44100;
		int out_channels = av_get_channel_layout_nb_channels(out_channel_layout);  

		//int out_buffer_size = av_samples_get_buffer_size(NULL,out_channels ,out_nb_samples,out_sample_fmt, 1); //Out Buffer Size   
		int64_t in_channel_layout = av_get_default_channel_layout(m_pAudioCodecCtx->channels);  

		if(au_convert_ctx == NULL)
		{
			au_convert_ctx = swr_alloc();  

			au_convert_ctx = swr_alloc_set_opts(au_convert_ctx,out_channel_layout, out_sample_fmt, out_sample_rate,  
				in_channel_layout, m_pAudioCodecCtx->sample_fmt , m_pAudioCodecCtx->sample_rate,0, NULL);  

			swr_init(au_convert_ctx);  
		}

		nPlanes = av_sample_fmt_is_planar(out_sample_fmt) ? m_pAudioCodecCtx->channels : 1;

	   uint8_t* out_frame[2] = {0};
	   setup_array(out_frame, (uint8_t*)m_pSamples, out_sample_fmt, out_nb_samples, nPlanes);

	  // int nConverted = swr_convert(au_convert_ctx, (uint8_t**)&m_pSamples, out_nb_samples,(const uint8_t **)m_pAudioFrame->data , m_pAudioFrame->nb_samples); 
	   int nConverted = swr_convert(au_convert_ctx, (uint8_t**)out_frame, out_nb_samples,(const uint8_t **)m_pAudioFrame->data , m_pAudioFrame->nb_samples); 
	   ASSERT(nConverted >0);

       int out_buffer_size = av_samples_get_buffer_size(NULL,out_channels ,m_pAudioFrame->nb_samples,out_sample_fmt, 1); 
       nLenOut = out_buffer_size;


#ifdef _PLAY_AUDIO_
   if(out_sample_fmt == AV_SAMPLE_FMT_S16)
   {
   		if( m_pAudioPlay == NULL )
		{
			m_pAudioPlay = new CWaveSound();
			if( m_pAudioPlay )
			{
				AudioOutParam.Bits = 16/*m_pAudioCodecCtx->channels*8*/;
				AudioOutParam.Channels = m_pAudioCodecCtx->channels;
				AudioOutParam.Sample = m_pAudioCodecCtx->sample_rate;

				if(AudioOutParam.Channels == 0 || AudioOutParam.Sample == 0)
				{
					if(aFormat == AFMT_AMR)
					{
					    AudioOutParam.Channels = 1;
						AudioOutParam.Sample = 8000;
						AudioOutParam.Bits = 16;
					}
					else
					{
						AudioOutParam.Channels = 2;
						AudioOutParam.Sample = 44100;
						AudioOutParam.Bits = 16;
					}
				}

				if( m_pAudioPlay->AudioOpen( &AudioOutParam ) < 0 )
				{
					TRACE("Error: AudioOpen() failed! \n");
					delete	m_pAudioPlay;
					m_pAudioPlay = NULL;
				}
			}
		}
		if( m_pAudioPlay && nLenOut > 0)
		{			
			m_pAudioPlay->AudioWrite( (BYTE*)m_pSamples, nLenOut );

//		   m_pAudioPlay->AudioWrite( (BYTE*)pFrame->data[0], pFrame->linesize[0] );
		}
   }

#endif
		
		if(pBufOut != NULL && pOutLen != NULL)
		{
	    	memcpy(pBufOut + *pOutLen, (BYTE*)m_pSamples, nLenOut);
			*pOutLen += nLenOut;
		}
		

		nTotalBytes += nLenOut;

	} //while

	return nTotalBytes > 0;
}



//函数作用：解码一帧数据
//参数值：
//inbuf ---输入Buffer，填充的是等待解码的数据
//inLen ---填充的数据大小
//nFrameType---帧的类型， 等于1为I帧
//
BOOL    CAVDecoder:: OnDecodeVideo(PBYTE inbuf, long inLen, VideoFormat vFormat, int nFrameType)
{
	//TRACE("OnDecodeVideo size: %d \n", inLen);

#ifdef _DEBUG
	if(vFormat == VFMT_H264)
	{
		if(!(inbuf[0] == 0x00 && inbuf[1] == 0x00 && inbuf[2] == 0x00 && inbuf[3] == 0x01))
		{
			TRACE("NOT H264 StartCode: %x%x%x%x \n", inbuf[0], inbuf[1], inbuf[2], inbuf[3]);
			//ASSERT(0);
		}
	}
#endif

	if(m_pVideoCodecCtx == NULL)
	{
		if(!OpenDecoder(GetVideoCodecID(vFormat)))
		{
			return E_FAIL;
		}
	}

	if(m_nFrameNumber1 == -1  && nFrameType != 1) //等到关键帧到来
	{
		return FALSE;
	}

	m_bInited = TRUE;

	int got_picture = 0;

	AVPacket avpkt;
	av_init_packet(&avpkt);

    avpkt.size = inLen;
	avpkt.data = inbuf;

	int len = avcodec_decode_video2(m_pVideoCodecCtx, m_picture, &got_picture, &avpkt);
	if (len < 0) 
	{
		TRACE("Error while decoding frame Len %d\n", inLen);
		return FALSE;
	}

	m_nFrameNumber1++;
	
	if (got_picture)
	{
		if(m_nWidth == 0 || m_nHeight == 0) 
		{
			//获取图像的宽高
			m_nWidth  = m_pVideoCodecCtx->width;
			m_nHeight = m_pVideoCodecCtx->height;

			//// allocate decoded raw m_picture 
			//out_picture = alloc_picture(PIX_FMT_YUV420P, m_pVideoCodecCtx->width, m_pVideoCodecCtx->height);
			//if (!out_picture) {
			//	TRACE("Could not allocate m_picture\n");
			//	return FALSE;
			//}

		}//if(m_nWidth == 0 || m_nHeight == 0) 


		if(m_pImgCtx == NULL)
		{

			m_pImgCtx = sws_getContext(m_pVideoCodecCtx->width, m_pVideoCodecCtx->height,
				PIX_FMT_YUV420P,
				m_pVideoCodecCtx->width, m_pVideoCodecCtx->height,
				PIX_FMT_BGR24,
				SWS_BICUBIC, NULL, NULL, NULL);

			if (m_pImgCtx == NULL)
			{
				TRACE("sws_getContext() failed \n");
				return FALSE;		
			}
		}

		if(m_pRgb24 == NULL)
		{
			int nRgbSize = m_nWidth * m_nHeight*4; //分配大一点空间
			m_pRgb24 = new BYTE[nRgbSize];	
			memset(m_pRgb24, 0, nRgbSize);

			m_nRgbSize = m_nWidth * m_nHeight * 3;
		}

		if(m_picture->key_frame) //解码出来的这一帧是关键帧
		{

		}
	

		if(m_pVideoCodecCtx->pix_fmt == PIX_FMT_YUV420P || m_pVideoCodecCtx->pix_fmt == PIX_FMT_YUVJ420P) 
		{
			uint8_t  *dest_y = m_picture->data[0];
			uint8_t  *dest_u = m_picture->data[1];
			uint8_t  *dest_v = m_picture->data[2];

			m_picture->data[1] = dest_u;
			m_picture->data[2] = dest_v;

#if 0

			uint8_t *rgb_src[3]= {m_pRgb24, NULL, NULL};
			int rgb_stride[3]={m_pVideoCodecCtx->width*3, 0, 0};

			//将YUV420转成RGB格式
			sws_scale(m_pImgCtx, m_picture->data, m_picture->linesize,  0, m_pVideoCodecCtx->height, rgb_src, rgb_stride);

			//显示图像
			if(m_OutputCB) 
			   m_OutputCB(m_Num, m_pRgb24, m_nRgbSize);

#else
			//上面输出的RGB图像是倒置的，下面这段代码将RGB图像做翻转处理

			//if(m_PicBuf == NULL || m_pFrameRGB == NULL)
			//{
			//	m_PicBytes = avpicture_get_size(PIX_FMT_BGR24, m_pVideoCodecCtx->width, m_pVideoCodecCtx->height);  
			//	m_PicBuf = new uint8_t[m_PicBytes];  
			//	avpicture_fill((AVPicture *)m_pFrameRGB, m_PicBuf, PIX_FMT_BGR24, m_pVideoCodecCtx->width, m_pVideoCodecCtx->height);  
			//}

			uint8_t *rgb_src[3]= {m_pRgb24, NULL, NULL};
			int rgb_stride[3]={m_pVideoCodecCtx->width*3, 0, 0};

			m_picture->data[0] += m_picture->linesize[0]*(m_pVideoCodecCtx->height-1);  
			m_picture->linesize[0] *= -1;                      
			m_picture->data[1] += m_picture->linesize[1]*(m_pVideoCodecCtx->height/2-1);  
			m_picture->linesize[1] *= -1; 
			m_picture->data[2] += m_picture->linesize[2]*(m_pVideoCodecCtx->height/2-1);  
			m_picture->linesize[2] *= -1;

			//将YUV420转为BGR24，并且翻转图像
			//sws_scale(m_pImgCtx, m_picture->data, m_picture->linesize, 0, m_pVideoCodecCtx->height, m_pFrameRGB->data, m_pFrameRGB->linesize);   
			sws_scale(m_pImgCtx, m_picture->data, m_picture->linesize,  0, m_pVideoCodecCtx->height, rgb_src, rgb_stride); 

			//显示图像
			if(m_OutputCB) 
			   m_OutputCB(m_Num, m_pRgb24, m_nRgbSize);

#endif

		}
	}

	return TRUE;
}


