#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <unistd.h>// VISUAL STUDIOÆúÓÃ
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "md5.h"
#include "../dll.h"

#define READ_DATA_SIZE	1024
#define MD5_SIZE		16
// #define MD5_STR_LEN		(MD5_SIZE * 2)

/**
 * compute the value of a string
 * @param  dest_str
 * @param  dest_len
 * @param  md5_str
 */
int getStrMD5(unsigned char *dest_str, unsigned int dest_len, char *md5_str)
{
	memset(md5_str, 0, 33);
	int i;
	unsigned char md5_value[MD5_SIZE];
	MD5_CTX md5;

	// init md5
	MD5Init(&md5);

	MD5Update(&md5, dest_str, dest_len);

	MD5Final(&md5, md5_value);

	// convert md5 value to md5 string
	char tmp[3];
	for(i = 0; i < MD5_SIZE; i++)
	{
		//sprintf(tmp, "%02x", md5_value[i]);
		sprintf_s(tmp, sizeof(tmp), "%02x", md5_value[i]);
		// strcat(md5_str, tmp);
		strcat_s(md5_str, sizeof(md5_str), tmp);
		// snprintf(md5_str + i * 2, 2+1, "%02x", md5_value[i]);
	}

	return 0;
}

/**
 * compute the value of a file
 * @param  file_path
 * @param  md5_str
 * @return 0: ok, -1: fail
 */
int getFileMD5(const char *file_path, char *md5_str)
{
	int i;
	int fd;
	int ret;
	unsigned char data[READ_DATA_SIZE];
	unsigned char md5_value[MD5_SIZE];
	MD5_CTX md5;

	fd = open(file_path, O_RDONLY);
	if (-1 == fd)
	{
		perror("open");
		return -1;
	}

	// init md5
	MD5Init(&md5);

	while (1)
	{
		ret = read(fd, data, READ_DATA_SIZE);
		if (-1 == ret)
		{
			perror("read");
			close(fd);
			return -1;
		}

		MD5Update(&md5, data, ret);

		if (0 == ret || ret < READ_DATA_SIZE)
		{
			break;
		}
	}

	close(fd);

	MD5Final(&md5, md5_value);

	// Êä³ö 
	memcpy(md5_str, md5_value, MD5_SIZE);
	return 0;
}

