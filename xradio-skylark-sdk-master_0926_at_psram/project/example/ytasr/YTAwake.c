/*********************************************************************************
  *Copyright(C),http://www.youngtone.cn
  *FileName:  YTAwake.cpp
  *Author:  李君
  *Version:  v1.0
  *Date:  2018.11.11
**********************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"

#include "YTAwake.h"
#include "YTLog.h"
#include "ytv_asr_tiny_interface.h"

#define YTASR_MEM_SIZE (135*1024)
static int nFlameSample = 80;
static int pOriginalWordIndex[1] = {-1};
static int nResultNumber = 0;
static char * szResultUTF8 = NULL;
static const int nMaxCmd = 1;
static char cFlag_VAD;
static int nFlame = 0;
///// for run thread
static void * 	thAwakeRunOne = NULL;
static void *	pEngine = NULL;
static void *	pMemory = NULL;
static void *	pContainer = NULL;
static int	bIsInit = 0;
static int	bIsAwake = 0;
static int 	bCanRunOne = 0;
static int 	bVadStart = 0;
static int 	bVadEnd = 0;
static AwakeCB *pAwakeCB = NULL;
#define  nRingBufLen (640*10*5)
static char RingBuf[nRingBufLen];
static void * pRingBuf = NULL;

static void GetResult()
{
#if 1
	ytv_asr_tiny_notify_data_end(pEngine);			
	ytv_asr_tiny_get_result(pEngine,nMaxCmd, &nResultNumber,pOriginalWordIndex);
	if(nResultNumber)szResultUTF8 = ytv_asr_tiny_get_one_command_gbk(pEngine,pOriginalWordIndex[0]);	
	YT_LOGI("Result:%s",szResultUTF8?szResultUTF8:"no result!!!");	
	ytv_asr_tiny_reset_engine(pEngine); 			
	szResultUTF8 = NULL;
#endif	
}



static int nTotalLen = 0;
static void PutVadDataForAwake(unsigned char *ppData,int nSize,int flag)
{
	if(flag == 1)
	{
		nSize/=sizeof(short);
		nFlame = nSize/nFlameSample;
		for(int i = 0; i < nFlame; i++)
		{	
			cFlag_VAD = 0;
			ytv_asr_tiny_put_pcm_data(pEngine,(short *)(ppData + i*nFlameSample*sizeof(short)),nFlameSample,&cFlag_VAD);
			if(cFlag_VAD == 'S')
			{	
				bVadEnd = 0;
				bVadStart = 1;			
				YT_LOGI("VadStart"); 
				if(pAwakeCB)pAwakeCB->local_vad_start();
			}
			else if(cFlag_VAD == 'Y' && !bVadEnd) 
			{	
				bVadStart = 0;
				bVadEnd = 1;
				YT_LOGI("VadEnd");	
				if(pAwakeCB)pAwakeCB->local_vad_end();
				break;			
			}
		}
	}
	else if(flag == 0)
	{
		if(!bVadEnd)
		nTotalLen +=YTRingBuf_WriteBytes(pRingBuf,ppData,nSize);
		//YT_LOGI("nTotalLen:%d",nTotalLen);
		/*
		for(int i = 0; i < nSize; i+=2)
		{
			YTRingBuf_Write2Bytes(pRingBuf,ppData[i],ppData[i+1]);
		}
		*/
	}
}

