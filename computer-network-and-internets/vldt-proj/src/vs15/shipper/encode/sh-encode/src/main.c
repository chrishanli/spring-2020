// main.c   29/03/2020
// J_Code
// @brief: 编码器控制台主函数

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <video_generator.h>
#ifndef _
#define VERSION "1.0.3"
#define BUILD_DATE __DATE__
#define SH_ERROR(x) { printf("Shipper: Error - "); printf x; printf("\n"); }
#endif

char input_file_path[200] = {0};
char output_video_path[200] = {0};
int length = 5000;
int colour_number = 8;
int video_width = 0;
int video_height = 0;


/**
 * @brief Print usage
 */
void printUsage() {
    printf("\n");
    printf("Shipper > Encoder (Version %s Build date: %s)\n", VERSION, BUILD_DATE);
    printf(" - Li Han, Miao Xinyu, Li Zhenlang & Chen Kechun of the University of Xiamen\n");
    printf(" - Powered by the J_Code library\n\n");
    printf("OVERVIEW: The Shipper Encoder\n\n");
    printf("USAGE: ");
    printf("encode input-file-path output-video-path [length] [options] \n\n");
    printf("input-file-path\t\tName and path of the input file.\n");
    printf("output-video-path\tName and path of the output video.\n");
    printf("length\t\t\tLength of the output video (ms) (default: 5000).\n\n");
    printf("OPTIONS:\n\n");
    printf("--colour-number\t\tNumber of colours (4 or 8, default: 8).\n");
    printf("--video-width\t\tWidth of frames in video (default: computed).\n");
    printf("--video-height\t\tHeight of frames in video (default: computed).\n");
    printf("");
    printf("--help\t\t\tPrint this help.\n");
    printf("--about\t\t\tAbout this app.\n");
    printf("\n");
	printf("EXAMPLES:\n\n");
    printf("Example for simple generation of default arguments: \n");
    printf("\tencode 1.bin hello.avi 1000\n");
    printf("\n");
    printf("Example for advanced generation: \n" );
    printf("\tencode 1.bin hello.avi 1000 --video-width 720 --video-height 480\n");
    printf("\n");
	printf("NOTE: Only AVI files could be generated through currently-available procedures. We feel sorry for the inconvenience caused therefrom. \n");
}


int parseArguments(int argc, const char * argv[]) {
    // first scan
    if (argc < 3) {
        SH_ERROR(("Arguments not completed."))
        return -1;
    }
    strcpy_s(input_file_path, sizeof(input_file_path), argv[1]);   // input file path
    strcpy_s(output_video_path, sizeof(output_video_path), argv[2]);
    for (int loop = 3; loop < argc; loop++) {
        if (0 == strcmp(argv[loop], "--colour-number")) {
            if(loop + 1 > argc - 1) {
                SH_ERROR(("Value for option '%s' missing.", argv[loop]))
                return -1;
            }
            char* endptr;
            colour_number = (int)strtol(argv[++loop], &endptr, 10);
            if (*endptr) {
                SH_ERROR(("Invalid or missing values for option '%s'.", argv[loop - 1]))
                return -1;
            }
            if (colour_number != 4  && colour_number != 8) {
                SH_ERROR(("Invalid color number. Valid color number includes 4 and 8."))
                return -1;
            }
        } else if (0 == strcmp(argv[loop], "--video-width")) {
            if(loop + 1 > argc - 1) {
                SH_ERROR(("Value for option '%s' missing.\n", argv[loop]))
                return -1;
            }
            char* endptr;
            video_width = (int)strtol(argv[++loop], &endptr, 10);
            if (*endptr || video_width < 0) {
                SH_ERROR(("Invalid or missing values for option '%s'.\n", argv[loop - 1]))
                return -1;
            }
        } else if (0 == strcmp(argv[loop], "--video-height")) {
            if(loop + 1 > argc - 1) {
                SH_ERROR(("Value for option '%s' missing.\n", argv[loop]))
                return -1;
            }
            char* endptr;
            video_height = (int)strtol(argv[++loop], &endptr, 10);
            if (*endptr || video_height < 0) {
                SH_ERROR(("Invalid or missing values for option '%s'.", argv[loop - 1]))
                return -1;
            }
        } else {
            if (0 == (length = atoi(argv[loop]))) {
                SH_ERROR(("Unknown option: '%s' in option line. Type 'encoder --help' for usage helps.\n", argv[loop]))
                return -1;
            }
        }
    }
    
    // second scan (format)
    char* endptr = NULL;
    for (int i = (int)strlen(output_video_path); i >= 0; i--) {
        if (output_video_path[i] == '.') {
            endptr = output_video_path + i;
            break;
        }
    }
    if (strcmp(endptr, ".avi")) {
        SH_ERROR(("Not supported video format(%s). Use .avi instead.", endptr))
        return -1;
    }
    
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
    if (argc < 2 || (0 == strcmp(argv[1],"--help"))) {
        printUsage();
        return 1;
    } else if (argc < 2 || (0 == strcmp(argv[1],"--about"))) {
        printAck();
        return 1;
    }
    if (parseArguments(argc, argv) < 0) {
        return 255;
    }
    
    generateDataVideo_c(input_file_path, output_video_path, length, video_width, video_height);
    
    return 0;
}

