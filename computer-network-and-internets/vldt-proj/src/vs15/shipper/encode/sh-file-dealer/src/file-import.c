<<<<<<< HEAD:src/vs15/shipper/encode/sh-file-dealer/src/file-import.c
// file-import.c   29/03/2020
// J_Code
// @brief: ±àÂëÆ÷ÎÄ¼şµ¼Èë»ú

// ÊäÈëÊı¾İ£¬²¢×ª»»ÎªÒ»¸öJ_DataÊı×é 
=======
// è¾“å…¥æ•°æ®ï¼Œå¹¶è½¬æ¢ä¸ºä¸€ä¸ªJ_Dataæ•°ç»„ 
>>>>>>> 0e4b4c744c880271eee04c60c95c8187093a2fff:src/encoder/file_dealer/file-import.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jc.h>
#include "dll.h"

<<<<<<< HEAD:src/vs15/shipper/encode/sh-file-dealer/src/file-import.c
// ¹¹½¨filePack 
SH_Pack* fileImport(const char filePath[], int desiredTime_ms) {
=======
// æ„å»ºfilePack 
SH_Pack* fileImport(const char filePath[]) {
>>>>>>> 0e4b4c744c880271eee04c60c95c8187093a2fff:src/encoder/file_dealer/file-import.c
	int getFileInfoStatus, i;
	SH_FileInfo fileinfo;
	getFileInfo(&fileinfo, filePath, &getFileInfoStatus);
	if (getFileInfoStatus < 0) {
		printf("SH_ERROR: Import Error - No such file or directory.\n");
		return NULL;
	}

	// ·ÖÎö£º¸ÃÎÄ¼şÊÇ·ñÄÜ¹»±»ÍêÕûÊäÈë
	int maxData = (desiredTime_ms / 1000 * MAX_FRAME_PER_SECOND - 3) * MAXIMUM_FILE_SIZE;
	if (fileinfo.file_size > maxData) {
		// ¸ÃÎÄ¼şÌ«´ó£¬±ØĞë±»½ØÈ¡µôÒ»²¿·Ö
		printf_s("SH_WARNING: The preferred video length is not enough, which provides capacity of only %d Bytes, while the inputed file sizes %d Bytes. The over flowed data will be diminished.\n", maxData, fileinfo.file_size);
		fileinfo.file_size = maxData;
	}
	
	// å¼•å…¥æˆåŠŸï¼Œæå–æ–‡ä»¶ä¿¡æ¯ï¼Œç¼–å†™ç¬¬ä¸€ä¸ªæ•°æ®åŒ…
	J_Data* filepackHead = (J_Data *)malloc(sizeof(J_Data) + 1 + sizeof(fileinfo));
	if (!filepackHead) {
		return NULL;
	}
	filepackHead->length = sizeof(int) + sizeof(fileinfo);
	int* pack_num_buff = (int *)filepackHead->data;	
	*pack_num_buff = 0;	// é¦–å¸§ï¼
	memcpy(filepackHead->data+sizeof(int), &fileinfo, sizeof(fileinfo));
	
	int pack_mod =  fileinfo.file_size % MAXIMUM_FILE_SIZE;		// æœ€åä¸€å¸§çš„é•¿åº¦ 
	int pack_int =  fileinfo.file_size / MAXIMUM_FILE_SIZE;
	int pack_num =  pack_int + 
					(pack_mod ? 1 : 0);
	
<<<<<<< HEAD:src/vs15/shipper/encode/sh-file-dealer/src/file-import.c
	// ÏÖÔÚ»ñÈ¡ÎÄ¼ş°ü 
	SH_Pack* newPack = (SH_Pack*)malloc(sizeof(SH_Pack) + sizeof(J_Data*) * (pack_num + 1));
=======
	// ç°åœ¨è·å–æ–‡ä»¶åŒ… 
	SH_Pack* newPack = (SH_Pack*)malloc(sizeof(SH_Pack) + sizeof(J_Data*) * (pack_num + sizeof(int)));
>>>>>>> 0e4b4c744c880271eee04c60c95c8187093a2fff:src/encoder/file_dealer/file-import.c
	if (!newPack) {
		free(filepackHead);
		return NULL;
	}
	newPack->packs = 1;
	newPack->data[0] = filepackHead;
<<<<<<< HEAD:src/vs15/shipper/encode/sh-file-dealer/src/file-import.c
	// ÏÖÔÚĞ´ÈëÎÄ¼ş°ü
	FILE* fp;
	errno_t err = fopen_s(&fp, filePath, "rb");
	if (err) {
=======
	// ç°åœ¨å†™å…¥æ–‡ä»¶åŒ…
	FILE* fp = fopen(filePath, "rb");
	if (!fp) {
>>>>>>> 0e4b4c744c880271eee04c60c95c8187093a2fff:src/encoder/file_dealer/file-import.c
		free(filepackHead);
		free(newPack);
		return NULL;
	}
	for (i = 0; i < pack_num - 1; i++) {
		J_Data* filepack = (J_Data *)malloc(sizeof(J_Data) + sizeof(int) + MAXIMUM_FILE_SIZE);	// unsafe
		filepack->length = sizeof(int) + MAXIMUM_FILE_SIZE;
		pack_num_buff = (int *)filepack->data;
		*pack_num_buff = i + 1; // å†™å…¥å¸§ç¼–å· 
		fread(filepack->data + sizeof(int), MAXIMUM_FILE_SIZE, 1, fp);
		// æ±‡å…¥åŒ…
		newPack->data[i + 1] = filepack;
		newPack->packs++;
	}
	// ç°åœ¨å†™å…¥å°¾åŒ…
	if (pack_mod) {
		J_Data* filepack = (J_Data *)malloc(sizeof(J_Data) + sizeof(int) + pack_mod);			// unsafe
		filepack->length = sizeof(int) + pack_mod;
		pack_num_buff = (int *)filepack->data;
		*pack_num_buff = pack_num;
		fread(filepack->data + sizeof(int), pack_mod, 1, fp);
		// æ±‡å…¥åŒ…
		newPack->data[pack_num] = filepack;
	} else {
		J_Data* filepack = (J_Data *)malloc(sizeof(J_Data) + sizeof(int) + MAXIMUM_FILE_SIZE);	// unsafe
		filepack->length = sizeof(int) + MAXIMUM_FILE_SIZE;
		pack_num_buff = (int *)filepack->data;
		*pack_num_buff = pack_num;
		fread(filepack->data + sizeof(int), MAXIMUM_FILE_SIZE, 1, fp);
		// æ±‡å…¥åŒ…
		newPack->data[pack_num] = filepack;
	}
	newPack->packs++;
	// ç»“æŸ 
	fclose(fp);
	/*
	printf("File Size: %d\n", fileinfo.file_size);
	printf("Written: %d packs\n" , pack_num);
	printf("Mod: %d\n", pack_mod);
	*/
	return newPack;
}

// é”€æ¯filePack 
void destroyFilePack(SH_Pack* pack) {
	// freeæ‰å…¶ä¸­æ‰€æœ‰J_Data
	int i = 0;
	for (i = 0; i < pack->packs; i++) {
		free(pack->data[i]);
	}
	// freeæ‰pack
	free(pack); 
}
