// video-import.cpp   29/03/2020
// J_Code
// @brief: 解码器视频导入机函数

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
// 鏈伐绋� 
#include <jc.h>
#include "video-import.h"
#include "../dll.h"

#define SH_REPORT_ERROR(x) {cout << "SH_ERROR: " << x << endl;}
#define _CRT_SECURE_NO_WARNINGS // //4996 閿欒璀﹀憡瑙ｅ喅 
#pragma warning(disable:4996)

using namespace std;
// 灏咥VFrame杞崲鎴怣at 
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

// 瑙嗛澶勭悊鍥炶皟鍑芥暟
int test(cv::Mat m, unsigned char  expected_frame_no, int* total_frame_no, const char datafilename[]) {
	if (m.empty()) {
		SH_REPORT_ERROR("Image extracted from ffmpeg failed.")
		return FAILURE;
	}
	cv::namedWindow("MyWindow", CV_WINDOW_AUTOSIZE);
	//鍦∕yWindow鐨勭獥涓腑鏄剧ず瀛樺偍鍦╥mg涓殑鍥剧墖
	cv::imshow("MyWindow", m);
	cv::waitKey(5000);
	cv::destroyWindow("MyWindow");	//閿�姣丮yWindow鐨勭獥鍙�
}


int dealWithVideo(	int fps_need, const char videofilename[], const char datafilename[],
					const char validfilename[],
					int (*foo)(cv::Mat, int*, int*, const char*, const char*)) {

	cv::Mat MatFrame;
	int average_fps;
	av_register_all();
	avformat_network_init();
	AVFormatContext* pFormatCtx;
	AVCodecContext* pCodecCtx, * pCodecCtxOrig;	//瀹氫箟AVCodecContext锛屼娇鐢ㄨ鍙橀噺鍙互灏嗗叾瀹氫箟涓篺fmpeg瑙ｇ爜绫荤殑绫绘垚鍛樸��
	AVCodec* pCodec;			//瀹氫箟AVCodec锛孉VCodec *鍙橀噺涓鸿В鐮佸櫒鎸囬拡銆�
	
	//涓簆FormatCtx缁撴瀯鐢宠绌洪棿 鎵撳紑鏂囦欢
	pFormatCtx = NULL;// avformat_alloc_context();

	if (avformat_open_input(&pFormatCtx, videofilename, NULL, NULL) != 0)
	{
		SH_REPORT_ERROR("in FFMPEG: Video file do not exist or can not access.")
		return 0;

	}

	//鍙栧嚭瑙嗛娴佷俊鎭�
	if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
	{
		SH_REPORT_ERROR("in FFMPEG: avformat_find_stream_info error.")
		return 0;
	}
//	printf("鏂囦欢淇℃伅-----------------------------------------\n");
//	av_dump_format(pFormatCtx, 0, videofilename, 0);
//	//av_dump_format鍙槸涓皟璇曞嚱鏁帮紝杈撳嚭鏂囦欢鐨勯煶銆佽棰戞祦鐨勫熀鏈俊鎭簡锛屽抚鐜囥�佸垎杈ㄧ巼銆侀煶棰戦噰鏍风瓑绛�
//	printf("-------------------------------------------------\n");

	//鏌ユ壘瑙嗛娴�
	int videoindex = -1;
	int i;
	for (i = 0; i < pFormatCtx->nb_streams; i++)
	{
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			videoindex = i;       //绗竴涓棰戞祦瀵瑰簲鐨勪笅鏍�
			break;
		}
	}
	if (videoindex == -1)
	{
		SH_REPORT_ERROR("In FFMPEG: Can't find video stream")
		return 0;
	}
	//int FrameCount = pFormatCtx->streams[i]->nb_frames;		//鑾峰彇瑙嗛鐨勬�诲抚鏁�
	if (pFormatCtx->streams[i]->avg_frame_rate.den != 0 && pFormatCtx->streams[i]->avg_frame_rate.num != 0) {
		// 姣忕澶氬皯甯�
		average_fps = pFormatCtx->streams[i]->avg_frame_rate.num / pFormatCtx->streams[i]->avg_frame_rate.den;
	}

	int ternal = average_fps / fps_need;
	if (ternal < 1) {
		// 鍑虹幇FPS閿欒 
		SH_REPORT_ERROR("In FFMPEG: Preferred sample frequency is higher than ave. FPS of the original video file, which is not allowed.")
		return 0;
	}
	pCodecCtxOrig = pFormatCtx->streams[videoindex]->codec;
	//pCodecCtxOrig = pFormatCtx->streams[videoStream]->codec;
	//鏌ユ壘璇ヨ棰戞祦瀵瑰簲鐨勮В鐮佸櫒AVCodec
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
	//鐢ㄨ瑙ｇ爜鍣ㄦ墦寮�璇ヨ棰戞枃浠�
	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
	{
		SH_REPORT_ERROR("In FFMPEG: avcodec_open2 error.")
		return 0;
	}
	//AVFrame  鏄瓨鍌ㄨ棰戞暟鎹殑缁撴瀯
	AVFrame* pframe = NULL;	//pframe  鏄粠瑙嗛娴佷腑璇诲嚭鏉ョ殑鍥剧墖鏁版嵁鏀惧湪鍏朵腑
	AVFrame* pframeRGB = NULL;   //pframeRGB 鏄�氳繃 sws_scale杞寲鏍煎紡涔嬪悗鎵�瑕佸偍瀛樼殑鏁版嵁缁撴瀯
	pframe = av_frame_alloc();
	pframeRGB = av_frame_alloc();

	if (pframeRGB == NULL)
	{
		SH_REPORT_ERROR("In FFMPEG: pframeRGB_alloc error.")
		return 0;
	}
	//鐢宠绌洪棿 閫氳繃寰楀埌PIX_FMT_RGB24鐨勪竴涓儚绱犳墍鍗犵敤鐨勫唴瀛�
	uint8_t* out_buffer = (uint8_t*)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_RGB24, pCodecCtx->width,
		pCodecCtx->height, 1));

	//灏唎ut_buffer 浠frameRGB鐨勫舰寮忓叧鑱旇捣鏉�
	av_image_fill_arrays(pframeRGB->data, pframeRGB->linesize, out_buffer, AV_PIX_FMT_RGB24,
		pCodecCtx->width, pCodecCtx->height, 1);
	//杞崲涓婁笅鏂�  
	AVPacket packet;
	int got_picture;
	struct  SwsContext* sws_ctx;
	sws_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
		pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL);
	int  j = 0;
	//cout << "鎬诲抚鏁帮細" << FrameCount << pFormatCtx->streams[i]->nb_frames << endl;
