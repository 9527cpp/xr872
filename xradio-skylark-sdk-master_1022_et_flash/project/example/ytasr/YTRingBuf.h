
#ifndef __YTRINGBUF_H__
#define __YTRINGBUF_H__
#include <stdint.h>

void *YTRingBuf_Init(int nSize,char * pBuffer);
void YTRingBuf_Reset(void *pRingbuf);
int YTRingBuf_WriteBytes(void *pRingbuf,int8_t* buf, int32_t buf_len);
void YTRingBuf_Write2Bytes(void *pRingbuf,uint8_t byte, uint8_t byte_2);
int YTRingBuf_GetData(void *pRingbuf,int8_t* buf, int32_t buf_len);
int YTRingBuf_GetDataCount(void *pRingbuf);

#endif


