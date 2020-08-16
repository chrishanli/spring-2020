

extern void bitmapToMat(cv::Mat& mat, Bitmap* bitmap);

extern int convertPackToFrames(SH_Pack* pack, cv::Mat mat[]);

extern int convertPackToVideo(SH_Pack* pack, const char videoFilename[], int frame_num, int fps, int videoWidth, int videoHeight);
