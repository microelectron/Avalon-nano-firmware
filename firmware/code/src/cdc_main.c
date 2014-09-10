<<<<<<< HEAD
/*
 * @brief USB to UART bridge example
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2012
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licensor disclaim any and
 * all warranties, express or implied, including all implied warranties of
 * merchantability, fitness for a particular purpose and non-infringement of
 * intellectual property rights.  NXP Semiconductors assumes no responsibility
 * or liability for the use of the software, conveys no license or rights under any
 * patent, copyright, mask work right, or any other intellectual property rights in
 * or to any products. NXP Semiconductors reserves the right to make changes
 * in the software without notification. NXP Semiconductors also makes no
 * representation or warranty that such application will be suitable for the
 * specified use without further testing or modification.
 *
 * @par
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 */
#include <string.h>
#include "board.h"
#include "app_usbd_cfg.h"
#include "cdc_uart.h"
#include "avalon_api.h"
#ifdef __CODE_RED
#include <NXP/crp.h>
#endif
#include "sha2.h"

#define A3233_TASK_LEN 88
#define A3233_NONCE_LEN	4
#define ICA_TASK_LEN 64
#define A3233_TIMER_TIMEOUT				(AVALON_TMR_ID1)
#define A3233_STAT_IDLE					1
#define A3233_STAT_WAITICA				2
#define A3233_STAT_CHKICA				3
#define A3233_STAT_PROCICA				4
#define A3233_STAT_RCVNONCE				5
#define A3233_STAT_PROTECT				6
#ifdef __CODE_RED
__CRP unsigned int CRP_WORD = CRP_NO_ISP;
#endif

//personal define
#define MM_BUF_LEN						39
#define MM_VERSION_LEN					8

static int g_module_id = 0;

static uint8_t g_pkg[MM_BUF_LEN];
static uint8_t g_act[MM_BUF_LEN];
static uint8_t MM_VERSION[MM_VERSION_LEN] = {2,0,1,4,0,8,2,7};

static void encode_pkg(uint8_t *p, int type, uint8_t *buf, unsigned int len)
{
	uint32_t tmp;
	uint16_t crc;
	uint8_t *data;
	memset(p, 0, AVA2_P_COUNT);		//p = g_act;
	p[0] = AVA2_H1;
	P[1] = AVA2_H2;

	p[2] = type;
	p[3] = 1;
	p[4] = 1;
	data = p + 5;
	memcpy(data+28, &g_module_id, 4);
	
	switch(type)
	{
		case AVA2_P_ACKDETECT:

			break;
		case AVA2_P_ACKMIDSTATE

			break;
		case AVA2_P_NONCE:
		
		default:
			break;
	}

	crc = crc16(data, AVA2_P_DATA_LEN);
	p[AVA2_P_COUNT - 2] = crc & 0x00ff;
	p[AVA2_P_COUNT - 1] = (crc & 0xff00) >> 8;
}


void send_pkg(int type, uint8_t *buf, unsigned int len)
{
	encode_pkg(g_act, type, buf, len);
	UCOM_Write(g_act, AVA2_P_COUNT);
}

static int decode_pkg(uint8_t *p, uint8_t *pi)
{
	int idx;
	int cnt;
	uint8_t *data = p + 5;
	idx = pm[3];
	cnt = pm[4];
	switch(p[2])
	{
		case AVA2_P_DETECT:
			send_pkg(AVA2_P_ACKDETECT,&MM_VERSION,MM_VERSION_LEN);
			break;

		case AVA2_P_MIDSTATUS:
			memset(pi, 0, ICA_TASK_LEN);
			send_pkg(AVA2_P_ACKMIDSTATUS, NULL, 0);
			a3233_status = A3233_STAT_WAITICA;
			break;

		case AVA2_P_DATA:
			memcpy(pi+32,data,32);
			a3233_status = A3233_STAT_PROCICA;
			break;

		default:
			break;
	}
	return 0;
}




