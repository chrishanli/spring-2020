// ldpc.h    09/03/2020
// J_Code
// brief: LDPC编解码元件头文件

#ifndef JC_LDPC_H
#define JC_LDPC_H

#define LPDC_METADATA_SEED 	38545
#define LPDC_MESSAGE_SEED 	785465

extern J_Data *encodeLDPC(J_Data* data, Int32* coderate_params);
extern Int32 decodeLDPChd(BYTE* data, Int32 length, Int32 wc, Int32 wr);
extern Int32 decodeLDPC(Float* enc, Int32 length, Int32 wc, Int32 wr, BYTE* dec);


#endif
