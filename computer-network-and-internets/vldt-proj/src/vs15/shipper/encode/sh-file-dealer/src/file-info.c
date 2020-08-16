// file-info.c   29/03/2020
// J_Code
// @brief: 编码器文件导入机的文件讯息处理机

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "dll.h"

// 拿到文件讯息 
void getFileInfo(SH_FileInfo *fileInfo, const char filepath[], int* status) {
	if (!fileInfo) return;
	FILE* fp;
	errno_t err = fopen_s(&fp, filepath, "rb");
	int i, pathSize = strlen(filepath);
	if (err) {
		*status = -1;
		return;
	}
	
	// 新建一结构体（宣告失败） 
	// 拿到 文件名
	for (i = pathSize; i >= 0; i--) {
		if (filepath[i] == '\\' || filepath[i] == '/') {
			// 找到文件名
			strcpy_s(fileInfo->filename, sizeof(fileInfo->filename), filepath + i + 1);
			break;
		}
	}
	
	// 拿到 文件大小
	fseek(fp, 0L, SEEK_END);
	fileInfo->file_size = ftell(fp);
	
	// 拿到MD5 Checksum 
	getFileMD5(filepath, fileInfo->checksum);
	
	fclose(fp);
	*status = 1;
}

// 比较文件讯息     
int checkFileInfo(const char filepath[], SH_FileInfo* fileInfo) {
	FILE* fp;
	errno_t err = fopen_s(&fp, filepath, "rb");
	SH_FileInfo fileInfoBuffer;
	int i, pathSize = strlen(filepath);
	if (err) {
		return 0;		// 文件打不开 
	}
	
	// 拿到 文件名
	for (i = pathSize; i >= 0; i--) {
		if (filepath[i] == '\\' || filepath[i] == '/') {
			// 找到文件名
			strcpy_s(fileInfoBuffer.filename, sizeof(fileInfoBuffer.filename), filepath + i + 1);
		}
	}
	// 比较文件名
	if (!strcmp(fileInfo->filename, fileInfoBuffer.filename)) {
		fclose(fp);
		return -1;	// 文件名不符合 
	} 
	
	// 拿到 文件大小
	fseek(fp, 0L, SEEK_END);
	// 比较文件大小
	if (ftell(fp) != fileInfo->file_size) {
		fclose(fp);
		return -2;	// 文件大小不符合 
	}
	
	// 拿到MD5 Checksum 
	getFileMD5(filepath, fileInfoBuffer.checksum);
	// 比较MD5 Checksum
	for (i = 0; i < 16; i++) {
		if (fileInfoBuffer.checksum[i] != fileInfo->checksum[i]){
			fclose(fp);
			return -3;	// MD5不符合 
		}

	} 
	fclose(fp);
	return 1;	// 文件符合 
}

