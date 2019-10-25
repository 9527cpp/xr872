/*
 * Copyright (C) 2017 XRADIO TECHNOLOGY CO., LTD. All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the
 *       distribution.
 *    3. Neither the name of XRADIO TECHNOLOGY CO., LTD. nor the names of
 *       its contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "kernel/os/os_thread.h"
#include "audio/pcm/audio_pcm.h"
#include "audio/manager/audio_manager.h"
#include "debug.h"
#include "common/framework/fs_ctrl.h"
#include "fs/fatfs/ff.h"

//#define AEC_SUPPORT
//#define ASR_SUPPORT

#ifdef AEC_SUPPORT
//TODO:add head file of aec algorithm
#endif

#ifdef ASR_SUPPORT
//TODO:add head file of asr algorithm
#endif

#define SAMPLE_SIZE     2
#define SAMPLE_10MS     160   /* for 16 kHz */
#define SAMPLE_RATE     16000

#define AUDIO_CARD_ID   AUDIO_SND_CARD_DEFAULT

#define AEC_THREAD_STACK_SIZE    (1024 * 4)

#ifdef ASR_SUPPORT
struct AsrContext {
	//TODO:add value of asr algorithm
	int keyword;
};
#endif

#ifdef AEC_SUPPORT
struct AecContext {
	//TODO:add value of aec algorithm
	int reserve;
};
#endif

struct audioContext {
	unsigned int ReadLen;
	void *ReadData;
	short *MicSig;
	short *AecRef;
	short *LOut;
};

static OS_Thread_t aec_thread;

#ifdef ASR_SUPPORT
static int Asr_Init(struct AsrContext *pAsr)
{
	memset(pAsr, 0, sizeof(*pAsr));

	//TODO:init asr algorithm.0 for success, -1 for fail.
	return 0;
}

static inline int Asr_ProcessSample(struct AsrContext *pAsr, void *sample, unsigned int len)
{
	//TODO:process a sample.
	//and you can save recognition result in value "keyword"
	return 0;
}

static void Asr_Deinit(struct AsrContext *pAsr)
{
	//TODO:deinit asr algorithm
}
#endif

#ifdef AEC_SUPPORT
static int Aec_Init(struct AecContext *pAec)
{
	memset(pAec, 0, sizeof(*pAec));

	//TODO:init asr algorithm.0 for success, -1 for fail.
	return 0;
}

static inline int Aec_ProcessTx(struct AecContext *pAec, short *MicSig, short *AecRef, short *LOut)
{
	//TODO:process a sample.
	//input: micsig and aecref
	//output: lout
	return 0;
}

static void Aec_Deinit(struct AecContext *pAec)
{
	//TODO:deinit asr algorithm
}
#endif

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
	if (status != HAL_OK) {
		printf("ac107 init fail.\n");
		return -1;
	}

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

	ret = snd_pcm_open(AUDIO_CARD_ID, PCM_IN, &pcm_cfg);
	if (ret != 0) {
		printf("pcm open error\n");
		ac107_pdm_deinit();
		return -1;
	}

	return 0;
}

static int audio_pcm_close(struct audioContext *pAudio)
{
	snd_pcm_close(AUDIO_CARD_ID, PCM_IN);
	ac107_pdm_deinit();

	return 0;
}

