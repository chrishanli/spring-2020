

#ifndef JC_DECODER_H
#define JC_DECODER_H

#define DLLIMPORT __declspec(dllexport)

#include <jc.h>

// 8/3/2020改进
DLLIMPORT extern J_Data* decodeJCode(const char src[], Int32 mode, Int32* status, BYTE expected_pack_no);  // Decode (标准)
// 8/3/2020改进
J_Data* decodeJCodeEx(const char src[], Int32 mode, Int32* status, J_Decoded_Symbol* symbols, BYTE expected_pack_no);  // Decode（专业）
// 9/3/2020改进
DLLIMPORT J_Data* decodeJCodeEx_using_found_symbol(	J_Found_Symbol* found_ms,
                                         			Int32 mode,
                                         			Int32* status,
                                         			J_Decoded_Symbol* symbols,
										 			int expected_pack_no);

DLLIMPORT extern int deprecated decoder_console_using_pngs_main(int argc, char **argv);

DLLIMPORT Int32 decoder_using_found_ms_main(J_Found_Symbol* found_ms,
                                  			int *_correspond_no,
                                  			const Char _dst_file[],
											const Char _dst_val_file[]);

#endif