/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/
static unsigned char 	golden_ob[] = "\x46\x79\xba\x4e\xc9\x98\x76\xbf\x4b\xfe\x08\x60\x82\xb4\x00\x25\x4d\xf6\xc3\x56\x45\x14\x71\x13\x9a\x3a\xfa\x71\xe4\x8f\x54\x4a\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x87\x32\x0b\x1a\x14\x26\x67\x4f\x2f\xa7\x22\xce";
static unsigned int 	a3233_stat = A3233_STAT_WAITICA;

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/**
 * @brief	main routine for blinky example
 * @return	Function should not exit.
 */
int main(void)
{
	uint8_t 		icarus_buf[ICA_TASK_LEN];
	unsigned int	icarus_buflen = 0;
	unsigned char 	work_buf[A3233_TASK_LEN];
	unsigned char	nonce_buf[A3233_NONCE_LEN];
	unsigned int	nonce_buflen = 0;
	unsigned int	last_freq = 0;
	uint32_t 		nonce_value = 0;
	Bool			isgoldenob = FALSE;
	Bool			timestart = FALSE;

	Board_Init();
	SystemCoreClockUpdate();

	/* Initialize avalon chip */
	AVALON_USB_Init();
	AVALON_TMR_Init();
	AVALON_LED_Init();
	AVALON_A3233_Init();

	while (1) {
		switch (a3233_stat) {
		case A3233_STAT_WAITICA:
			icarus_buflen = UCOM_Read_Cnt();
			if (icarus_buflen > 0) {
				timestart = FALSE;
				AVALON_TMR_Kill(A3233_TIMER_TIMEOUT);
				a3233_stat = A3233_STAT_CHKICA;
				break;
			}

			if (!timestart) {
				AVALON_TMR_Set(A3233_TIMER_TIMEOUT, 50, NULL);
				timestart = TRUE;
			}

			if (AVALON_TMR_IsTimeout(A3233_TIMER_TIMEOUT)) {
				/* no data */
				timestart = FALSE;
				AVALON_TMR_Kill(A3233_TIMER_TIMEOUT);
				a3233_stat = A3233_STAT_IDLE;
			}
			break;

		case A3233_STAT_IDLE:
			AVALON_LED_Rgb(AVALON_LED_GREEN);
			icarus_buflen = UCOM_Read_Cnt();
			if (icarus_buflen > 0) {
				a3233_stat = A3233_STAT_CHKICA;
			}

			if (AVALON_A3233_IsPowerEn()) {
				AVALON_A3233_PowerEn(FALSE);
			}
			break;

		case A3233_STAT_CHKICA:
			if (AVALON_A3233_IsTooHot()) {
				a3233_stat = A3233_STAT_PROTECT;
				timestart = FALSE;
				AVALON_TMR_Kill(A3233_TIMER_TIMEOUT);
				break;
			}

			icarus_buflen = UCOM_Read_Cnt();
			if (icarus_buflen >= ICA_TASK_LEN) {
				timestart = FALSE;
				AVALON_TMR_Kill(A3233_TIMER_TIMEOUT);
				a3233_stat = A3233_STAT_PROCICA;
				break;
			}

			if (!timestart) {
				AVALON_TMR_Set(A3233_TIMER_TIMEOUT, 80, NULL);
				timestart = TRUE;
			}

			if (AVALON_TMR_IsTimeout(A3233_TIMER_TIMEOUT)) {
				/* data format error */
				timestart = FALSE;
				AVALON_TMR_Kill(A3233_TIMER_TIMEOUT);
				UCOM_FlushRxRB();
				a3233_stat = A3233_STAT_IDLE;
			}
			break;

		case A3233_STAT_PROTECT:
			if (!AVALON_A3233_IsTooHot()) {
				timestart = FALSE;
				AVALON_TMR_Kill(A3233_TIMER_TIMEOUT);
				a3233_stat = A3233_STAT_CHKICA;
				break;
			}

			if (AVALON_A3233_IsPowerEn())
				AVALON_A3233_PowerEn(FALSE);

			if (!timestart) {
				AVALON_TMR_Set(A3233_TIMER_TIMEOUT, 10000, NULL);
				AVALON_LED_Blink(AVALON_LED_RED);
				timestart = TRUE;
			}

			if (AVALON_TMR_IsTimeout(A3233_TIMER_TIMEOUT)) {
				AVALON_TMR_Set(A3233_TIMER_TIMEOUT, 10000, NULL);
			}
			break;

		case A3233_STAT_PROCICA:
			memset(icarus_buf, 0, ICA_TASK_LEN);
			UCOM_Read(icarus_buf, ICA_TASK_LEN);
			memset(work_buf, 0, A3233_TASK_LEN);

			if (!memcmp(golden_ob, icarus_buf, ICA_TASK_LEN))
				isgoldenob = TRUE;
			else
				isgoldenob = FALSE;

			data_convert(icarus_buf);
			data_pkg(icarus_buf, work_buf);

			if (!AVALON_A3233_IsPowerEn() || (last_freq != AVALON_A3233_FreqNeeded())) {
				last_freq = AVALON_A3233_FreqNeeded();
				AVALON_A3233_PowerEn(TRUE);
				AVALON_A3233_Reset();
				((unsigned int*)work_buf)[1] = AVALON_A3233_PllCfg(last_freq, NULL);
			}

			{
				unsigned int r,g,b;
				r = last_freq - AVALON_A3233_FreqMin();
				if (r > 255 )
					r = 255;
				g = 0;
				b = 255 - r;
				AVALON_LED_Rgb((r << 16) | (g << 8) | b);
			}

			if (isgoldenob) {
				work_buf[81] = 0x1;
				work_buf[82] = 0x73;
				work_buf[83] = 0xa2;
			}

			UART_Write(work_buf, A3233_TASK_LEN);
			UART_FlushRxRB();
			a3233_stat = A3233_STAT_RCVNONCE;
			break;

		case A3233_STAT_RCVNONCE:
			nonce_buflen = UART_Read_Cnt();
			if (nonce_buflen >= A3233_NONCE_LEN) {
				UART_Read(nonce_buf, A3233_NONCE_LEN);
				PACK32(nonce_buf, &nonce_value);
				nonce_value = ((nonce_value >> 24) | (nonce_value << 24) | ((nonce_value >> 8) & 0xff00) | ((nonce_value << 8) & 0xff0000));
				nonce_value -= 0x1000;
				UNPACK32(nonce_value, nonce_buf);

				UCOM_Write(nonce_buf, A3233_NONCE_LEN);

#ifdef A3233_FREQ_DEBUG
				{
					char freq[20];

					m_sprintf(freq, "%04d%04d%04d%04d", AVALON_A3233_FreqNeeded(), AVALON_I2C_TemperRd(), (int)AVALON_A3233_IsTooHot(), AVALON_A3233_ADCGuard(0));
					UCOM_Write(freq, 16);
				}
#endif
				timestart = FALSE;
				AVALON_TMR_Kill(A3233_TIMER_TIMEOUT);
				a3233_stat = A3233_STAT_WAITICA;
				break;
			}

			if (!timestart) {
				AVALON_TMR_Set(A3233_TIMER_TIMEOUT, 2000, NULL);
				timestart = TRUE;
			}

			if (AVALON_TMR_IsTimeout(A3233_TIMER_TIMEOUT)) {
				/* no nonce response */
				timestart = FALSE;
				AVALON_TMR_Kill(A3233_TIMER_TIMEOUT);
				a3233_stat = A3233_STAT_IDLE;
				break;
			}

			if (UCOM_Read_Cnt()) {
				timestart = FALSE;
				AVALON_TMR_Kill(A3233_TIMER_TIMEOUT);
				a3233_stat = A3233_STAT_CHKICA;
			}

			break;

		default:
			a3233_stat = A3233_STAT_IDLE;
			break;
		}
		/* Sleep until next IRQ happens */
		__WFI();
	}
}

