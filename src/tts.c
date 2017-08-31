#include "platform_api.h"
#include "CApi.h"
#include "eJTTS.h"
#define TTS_BUFF_SIZE (1024 * 64)
//语音数据达到该大小时开始播放
#define TEXT_BUF_SIZE_MAX	(4096)
#define TEXT_BUF_SIZE_MIN	(256)
#if (CHIP_ASIC_ID == CHIP_ASIC_ID_8809)
#define PMD_POWER_TYPE		(PMD_POWER_EARPIECE)
#define DM_AUDIO_ON_TYPE	(DM_AUDIO_MODE_EARPIECE)
#define DM_AUDIO_OFF_TYPE	(DM_AUDIO_MODE_LOUDSPEAKER)
#endif
#if (CHIP_ASIC_ID == CHIP_ASIC_ID_8955)
#define PMD_POWER_TYPE		(PMD_POWER_LOUD_SPEAKER)
#define DM_AUDIO_ON_TYPE	(DM_AUDIO_MODE_LOUDSPEAKER)
#define DM_AUDIO_OFF_TYPE	(DM_AUDIO_MODE_EARPIECE)
#endif
enum
{
	TTS_STATE_NO_INIT,
	TTS_STATE_IDLE,
	TTS_STATE_SYNTHESIZING,
	TTS_STATE_PLAYING,
	TTS_STATE_ERROR,
};
extern const unsigned char CNPackage[];
typedef struct
{
	unsigned long Handle;
	unsigned char *pHeap;
	s8 *Data;
	uint32_t Pos;
	MyAPIFunc PCMCB;
	MyAPIFunc TTSCB;

    uint16_t sample_rate;
    uint8_t State;
    uint8_t bit_rate;
}TTS_CtrlStruct;

TTS_CtrlStruct TTSCtrl;
extern BOOL Speaker_Open(VOID);

void TTS_SpeakerModeStart()   //Added by Jinzh:20070616
{

    pmd_SetLevel(PMD_POWER_TYPE,0);//mute opal audio pa
	COS_Sleep(3);
//	pmd_EnablePower(PMD_POWER_LOUD_SPEAKER, 1);//enable opal audio pa
	pmd_EnablePower(PMD_POWER_TYPE, 1);//enable opal audio pa
//	pmd_EnablePower(PMD_POWER_AUDIO, 1);//enable opal audio pa
//	pmd_EnablePower(PMD_POWER_STEREO_DAC, 1);//enable opal audio pa
	COS_Sleep(3);
#if (CHIP_ASIC_ID == CHIP_ASIC_ID_8809)
	pmd_SetLevel(PMD_POWER_TYPE,1);
	COS_Sleep(3);
	pmd_SetLevel(PMD_POWER_TYPE,3);
	COS_Sleep(3);
	pmd_SetLevel(PMD_POWER_TYPE,5);
	COS_Sleep(3);
	pmd_SetLevel(PMD_POWER_TYPE,7);
#endif
#if (CHIP_ASIC_ID == CHIP_ASIC_ID_8955)
	pmd_SetLevel(PMD_POWER_TYPE,1);
	COS_Sleep(3);
	pmd_SetLevel(PMD_POWER_TYPE,3);
	COS_Sleep(3);
	pmd_SetLevel(PMD_POWER_TYPE,5);
	COS_Sleep(3);
	pmd_SetLevel(PMD_POWER_TYPE,7);
	COS_Sleep(3);
	pmd_SetLevel(PMD_POWER_TYPE,10);
	COS_Sleep(3);
	pmd_SetLevel(PMD_POWER_TYPE,14);
#endif
}

void TTS_SpeakerModeEnd()   //Added by Jinzh:20070616
{
//	pmd_EnablePower(PMD_POWER_LOUD_SPEAKER, 0);
//	pmd_EnablePower(PMD_POWER_AUDIO, 0);//enable opal audio pa
//	pmd_EnablePower(PMD_POWER_STEREO_DAC, 0);//enable opal audio pa
	pmd_SetLevel(PMD_POWER_TYPE,0);//mute opal audio pa
}

