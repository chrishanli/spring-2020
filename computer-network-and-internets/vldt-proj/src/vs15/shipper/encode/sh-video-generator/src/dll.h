#ifndef _DLL_H_
#define _DLL_H_

#define DLLIMPORT __declspec(dllexport)

#ifdef __cplusplus
extern "C" {
#endif


DLLIMPORT int generateDataVideo_c(const char filePath[], const char videoPath[], int length, int videoWidth, int videoHeight);


#ifdef __cplusplus
}
#endif

#endif