=======
/*
 * @brief USB to UART bridge example
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2012
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licensor disclaim any and
 * all warranties, express or implied, including all implied warranties of
 * merchantability, fitness for a particular purpose and non-infringement of
 * intellectual property rights.  NXP Semiconductors assumes no responsibility
 * or liability for the use of the software, conveys no license or rights under any
 * patent, copyright, mask work right, or any other intellectual property rights in
 * or to any products. NXP Semiconductors reserves the right to make changes
 * in the software without notification. NXP Semiconductors also makes no
 * representation or warranty that such application will be suitable for the
 * specified use without further testing or modification.
 *
 * @par
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 */
#include <string.h>
#include "board.h"
#include "app_usbd_cfg.h"
#include "cdc_uart.h"
#include "avalon_api.h"
#ifdef __CODE_RED
#include <NXP/crp.h>
#endif
#include "sha2.h"
#include "protocol.h"
#include "crc.h"


#define A3233_TASK_LEN 88
#define A3233_NONCE_LEN	4
#define ICA_TASK_LEN 64
#define A3233_TIMER_TIMEOUT				(AVALON_TMR_ID1)
#define A3233_STAT_IDLE					1
#define A3233_STAT_WAITICA				2
#define A3233_STAT_CHKICA				3
#define A3233_STAT_PROCICA				4
#define A3233_STAT_RCVNONCE				5
#define A3233_STAT_PROTECT				6
#define A3233_STAT_PROCMM				7

