// file-info.c   29/03/2020
// J_Code
// @brief: �������ļ���������ļ�ѶϢ�����

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "dll.h"

// �õ��ļ�ѶϢ 
void getFileInfo(SH_FileInfo *fileInfo, const char filepath[], int* status) {
	if (!fileInfo) return;
	FILE* fp;
	errno_t err = fopen_s(&fp, filepath, "rb");
	int i, pathSize = strlen(filepath);
	if (err) {
		*status = -1;
		return;
	}
	
	// �½�һ�ṹ�壨����ʧ�ܣ� 
	// �õ� �ļ���
	for (i = pathSize; i >= 0; i--) {
		if (filepath[i] == '\\' || filepath[i] == '/') {
			// �ҵ��ļ���
			strcpy_s(fileInfo->filename, sizeof(fileInfo->filename), filepath + i + 1);
			break;
		}
	}
	
	// �õ� �ļ���С
	fseek(fp, 0L, SEEK_END);
	fileInfo->file_size = ftell(fp);
	
	// �õ�MD5 Checksum 
	getFileMD5(filepath, fileInfo->checksum);
	
	fclose(fp);
	*status = 1;
}

// �Ƚ��ļ�ѶϢ     
int checkFileInfo(const char filepath[], SH_FileInfo* fileInfo) {
	FILE* fp;
	errno_t err = fopen_s(&fp, filepath, "rb");
	SH_FileInfo fileInfoBuffer;
	int i, pathSize = strlen(filepath);
	if (err) {
		return 0;		// �ļ��򲻿� 
	}
	
	// �õ� �ļ���
	for (i = pathSize; i >= 0; i--) {
		if (filepath[i] == '\\' || filepath[i] == '/') {
			// �ҵ��ļ���
			strcpy_s(fileInfoBuffer.filename, sizeof(fileInfoBuffer.filename), filepath + i + 1);
		}
	}
	// �Ƚ��ļ���
	if (!strcmp(fileInfo->filename, fileInfoBuffer.filename)) {
		fclose(fp);
		return -1;	// �ļ��������� 
	} 
	
	// �õ� �ļ���С
	fseek(fp, 0L, SEEK_END);
	// �Ƚ��ļ���С
	if (ftell(fp) != fileInfo->file_size) {
		fclose(fp);
		return -2;	// �ļ���С������ 
	}
	
	// �õ�MD5 Checksum 
	getFileMD5(filepath, fileInfoBuffer.checksum);
	// �Ƚ�MD5 Checksum
	for (i = 0; i < 16; i++) {
		if (fileInfoBuffer.checksum[i] != fileInfo->checksum[i]){
			fclose(fp);
			return -3;	// MD5������ 
		}

	} 
	fclose(fp);
	return 1;	// �ļ����� 
}

