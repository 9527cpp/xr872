#ifndef __YTAWAKE_DEMO_H__
#define __YTAWAKE_DEMO_H__




#ifdef __cplusplus
extern "C"{
#endif

#if  MTK_YT_TEST_FILE
void YTAwakeRunTestFile();
#elif MTK_YT_TEST_STREAM
void YTAwakeRunTestRecorder();
#endif

#ifdef __cplusplus
}
#endif

#endif