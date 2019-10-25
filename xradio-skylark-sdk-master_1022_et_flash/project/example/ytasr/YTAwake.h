/*********************************************************************************
  *Copyright(C),http://www.youngtone.cn
  *FileName:  YTAwake.h
  *Author:  Àî¾ý
  *Version:  v1.0
  *Date:  2018.11.11
**********************************************************************************/
#ifndef __YT_AWAKE_H__
#define __YT_AWAKE_H__


#define DEBUG_WITH_INDEX_INFO 0

typedef struct tagAwakeCB
{
	void (*local_vad_start)();
	void (*local_vad_end)();
	void (*local_awake)(int);
	void (*local_not_awake)();
	void (*cloud_vad_start)();
	void (*cloud_vad_end)();
}AwakeCB;

#ifdef __cplusplus
extern "C"{
#endif

int YTAwake_Init();
void YTAwake_Reinit();
void YTAwake_Start();
void YTAwake_Stop();
void YTAwake_SetCallback(AwakeCB * pAwakeCB);
void YTAwake_OnData(unsigned char * buf, int len);
void YTAwake_SetAwakeFlag(int bAwake);
int YTAwake_GetAwakeFlag();

#ifdef __cplusplus
}
#endif	

#endif
