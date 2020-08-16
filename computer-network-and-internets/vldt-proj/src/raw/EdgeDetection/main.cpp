#include"EdgeDetection.h"
using namespace std;
using namespace cv;



int main(int argc, char* argv[])
{
    
    Mat src = imread("C:\\Users\\40962\\Pictures\\Ê¾ÀýÍ¼Æ¬.png");
    if (src.empty())
    {
        cout << "image is empty" << endl;
        return -1;
    }

    imshow("input", src);
    EdgeDetection ed(src);
    ed.cannyProcess(80, 160);
    ed.getContours();

    waitKey(0);
    return 0;
}