static void RunOneTask(void *arg)
{
#if 1
	int nRunOne = -1;
	int nGetResultStartTime = 0;
	int nGetResultStopTime = 0;
    YT_LOGI("Entry RunOneTask!!");
	char pBuf[640];
	int len = 0;
	int nDataTotalLen = 0;
	//return ;
	
	while(1)
	{
		while(bCanRunOne)
		{	
		#if 1
			len = YTRingBuf_GetDataCount(pRingBuf);
			//YT_LOGI("len:%d",len);
			if(len >= 640)
			{
				memset(pBuf,0,640);
				len = YTRingBuf_GetData(pRingBuf,pBuf,640);
				nDataTotalLen+=len;
				//YT_LOGI("nDataTotalLen:%d",nDataTotalLen);
				
				PutVadDataForAwake(pBuf,len,1);
		#endif	
				if(bVadStart)
				{	
				#if 1
					nRunOne = ytv_asr_tiny_run_one_step(pEngine);
					YT_LOGI("nRunOne:%d",nRunOne);
				#endif
				}
				else if(bVadEnd)
				{

				#if 0
					nDataTotalLen = 0;
					nTotalLen = 0;
					
				#if 1
					nGetResultStartTime = xTaskGetTickCount();
					GetResult();
					nGetResultStopTime = xTaskGetTickCount();
					YT_LOGI("GetResult time = %d",nGetResultStopTime - nGetResultStartTime);
					if(pAwakeCB){
						if(nResultNumber){YTAwake_SetAwakeFlag(1);pAwakeCB->local_awake();}
						else {YTAwake_SetAwakeFlag(0);pAwakeCB->local_not_awake();}
					}				
					bVadEnd = 0;
					YTRingBuf_Reset(pRingBuf);
				#endif	
				#else
					nRunOne = ytv_asr_tiny_run_one_step(pEngine);
					YT_LOGI("nRunOne:%d",nRunOne);
					if(nRunOne==0)
					{
						nDataTotalLen = 0;
						nTotalLen = 0;				
						nGetResultStartTime = xTaskGetTickCount();
						GetResult();
						nGetResultStopTime = xTaskGetTickCount();
						YT_LOGI("GetResult time = %d",nGetResultStopTime - nGetResultStartTime);
						if(pAwakeCB){
							if(nResultNumber){YTAwake_SetAwakeFlag(1);pAwakeCB->local_awake();}
							else {YTAwake_SetAwakeFlag(0);pAwakeCB->local_not_awake();}
						}				
						bVadEnd = 0;
						YTRingBuf_Reset(pRingBuf);
					}
				#endif
				}
		#if 1		
			}
			else 
			{
				//nRunOne = ytv_asr_tiny_run_one_step(pEngine);
				if(bVadEnd)
				{
					nDataTotalLen = 0;
					nTotalLen = 0;
					
				#if 1
					nGetResultStartTime = xTaskGetTickCount();
					GetResult();
					nGetResultStopTime = xTaskGetTickCount();
					YT_LOGI("GetResult time = %d",nGetResultStopTime - nGetResultStartTime);
					if(pAwakeCB){
						if(nResultNumber){YTAwake_SetAwakeFlag(1);pAwakeCB->local_awake();}
						else {YTAwake_SetAwakeFlag(0);pAwakeCB->local_not_awake();}
					}				
					bVadEnd = 0;
					YTRingBuf_Reset(pRingBuf);
				#endif						
				}
				else
				vTaskDelay(10);
			}
		#else
			vTaskDelay(10);
		#endif
		}
		vTaskDelay(10);
	}
#endif	
}



int YTAwake_Init()
{
#if 1
	if(bIsInit)return bIsInit;
////////////////////////////////////////	
	YT_LOGI("Entry Engine Init");	
	bIsInit = 0;
	pMemory = (void *)malloc(YTASR_MEM_SIZE);
	if(!pMemory){YT_LOGI("Memory Alloc Fail");return bIsInit;}
	YT_LOGI("pMemory:%p",pMemory);		
 	pContainer = ytv_asr_tiny_create_container(NULL,NULL,3,NULL);
	bIsInit = (ytv_asr_tiny_create_engine(pContainer,(char*)pMemory,YTASR_MEM_SIZE,&pEngine) == 0)? 1:0;
	if(bIsInit) {YT_LOGI("Engine Init Success");}
	else {YT_LOGI("Engine Init Fail");}
	YT_LOGI("Leave Engine Init");
////////////////////////////////////////	
	YT_LOGI("Entry Task Init");
	BaseType_t x = xTaskCreate(RunOneTask,"RunOneTask",1024*4,NULL,1,&thAwakeRunOne);	
	if (x != pdPASS) {bIsInit = 0;YT_LOGI("create task: failed");} 
	else{bIsInit = 1;YT_LOGI("create task: succeeded"); }	
	YT_LOGI("Leave Task Init");	
#endif	
	pRingBuf = YTRingBuf_Init(nRingBufLen,RingBuf);
	return bIsInit;
}

void YTAwake_Reinit()
{
	if(!bIsInit)return;
	YT_LOGI("Reinit");
	if(pMemory)free(pMemory);
	pMemory = NULL;
	if(pContainer)free(pContainer);
	pContainer = NULL;
	if(pEngine)free(pEngine);
	pEngine = NULL;
	bIsInit = 0;
}

void YTAwake_SetCallback(AwakeCB * pArg)
{
	if(!bIsInit)return;
	pAwakeCB = pArg;
}

void YTAwake_Start()
{
	if(!bIsInit)return;
	YT_LOGI("Awake Start");
	bCanRunOne = 1;
	bVadEnd = 0;
	bVadStart = 0;
}

void YTAwake_Stop()
{
	if(!bIsInit)return;
	YT_LOGI("Awake Stop");
	bCanRunOne = 0;
	bVadEnd = 0;
	bVadStart = 0;
}

void YTAwake_OnData(unsigned char * buf, int len)
{
	if(!bIsInit)return;		
#if DEBUG_WITH_INDEX_INFO	// 用于测试(校验是否漏数据)
	YT_LOGI("nIndex = %d",buf[0]);
#endif
	if(!bVadEnd)
#if DEBUG_WITH_INDEX_INFO	// 用于测试(校验是否漏数据)
	PutVadDataForAwake(buf+1,len,0);
#else
	PutVadDataForAwake(buf,len,0);
	//PutVadDataForAwake(buf,len,1);
#endif
}

void YTAwake_SetAwakeFlag(int bAwake)
{
	bIsAwake = bAwake;
}
int YTAwake_GetAwakeFlag()
{
	return bIsAwake;
}


