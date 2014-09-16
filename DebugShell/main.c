#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "cgminer/protocol.h"


struct avalon_pkg {
	uint8_t head[2];
	uint8_t type;
	uint8_t idx;
	uint8_t cnt;
	uint8_t data[32];
	uint8_t crc[2];
};
static int avalon2_init_pkg(struct avalon_pkg *pkg, uint8_t type, uint8_t idx, uint8_t cnt)
{
	unsigned short crc;

	pkg->head[0] = AVA2_H1;
	pkg->head[1] = AVA2_H2;

	pkg->type = type;
	pkg->idx = idx;
	pkg->cnt = cnt;
	//memcpy(pkg->data,"jjjjjwwwwwkkkkkllllliiiiiooooo33",32);
	strcpy(pkg->data,"jjjjjwwwwwkkkkkllllliiiiiooooo33");
	strcpy(pkg->crc,"56");
	//struct avalon_pkg detect_pkg2 = {"AV",'2','1','1',"jjjjjwwwwwkkkkkllllliiiiiooooo33","56"};
	/*
	crc = crc16(pkg->data, AVA2_P_DATA_LEN);

	pkg->crc[0] = (crc & 0xff00) >> 8;
	pkg->crc[1] = crc & 0x00ff;
*/
	return type;
}

/*
struct avalon_pkg detect_pkg = {
	.head="AV",
	.type='6',
	.idx='1',
	.cnt='1',
	.data="111112222233333444445555566666AA",
	.crc="56"
};
struct avalon_pkg detect_pkg3 = {
	.head="AV",
	.type='6',
	.idx='1',
	.cnt='1',
	.data="111112222233333444445555566666AA",
	.crc="56"
};
*/

struct icarus_pkg {
	uint8_t midstate[32];
	uint8_t check;
	uint8_t data;
	uint8_t cmd;
	uint8_t prefix;
	uint8_t unused[16];
	uint8_t work[12];
};

struct icarus_pkg ica_pkg = { "midstatemidstatemidstatemidstate",'c','d','1','3',"1234567891234567","121212121212"};

int main(void)
{
	struct avalon_pkg detect_pkg = {"AV",2,1,1,"jjjjjwwwwwkkkkkllllliiiiiooooo33","56"};
	struct avalon_pkg detect_pkg2 = {"AV",'2','1','1',"jjjjjwwwwwkkkkkllllliiiiiooooo33","56"};
	printf("detect_pkg =%s\ndetect_pkg2=%s\n",(char*)&detect_pkg,(char*)&detect_pkg2);
	
	
	/*	
	char a[] = "41561B0101d8f8ef6712146495c44192c07145fd6d974bf4bb8f41371d65c90d1e9cb18a1700005667";
	int ret = sizeof(a);
	int str = strlen(a);
	*/
	//struct avalon_pkg *p = &detect_pkg;
	//struct icarus_pkg *p2;
	//uint8_t a[4] = "abcd";
	//memcpy(detect_pkg.data+4,a,8);
	//printf("%c%c\n%c%c%c\n%s\n%c%c\n",detect_pkg.head[0],detect_pkg.head[1],detect_pkg.type,detect_pkg.idx,detect_pkg.cnt,detect_pkg.data,detect_pkg.crc[0],detect_pkg.crc[1]);
	//printf("%c%c\n%c%c%c%s\n%c%c\n",*p.head[0],*p.head[1],*p.tpye,*p.idx,p.cnt,*p.data,*p.crc[0],*p.crc[1]);
	/*
	printf("%s\n",(char *)&detect_pkg2+5);	
	memcpy(array,(char *)&detect_pkg2,39);
	printf("%s\n",array);
	memcpy(array,&detect_pkg2,39);
	*/
	//memcpy((uint8_t *)&detect_pkg,array,39);
	//memcpy(&detect_pkg,array,39);
	
	//printf("%s\n",array);
	
	//printf("%s\n",detect_pkg2.head);
	
	//memcpy((&detect_pkg2)+5,(&ica_pkg)+32,32);					//failed
	//memcpy((char*)(detect_pkg2+5),(char*)(ica_pkg+32),32);		//error
	//memcpy((char*)(&detect_pkg2+5),(char*)(&ica_pkg+32),32);		//failed
	//memcpy((&detect_pkg2)+5,(&ica_pkg)+32,32);					//failed
	//memcpy((char*)(&detect_pkg2)+5,(char*)(&ica_pkg)+32,32);		//yes	
	//memcpy((uint8_t*)(&detect_pkg2)+5,(uint8_t*)(&ica_pkg),32);	//yes
	//memcpy(detect_pkg2.data,(uint8_t*)(&ica_pkg),32);				//yes
	//memcpy(&detect_pkg2.data,(uint8_t*)(&ica_pkg),32);			//yes

	//struct avalon_pkg *p = &detect_pkg2;
	//printf("%c%c%c%c%c%s%c%c\n",p->head[0],p->head[1],p->type,p->idx,p->cnt,p->data,p->crc[0],p->crc[1]);
	
	uint8_t array[39]="AV211jjjjjwwwwwkkkkkllllliiiiiooooo3356";
	struct avalon_pkg data_pkg;
	int ret = avalon2_init_pkg(&data_pkg,AVA2_P_DATA,1,1);
	//strcpy(array,(uint8_t*)&data_pkg);
	printf("%d\n",ret);

	memcpy(array,(uint8_t*)&data_pkg,32);
	printf("data_pkg=%s\n",(char*)&data_pkg);
	printf("array   =%s\n",array);
	printf("array[2] = %i\narray[3] = %d\narray[4] = %d\n",array[2],array[3],array[4]);
	return 0;
}