#ifdef __CODE_RED
__CRP unsigned int CRP_WORD = CRP_NO_ISP;
#endif

#define MM_BUF_LEN						39

static uint8_t ret_pkg[MM_BUF_LEN];

static uint8_t MM_VERSION[MM_VERSION_LEN] = "20140827";
static unsigned int 	a3233_stat = A3233_STAT_WAITICA;

static int UCOM_send_pkg(uint8_t *p,int type,unsigned char *buf)
{
	
		uint16_t crc;
		memset(p, 0, AVA2_P_COUNT);
		p[0] = AVA2_H1;
		p[1] = AVA2_H2;
	
		p[2] = type;
		p[3] = 1;
		p[4] = 1;
		
		switch(type)
		{
			case AVA2_P_ACKDETECT:
				memcpy(p+5,&MM_VERSION,MM_VERSION_LEN);
				break;
			case AVA2_P_ACKMIDSTATE:
				break;
			case AVA2_P_NONCE:
				memcpy(p+5,buf,4);
				break;
			
			default:
				a3233_stat = A3233_STAT_WAITICA;
				break;
		}
		crc = crc16(p+5, AVA2_P_DATA_LEN);
		p[AVA2_P_COUNT - 2] = crc & 0x00ff;
		p[AVA2_P_COUNT - 1] = (crc & 0xff00) >> 8;
		UCOM_Write(p,MM_BUF_LEN);
		return 0;
}



static int decode_pkg(uint8_t *p, uint8_t *icarusbuf)
{
	unsigned char   package_error[] = "Illegal data package";
	unsigned int expected_crc;
	unsigned int actual_crc;
	int idx;
	int cnt;
	int ret;
	uint8_t *data = p + 5;
	
	idx = p[3];
	cnt = p[4];

	expected_crc = (p[AVA2_P_COUNT - 1] & 0xff) | ((p[AVA2_P_COUNT - 2] & 0xff) << 8);
	actual_crc = crc16(data, AVA2_P_DATA_LEN);
	actual_crc = crc16(data, AVA2_P_DATA_LEN);
	/*
	if(expected_crc != actual_crc) {
		return 1;
	}
	*/
	switch(p[2])
	{
		case AVA2_P_DETECT:
			//ret = UCOM_send_pkg(ret_pkg,AVA2_P_ACKDETECT,NULL);
			//if(0 != ret) return 1;
			//a3233_stat = A3233_STAT_WAITICA;
			break;

		case AVA2_P_MIDSTATE:
			memset(icarusbuf, 0, ICA_TASK_LEN);
			memcpy(icarusbuf,data,32);
			ret = UCOM_send_pkg(ret_pkg,AVA2_P_ACKMIDSTATE,NULL);
			//if(0 != ret) return 2;
			a3233_stat = A3233_STAT_WAITICA;
			break;

		case AVA2_P_DATA:
			memcpy(icarusbuf+32,data,32);
			a3233_stat = A3233_STAT_PROCICA;
			break;

		default:
			//UCOM_Write(package_error,sizeof(package_error));		
			a3233_stat = A3233_STAT_WAITICA;
			break;
	}
	return 0;
}

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/
static unsigned char 	golden_ob[] = "\x46\x79\xba\x4e\xc9\x98\x76\xbf\x4b\xfe\x08\x60\x82\xb4\x00\x25\x4d\xf6\xc3\x56\x45\x14\x71\x13\x9a\x3a\xfa\x71\xe4\x8f\x54\x4a\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x87\x32\x0b\x1a\x14\x26\x67\x4f\x2f\xa7\x22\xce";

