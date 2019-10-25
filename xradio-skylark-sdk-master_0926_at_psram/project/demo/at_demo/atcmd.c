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

#include "serial.h"
#include "atcmd.h"
#include "atcmd/at_command.h"

#include "kernel/os/os.h"
#include "net/wlan/wlan.h"
#include "common/framework/sysinfo.h"
#include "common/framework/net_ctrl.h"
#include "driver/chip/hal_clock.h"


#define ATCMD_THREAD_STACK_SIZE	(4 * 1024)

static OS_Thread_t g_atcmd_thread;

extern void at_cmd_init(void);
extern void at_cmd_exec(void);
extern void at_cmd_reinit(void);

static void atcmd_task(void *arg)
{
	ATCMD_DBG("%s() start...\n", __func__);

	while (1) {
		at_cmd_exec();
	}

	ATCMD_DBG("%s() exit\n", __func__);
	OS_ThreadDelete(&g_atcmd_thread);
}

int at_uart_reinit(at_serial_para_t *para)
{
	HAL_UART_DeInit(SERIAL_UART_ID);
	serial_bezero_g_serial();
	at_cmd_reinit();

	serial_stop();
	serial_deinit(SERIAL_UART_ID);
	serial_disable();


	if(para->stopBit == 2)
	{
		para->stopBit = UART_STOP_BITS_2;
	}
	else
	{
		para->stopBit = UART_STOP_BITS_1;
	}
	if(para == NULL)
	{
		printf("if(para == NULL)\r\n");
		at_serial(para);
		serial_init(SERIAL_UART_ID, para->baudrate, UART_DATA_BITS_8, UART_PARITY_NONE, UART_STOP_BITS_1, para->hwfc);
	}
	else
	{
		printf(	"else	else	else\r\n");
		serial_init(SERIAL_UART_ID, para->baudrate, para->dataBit, para->parity, para->stopBit, para->hwfc);
	}
	serial_start();

	return 0;
}


/* start at command */


extern void wakeup_gpio_init(void);
void atcmd_start(void)
{
	at_serial_para_t para;

	at_cmd_init();
	at_serial(&para);

	serial_init(SERIAL_UART_ID, 115200, UART_DATA_BITS_8, UART_PARITY_NONE, UART_STOP_BITS_1, 1);

	serial_start();

//	wakeup_gpio_init();
	/* start atcmd task */
	if (OS_ThreadCreate(&g_atcmd_thread,
		                "atcmd",
		                atcmd_task,
		                NULL,
		                OS_THREAD_PRIO_CONSOLE,
		                ATCMD_THREAD_STACK_SIZE) != OS_OK) {
		ATCMD_ERR("create serial task failed\n");
		return ;
	}
	at_dump("\n+EVT:1\n");

}
