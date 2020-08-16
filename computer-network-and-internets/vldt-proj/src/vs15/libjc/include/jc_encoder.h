//
//  jc-encoder.h
//  jc-encoder-dylib
//
//  Created by Han Li on 13/3/2020.
//  Copyright © 2020 Han Li. All rights reserved.
//

#ifndef jc_encoder_h
#define jc_encoder_h

#ifdef __cplusplus
extern "C" {
#endif

extern int encoder_console_using_pngs_main(int argc, char *argv[]);
extern Bitmap* encoder_using_J_Data(J_Data* data);


#ifdef __cplusplus
}
#endif

#endif /* jc_encoder_h */
