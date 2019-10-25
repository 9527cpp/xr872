#ifndef __YTV_ASR_TINY_GEM_INTERFACE_HEADER__
#define __YTV_ASR_TINY_GEM_INTERFACE_HEADER__



typedef void*  YTV_ASR_GEM_CONTAINER_POINTER;
typedef void*  YTV_ASR_GEM_ENGINE_POINTER;






#ifdef __cplusplus
	extern "C" {
#endif



	//TERENCE---2017/03/25


	YTV_ASR_GEM_CONTAINER_POINTER ytv_asr_tiny_create_container(void *pReserved, char *strReservedOne,unsigned int nLangID, char *strReservedTwo);
	int ytv_asr_tiny_create_engine(YTV_ASR_GEM_CONTAINER_POINTER pContainer,char *pMemoryBufferForEngine,int nMemSizeInByte,YTV_ASR_GEM_ENGINE_POINTER* ppEngineInstance);

	int ytv_asr_tiny_put_pcm_data(YTV_ASR_GEM_ENGINE_POINTER pEngineInstance,short *pInputSample, int nSampleNumber, char *pFlag_VAD);

	int ytv_asr_tiny_run_one_step(YTV_ASR_GEM_ENGINE_POINTER pEngineInstance);

	int ytv_asr_tiny_notify_data_end(YTV_ASR_GEM_ENGINE_POINTER pEngineInstance);

	int ytv_asr_tiny_get_result(YTV_ASR_GEM_ENGINE_POINTER pEngineInstance,int N_MAX, int *pResultNumber, int *pOriginalWordIndex);

	int ytv_asr_tiny_reset_engine(YTV_ASR_GEM_ENGINE_POINTER pEngineInstance);
	int ytv_asr_tiny_reset_ns(YTV_ASR_GEM_ENGINE_POINTER pEngineInstance);

	int ytv_asr_tiny_free_engine(YTV_ASR_GEM_ENGINE_POINTER pEngineInstance);
	int ytv_asr_tiny_free_container(YTV_ASR_GEM_CONTAINER_POINTER pContainer);


	char *ytv_asr_tiny_get_one_command_gbk(YTV_ASR_GEM_ENGINE_POINTER pEngineInstance, int nCommandIndex);
	
	int ytv_asr_tiny_set_callback_for_debug(void (*)(char *strMessage, unsigned int *pTimeSnapshot));


	int ytv_asr_tiny_get_session_index(YTV_ASR_GEM_ENGINE_POINTER pEngineInstance);





#ifdef __cplusplus
	}
#endif


#endif
