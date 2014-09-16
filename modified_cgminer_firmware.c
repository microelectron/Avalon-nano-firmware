2014/9/12




/*************************************************************************/
//只简单的初始化数据包;
static int avalon_init_pkg(struct avalon2_pkg *pkg, struct ICARUS_WORK *icarusPkg,uint8_t type, uint8_t idx, uint8_t cnt)
	{
		unsigned short crc; 		//16bits
		pkg->head[0] = AVA2_H1;
		pkg->head[1] = AVA2_H2;
	
		pkg->type = type;
		pkg->idx = idx;
		pkg->cnt = cnt;
	
		crc = crc16(pkg->data, AVA2_P_DATA_LEN);
	
		pkg->crc[0] = (crc & 0xff00) >> 8;
		pkg->crc[1] = crc & 0x00ff;
		return 0;
}

/*************************************************************************/

//只是简单的返回包类型;
static int decode_pkg(struct thr_info *thr, struct avalon2_pkg *ar, uint8_t *pkg,)
{

	int err;
	unsigned int expected_crc;
	unsigned int actual_crc;
	int type = AVA2_GETS_ERROR;

	memcpy((uint8_t *)ar, pkg, AVA2_READ_SIZE);

	if (ar->head[0] == AVA2_H1 && ar->head[1] == AVA2_H2) {
		expected_crc = crc16(ar->data, AVA2_P_DATA_LEN);
		actual_crc = (ar->crc[0] & 0xff) |
			((ar->crc[1] & 0xff) << 8);

		type = ar->type;
		/*
		if (expected_crc != actual_crc)
			goto out;
		*/
		
	}

	return type;
}


/*************************************************************************/

static int icarus_get_nonce(struct cgpu_info *icarus, unsigned char *buf, struct timeval *tv_start,
			    struct timeval *tv_finish, struct thr_info *thr, int read_time)
{
	struct ICARUS_INFO *info = (struct ICARUS_INFO *)(icarus->device_data);
	int err, amt, rc;
	//struct avalon2_pkg detecte_pkg;
	if (icarus->usbinfo.nodev)
		return ICA_NONCE_ERROR;

	cgtime(tv_start);
	err = usb_read_ii_timeout_cancellable(icarus, info->intinfo, (char *)buf,
					      AVA2_READ_SIZE, &amt, read_time,
					      C_GETRESULTS);
	cgtime(tv_finish);

	if (err < 0 && err != LIBUSB_ERROR_TIMEOUT) {
		applog(LOG_ERR, "%s%i: Comms error (rerr=%d amt=%d)", icarus->drv->name,
		       icarus->device_id, err, amt);
		dev_error(icarus, REASON_DEV_COMMS_ERROR);
		return ICA_NONCE_ERROR;
	}

	if (amt >= AVA2_READ_SIZE)
		return ICA_NONCE_OK;

	rc = SECTOMS(tdiff(tv_finish, tv_start));
	if (thr && thr->work_restart) {
		applog(LOG_DEBUG, "Icarus Read: Work restart at %d ms", rc);
		return ICA_NONCE_RESTART;
	}

	if (amt > 0)
		applog(LOG_DEBUG, "Icarus Read: Timeout reading for %d ms", rc);
	else
		applog(LOG_DEBUG, "Icarus Read: No data for %d ms", rc);
	return ICA_NONCE_TIMEOUT;
}
/*************************************************************************/



//...........................................
icarus_detect_one()
//...........................................
struct avalon2_pkg detect_pkg,ackdetect_pkg;
unsigned char nano_ret_pkg[AVA2_READ_SIZE];
int pkg_type;
/*************************************************************************/
		avalon_init_pkg(&detect_pkg,NULL,AVA2_P_DETECT,1,1);

/*************************************************************************/

/*************************************************************************/

		ret = icarus_get_nonce(icarus, nano_ret_pkg, &tv_start, &tv_finish, NULL, 100);
		if (ret != ICA_NONCE_OK)
			continue;
		pkg_type = decode_pkg(NULL,&ackdetect_pkg,(uint8_t *)nano_ret_pkg);
		if(pkg_type == AVA2_P_ACKDETECT)
			{
				memcpy(nonce_bin,nano_ret_pkg+21,4);
			}
