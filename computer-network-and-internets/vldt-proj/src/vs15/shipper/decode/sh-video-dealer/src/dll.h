#ifndef _DLL_H_
#define _DLL_H_

#define DLLIMPORT __declspec(dllexport)

extern "C" {
DLLIMPORT int doVideoDecoding(int fps, const char videofilename[], const char datafilename[], const char validfilename[]);
}
#endif
