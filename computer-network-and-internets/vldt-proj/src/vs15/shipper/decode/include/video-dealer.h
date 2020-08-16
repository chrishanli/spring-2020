#ifndef _DLL_H_
#define _DLL_H_

#ifdef __cplusplus
extern "C" {
#endif

extern int doVideoDecoding(int fps, const char videofilename[], const char datafilename[], const char output_verify_path[]);

#ifdef __cplusplus
}
#endif

#endif
