
#include <jc.h>
#define MD5_SIZE		16

typedef struct {
	char filename[20];
	unsigned long file_size;
	unsigned char checksum[16];
} SH_FileInfo;

typedef struct {
	int packs;
	J_Data* data[];
} SH_Pack; 		// 一个文件包映像 


// 计算文件MD5 
extern int getFileMD5(const char *file_path, char *md5_str);

// 计算文件讯息 
extern void getFileInfo(SH_FileInfo *fileInfo, const char filepath[], int* status);

// 比照文件讯息 
extern int checkFileInfo(const char filepath[], SH_FileInfo* fileInfo);

extern SH_Pack* fileImport(const char filePath[]);

extern void destroyFilePack(SH_Pack* pack);
