#include "include_global.h"
#include "comlib.h"

__int64 fsize(FILE * fp)
{
	//��ǰλ��
	fpos_t fpos;
	//��ȡ��ǰλ��
	fgetpos(fp, &fpos);
	_fseeki64(fp, 0, SEEK_END);
	__int64 n = _ftelli64(fp);
	//�ָ�֮ǰ��λ��
	fsetpos(fp, &fpos);
	return n;
}

//���������ָʾ��������Ϣ
sttAlgoProg stHAPProg;

sttAlgoProg::sttAlgoProg()
{
	ProgPo = -1;
	strcpy(pcErrorMessage, "");
	strcpy(pcAlgoStatus, "");
}