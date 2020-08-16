#include "EdgeDetection.h"

EdgeDetection::EdgeDetection(cv::Mat image)
{
    m_img = image;
}

bool EdgeDetection::cannyProcess(unsigned int downThreshold, unsigned int upThreshold)
{
    bool ret = true;

    if (m_img.empty())
    {
        ret = false;
    }

    cv::Canny(m_img, m_canny, downThreshold, upThreshold);
    //cv::imshow("Canny", m_canny);

    return ret;
}

bool EdgeDetection::getContours()
{
    bool ret = true;
    if (m_canny.empty())
    {
        ret = false;
    }
    //返回指定形状和尺寸的结构元素。
    cv::Mat k = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3), cv::Point(-1, -1));
    cv::dilate(m_canny, m_canny, k);
    //imshow("dilate", m_canny);

    // 轮廓发现与绘制
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    findContours(m_canny, contours, cv::RETR_EXTERNAL, CHAIN_APPROX_SIMPLE, Point());

    vector<vector<Point>> conpoint(contours.size());
    vector<Rect> boundRect(contours.size());
    for (size_t i = 0; i < contours.size(); ++i)
    {
    
        // 最小外接轮廓
        RotatedRect rrt = minAreaRect(contours[i]);
        Point2f pts[4];

        rrt.points(pts);

        for (int i = 0; i < 4; i++) {
            if (i % 4 == 0) {
                pts[i].x += 10;
                pts[i].y += 10;
            }
            else if (i % 4 == 1) {
                pts[i].x -= 10;
                pts[i].y += 10;
            }
            else if (i % 4 == 2) {
                pts[i].x -= 10;
                pts[i].y -= 10;
            }
            else if (i % 4 == 3) {
                pts[i].x += 10;
                pts[i].y -= 10;
            }
        }
       

        Point2f dstPoints[4];

        dstPoints[0] = Point2f(m_img.cols, m_img.rows);
        dstPoints[1] = Point2f(0, m_img.rows);
        dstPoints[2] = Point2f(0, 0);
        dstPoints[3] = Point2f(m_img.cols, 0);
        Mat warpPerspective_mat(3, 3, CV_32FC1);
        Mat warpPerspective_dst = Mat::zeros(m_img.rows, m_img.cols, m_img.type());
        warpPerspective_mat = getPerspectiveTransform(pts, dstPoints);
        warpPerspective(m_img, warpPerspective_dst, warpPerspective_mat, m_img.size());
        imshow("变换后：", warpPerspective_dst);
        Mat dst;
        warpPerspective_dst.copyTo(dst);
        
        //int转换为string：  
        std::stringstream ss;
        std::string str;
        ss << i;
        ss >> str;

        imwrite(FilePath + str + "result.jpg", dst);
        
    }





    imshow("contours", m_img);
    return ret;
}

EdgeDetection::~EdgeDetection()
{
}