// WaveSound.h: interface for the CWaveSound class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WAVESOUND_H__1A507D40_67E7_4375_AA97_12FF9DA8B4C3__INCLUDED_)
#define AFX_WAVESOUND_H__1A507D40_67E7_4375_AA97_12FF9DA8B4C3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include"mmsystem.h"
#include <afxmt.h>


typedef	struct
{
	int		Sample;
	int		Bits;
	int		Channels;
	int     uDeviceID;
}AUDIO_OUT_PARAM,*PAUDIO_OUT_PARAM;

class CWaveSound  
{
public:	
	CWaveSound();
	virtual ~CWaveSound();

	void InputRefData(PBYTE pBuf);	
	BOOL ResetSound();
	BOOL SetVolume(DWORD nVolume);	
	DWORD GetAudioTimeStamp();	
	int GetPlaySeqNum();
	BOOL InputData(PBYTE pBuf, DWORD dwDataLen, DWORD nUser=0);

	BOOL PlaySound();
	BOOL PauseSound();
	BOOL CloseSound();
	
	static void CALLBACK waveOutProc(HWAVEOUT hwo,UINT uMsg, DWORD dwInstance, DWORD dwParam1,DWORD dwParam2 );
	BOOL OpenSound(WAVEFORMATEX& PcmWF,DWORD nBlockNum,DWORD nBlcokSize,DWORD nFlag,LONG nPort);

	int		AudioOpen(PAUDIO_OUT_PARAM pAop);
	void	AudioClose();
	int		AudioWrite(BYTE *pBuf, int BufLen);

private:
	DWORD			m_nBlockNum;
	DWORD			m_nBlockSize;
	WAVEHDR*		m_pWaveHead;
	BOOL			m_bOpenSound;
	HWAVEOUT		m_hWaveOut;
	HANDLE			m_hEventSound;
	DWORD			m_nWaveWriteCount;
	
	CMutex m_mutex;
	char* m_pRefBuf;
	LONG m_nPort;
	DWORD m_nVolume;
	DWORD m_nCurUser;
	DWORD m_nWavePlayCount;
};

#endif // !defined(AFX_WAVESOUND_H__1A507D40_67E7_4375_AA97_12FF9DA8B4C3__INCLUDED_)
