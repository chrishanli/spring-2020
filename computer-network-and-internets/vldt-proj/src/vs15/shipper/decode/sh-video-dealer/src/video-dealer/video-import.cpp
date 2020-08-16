// video-import.cpp   29/03/2020
// J_Code
// @brief: ��������Ƶ���������

#include <iostream>
// FFmpeg
extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/avutil.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
}

// OpenCV
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui_c.h>
// 本工程 
#include <jc.h>
#include "video-import.h"
#include "../dll.h"

#define SH_REPORT_ERROR(x) {cout << "SH_ERROR: " << x << endl;}
#define _CRT_SECURE_NO_WARNINGS // //4996 错误警告解决 
#pragma warning(disable:4996)

using namespace std;
// 将AVFrame转换成Mat 
cv::Mat AvFrame_to_CVMat(AVFrame* frame)
{
	AVFrame dst;
	cv::Mat MatFrame;
	memset(&dst, 0, sizeof(dst));
	int width = frame->width, height = frame->height;
	MatFrame = cv::Mat(height, width, CV_8UC3);
	dst.data[0] = (uint8_t*)MatFrame.data;
	avpicture_fill((AVPicture*)&dst, dst.data[0], AV_PIX_FMT_BGR24, width, height);
	struct SwsContext* convert_ctx = NULL;
	enum AVPixelFormat src_pixfmt = (enum AVPixelFormat)frame->format;
	enum AVPixelFormat dst_pixfmt = AV_PIX_FMT_BGR24;
	convert_ctx = sws_getContext(width, height, src_pixfmt, width, height, dst_pixfmt, SWS_FAST_BILINEAR, NULL, NULL, NULL);
	sws_scale(convert_ctx, frame->data, frame->linesize, 0, height, dst.data, dst.linesize);
	sws_freeContext(convert_ctx);
	return MatFrame;
}

// 视频处理回调函数
int test(cv::Mat m, unsigned char  expected_frame_no, int* total_frame_no, const char datafilename[]) {
	if (m.empty()) {
		SH_REPORT_ERROR("Image extracted from ffmpeg failed.")
		return FAILURE;
	}
	cv::namedWindow("MyWindow", CV_WINDOW_AUTOSIZE);
	//在MyWindow的窗中中显示存储在img中的图片
	cv::imshow("MyWindow", m);
	cv::waitKey(5000);
	cv::destroyWindow("MyWindow");	//销毁MyWindow的窗口
}


