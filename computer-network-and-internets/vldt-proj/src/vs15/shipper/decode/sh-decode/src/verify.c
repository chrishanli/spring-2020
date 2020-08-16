// verify.c    09/03/2020
// J_Code
// @brief: 解码器验证函数

#include <stdio.h>
#include <file-dealer.h>

int verify(const char filepath[]) {
	FILE *fp;
	errno_t err = fopen_s(&fp, "tmp.bin", "rb");
	if (err) return -1;
	
	// 读取一个结构体
	SH_FileInfo fileInfo;
	fread(&fileInfo, sizeof(fileInfo), 1, fp);
	fclose(fp);
	
	// 比照目的文件 
	unsigned char md5s[MD5_SIZE];
	getFileMD5(filepath, md5s);
	int i;
	for (i = 0; i < MD5_SIZE; i++) {
		if (fileInfo.checksum[i] != md5s[i]) {
			break;
		}
	}
	if (i != MD5_SIZE) {
		printf("SH_WARNING: File checksum verification NOT passed!\n");
		printf("Original File Info:\n");
		printf(" File name: %s\n", fileInfo.filename);
		printf(" File size: %d Bytes\n", fileInfo.file_size);
		return 0;
	} else {
		printf("SH_INFO: File checksum verification passed.\n");
		printf("-----Original File Info-----\n");
		printf(" File name: %s\n", fileInfo.filename);
		printf(" File size: %d Bytes\n", fileInfo.file_size);
		return 1;
	}
}