/*************************************************************************/
if(nano_ret_pkg[2] == AVA2_P_ACKDETECT)
			{
				memcpy(nonce_bin,nano_ret_pkg+21,4);
			}
/*************************************************************************/


//...........................................
icarus_scanwork()
//...........................................

/*************************************************************************/
struct avalon2_pkg nano_ret_pkg;
	struct avalon2_pkg send_pkg;
	uint8_t nonce_buf[AVA2_READ_SIZE];
	int nonce_ret;
/*************************************************************************/
	memset(&send_pkg,sizeof(send_pkg));
	memcpy(send_pkg.data,(char*)(&workdata),+32);
	avalon2_init_pkg(&send_pkg,AVA2_P_MIDSTATE,1,1);
nano_write:

/*************************************************************************/
ret = icarus_get_nonce(icarus, nonce_buf, &tv_start, &tv_finish, thr, info->read_time);

if(ret == ICA_NONCE_OK )
		{
			nonce_ret = decode_pkg(thr,nano_ret_pkg,nonce_buf);
			if(nonce_ret == AVA2_P_ACKMIDSTATE)
				{
					memset(&send_pkg,0,sizeof(send_pkg));
					memcpy(send_pkg.data,(char*)(&workdata)+32,32);
					avalon2_init_pkg(&send_pkg,AVA2_P_DATA,1,1);
					goto nano_write;				
				}
			else if(nonce_ret == AVA2_P_NONCE) 
				{
					memset(nonce_bin, 0, sizeof(nonce_bin));
					memcpy(nonce_bin,nonce_buf+21,4);
					memcpy((char *)&nonce, nonce_bin, ICARUS_READ_SIZE);
					nonce = htobe32(nonce);
				}
			else
				{
					goto out;
				}

		}

/*************************************************************************/
2014/9/15 Nano firmware
/*************************************************************************/
static int avalon_init_pkg(uint8_t *p,int type)
	{
		
			uint16_t crc;
			uint8_t *data;
			memset(p, 0, AVA2_P_COUNT);
			p[0] = AVA2_H1;
			p[1] = AVA2_H2;
		
			p[2] = type;
			p[3] = 1;
			p[4] = 1;
			switch(type)
			{
				case AVA2_P_ACKDETECT:
					memcpy(data+16,&MM_VERSION,MM_VERSION_LEN);
					break;
				case AVA2_P_ACKMIDSTATE:
					break;
				case AVA2_P_NONCE:
					memcpy(data+16,buf,4);
					break;
				
				default:
					a3233_stat = A3233_STAT_WAITICA;
					break;
			}
			crc = crc16(data, AVA2_P_DATA_LEN);
			p[AVA2_P_COUNT - 2] = crc & 0x00ff;
			p[AVA2_P_COUNT - 1] = (crc & 0xff00) >> 8;
			return 0;
}
/*************************************************************************/
static int avalon_init_pkg(uint8_t *p,int type)
	{
		
			uint16_t crc;
			//uint8_t *data;
			//memset(p, 0, AVA2_P_COUNT);
			p[0] = AVA2_H1;
			p[1] = AVA2_H2;
			//data = p + 5;

			p[2] = type;
			p[3] = 1;
			p[4] = 1;
			crc = crc16(p+5, AVA2_P_DATA_LEN);
			p[AVA2_P_COUNT - 2] = crc & 0x00ff;
			p[AVA2_P_COUNT - 1] = (crc & 0xff00) >> 8;
			return 0;
}
/*************************************************************************/

unsigned int ret = p[2] ;

static int decode_pkg(uint8_t *p)
{
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

	if(expected_crc != actual_crc) {
		return AVA2_P_CRCERROR;
	}

	return p[2];
}
/*************************************************************************/
if(mm_buffer[2]== AVA2_P_DETECT) {
					avalon_init_pkg(ret_pkg,AVA2_P_ACKDETECT,NULL);
					UCOM_Write(ret_pkg,AVA2_P_COUNT);
					AVALON_LED_Rgb(AVALON_LED_BLUE);
					a3233_stat = A3233_STAT_WAITICA;
					break;
				}
			else if (mm_buffer[2] == AVA2_P_MIDSTATE) {
					memset(icarus_buf,0,ICA_TASK_LEN);
					memcpy(icarus_buf,mm_buffer+5,32);
					avalon_init_pkg(ret_pkg,AVA2_P_ACKMIDSTATE,NULL);
					UCOM_Write(ret_pkg,AVA2_P_COUNT);			
					AVALON_LED_Rgb(AVALON_LED_GREEN);
					a3233_stat = A3233_STAT_WAITICA;
					break;
				}
			else if (mm_buffer[2] == AVA2_P_DATA) {
					memcpy(icarus_buf+32,mm_buffer+5,32);					
					AVALON_LED_Rgb(AVALON_LED_BLACK);
				}
			else {
					a3233_stat = A3233_STAT_WAITICA;
					break;
				}
