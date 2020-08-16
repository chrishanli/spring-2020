#ifndef JC_DECODER_H
#define JC_DECODER_H


extern J_Data* decodeJCode(const char src[], Int32 mode, Int32* status);    // Decode (标准)
// 8/3/2020改进
extern J_Data* decodeJCodeEx(const char src[], Int32 mode, Int32* status, J_Decoded_Symbol* symbols);  // Decode（专业）
// 9/3/2020改进
extern J_Data* decodeJCodeEx_using_found_symbol(	J_Found_Symbol* found_ms,
                                                	Int32 mode,
                                                	Int32* status,
                                                	J_Decoded_Symbol* symbols,
													int expected_pack_no);

extern int decoder_console_using_pngs_main(int argc, char **argv);
extern int decoder_console_using_pngs_main(int argc, char **argv);
extern Int32 decoder_using_found_ms_main(	J_Found_Symbol* found_ms,
											int *_correspond_no,			// ????21/3/2020 
											const Char _dst_file[],
											const Char _dst_val_file[]);

				                                
#endif