//	cout << "甯х巼锛�" << average_fps << endl
//		<<"闂撮殧锛�"<<ternal<<endl;
	int expected_frame_no = 0;
	int already_frame_c = 0, total_frame_c = 0, ret;
	while (av_read_frame(pFormatCtx, &packet) >= 0)			//浠巔FormatCtx涓�甯т竴甯ц鍙栧帇缂╃殑瑙嗛鏁版嵁锛屽瓨杩沺packet涓�
	{
		if (packet.stream_index == videoindex)    //鎵惧埌瑙嗛娴�
		{
			//瑙ｇ爜涓�甯ц棰戞暟鎹�,杈撳叆涓�涓帇缂╃紪鐮佺殑缁撴瀯浣損packet锛岃緭鍑轰竴涓В鐮佸悗鐨勭粨鏋勪綋pframe銆�
			avcodec_decode_video2(pCodecCtx, pframe, &got_picture, &packet);
			if (got_picture) {
				j++;
				if (j % ternal == 0) {
					MatFrame = AvFrame_to_CVMat(pframe);
					// 鍥炶皟 
					ret = foo(MatFrame, &expected_frame_no, &total_frame_c, datafilename, validfilename);
					// 鍒ゆ柇鏄惁缁堟
					if (ret == STOP) {
						break;	// 缁堟寰幆 
					} else if (ret == SUCCESS) {
						// 鎴愬姛鎵弿涓�涓� 
						already_frame_c++;
						cout << "SH_INFO: Received Pack: " << expected_frame_no++ << endl;
						// 濡傛灉鍒板ご灏辩粓姝㈠惊鐜� 
						if (already_frame_c == total_frame_c)
							break;
					} else if (ret == LOST) {
						// 涓㈠け涓�涓垨澶氫釜鍖�
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