UINT16  wstrlen(const uint16_t* str)
{
    const uint16_t *eos = str;
	if(eos == NULL) return 0;

	while( *eos++ ) ;

    return( (UINT16)(eos - str - 1) );
}

void pcm_play_callback(APBS_STREAM_STATUS_T status)
{
	APBS_STREAM_STATUS_T stream_status = (APBS_STREAM_STATUS_T)status;
	//CORE("TTS pcm_play_callback:;stream_status=%u, TTSCtrl.State= %u ",  stream_status, TTSCtrl.State);
    switch(stream_status)
    {
    case STREAM_STATUS_REQUEST_DATA:
		break;
	case STREAM_STATUS_NO_MORE_DATA:
    case STREAM_STATUS_ERR:
		jtTTS_SynthStop(TTSCtrl.Handle);
		break;
    case STREAM_STATUS_END:
    	MCI_DataFinished();
	   	if(TTSCtrl.State != TTS_STATE_NO_INIT)
	   	{
	   		TTSCtrl.State = TTS_STATE_IDLE;
	   		if (TTSCtrl.PCMCB)
	   		{
	   			TTSCtrl.PCMCB(NULL);
	   		}
	   	}
	   	TTS_SpeakerModeEnd();
	   	DM_SetAudioMode(DM_AUDIO_OFF_TYPE);
	   	break;
	default:
		break;
     }

}

BOOL get_wav_format(unsigned long handle, uint16_t *sample_rate, uint8_t *bit_rate)
{
	long wav_format = -1;
	jtErrCode nErr = jtTTS_ERR_NONE;

	nErr = jtTTS_GetParam(handle, jtTTS_PARAM_WAV_FORMAT, &wav_format);
	//kal_prompt_trace(MOD_EJTTS, "jtTTS_PARAM_WAV_FORMAT = %u %u", wav_format, nErr);

	if (jtTTS_ERR_NONE != nErr || -1 == wav_format)
	{
		CORE("TTS test, get wav format error!");
		return FALSE;
	}

	switch(wav_format)
	{
	case jtTTS_FORMAT_PCM_NORMAL:
	//此处依音库而定，用户需要手动修改
		*bit_rate = 16;
		*sample_rate = 16000;
		break;
	case jtTTS_FORMAT_PCM_8K8B:
		*bit_rate = 8;
		*sample_rate = 8000;
		break;
	case jtTTS_FORMAT_PCM_8K16B:
		*bit_rate = 16;
		*sample_rate = 8000;
		break;
	case jtTTS_FORMAT_PCM_16K8B:
		*bit_rate = 8;
		*sample_rate = 16000;
		break;
	case jtTTS_FORMAT_PCM_16K16B:
		*bit_rate = 16;
		*sample_rate = 16000;
		break;
	case jtTTS_FORMAT_PCM_11K8B:
		*bit_rate = 8;
		*sample_rate = 11025;
		break;
	case jtTTS_FORMAT_PCM_11K16B:
		*bit_rate = 16;
		*sample_rate = 11025;
		break;
	case jtTTS_FORMAT_PCM_22K8B:
		*bit_rate = 8;
		*sample_rate = 22050;
		break;
	case jtTTS_FORMAT_PCM_22K16B:
		*bit_rate = 16;
		*sample_rate = 22050;
		break;
	case jtTTS_FORMAT_PCM_44K8B:
		*bit_rate = 8;
		*sample_rate = 44100;
		break;
	case jtTTS_FORMAT_PCM_44K16B:
		*bit_rate = 16;
		*sample_rate = 44100;
		break;
	default:
		*bit_rate = 16;
		*sample_rate = 16000;
		break;
	}

	return TRUE;
}