/*****************************************************************************
 * Private functions
 ****************************************************************************/


/*****************************************************************************
 * Public functions
 ****************************************************************************/


/**
 * @brief	main routine for blinky example
 * @return	Function should not exit.
 */
int main(void)
{
	uint8_t 		icarus_buf[ICA_TASK_LEN];
	unsigned int	icarus_buflen = 0;
	uint8_t 		mm_buffer[MM_BUF_LEN];
	unsigned int 	mm_buffer_len = 0;
	unsigned char 	work_buf[A3233_TASK_LEN];
	unsigned char	nonce_buf[A3233_NONCE_LEN];
	unsigned int	nonce_buflen = 0;
	unsigned int	last_freq = 0;
	uint32_t 		nonce_value = 0;
	unsigned int 	ret;
	Bool			isgoldenob = FALSE;
	Bool			timestart = FALSE;

	Board_Init();
	SystemCoreClockUpdate();

	/* Initialize avalon chip */
	AVALON_USB_Init();
	AVALON_TMR_Init();
	AVALON_LED_Init();
	AVALON_A3233_Init();

	while (1) {
		switch (a3233_stat) {
		case A3233_STAT_WAITICA:
			mm_buffer_len = UCOM_Read_Cnt();
			if (mm_buffer_len > 0) {
				timestart = FALSE;
				AVALON_TMR_Kill(A3233_TIMER_TIMEOUT);
				a3233_stat = A3233_STAT_CHKICA;
				break;
			}

			if (!timestart) {
				AVALON_TMR_Set(A3233_TIMER_TIMEOUT, 50, NULL);
				timestart = TRUE;
			}

			if (AVALON_TMR_IsTimeout(A3233_TIMER_TIMEOUT)) {
				/* no data */
				timestart = FALSE;
				AVALON_TMR_Kill(A3233_TIMER_TIMEOUT);
				a3233_stat = A3233_STAT_IDLE;
			}
			break;

		case A3233_STAT_IDLE:
			mm_buffer_len = UCOM_Read_Cnt();
			if (mm_buffer_len > 0) {
				a3233_stat = A3233_STAT_CHKICA;
			}

			if (AVALON_A3233_IsPowerEn()) {
				AVALON_A3233_PowerEn(FALSE);
			}
			break;

		case A3233_STAT_CHKICA:
			if (AVALON_A3233_IsTooHot()) {
				a3233_stat = A3233_STAT_PROTECT;
				timestart = FALSE;
				AVALON_TMR_Kill(A3233_TIMER_TIMEOUT);
				break;
			}

			mm_buffer_len = UCOM_Read_Cnt();
			if (mm_buffer_len >= MM_BUF_LEN) {
				timestart = FALSE;
				AVALON_TMR_Kill(A3233_TIMER_TIMEOUT);
				a3233_stat = A3233_STAT_PROCMM;
				break;
			}

			if (!timestart) {
				AVALON_TMR_Set(A3233_TIMER_TIMEOUT, 80, NULL);
				timestart = TRUE;
			}

			if (AVALON_TMR_IsTimeout(A3233_TIMER_TIMEOUT)) {
				/* data format error */
				timestart = FALSE;
				AVALON_TMR_Kill(A3233_TIMER_TIMEOUT);
				UCOM_FlushRxRB();
				a3233_stat = A3233_STAT_IDLE;
			}
			break;

		case A3233_STAT_PROCMM:		
			memset(mm_buffer, 0, MM_BUF_LEN);
			UCOM_Read(mm_buffer, MM_BUF_LEN);
			ret = decode_pkg(mm_buffer,icarus_buf);
			break;
				
		case A3233_STAT_PROTECT:
			if (!AVALON_A3233_IsTooHot()) {
				timestart = FALSE;
				AVALON_TMR_Kill(A3233_TIMER_TIMEOUT);
				a3233_stat = A3233_STAT_CHKICA;
				break;
			}

			if (AVALON_A3233_IsPowerEn())
				AVALON_A3233_PowerEn(FALSE);

			if (!timestart) {
				AVALON_TMR_Set(A3233_TIMER_TIMEOUT, 10000, NULL);
				AVALON_LED_Blink(AVALON_LED_RED);
				timestart = TRUE;
			}

			if (AVALON_TMR_IsTimeout(A3233_TIMER_TIMEOUT)) {
				AVALON_TMR_Set(A3233_TIMER_TIMEOUT, 10000, NULL);
			}
			break;

		case A3233_STAT_PROCICA:
			memset(work_buf, 0, A3233_TASK_LEN);
			
			if (!memcmp(golden_ob, icarus_buf, ICA_TASK_LEN))
				isgoldenob = TRUE;
			else
				isgoldenob = FALSE;
			
			
			UCOM_Write(icarus_buf,32);
			UCOM_Write(icarus_buf+32,32);
			
			a3233_stat = A3233_STAT_WAITICA;
			break;
			
			data_convert(icarus_buf);
			data_pkg(icarus_buf, work_buf);

			if (!AVALON_A3233_IsPowerEn() || (last_freq != AVALON_A3233_FreqNeeded())) {
				last_freq = AVALON_A3233_FreqNeeded();
				AVALON_A3233_PowerEn(TRUE);
				AVALON_A3233_Reset();
				((unsigned int*)work_buf)[1] = AVALON_A3233_PllCfg(last_freq, NULL);
			}

			{
				unsigned int r,g,b;
				r = last_freq - AVALON_A3233_FreqMin();
				if (r > 255 )
					r = 255;
				g = 0;
				b = 255 - r;
				AVALON_LED_Rgb((r << 16) | (g << 8) | b);
			}

			if (isgoldenob) {
				work_buf[81] = 0x1;
				work_buf[82] = 0x73;
				work_buf[83] = 0xa2;
			}

			UART_Write(work_buf, A3233_TASK_LEN);
			UART_FlushRxRB();
			a3233_stat = A3233_STAT_RCVNONCE;
			break;

		case A3233_STAT_RCVNONCE:
			nonce_buflen = UART_Read_Cnt();
			if (nonce_buflen >= A3233_NONCE_LEN) {
				UART_Read(nonce_buf, A3233_NONCE_LEN);
				PACK32(nonce_buf, &nonce_value);
				nonce_value = ((nonce_value >> 24) | (nonce_value << 24) | ((nonce_value >> 8) & 0xff00) | ((nonce_value << 8) & 0xff0000));
				nonce_value -= 0x1000;
				UNPACK32(nonce_value, nonce_buf);
				UCOM_send_pkg(ret_pkg,AVA2_P_NONCE, nonce_buf);
				

#ifdef A3233_FREQ_DEBUG
				{
					char freq[20];

					m_sprintf(freq, "%04d%04d%04d%04d", AVALON_A3233_FreqNeeded(), AVALON_I2C_TemperRd(), (int)AVALON_A3233_IsTooHot(), AVALON_A3233_ADCGuard(0));
					UCOM_Write(freq, 16);
				}
#endif
				timestart = FALSE;
				AVALON_TMR_Kill(A3233_TIMER_TIMEOUT);
				a3233_stat = A3233_STAT_WAITICA;
				break;
			}

			if (!timestart) {
				AVALON_TMR_Set(A3233_TIMER_TIMEOUT, 2000, NULL);
				timestart = TRUE;
			}

			if (AVALON_TMR_IsTimeout(A3233_TIMER_TIMEOUT)) {
				/* no nonce response */
				timestart = FALSE;
				AVALON_TMR_Kill(A3233_TIMER_TIMEOUT);
				a3233_stat = A3233_STAT_IDLE;
				break;
			}

			if (UCOM_Read_Cnt()) {
				timestart = FALSE;
				AVALON_TMR_Kill(A3233_TIMER_TIMEOUT);
				a3233_stat = A3233_STAT_CHKICA;
			}

			break;

		default:
			a3233_stat = A3233_STAT_IDLE;
			break;
		}
		/* Sleep until next IRQ happens */
		__WFI();
	}
}

>>>>>>> Avalon-nano-mm-progress
