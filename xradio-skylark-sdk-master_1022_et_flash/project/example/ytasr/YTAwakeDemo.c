#include "YTAwake.h"
#include "YTAwakeDemo.h"
#if MTK_YT_TEST_FILE
#include "YTPcmData.h"
#endif
#include "FreeRTOS.h"
#include "task.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "YTLog.h"



#define NEED_PRINT_TIME_INFO 1

#ifdef __cplusplus
extern "C"{
#endif

#if defined(MTK_MINICLI_ENABLE)
extern void cli_line(char *s);
extern bool bPreparePrint;
#endif

#if NEED_PRINT_TIME_INFO
int nStartTime = 0;
int nStopTime = 0;
#endif

#ifdef __cplusplus
}
#endif




static int in_session = 0;


//// for ytawake callback func
void local_vad_start()
{
	//LOGI("local_vad_start");
#if NEED_PRINT_TIME_INFO// 打印接下来 2s 的cpu情况
	nStartTime = xTaskGetTickCount();
#if defined(MTK_MINICLI_ENABLE)	
	bPreparePrint = false;
	cli_line("cpu");
#endif
#endif	
}

void local_vad_end()
{
	//LOGI("local_vad_end");	
}
 
void local_awake()
{
	YT_LOGI("local_awake");
#if NEED_PRINT_TIME_INFO		// 打印接下来 2s 的cpu情况		
	nStopTime = xTaskGetTickCount();
	YT_LOGI("spend time = %d",nStopTime - nStartTime);	
#if defined(MTK_MINICLI_ENABLE)	
	bPreparePrint = true;
#endif
#endif	

	/*add your code here*/
	//*YT,CMD,1#
	//printf();  or serial().wirte();
	in_session = 0;
}
 
void local_not_awake()
{
	YT_LOGI("local_not_awake");
#if	NEED_PRINT_TIME_INFO
	nStopTime = xTaskGetTickCount();
	YT_LOGI("spend time = %d",nStopTime - nStartTime);	
#if defined(MTK_MINICLI_ENABLE)		// 打印接下来 2s 的cpu情况		
	bPreparePrint = true;
#endif
#endif

	/*add your code here*/
	//*YT,CMD,1#
	//printf();  or serial().wirte();
	in_session = 0;
}
 
void cloud_vad_start()
{
	//LOGI("cloud_vad_start");
}
 
void cloud_vad_end()
{
	//LOGI("cloud_vad_end");
}







#if  MTK_YT_TEST_FILE
static const unsigned char nhxl_pcm[] = {

	#include "1REC1.pcm.txt"
};


int nOnePiceSize = 640;
#if DEBUG_WITH_INDEX_INFO	// 用于测试(校验是否漏数据)
char pTempData[641]={0};
#else
char pTempData[640]={0};	
#endif

void ReadPcmData()
{
	int dataStartTime = 0;
	int dataStopTime = 0;
	int need_or_no_need = 1;
#if DEBUG_WITH_INDEX_INFO
	unsigned char nStep = 0;
#endif
	int nCurrentIndex = 0;
	int nPcmTotalSize = sizeof(nhxl_pcm)/sizeof(unsigned char);
	YT_LOGI("nPcmTotalSize = %d",nPcmTotalSize);

	dataStartTime = xTaskGetTickCount();
	while(nCurrentIndex + nOnePiceSize < nPcmTotalSize)
	{
#if DEBUG_WITH_INDEX_INFO	// 用于测试(校验是否漏数据)
		nStep++;		
		memcpy(&pTempData[0],&nStep,1);
		memcpy(pTempData+1,nhxl_pcm+nCurrentIndex,nOnePiceSize);
		YT_LOGI("nStep = %d",nStep);
		YT_LOGI("nCurrentIndex = %d",nCurrentIndex);
		nCurrentIndex += nOnePiceSize;
		YTAwake_OnData((unsigned char *)pTempData,nOnePiceSize);
#else 
	#if 1
		memcpy(pTempData,nhxl_pcm+nCurrentIndex,nOnePiceSize);
		nCurrentIndex += nOnePiceSize;
		YTAwake_OnData((unsigned char *)pTempData,nOnePiceSize);
	#else	
		int i = 0;
		int j = 0;
		for(i = 0,j=0;i<nOnePiceSize;i+=2)//320个 样点 数据
		{
			if(i%64!=0)
			{
				pTempData[j++] = *(nhxl_pcm+nCurrentIndex);
				pTempData[j++] = *(nhxl_pcm+nCurrentIndex+1);
			}
			nCurrentIndex+=2;
		}
		YTAwake_OnData((unsigned char *)pTempData,j);
	#endif	
#endif	
		
		vTaskDelay(20);	/// 8k 20 16k 10	
	}
	
	dataStopTime = xTaskGetTickCount();
	YT_LOGI("data finished time = %d, nCurrentIndex  =%d",dataStopTime - dataStartTime,nCurrentIndex);
}