jtErrCode TTS_OutputVoicePCMProc(void* pParameter,
	long iOutputFormat, void* pData, long iSize)
{
    INT32 result;
    //jtErrCode Error;
	unsigned char *buf_pcm, *buf_src;
   	UINT32 buf_len;
	UINT32 pcm_len;

	//CORE("TTS pData = %x  iSize= %u state = %u", pData, iSize, TTSCtrl.State);
	if(iSize <= 0)
	{
		CORE("TTS iSize<0, Finished Data ");
		TTSCtrl.State = TTS_STATE_IDLE;
		MCI_DataFinished();
		if (TTSCtrl.TTSCB)
		{
			TTSCtrl.TTSCB(NULL);
		}
	}

	switch(TTSCtrl.State)
	{
	case TTS_STATE_SYNTHESIZING:
		//第一次
		memcpy(TTSCtrl.Data + TTSCtrl.Pos, pData, iSize);
		TTSCtrl.Pos += iSize;
		MCI_SetBuffer( (UINT32*)TTSCtrl.Data, TTS_BUFF_SIZE);
		MCI_AddedData( TTSCtrl.Pos);
		Speaker_Open();
		aud_LoudspeakerWithEarpiece(TRUE);
		result = MCI_AUD_StreamPlayPCM( (UINT32*)TTSCtrl.Data,
				TTS_BUFF_SIZE,
				MCI_PLAY_MODE_STREAM_PCM,
				pcm_play_callback,
				TTSCtrl.sample_rate,
				TTSCtrl.bit_rate);
		if (0 != result)
		{
			CORE("TTS Play Failed = %d", result);
			jtTTS_SynthStop(TTSCtrl.Handle);
			TTSCtrl.State = TTS_STATE_IDLE;
			//hal_DbgAssert("!");
		}
		else
		{
#if defined(VOLUME_WITH_15_LEVEL) || defined(VOLUME_WITH_7_LEVEL)
			DM_SetAudioVolume(DM_AUDIO_SPK_VOL_15);
#else
			DM_SetAudioVolume(DM_AUDIO_SPK_18dB);
#endif
			DM_SetAudioMode(DM_AUDIO_ON_TYPE);
			TTS_SpeakerModeStart();
			TTSCtrl.State = TTS_STATE_PLAYING;
		}

		break;

	case TTS_STATE_PLAYING:

		buf_src = (unsigned char*) pData;
		pcm_len = iSize;
/* 获取写入空间 */
		MCI_GetWriteBuffer( (UINT32 **)&buf_pcm, (UINT32 *)&buf_len);
		//CORE("TTS %u %u", buf_len, pcm_len);
//如果播放buffer后部不够整段合成buffer放入，先将部分放入buffer末尾，
//待播放buffer前部空出后，剩下的放入前部
		while (buf_len < pcm_len)
		{

			if (buf_len > 0)
			{
				memcpy(buf_pcm, buf_src, buf_len);
				pcm_len -= buf_len;
				buf_src += buf_len;
				MCI_AddedData( buf_len);
			}
			COS_Sleep(5);
			MCI_GetWriteBuffer( (UINT32 **)&buf_pcm, &buf_len);
		}

		memcpy(buf_pcm, buf_src, pcm_len);
		MCI_AddedData(pcm_len);
		//TTSCtrl.Pos += iSize;
		break;

	case TTS_STATE_IDLE:
		jtTTS_SynthStop(TTSCtrl.Handle);
	//	mmi_tts_send_notify(TTS_STATE_NOTIFY_ERROR);
		break;

	default:
		jtTTS_SynthStop(TTSCtrl.Handle);
		break;
	}



	return jtTTS_ERR_NONE;
}

