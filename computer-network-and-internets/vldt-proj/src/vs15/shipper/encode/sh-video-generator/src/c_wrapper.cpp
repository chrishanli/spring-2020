
#include "video/video_generator.h"
#include "dll.h"

#ifdef __cplusplus
extern "C" {
#endif

int generateDataVideo_c(const char filePath[], const char videoPath[], int length, int videoWidth, int videoHeight) {
	return generateDataVideo(filePath, videoPath, length, videoWidth, videoHeight);
}

#ifdef __cplusplus
}
#endif
