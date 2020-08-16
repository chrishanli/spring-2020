#ifndef _DLL_H_
#define _DLL_H_

#define DLLIMPORT __declspec(dllexport)

#include "jc.h"
DLLIMPORT extern int encoder_console_using_pngs_main(int argc, char *argv[]);
DLLIMPORT extern Bitmap* encoder_using_J_Data(J_Data* data);

#endif