void __TTS_Init(void)
{
	jtErrCode Error;
	long nSize;
	MCI_TaskInit();
	Error = jtTTS_GetExtBufSize(CNPackage, NULL, NULL, &nSize);
	if(Error != jtTTS_ERR_NONE)
	{
		CORE("TTS test, get buf size error!");
		TTSCtrl.State = TTS_STATE_ERROR;
		return ;
	}

	//CORE("TTS mem len %u", nSize);
	TTSCtrl.pHeap = COS_MALLOC(nSize);
	if (NULL == TTSCtrl.pHeap)
	{
		CORE("TTS pHeap, no memrey error!");
		TTSCtrl.State = TTS_STATE_ERROR;
		return ;
	}

	TTSCtrl.Data = COS_MALLOC(TTS_BUFF_SIZE);
	//CORE("%x", TTSCtrl.Data);
	if (NULL == TTSCtrl.Data)
	{
		CORE("TTS TTSData, no memrey error!");
		TTSCtrl.State = TTS_STATE_ERROR;
		COS_FREE(TTSCtrl.pHeap);
		return ;
	}

	memset(TTSCtrl.pHeap, 0, nSize);
	Error = jtTTS_Init(CNPackage, NULL, NULL, &TTSCtrl.Handle, TTSCtrl.pHeap);

	if(Error != jtTTS_ERR_NONE)
	{
		CORE("TTS test, init tts error!");
		COS_FREE(TTSCtrl.pHeap);
		COS_FREE(TTSCtrl.Data);
		TTSCtrl.pHeap = NULL;
		TTSCtrl.Handle = 0;
		TTSCtrl.State = TTS_STATE_ERROR;
		return ;
	}
	TTSCtrl.State = TTS_STATE_IDLE;
	jtTTS_SetParam(TTSCtrl.Handle, jtTTS_PARAM_CALLBACK_USERDATA, NULL);
	jtTTS_SetParam(TTSCtrl.Handle, jtTTS_PARAM_INPUTTXT_MODE, jtTTS_INPUT_TEXT_DIRECT);
	jtTTS_SetParam(TTSCtrl.Handle, jtTTS_PARAM_CODEPAGE, jtTTS_CODEPAGE_UNICODE_BE);
	jtTTS_SetParam(TTSCtrl.Handle, jtTTS_PARAM_OUTPUT_CALLBACK, (long)TTS_OutputVoicePCMProc);
	jtTTS_SetParam(TTSCtrl.Handle, jtTTS_PARAM_VOLUME, jtTTS_VOLUME_MAX);
	jtTTS_SetParam(TTSCtrl.Handle, jtTTS_PARAM_ENG_MODE, jtTTS_ENG_AUTO);
	jtTTS_SetParam(TTSCtrl.Handle, jtTTS_PARAM_SPEAK_STYLE, jtTTS_SPEAK_STYLE_NORMAL);
	jtTTS_SetParam(TTSCtrl.Handle, jtTTS_PARAM_SPEED, jtTTS_SPEED_NORMAL + 100);
	jtTTS_SetParam(TTSCtrl.Handle, jtTTS_PARAM_SOUNDEFFECT, jtTTS_SOUNDEFFECT_BASE);
	jtTTS_SetParam(TTSCtrl.Handle, jtTTS_PARAM_PITCH, jtTTS_PITCH_NORMAL);
	TTSCtrl.Pos = 0;
	//DM_SetAudioMode(DM_AUDIO_MODE_LOUDSPEAKER);
	return ;
}


int32_t __TTS_Play(void *Data, uint32_t Len, void *PCMCB, void *TTSCB)
{
	//long nSize = 0;

	//unsigned char *pHeap;
	jtErrCode Error;
    //INT32 Result;

	TTSCtrl.Pos = 0;
	if (TTSCtrl.State == TTS_STATE_ERROR)
	{
	    return -1;
	}

//	if (TTSCtrl.State != TTS_STATE_IDLE)
//	{
//		return -1;
//	}

	get_wav_format(TTSCtrl.Handle, &TTSCtrl.sample_rate, &TTSCtrl.bit_rate);
	CORE("TTS nSamplesPerSec is:%u, the wBitsPerSample is: %u", TTSCtrl.sample_rate, TTSCtrl.bit_rate);

	TTSCtrl.State = TTS_STATE_SYNTHESIZING;
    MCI_AudioStopBuffer();
    MCI_DataFinished();
    jtTTS_SynthStop(TTSCtrl.Handle);
	Error = jtTTS_SynthesizeText(TTSCtrl.Handle, Data,  Len);
	TTSCtrl.PCMCB = (MyAPIFunc)PCMCB;
	TTSCtrl.TTSCB = (MyAPIFunc)TTSCB;
	return Error;

}
