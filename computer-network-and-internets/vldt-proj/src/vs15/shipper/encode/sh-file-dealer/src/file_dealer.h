
#include <jc.h>

// 26/3/2020 ��ֹ����ȷ��ʱ������������֡��
#define MAX_FRAME_PER_SECOND 9

typedef struct {
	char filename[20];
	unsigned long file_size;
	unsigned char checksum[16];
} SH_FileInfo;

typedef struct {
	int packs;
	J_Data* data[];
} SH_Pack; 		// һ���ļ���ӳ�� 