static int audio_record_start(struct audioContext *pAudio)
{
	int ret = 0;

	memset(pAudio, 0, sizeof(*pAudio));

	ret = audio_pcm_open(pAudio);
	if (ret) {
		return -1;
	}

	pAudio->ReadLen = SAMPLE_10MS * SAMPLE_SIZE * 4;
	pAudio->ReadData = (void *)malloc(pAudio->ReadLen);
	if (pAudio->ReadData == NULL) {
		goto err;
	}

	pAudio->AecRef = (short *)malloc(SAMPLE_10MS * SAMPLE_SIZE);
	if (pAudio->AecRef == NULL) {
		goto err;
	}

	pAudio->MicSig = (short *)malloc(SAMPLE_10MS * SAMPLE_SIZE * 2);
	if (pAudio->MicSig == NULL) {
		goto err;
	}

	pAudio->LOut = (short *)malloc(SAMPLE_10MS * SAMPLE_SIZE);
	if (pAudio->LOut == NULL) {
		goto err;
	}

	return 0;

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
	int i;
	int ret;

	ret = snd_pcm_read(AUDIO_CARD_ID, pAudio->ReadData, pAudio->ReadLen);
	if (ret != pAudio->ReadLen) {
		printf("snd_pcm_read fail.\n");
		return -1;
	}

	for (i = 0; i < SAMPLE_10MS; i++) {
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

#ifdef ASR_SUPPORT
static void wakeup_result_process(struct audioContext *pAudio, int isKeyword)
{
	if (isKeyword) {
		printf("keyword recognized success.\n");
	}
}
#endif

static void aec_asr_task(void *arg)
{
	int ret;
	struct audioContext aContext;

#ifdef ASR_SUPPORT
	struct AsrContext AsrContext;
	ret = Asr_Init(&AsrContext);
	if (ret) {
		goto exit0;
	}
#endif

#ifdef AEC_SUPPORT
	struct AecContext AecContext;
	ret = Aec_Init(&AecContext);
	if (ret) {
		goto exit1;
	}
#endif


	if (fs_ctrl_mount(FS_MNT_DEV_TYPE_SDCARD, 0) != 0) {
		printf("mount fail\n");
		return -1;
	}
	FIL file;
	unsigned int writeLen;
	f_unlink("record/audio.pcm");
	f_open(&file, "record/audio.pcm", FA_CREATE_ALWAYS | FA_READ | FA_WRITE);
	
	unsigned int tick;
	unsigned int startTime;
	unsigned int nowTime;
	int  SAVE_RECORD_DATA_DURATION_MS= 15000;

	tick = OS_GetTicks();
	startTime = OS_TicksToMSecs(tick);

	ret = audio_record_start(&aContext);
	if (ret) {
		goto exit2;
	}
	printf("start\r\n");
	while (1) {
		audio_get_micsig_aecref(&aContext);
#ifdef AEC_SUPPORT
		Aec_ProcessTx(&AecContext, aContext.MicSig, aContext.AecRef, aContext.LOut);
#endif

#ifdef ASR_SUPPORT
#ifdef AEC_SUPPORT
		Asr_ProcessSample(&AsrContext, aContext.LOut, SAMPLE_10MS);
#else
		Asr_ProcessSample(&AsrContext, aContext.MicSig, SAMPLE_10MS);
#endif
		wakeup_result_process(&aContext, AsrContext.keyword);
#endif

		f_write(&file, aContext.MicSig, aContext.ReadLen/2, &writeLen);

		tick = OS_GetTicks();
		nowTime = OS_TicksToMSecs(tick);
		if ((nowTime - startTime) > SAVE_RECORD_DATA_DURATION_MS) {
			break;
		}

		printf("writeLen:%d\r\n",writeLen);
	}
	f_close(&file);
	printf("end\r\n");
	audio_record_stop(&aContext);
exit2:
#ifdef AEC_SUPPORT
	Aec_Deinit(&AecContext);
exit1:
#endif

#ifdef ASR_SUPPORT
	Asr_Deinit(&AsrContext);
exit0:
#endif
	OS_ThreadDelete(&aec_thread);
}

int aec_asr_start()
{
	if (OS_ThreadCreate(&aec_thread,
                        "aec_task",
                        aec_asr_task,
                        NULL,
                        OS_THREAD_PRIO_APP,
                        AEC_THREAD_STACK_SIZE) != OS_OK) {
		printf("thread create error\n");
		return -1;
	}
	return 0;
}

