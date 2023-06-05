#include <iostream>
#include<opencv2/opencv.hpp>

using namespace cv;
using namespace std;

// int main()
// {
//     Mat frame;
//     VideoCapture cap(0);
//
//     if (!cap.isOpened())
//     {
//         cout << "NO capture" << endl;
//         return -1;
//     }
//
//     const string window_name = "Frame Capture";
//     namedWindow(window_name);
//
//     while (true)
//     {
//         cap >> frame;
//         imshow(window_name, frame);
//         const int key = waitKey(10);
//         if (key == 27)
//             break;
//     }
//
//     destroyWindow(window_name);
//     
//     return 0;
// }