void YTAwakeTestFileTask(void *arg)
{
	YT_LOGI("Entry YTAwakeTestFileTask");

	
	AwakeCB *pAwakeCB = NULL;
	
	pAwakeCB = (AwakeCB *)malloc(sizeof(AwakeCB)) ;
	if(pAwakeCB)
	{
		pAwakeCB->local_vad_start = local_vad_start;
		pAwakeCB->local_vad_end = local_vad_end;
		pAwakeCB->local_awake= local_awake;
		pAwakeCB->local_not_awake= local_not_awake;
		pAwakeCB->cloud_vad_start = cloud_vad_start;
		pAwakeCB->cloud_vad_end = cloud_vad_end;		
	}
	YTAwake_SetCallback(pAwakeCB);

	YTAwake_Start();
	
	
#if 0	// 慢速测试
	while(1)
	{	
		vTaskDelay(1000);
		YTAwake::Instance().Stop();
		vTaskDelay(1000);
		YTAwake::Instance().Start();		
		ReadPcmData();
		vTaskDelay(5000);		
	}
#else	// 快速测试
	while(1)
	{
		vTaskDelay(1000);	
		int iCount = 25;
		int i = 1;
		while(i < iCount)
		{
			if(in_session ==0)
			{
				in_session = 1;
				YT_LOGI("Round %d",i);		
				ReadPcmData();
				i++;	
			}
			else vTaskDelay(10);
		}	
	}
#endif
}

//// 暂时用文件模拟实时音频流数据 后续接recorder线程
void YTAwakeRunTestFile()
{
	void *thTestFile;
	BaseType_t x = 0;

	YTAwake_Init();
	x = xTaskCreate(YTAwakeTestFileTask,"YTAwakeTestFileTask",1024*4,NULL,1,&thTestFile);
	
	if (x != pdPASS) {YT_LOGE("create task: failed");}
	else {YT_LOGI("create task: succeeded");}
}

#elif MTK_YT_TEST_STREAM

#include "audio/pcm/audio_pcm.h"
#include "audio/manager/audio_manager.h"
#include "common/framework/fs_ctrl.h"
#include "fs/fatfs/ff.h"


#define SAVE_RECORD_DATA_DURATION_MS   15000


/*for ac107*/
struct audioContext 
{	
	unsigned int ReadLen;	
	void *ReadData;	
	short *MicSig;	
	short *AecRef;	
	short *LOut;
};

#define SAMPLE_SIZE     2
#define SAMPLE_10MS     160   /* for 16 kHz */
#define SAMPLE_RATE     16000

extern HAL_Status ac107_pdm_init(Audio_Device device, uint16_t volume, uint32_t sample_rate);
extern HAL_Status ac107_pdm_deinit(void);

static int audio_pcm_open(struct audioContext *pAudio)
{	
	int ret = 0;	
	HAL_Status status;	
	struct pcm_config pcm_cfg;	
	memset(pAudio, 0, sizeof(*pAudio));	
	/* start ac107, pdm mode */	
	status = ac107_pdm_init(AUDIO_IN_DEV_AMIC, 31, SAMPLE_RATE);	
	if (status != HAL_OK) {		printf("ac107 init fail.\n");		return -1;	}	
	/* enable 4 channels */    
	audio_manager_handler(AUDIO_SND_CARD_DEFAULT, AUDIO_MANAGER_SET_ROUTE, AUDIO_IN_DEV_AMIC, 1);    
	audio_manager_handler(AUDIO_SND_CARD_DEFAULT, AUDIO_MANAGER_SET_ROUTE, AUDIO_IN_DEV_LINEIN, 1);    
	audio_manager_handler(AUDIO_SND_CARD_DEFAULT, AUDIO_MANAGER_SET_ROUTE, AUDIO_IN_DEV_DMIC, 1);	
	/* set volume of amic and linein */	
	audio_manager_handler(AUDIO_SND_CARD_DEFAULT, AUDIO_MANAGER_SET_VOLUME_LEVEL, AUDIO_IN_DEV_AMIC, 3);	
	audio_manager_handler(AUDIO_SND_CARD_DEFAULT, AUDIO_MANAGER_SET_VOLUME_LEVEL, AUDIO_IN_DEV_LINEIN, 7);	
	pcm_cfg.channels = 4;	
	pcm_cfg.format = PCM_FORMAT_S16_LE;	
	pcm_cfg.period_count = 2;	
	pcm_cfg.period_size = SAMPLE_10MS;	
	pcm_cfg.rate = SAMPLE_RATE;	

	printf("ac107 init successs\n");
	
	ret = snd_pcm_open(AUDIO_SND_CARD_DEFAULT, PCM_IN, &pcm_cfg);	
	if (ret != 0) 
	{		
		printf("pcm open error\n");		
		ac107_pdm_deinit();		
		return -1;	
	}	
	return 0;
}

