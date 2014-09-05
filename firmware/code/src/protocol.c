#include "../inc/protocol.h"

int receive_pkg()
{
	struct avalon2_pkg RxAvalongPkg;
	memset(RxAvalongPkg, 0, sizeof(RxAvalongPkg));
	UCOM_Read(RxAvalongPkg,ICA_TASK_LEN);


    return 0;
}


int append_pkg()
{

    return 0;
}

