#include "YTRingBuf.h"
#include <stdlib.h>
#include <string.h>


typedef struct http_download_ring_buf_sturct{
    int8_t *buffer;
    int32_t buf_size;
    int32_t write_ptr;
    int32_t read_ptr;
} http_download_ring_buf_struct_t;





static void * http_download_ring_buf_init(int nSize,char * pBuffer){
	http_download_ring_buf_struct_t *g_http_download_ringbuf = (http_download_ring_buf_struct_t*)malloc(sizeof(http_download_ring_buf_struct_t));
	g_http_download_ringbuf->write_ptr = 0;
	g_http_download_ringbuf->read_ptr = 0;
    g_http_download_ringbuf->buf_size = nSize;	
	g_http_download_ringbuf->buffer = (int8_t*)pBuffer;	
	return (void *)g_http_download_ringbuf;
}
static int32_t http_download_ring_buf_free_space(void *pRingbuf)
{
    int32_t count = 0;
	
	http_download_ring_buf_struct_t *g_http_download_ringbuf = (http_download_ring_buf_struct_t *)pRingbuf;
	
	if (g_http_download_ringbuf->read_ptr > g_http_download_ringbuf->write_ptr){
		count = g_http_download_ringbuf->read_ptr  - g_http_download_ringbuf->write_ptr;
	}else if (g_http_download_ringbuf->read_ptr == g_http_download_ringbuf->write_ptr){
		count = g_http_download_ringbuf->buf_size;
	}else{
		count = g_http_download_ringbuf->buf_size - (g_http_download_ringbuf->write_ptr - g_http_download_ringbuf->read_ptr);
	}
    return count;
}
static int32_t http_download_ring_buf_data_count(void *pRingbuf)
{
    int32_t count = 0;

	http_download_ring_buf_struct_t *g_http_download_ringbuf = (http_download_ring_buf_struct_t *)pRingbuf;
	
	if (g_http_download_ringbuf->write_ptr >= g_http_download_ringbuf->read_ptr){
		count = g_http_download_ringbuf->write_ptr - g_http_download_ringbuf->read_ptr;	
	}else{
		count = g_http_download_ringbuf->buf_size  - (g_http_download_ringbuf->read_ptr - g_http_download_ringbuf->write_ptr);
	}
	return count;    
}
#if 0
int32_t http_download_ring_buf_put_data(int8_t* buf, int32_t buf_len)
{
	int16_t temp_len;
    if (buf_len > http_download_ring_buf_free_space()) {
		LOG_I(common, "ring_buf overflow!!, free count = %d", http_download_ring_buf_free_space());
		return 0;
    }    
    if (buf_len + g_http_download_ringbuf->write_ptr > HTT_DOWNLOAD_MAX_SIZE){
		temp_len = g_http_download_ringbuf->buf_size - g_http_download_ringbuf->write_ptr;
        memcpy(g_http_download_ringbuf->buffer + g_http_download_ringbuf->write_ptr, buf, temp_len);
        memcpy(g_http_download_ringbuf->buffer, buf + temp_len, buf_len - temp_len);
        g_http_download_ringbuf->write_ptr = buf_len - temp_len;
    } else {
        memcpy(g_http_download_ringbuf->buffer + g_http_download_ringbuf->write_ptr, buf, buf_len);
        g_http_download_ringbuf->write_ptr += buf_len;
    }
    //printf("read ptr : %d, write ptr : %d\n", g_http_download_ringbuf->read_ptr, g_http_download_ringbuf->write_ptr);
    return buf_len;
}
#endif
#if 1
static int32_t http_download_ring_buf_put_data(void *pRingbuf,int8_t* buf, int32_t buf_len)
{
	int16_t temp_len;

	http_download_ring_buf_struct_t *g_http_download_ringbuf = (http_download_ring_buf_struct_t *)pRingbuf;
	
    if (buf_len > http_download_ring_buf_free_space(g_http_download_ringbuf)) {
		
		return 0;
    }    
    if ((buf_len + g_http_download_ringbuf->write_ptr) > g_http_download_ringbuf->buf_size - 1){		
		temp_len = g_http_download_ringbuf->buf_size - g_http_download_ringbuf->write_ptr;		
        memcpy(g_http_download_ringbuf->buffer + g_http_download_ringbuf->write_ptr, buf, temp_len);
		if (((buf_len - temp_len) < g_http_download_ringbuf->read_ptr) && (buf_len - temp_len > 0)){
        	memcpy(g_http_download_ringbuf->buffer, buf + temp_len, buf_len - temp_len);
        	g_http_download_ringbuf->write_ptr = buf_len - temp_len;
		}else{
			g_http_download_ringbuf->write_ptr = 0;
		}
    } else {
        memcpy(g_http_download_ringbuf->buffer + g_http_download_ringbuf->write_ptr, buf, buf_len);
        
		g_http_download_ringbuf->write_ptr = ((g_http_download_ringbuf->write_ptr + buf_len) % (g_http_download_ringbuf->buf_size - 1));
    }
    return buf_len;
}
#endif
/**
* @brief       This function used to read data from ring buffer
* @param[in]   ring_buf: Pointer to a ring buffer
* @param[in/out]   buf: Pointer to buffer
* @param[in]   buf_len: buffer size
* @return      int32_t: the read data size
*/
static int32_t http_download_ring_buf_get_data(void *pRingbuf,int8_t* buf, int32_t buf_len)
{

    int32_t consume_len;

	http_download_ring_buf_struct_t *g_http_download_ringbuf = (http_download_ring_buf_struct_t *)pRingbuf;
	
    int32_t end_ind  = g_http_download_ringbuf->write_ptr;
    
    if (buf_len > http_download_ring_buf_data_count(g_http_download_ringbuf)) 
	{				
		if (http_download_ring_buf_data_count(g_http_download_ringbuf) == 0)
		{
			//printf("ring_buf data is empty!!!!\r\n");
			return 0;
		}
		else
		{
			buf_len = http_download_ring_buf_data_count(g_http_download_ringbuf);
            //printf("buf len : %d\n", buf_len);
		}
    }
    if (g_http_download_ringbuf->read_ptr < end_ind) 
	{
		//printf("1\r\n");
        if (end_ind - g_http_download_ringbuf->read_ptr > buf_len) 
		{
            consume_len = buf_len;
            memcpy(buf, g_http_download_ringbuf->buffer + g_http_download_ringbuf->read_ptr, buf_len);
            g_http_download_ringbuf->read_ptr += buf_len;
			//printf("2\r\n");
        } 
		else 
		{
            consume_len = end_ind - g_http_download_ringbuf->read_ptr ;
            memcpy(buf, g_http_download_ringbuf->buffer + g_http_download_ringbuf->read_ptr, consume_len);
            g_http_download_ringbuf->read_ptr = end_ind;
			//printf("3\r\n");
        }
    } 
	else 
    {

		//printf("4\r\n");
        if (g_http_download_ringbuf->buf_size - g_http_download_ringbuf->read_ptr > buf_len) 
		{
            consume_len = buf_len;
            memcpy(buf, g_http_download_ringbuf->buffer + g_http_download_ringbuf->read_ptr, buf_len);
            g_http_download_ringbuf->read_ptr += buf_len;
			//printf("5\r\n");
        } 
		else 
		{
			
            int32_t copyed_len = g_http_download_ringbuf->buf_size - g_http_download_ringbuf->read_ptr;
            memcpy(buf, g_http_download_ringbuf->buffer + g_http_download_ringbuf->read_ptr, copyed_len);
            g_http_download_ringbuf->read_ptr = 0;
			//printf("6\r\n");
            if (end_ind > buf_len - copyed_len) 
			{
                consume_len = buf_len;
                memcpy(buf + copyed_len, g_http_download_ringbuf->buffer, buf_len - copyed_len);
                g_http_download_ringbuf->read_ptr += buf_len - copyed_len;
				//printf("7\r\n");
            } 
			else 
            {
                consume_len = copyed_len + end_ind;
                memcpy(buf + copyed_len, g_http_download_ringbuf->buffer, end_ind);
                g_http_download_ringbuf->read_ptr = end_ind;
				//printf("8\r\n");
            }
        }
    }

     return consume_len;
}
/*
HTT_DOWNLOAD_MAX_SIZE: the size should be 2^(addr_mask) byte alignment

*/
static void http_download_ring_buf_put_two_bytes(void *pRingbuf,uint8_t byte, uint8_t byte_2)
{
	http_download_ring_buf_struct_t *g_http_download_ringbuf = (http_download_ring_buf_struct_t *)pRingbuf;
	
	*(g_http_download_ringbuf->buffer +  g_http_download_ringbuf->write_ptr) = byte;
	*(g_http_download_ringbuf->buffer +  g_http_download_ringbuf->write_ptr + 1) = byte_2;
	 g_http_download_ringbuf->write_ptr += 2;
	if (g_http_download_ringbuf->write_ptr >= g_http_download_ringbuf->buf_size){
		g_http_download_ringbuf->write_ptr = 0;
	}
}