static int audio_pcm_close(struct audioContext *pAudio)
{	
	snd_pcm_close(AUDIO_SND_CARD_DEFAULT, PCM_IN);	
	ac107_pdm_deinit();	
	return 0;
}
static int audio_record_start(struct audioContext *pAudio)
{	
	int ret = 0;	
	memset(pAudio, 0, sizeof(*pAudio));	
	ret = audio_pcm_open(pAudio);	
	if (ret) {	return -1;	}	
	pAudio->ReadLen = SAMPLE_10MS * SAMPLE_SIZE * 4;	
	pAudio->ReadData = (void *)malloc(pAudio->ReadLen);	
	if(pAudio->ReadData == NULL) {		goto err;	}	
	pAudio->AecRef = (short *)malloc(SAMPLE_10MS * SAMPLE_SIZE);	
	if(pAudio->AecRef == NULL) {		goto err;	}	
	pAudio->MicSig = (short *)malloc(SAMPLE_10MS * SAMPLE_SIZE * 2);	
	if (pAudio->MicSig == NULL) {		goto err;	}	
	pAudio->LOut = (short *)malloc(SAMPLE_10MS * SAMPLE_SIZE);	
	if (pAudio->LOut == NULL) {		goto err;	}	return 0;
err:	
	free(pAudio->ReadData);	
	free(pAudio->MicSig);	
	free(pAudio->AecRef);	
	free(pAudio->LOut);	
	audio_pcm_close(pAudio);	
	return -1;
}


static int audio_get_micsig_aecref(struct audioContext *pAudio)
{	
	int i;	int ret;	
	ret = snd_pcm_read(AUDIO_SND_CARD_DEFAULT, pAudio->ReadData, pAudio->ReadLen);	
	if (ret != pAudio->ReadLen) 
	{		
		printf("snd_pcm_read fail.\n");		
		return -1;	
	}	
	for (i = 0; i < SAMPLE_10MS; i++) 
	{	
		pAudio->AecRef[i] = ((short *)pAudio->ReadData)[4 * i + 1];		
		pAudio->MicSig[i] = ((short *)pAudio->ReadData)[4 * i + 2];		
		pAudio->MicSig[i + SAMPLE_10MS] = ((short *)pAudio->ReadData)[4 * i + 3];	
	}	
	return 0;
}

static void audio_record_stop(struct audioContext *pAudio)
{	
	free(pAudio->LOut);	
	free(pAudio->MicSig);	
	free(pAudio->AecRef);	
	free(pAudio->ReadData);	
	audio_pcm_close(pAudio);
}


#define DEBUG_WITH_FILE 0

