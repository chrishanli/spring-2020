#include<iostream>

extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/avutil.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
}

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core/core.hpp"
#include <opencv2/highgui/highgui_c.h>


#define _CRT_SECURE_NO_WARNINGS // //4996 ���󾯸��� 
#pragma warning(disable:4996)

using namespace std;


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

void test(cv::Mat m)		//�����Ƿ�ɹ�ת��Mat����
{
	if (m.empty()) {
		cout << "ͼ�����ʧ��" << endl;
		return;
	}
	cv::namedWindow("MyWindow", CV_WINDOW_AUTOSIZE);

	//��MyWindow�Ĵ�������ʾ�洢��img�е�ͼƬ

	cv::imshow("MyWindow", m);

	cv::waitKey(5000);
	

	cv::destroyWindow("MyWindow");	//����MyWindow�Ĵ���


}

void dealWithVideo(int fps_need,const char videofilename[],const char datafilename[]) {

	cv::Mat MatFrame;
	int average_fps;
	av_register_all();
	avformat_network_init();
	AVFormatContext* pFormatCtx;
	AVCodecContext* pCodecCtx, * pCodecCtxOrig;	//����AVCodecContext��ʹ�øñ������Խ��䶨��Ϊffmpeg����������Ա��
	AVCodec* pCodec;			//����AVCodec��AVCodec *����Ϊ������ָ�롣


	//ΪpFormatCtx�ṹ����ռ� ���ļ�
	pFormatCtx = NULL;// avformat_alloc_context();

	if (avformat_open_input(&pFormatCtx, videofilename, NULL, NULL) != 0)
	{
		printf("avformat_open_input  error!!!\n");
		return;

	}

	//ȡ����Ƶ����Ϣ
	if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
	{
		printf("avformat_find_stream_info error !!!\n");
		return;
	}


	printf("�ļ���Ϣ-----------------------------------------\n");
	av_dump_format(pFormatCtx, 0, videofilename, 0);
	//av_dump_formatֻ�Ǹ����Ժ���������ļ���������Ƶ���Ļ�����Ϣ�ˣ�֡�ʡ��ֱ��ʡ���Ƶ�����ȵ�
	printf("-------------------------------------------------\n");



	//������Ƶ��
	int videoindex = -1;
	int i;
	for (i = 0; i < pFormatCtx->nb_streams; i++)
	{
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			videoindex = i;       //��һ����Ƶ����Ӧ���±�
			break;
		}
	}
	if (videoindex == -1)
	{
		printf("can't find video stream");
		return;
	}

	//int FrameCount = pFormatCtx->streams[i]->nb_frames;		//��ȡ��Ƶ����֡��


	if (pFormatCtx->streams[i]->avg_frame_rate.den != 0 && pFormatCtx->streams[i]->avg_frame_rate.num != 0) {
		// ÿ�����֡
		average_fps = pFormatCtx->streams[i]->avg_frame_rate.num / pFormatCtx->streams[i]->avg_frame_rate.den;
	}

	int ternal = average_fps / fps_need;



	pCodecCtxOrig = pFormatCtx->streams[videoindex]->codec;
	//pCodecCtxOrig = pFormatCtx->streams[videoStream]->codec;

	//���Ҹ���Ƶ����Ӧ�Ľ�����AVCodec
	pCodec = avcodec_find_decoder(pCodecCtxOrig->codec_id);
	if (pCodec == NULL)
	{

		printf("cann't find  decoder ");
		return;

	}

	// Copy context.
	pCodecCtx = avcodec_alloc_context3(pCodec);
	if (avcodec_copy_context(pCodecCtx, pCodecCtxOrig) != 0) {
		printf("Couldn't copy codec context");
		return; // Error copying codec context.
	}



	//�øý������򿪸���Ƶ�ļ�
	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
	{
		printf("avcodec_open2  error !!!\n");
		return;
	}


	//AVFrame  �Ǵ洢��Ƶ���ݵĽṹ
	AVFrame* pframe = NULL;	//pframe  �Ǵ���Ƶ���ж�������ͼƬ���ݷ�������
	AVFrame* pframeRGB = NULL;   //pframeRGB ��ͨ�� sws_scaleת����ʽ֮����Ҫ��������ݽṹ

	pframe = av_frame_alloc();
	pframeRGB = av_frame_alloc();

	if (pframeRGB == NULL)
	{
		printf("pframeRGB  alloc error !!!\n");
		return;
	}

	//����ռ� ͨ���õ�PIX_FMT_RGB24��һ��������ռ�õ��ڴ�
	uint8_t* out_buffer = (uint8_t*)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_RGB24, pCodecCtx->width,
		pCodecCtx->height, 1));

	//��out_buffer ��pframeRGB����ʽ��������
	av_image_fill_arrays(pframeRGB->data, pframeRGB->linesize, out_buffer, AV_PIX_FMT_RGB24,
		pCodecCtx->width, pCodecCtx->height, 1);



	//ת��������  

	AVPacket packet;
	int got_picture;
	struct  SwsContext* sws_ctx;

	sws_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
		pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL);





	int  j = 0;

	//cout << "��֡����" << FrameCount << pFormatCtx->streams[i]->nb_frames << endl;
	cout << "֡�ʣ�" << average_fps << endl
		<<"�����"<<ternal<<endl;
	while (av_read_frame(pFormatCtx, &packet) >= 0)			//��pFormatCtxһ֡һ֡��ȡѹ������Ƶ���ݣ����ppacket��
	{
		if (packet.stream_index == videoindex)    //�ҵ���Ƶ��
		{
			//����һ֡��Ƶ����,����һ��ѹ������Ľṹ��ppacket�����һ�������Ľṹ��pframe��
			avcodec_decode_video2(pCodecCtx, pframe, &got_picture, &packet);


			if (got_picture) {
				j++;
				if (j % ternal == 0) {
					cout << "��" << j << "֡" << endl;
					MatFrame = AvFrame_to_CVMat(pframe);
					test(MatFrame);
				}
				
				//if (pframe->key_frame == 1) {			//�ؼ�֡
				//	cout << "�ؼ�֡" << endl;
				//	MatFrame = AvFrame_to_CVMat(pframe);

				//	test(MatFrame);		//�����Ƿ�ɹ�ת��Mat����
				//}
				
			}

		}

		av_packet_unref(&packet);
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
}

int main() {
	const char videofilename[] = "C:\\Users\\������\\Videos\\Captures\\��Խ���� 2019_6_3 15_01_18.mp4";
	char datafilename[] = "D:\\VIDEOtoImage";

	dealWithVideo(6,videofilename, datafilename);
	return 0;
}