static void http_download_ring_buf_get_read_pointer(void *pRingbuf,uint8_t **buffer, uint32_t *length)
{
    int32_t count = 0;

	http_download_ring_buf_struct_t *g_http_download_ringbuf = (http_download_ring_buf_struct_t *)pRingbuf;
	
    if (g_http_download_ringbuf->write_ptr >= g_http_download_ringbuf->read_ptr) {
		count = g_http_download_ringbuf->write_ptr - g_http_download_ringbuf->read_ptr;
    } else {
		count = g_http_download_ringbuf->buf_size - g_http_download_ringbuf->read_ptr;
    }
    *buffer = (uint8_t*)(g_http_download_ringbuf->buffer + g_http_download_ringbuf->read_ptr);
    *length = count;
}

static void http_download_ring_buf_update_read_pointer(void *pRingbuf,uint32_t RequestSize){
	http_download_ring_buf_struct_t *g_http_download_ringbuf = (http_download_ring_buf_struct_t *)pRingbuf;
	
	g_http_download_ringbuf->read_ptr += RequestSize;
	if (g_http_download_ringbuf->read_ptr >= g_http_download_ringbuf->buf_size){
		g_http_download_ringbuf->read_ptr -= g_http_download_ringbuf->buf_size;
	}
}

static void http_download_ring_buf_lseek(void *pRingbuf,uint32_t pos){
	http_download_ring_buf_struct_t *g_http_download_ringbuf = (http_download_ring_buf_struct_t *)pRingbuf;
	g_http_download_ringbuf->read_ptr = pos;
}


void *YTRingBuf_Init(int nSize,char * pBuffer)
{
	return http_download_ring_buf_init( nSize, pBuffer);
}

void YTRingBuf_Reset(void *pRingbuf)
{
	http_download_ring_buf_struct_t *g_http_download_ringbuf = (http_download_ring_buf_struct_t *)pRingbuf;
	g_http_download_ringbuf->write_ptr = 0;
	g_http_download_ringbuf->read_ptr = 0;
}

void YTRingBuf_Write2Bytes(void *pRingbuf,uint8_t byte, uint8_t byte_2)
{
	http_download_ring_buf_put_two_bytes(pRingbuf, byte, byte_2);
}

int YTRingBuf_WriteBytes(void *pRingbuf,int8_t* buf, int32_t buf_len)
{
	return http_download_ring_buf_put_data(pRingbuf,buf,buf_len);
}

int YTRingBuf_GetData(void *pRingbuf,int8_t* buf, int32_t buf_len)
{
	return http_download_ring_buf_get_data(pRingbuf,buf,buf_len);
}

int YTRingBuf_GetDataCount(void *pRingbuf)
{
	return http_download_ring_buf_data_count(pRingbuf);
}