/*************************************************************************/
case A3233_STAT_MM_PROC:
			memset(mm_buffer, 0, AVA2_P_COUNT);
			UCOM_Read(mm_buffer, AVA2_P_COUNT);
			switch(mm_buffer[2]) {
				case AVA2_P_DETECT:
					memset(&ret_pkg,0,AVA2_P_COUNT);
					memcpy(ret_pkg+21,&MM_VERSION,MM_VERSION_LEN);
					avalon_init_pkg(ret_pkg,AVA2_P_ACKDETECT);
					UCOM_Write(ret_pkg,AVA2_P_COUNT);
					AVALON_LED_Rgb(AVALON_LED_BLUE);
					a3233_stat = A3233_STAT_WAITICA;
					break;
				case AVA2_P_MIDSTATE:
					memset(icarus_buf,0,ICA_TASK_LEN);
					memcpy(icarus_buf,mm_buffer+5,32);
					memset(&ret_pkg,0,AVA2_P_COUNT);
					avalon_init_pkg(ret_pkg,AVA2_P_ACKMIDSTATE);
					UCOM_Write(ret_pkg,AVA2_P_COUNT);			
					AVALON_LED_Rgb(AVALON_LED_GREEN);
					a3233_stat = A3233_STAT_WAITICA;
					break;
				
				case AVA2_P_DATA:
					memcpy(icarus_buf+32,mm_buffer+5,32);					
					AVALON_LED_Rgb(AVALON_LED_BLACK);
					a3233_stat = A3233_STAT_PROCICA;
					break;

				default:
					a3233_stat = A3233_STAT_WAITICA;
				}
			break;
/*************************************************************************/
case A3233_STAT_MM_PROC:
			memset(mm_buffer, 0, AVA2_P_COUNT);
			UCOM_Read(mm_buffer, AVA2_P_COUNT);
			tmp = decode_pkg(mm_buffer);
			switch(tmp) {
				case AVA2_P_DETECT:
					memset(&ret_pkg,0,AVA2_P_COUNT);
					memcpy(ret_pkg+21,&MM_VERSION,MM_VERSION_LEN);
					avalon_init_pkg(ret_pkg,AVA2_P_ACKDETECT);
					UCOM_Write(ret_pkg,AVA2_P_COUNT);
					
					a3233_stat = A3233_STAT_WAITICA;
					break;
				case AVA2_P_MIDSTATE:
					memset(icarus_buf,0,ICA_TASK_LEN);
					memcpy(icarus_buf,mm_buffer+5,32);
					memset(&ret_pkg,0,AVA2_P_COUNT);
					avalon_init_pkg(ret_pkg,AVA2_P_ACKMIDSTATE);
					UCOM_Write(ret_pkg,AVA2_P_COUNT);			
					
					a3233_stat = A3233_STAT_WAITICA;
					break;
				
				case AVA2_P_DATA:
					memcpy(icarus_buf+32,mm_buffer+5,32);					
					AVALON_LED_Rgb(AVALON_LED_BLACK);
					a3233_stat = A3233_STAT_PROCICA;
					break;

				default:
					a3233_stat = A3233_STAT_WAITICA;
				}
			break;

		case A3233_STAT_PROCICA:

/*************************************************************************/





/*************************************************************************/




/*************************************************************************/




/*************************************************************************/



/*************************************************************************/





/*************************************************************************/




/*************************************************************************/




/*************************************************************************/




/*************************************************************************/





/*************************************************************************/




/*************************************************************************/




/*************************************************************************/




/*************************************************************************/





/*************************************************************************/




/*************************************************************************/




/*************************************************************************/