int dealWithVideo(	int fps_need, const char videofilename[], const char datafilename[],
					const char validfilename[],
					int (*foo)(cv::Mat, int*, int*, const char*, const char*)) {

	cv::Mat MatFrame;
	int average_fps;
	av_register_all();
	avformat_network_init();
	AVFormatContext* pFormatCtx;
	AVCodecContext* pCodecCtx, * pCodecCtxOrig;	//定义AVCodecContext，使用该变量可以将其定义为ffmpeg解码类的类成员。
	AVCodec* pCodec;			//定义AVCodec，AVCodec *变量为解码器指针。
	
	//为pFormatCtx结构申请空间 打开文件
	pFormatCtx = NULL;// avformat_alloc_context();

	if (avformat_open_input(&pFormatCtx, videofilename, NULL, NULL) != 0)
	{
		SH_REPORT_ERROR("in FFMPEG: Video file do not exist or can not access.")
		return 0;

	}

	//取出视频流信息
	if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
	{
		SH_REPORT_ERROR("in FFMPEG: avformat_find_stream_info error.")
		return 0;
	}
//	printf("文件信息-----------------------------------------\n");
//	av_dump_format(pFormatCtx, 0, videofilename, 0);
//	//av_dump_format只是个调试函数，输出文件的音、视频流的基本信息了，帧率、分辨率、音频采样等等
//	printf("-------------------------------------------------\n");

	//查找视频流
	int videoindex = -1;
	int i;
	for (i = 0; i < pFormatCtx->nb_streams; i++)
	{
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			videoindex = i;       //第一个视频流对应的下标
			break;
		}
	}
	if (videoindex == -1)
	{
		SH_REPORT_ERROR("In FFMPEG: Can't find video stream")
		return 0;
	}
	//int FrameCount = pFormatCtx->streams[i]->nb_frames;		//获取视频的总帧数
	if (pFormatCtx->streams[i]->avg_frame_rate.den != 0 && pFormatCtx->streams[i]->avg_frame_rate.num != 0) {
		// 每秒多少帧
		average_fps = pFormatCtx->streams[i]->avg_frame_rate.num / pFormatCtx->streams[i]->avg_frame_rate.den;
	}

	int ternal = average_fps / fps_need;
	if (ternal < 1) {
		// 出现FPS错误 
		SH_REPORT_ERROR("In FFMPEG: Preferred sample frequency is higher than ave. FPS of the original video file, which is not allowed.")
		return 0;
	}
	pCodecCtxOrig = pFormatCtx->streams[videoindex]->codec;
	//pCodecCtxOrig = pFormatCtx->streams[videoStream]->codec;
	//查找该视频流对应的解码器AVCodec
	pCodec = avcodec_find_decoder(pCodecCtxOrig->codec_id);
	if (pCodec == NULL)
	{
		SH_REPORT_ERROR("In FFMPEG: Can't find decoder.")
		return 0;
	}
	// Copy context.
	pCodecCtx = avcodec_alloc_context3(pCodec);
	if (avcodec_copy_context(pCodecCtx, pCodecCtxOrig) != 0) {
		SH_REPORT_ERROR("In FFMPEG: Couldn't copy codec context")
		return 0; // Error copying codec context.
	}
	//用该解码器打开该视频文件
	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
	{
		SH_REPORT_ERROR("In FFMPEG: avcodec_open2 error.")
		return 0;
	}
	//AVFrame  是存储视频数据的结构
	AVFrame* pframe = NULL;	//pframe  是从视频流中读出来的图片数据放在其中
	AVFrame* pframeRGB = NULL;   //pframeRGB 是通过 sws_scale转化格式之后所要储存的数据结构
	pframe = av_frame_alloc();
	pframeRGB = av_frame_alloc();

	if (pframeRGB == NULL)
	{
		SH_REPORT_ERROR("In FFMPEG: pframeRGB_alloc error.")
		return 0;
	}
	//申请空间 通过得到PIX_FMT_RGB24的一个像素所占用的内存
	uint8_t* out_buffer = (uint8_t*)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_RGB24, pCodecCtx->width,
		pCodecCtx->height, 1));

	//将out_buffer 以pframeRGB的形式关联起来
	av_image_fill_arrays(pframeRGB->data, pframeRGB->linesize, out_buffer, AV_PIX_FMT_RGB24,
		pCodecCtx->width, pCodecCtx->height, 1);
	//转换上下文  
	AVPacket packet;
	int got_picture;
	struct  SwsContext* sws_ctx;
	sws_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
		pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL);
	int  j = 0;
	//cout << "总帧数：" << FrameCount << pFormatCtx->streams[i]->nb_frames << endl;
//	cout << "帧率：" << average_fps << endl
//		<<"间隔："<<ternal<<endl;
	int expected_frame_no = 0;
	int already_frame_c = 0, total_frame_c = 0, ret;
	while (av_read_frame(pFormatCtx, &packet) >= 0)			//从pFormatCtx一帧一帧读取压缩的视频数据，存进ppacket中
	{
		if (packet.stream_index == videoindex)    //找到视频流
		{
			//解码一帧视频数据,输入一个压缩编码的结构体ppacket，输出一个解码后的结构体pframe。
			avcodec_decode_video2(pCodecCtx, pframe, &got_picture, &packet);
			if (got_picture) {
				j++;
				if (j % ternal == 0) {
					MatFrame = AvFrame_to_CVMat(pframe);
					// 回调 
					ret = foo(MatFrame, &expected_frame_no, &total_frame_c, datafilename, validfilename);
					// 判断是否终止
					if (ret == STOP) {
						break;	// 终止循环 
					} else if (ret == SUCCESS) {
						// 成功扫描一个 
						already_frame_c++;
						cout << "SH_INFO: Received Pack: " << expected_frame_no++ << endl;
						// 如果到头就终止循环 
						if (already_frame_c == total_frame_c)
							break;
					} else if (ret == LOST) {
						// 丢失一个或多个包
						already_frame_c = expected_frame_no;
						cout << "SH_INFO: Received Pack: " << expected_frame_no++ << endl;
						//SH_REPORT_ERROR("Packet lost detected, but is ignored. Next packet is " << expected_frame_no)
						if (already_frame_c == total_frame_c)
							break;
					}
				}
			}
		}
		av_packet_unref(&packet);
	}
	if (expected_frame_no != total_frame_c) {
		SH_REPORT_ERROR("Decode process terminated unexpectedly on frame " << expected_frame_no << ", while total " << total_frame_c << " frames shall be dealed.")
	} else {
		cout << "SH_INFO: Decode succeedded. " << endl;
	}
	
	av_free(out_buffer);
	av_frame_free(&pframeRGB);
	// Free the YUV frame.
	av_frame_free(&pframe);
	// Close the codecs.
	avcodec_close(pCodecCtx);
	avcodec_close(pCodecCtxOrig);
	// Close the video file.
	avformat_close_input(&pFormatCtx);
	
	return already_frame_c;
}



