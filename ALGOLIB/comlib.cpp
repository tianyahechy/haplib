#include "include_global.h"
#include "comlib.h"

__int64 fsize(FILE * fp)
{
	//当前位置
	fpos_t fpos;
	//获取当前位置
	fgetpos(fp, &fpos);
	_fseeki64(fp, 0, SEEK_END);
	__int64 n = _ftelli64(fp);
	//恢复之前的位置
	fsetpos(fp, &fpos);
	return n;
}

//定义进度条指示、出错信息
sttAlgoProg stHAPProg;

sttAlgoProg::sttAlgoProg()
{
	ProgPo = -1;
	strcpy(pcErrorMessage, "");
	strcpy(pcAlgoStatus, "");
}