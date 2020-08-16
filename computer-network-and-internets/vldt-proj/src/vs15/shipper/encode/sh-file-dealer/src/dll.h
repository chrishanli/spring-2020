#ifndef _DLL_H_
#define _DLL_H_

#define DLLIMPORT __declspec(dllexport)

#include "file_dealer.h"

// 计算文件MD5 
DLLIMPORT extern int getFileMD5(const char *file_path, char *md5_str);

// 计算文件讯息 
DLLIMPORT extern void getFileInfo(SH_FileInfo *fileInfo, const char filepath[], int* status);

// 比照文件讯息 
DLLIMPORT extern int checkFileInfo(const char filepath[], SH_FileInfo* fileInfo);

DLLIMPORT extern SH_Pack* fileImport(const char filePath[], int desiredTime);

DLLIMPORT extern void destroyFilePack(SH_Pack* pack);

#endif
