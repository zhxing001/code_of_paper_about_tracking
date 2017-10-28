#ifndef GETROI_MOUSE_H
#define GETROI_MOUSE_H
#include <opencv2/opencv.hpp>
#include<opencv2\highgui\highgui.hpp>
using namespace std;
using namespace cv;


//---------------------【函数声明部分】---------------------------------

bool g_bDrawingBox = false;   //是否进行绘制的标志
Rect g_rectange = Rect(-1, -1, 0, 0);

void DrawRectagle(cv::Mat &img, cv::Rect box);
Rect& GetRoi(cv::String &WINDOWNAME, cv::Mat srcImage);
void on_MouseHandle(int event, int x, int y, int flags, void *param);


	//准备工作
	//cv::String TEST = "TEST";
	//Mat Image = imread("zx.jpg");
	//namedWindow(TEST);
	//imshow(TEST, Image);
	//GetRoi(TEST, Image);

Rect& GetRoi(cv::String &WINDOWNAME, cv::Mat srcImage)
{


	setMouseCallback(WINDOWNAME, on_MouseHandle, (void*)&srcImage);
	cv::Mat tempImage;
	//程序主循环
	while (1)            //死循环
	{
		srcImage.copyTo(tempImage);     //复制原图到临时变量
		if (g_bDrawingBox)
			DrawRectagle(srcImage, g_rectange);
		//当进行绘制标识符为真时，进行绘制
		imshow(WINDOWNAME, tempImage);

		if (waitKey(10) == 27)
			break;             //按下ESC时退出
	}
	return g_rectange;
}

void on_MouseHandle(int event, int x, int y, int flags, void *param)
{
	Mat &image = *(cv::Mat*) param;
	switch (event)
	{
	case EVENT_MOUSEMOVE:    //鼠标移动
	{
		if (g_bDrawingBox)      //如果是否要进行绘制的标识符为真，则记录下长和宽到RECT型的变量中
		{
			g_rectange.width = x - g_rectange.x;
			g_rectange.height = y - g_rectange.y;
		}

	}break;

	case EVENT_LBUTTONDOWN:      //左键按下时
	{ g_bDrawingBox = true;
	g_rectange = Rect(x, y, 0, 0);  //记录起点
	}
	break;
	case EVENT_LBUTTONUP:        //左键抬起时
	{
		g_bDrawingBox = false;
		//置位标识符为FALSE
		//对高和宽小于零的处理

		if (g_rectange.width < 0)
		{
			g_rectange.x += g_rectange.width;
			g_rectange.width *= -1;
		}
		if (g_rectange.height < 0)
		{
			g_rectange.y += g_rectange.height;
			g_rectange.height *= -1;
		}
		DrawRectagle(image, g_rectange);
	}
	break;
	}
}


//功能：自定义的矩形绘制函数，调用库函数reetangle


void DrawRectagle(cv::Mat &img, cv::Rect box)
{
	rectangle(img, box.tl(), box.br(), Scalar(0, 255, 255));
	//绿色
}

#endif