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

#include <stdio.h>
#include <string.h>
#include "common/framework/fs_ctrl.h"
#include "fs/fatfs/ff.h"
#include "common/apps/recorder_app.h"
#include "common/framework/platform_init.h"
#include "ExampleCustomerWriter.h"
#include "kernel/os/os_time.h"
#include "audio/pcm/audio_pcm.h"
#include "audio/manager/audio_manager.h"

static void cedarx_record(void)
{
	recorder_base *recorder;
	rec_cfg cfg;
	char music_url[64];
	CdxWriterT *writer;

	recorder = recorder_create();
	if (recorder == NULL) {
		printf("recorder create fail, exit\n");
		return;
	}

	/* record a 15s amr media */
	cfg.type = XRECODER_AUDIO_ENCODE_AMR_TYPE;
	printf("===start record amr now, last for 15s===\n");
	recorder->start(recorder, "file://record/1.amr", &cfg);
	OS_Sleep(15);
	recorder->stop(recorder);
	printf("record amr over.\n");

	/* record a 15s pcm media */
	cfg.type = XRECODER_AUDIO_ENCODE_PCM_TYPE;
	cfg.sample_rate = 8000;
	cfg.chan_num = 1;
	cfg.bitrate = 12200;
	cfg.sampler_bits = 16;
	printf("===start record pcm now, last for 15s===\n");
	recorder->start(recorder, "file://record/1.pcm", &cfg);
	OS_Sleep(15);
	recorder->stop(recorder);
	printf("record pcm over.\n");

	/* record a 15s amr media by customer writer */
	cfg.type = XRECODER_AUDIO_ENCODE_AMR_TYPE;
	printf("===start record amr by customer writer now, last for 15s===\n");
	writer = ExampleCustomerWriterCreat();
	if (writer == NULL) {
		goto exit;
	}
	sprintf(music_url, "customer://%p", writer);
	recorder->start(recorder, music_url, &cfg);
	OS_Sleep(15);
	recorder->stop(recorder);
	printf("record amr over.\n");

exit:
	recorder_destroy(recorder);
}

#define SAVE_RECORD_DATA_DURATION_MS   15000

static void audio_driver_record(void)
{
	int ret;
	FIL file;
	void *data;
	unsigned int len;
	unsigned int writeLen;
	struct pcm_config config;
	unsigned int tick;
	unsigned int startTime;
	unsigned int nowTime;

	f_unlink("record/audio.pcm");
	f_open(&file, "record/audio.pcm", FA_CREATE_ALWAYS | FA_READ | FA_WRITE);

	config.channels = 1;
	config.format = PCM_FORMAT_S16_LE;
#if 0	
	config.period_count = 2;
	config.period_size = 1024;
	config.rate = 8000;
#else
	config.period_count = 2;//must be 2
	config.period_size = 1024;
	config.rate = 16000;
#endif
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

	tick = OS_GetTicks();
	startTime = OS_TicksToMSecs(tick);

	printf("===start record pcm by audio driver, last for %dms===\n", SAVE_RECORD_DATA_DURATION_MS);
	while (1) {
		ret = snd_pcm_read(AUDIO_SND_CARD_DEFAULT, data, len);
		if (ret != len) {
			printf("snd_pcm_read fail.\n");
		}
		printf("ret = %d\r\n",ret);
		
		f_write(&file, data, len, &writeLen);

		tick = OS_GetTicks();
		nowTime = OS_TicksToMSecs(tick);
		if ((nowTime - startTime) > SAVE_RECORD_DATA_DURATION_MS) {
			break;
		}
	}
	printf("record pcm over.\n");

	free(data);

err1:
	snd_pcm_close(AUDIO_SND_CARD_DEFAULT, PCM_IN);
err:
	f_close(&file);
	return;
}


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


int main(void)
{
	platform_init();

	if (fs_ctrl_mount(FS_MNT_DEV_TYPE_SDCARD, 0) != 0) {
		printf("mount fail\n");
		return -1;
	}

	printf("audio record start.\n");
	int ret;	
	struct audioContext aContext;
	ret = audio_record_start(&aContext);
	
	FIL file;
	unsigned int writeLen;
	f_unlink("record/audio.pcm");
	f_open(&file, "record/audio.pcm", FA_CREATE_ALWAYS | FA_READ | FA_WRITE);
	
	unsigned int tick;
	unsigned int startTime;
	unsigned int nowTime;


	tick = OS_GetTicks();
	startTime = OS_TicksToMSecs(tick);
	if (ret) 
	{		
	 	goto exit2;	
	}	
	while (1) 
	{		
		audio_get_micsig_aecref(&aContext);
		
		f_write(&file, aContext.MicSig, aContext.ReadLen/2, &writeLen);

		tick = OS_GetTicks();
		nowTime = OS_TicksToMSecs(tick);
		if ((nowTime - startTime) > SAVE_RECORD_DATA_DURATION_MS) {
			break;
		}

		printf("writeLen:%d\r\n",writeLen);
	}
	f_close(&file);
	audio_record_stop(&aContext);	
exit2:
	
/*
	audio_manager_handler(AUDIO_SND_CARD_DEFAULT, AUDIO_MANAGER_SET_VOLUME_LEVEL, AUDIO_IN_DEV_AMIC, 3);
	//audio_manager_handler(AUDIO_SND_CARD_DEFAULT, AUDIO_MANAGER_SET_VOLUME_GAIN, AUDIO_IN_DEV_AMIC, VOLUME_GAIN_32dB);
	printf("start to use cedarx to record amr/pcm\n");
	cedarx_record();
	printf("start to use audio driver to record pcm\n");
	audio_driver_record();
*/
	printf("audio record over.\n");
	return 0;
}
