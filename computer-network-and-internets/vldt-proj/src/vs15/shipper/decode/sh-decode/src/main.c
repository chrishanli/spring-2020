// main.c    09/03/2020
// J_Code
// @brief: 解码器控制台主函数

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <video-dealer.h>

#ifndef _
#define VERSION    "2.0.0"
#define BUILD_DATE __DATE__
#define SH_ERROR(x) { printf("Shipper: Error - "); printf x; printf("\n"); }
#endif

char input_video_path[200] = {0};
char output_file_path[200] = {0};
char output_verify_path[200] = {0};
int sample_freq = 28;
char is_verify_applied = 0;

void printUsage() {
    printf("\n");
    printf("Shipper > Decoder (Version %s Build date: %s)\n", VERSION, BUILD_DATE);
    printf(" - Li Han, Miao Xinyu, Li Zhenlang & Chen Kechun of the University of Xiamen\n");
    printf(" - Powered by the J_Code library & OpenCV 2.4.1.1\n\n");
    printf("OVERVIEW: The Shipper Decoder\n\n");
    printf("USAGE: ");
    printf(" decode input-video-path output-file-path output-verify-file [options]\n\n");
    printf("input-video-path\tName and path of the input video.\n");
    printf("output-file-path\tName and path of the output file.\n");
    printf("output-verify-file\tName and path of the verify file. \n");
    printf("\n");
    printf("OPTIONS:\n\n");
    printf("--sample-freq\t\tSample frequency (i.e. Frames to extract from video) \n\t\t\t within 1 second. (default: 28)\n");
    printf("--verify\t\tToggle verification procedure after decoding\n\t\t\t (default: no).\n");
	printf("--help\t\t\tPrint this help.\n");
    printf("--about\t\t\tAbout this app.\n");
    printf("\n");
	printf("EXAMPLES:\n\n");
    printf("Example for simple decode: \n");
    printf("\tdecode hello.avi 1.bin 1_v.bin\n");
    printf("\n");
    printf("Example for advanced decode which draws 12 frames per second from video, and perform an MD5 check after decoding: \n" );
    printf("\tencode hello.avi 1.bin 1_v.bin --sample-freq 12 --verify\n");
    printf("\n");
}


int parseArguments(int argc, const char * argv[]) {
    // first scan
    if (argc < 4) {
        SH_ERROR(("Arguments not completed."))
        return -1;
    }
    strcpy_s(input_video_path, sizeof(input_video_path), argv[1]);   // input file path
    strcpy_s(output_file_path, sizeof(output_file_path), argv[2]);
    strcpy_s(output_verify_path, sizeof(output_verify_path), argv[3]);
    for (int loop = 4; loop < argc; loop++) {
        if (0 == strcmp(argv[loop], "--sample-freq")) {
            if(loop + 1 > argc - 1) {
                SH_ERROR(("Value for option '%s' missing.\n", argv[loop]))
                return -1;
            }
            char* endptr;
            sample_freq = (int)strtol(argv[++loop], &endptr, 10);
            if (*endptr || sample_freq < 0) {
                SH_ERROR(("Invalid or missing values for option '%s'.\n", argv[loop - 1]))
                return -1;
            }
        } else if (0 == strcmp(argv[loop], "--verify")) {
        	is_verify_applied = 1;
        }
    }
    
//    if (sample_freq >= 40) {
//        SH_ERROR(("Sample frequency too high."))
//    }
    
    return 1;
}

void printAck() {
    printf("+-------------------------------+\n");
    printf("        Shipper Decoder         \n");
    printf("    Powered by J_Code 1.0.1.31  \n");
    printf("       Han Li Studios 2020      \n");
    printf("            Groupmates:         \n");
    printf("       Li Han, Miao Xinyu,      \n");
    printf("           Li Zhenlang          \n");
    printf("              Thank:            \n");
    printf("        Xiamen University       \n");
    printf("         The OpenCV Group       \n");
    printf("          Fraunhofer SIT        \n");
    printf("                                \n");
    printf("       All Rights Reserved.     \n");
    printf("+-------------------------------+\n");
}

int main(int argc, const char * argv[]) {
    // insert code here...
    if (argc < 2 || (0 == strcmp(argv[1],"--help"))) {
        printUsage();
        return 0;
    } else if (0 == strcmp(argv[1],"--about")) {
        printAck();
        return 0;
    }
    if (parseArguments(argc, argv) < 0) {
        return 255;
    }
    
    printf("----------Info----------\n");
    printf(" input video path: %s\n", input_video_path);
    printf(" output file path: %s\n", output_file_path);
    printf(" output verify file path: %s\n", output_verify_path);
    printf(" sample frequency: %d\n", sample_freq);
    printf("------------------------\n");
    
    // 删除目的文件（如有）
	remove(output_file_path);
	remove(output_verify_path);
    
    // 开始做事 
    int ret = doVideoDecoding(sample_freq, input_video_path, output_file_path, output_verify_path);
    if (ret) {
    	printf("Shipper Info: Programme exited with %d frames dealt.", ret);
    	// 如果申请verify 
    	if (is_verify_applied) {
    		verify(output_file_path);
		}
		return 0;
	}
    
    return 1;
}