static void audio_driver_record(void)
{
	int ret;
	void *data;
	unsigned int len;
	unsigned int writeLen;
	struct pcm_config config;

		
#if DEBUG_WITH_FILE // use file debug --> write to sd
	unsigned int tick;
	unsigned int startTime;
	unsigned int nowTime;
	FIL file;
	if (fs_ctrl_mount(FS_MNT_DEV_TYPE_SDCARD, 0) != 0) {
		printf("mount fail\n");
		return -1;
	}
	f_unlink("record/audio.pcm");
	f_open(&file, "record/audio.pcm", FA_CREATE_ALWAYS | FA_READ | FA_WRITE);
#endif	

#if !PRJCONF_AC107_SOUNDCARD_EN/// use internal
	audio_manager_handler(AUDIO_SND_CARD_DEFAULT, AUDIO_MANAGER_SET_VOLUME_LEVEL, AUDIO_IN_DEV_AMIC, 3);
	audio_manager_handler(AUDIO_SND_CARD_DEFAULT, AUDIO_MANAGER_SET_VOLUME_GAIN, AUDIO_IN_DEV_AMIC, VOLUME_GAIN_39dB);

	config.channels = 1;
	config.format = PCM_FORMAT_S16_LE;
	config.period_count = 1;
	config.period_size = 640;
	config.rate = 16000;
	ret = snd_pcm_open(AUDIO_SND_CARD_DEFAULT, PCM_IN, &config);
	if (ret) {
		printf("snd_pcm_open fail.\n");
		goto err;
	}
	len = config.channels * config.period_count * config.period_size;
	data = malloc(len);
	if (data == NULL) {
		goto err1;
	}
#else /// use ac107
	struct audioContext aContext;
	ret = audio_record_start(&aContext);
	if (ret) {
		goto err;
	}
	printf("start\r\n");
#endif	

#if DEBUG_WITH_FILE
	tick = OS_GetTicks();
	startTime = OS_TicksToMSecs(tick);
	printf("===start record pcm by audio driver, last for %dms===\n", SAVE_RECORD_DATA_DURATION_MS);
#endif

	while (1) {
	#if !PRJCONF_AC107_SOUNDCARD_EN	
		ret = snd_pcm_read(AUDIO_SND_CARD_DEFAULT, data, len);
		if (ret != len) {
			printf("snd_pcm_read fail.\n");
		}
	#else
		audio_get_micsig_aecref(&aContext);
	#endif
#if DEBUG_WITH_FILE	
	#if PRJCONF_AC107_SOUNDCARD_EN	
		f_write(&file, aContext.MicSig, aContext.ReadLen/4, &writeLen);
	#else
		f_write(&file, data, len, &writeLen);
	#endif
		printf("writeLen:%d\r\n",writeLen);

		tick = OS_GetTicks();
		nowTime = OS_TicksToMSecs(tick);
		if ((nowTime - startTime) > SAVE_RECORD_DATA_DURATION_MS) {
			break;
		}
#else	
	#if PRJCONF_AC107_SOUNDCARD_EN	
		YTAwake_OnData((unsigned char *)aContext.MicSig,aContext.ReadLen/4);
	#else
		YTAwake_OnData((unsigned char *)data,len);
	#endif	
#endif
	}
	printf("record pcm over.\n");
	free(data);
err1:
#if !PRJCONF_AC107_SOUNDCARD_EN
	snd_pcm_close(AUDIO_SND_CARD_DEFAULT, PCM_IN);
#else
	printf("end\r\n");
	audio_record_stop(&aContext);
#endif
err:

#if DEBUG_WITH_FILE
	f_close(&file);
#endif
	return;
} 

void YTAwakeTestRecorderTask(void *arg)
{
	YT_LOGI("YTAwakeTestRecorderTask");
	
	AwakeCB *pAwakeCB = NULL;
	
	pAwakeCB = (AwakeCB *)malloc(sizeof(AwakeCB)) ;
	if(pAwakeCB)
	{
		pAwakeCB->local_vad_start = local_vad_start;
		pAwakeCB->local_vad_end = local_vad_end;
		pAwakeCB->local_awake= local_awake;
		pAwakeCB->local_not_awake= local_not_awake;
		pAwakeCB->cloud_vad_start = cloud_vad_start;
		pAwakeCB->cloud_vad_end = cloud_vad_end;		
	}
	YTAwake_SetCallback(pAwakeCB);

	YTAwake_Start();

	while(1)
	{
		vTaskDelay(5000);
		YT_LOGI("----------------------");
		YT_LOGI("-----start record-----");
		YT_LOGI("----------------------");
		audio_driver_record();
	}
}
void YTAwakeRunTestRecorder()
{
	void *thTestRecorder;
	YT_LOGI("YTAwakeRunTestRecorder");
	YTAwake_Init();
	BaseType_t x = xTaskCreate(YTAwakeTestRecorderTask,"YTAwakeTestRecorderTask",1024*4,NULL,1,&thTestRecorder);
	if (x != pdPASS) {YT_LOGE("create task: failed");}
	else {YT_LOGI("create task: succeeded");}	
}
#endif
