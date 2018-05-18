#include<opencv2/core/core.hpp>
#include<opencv2\highgui\highgui.hpp>
#include<opencv2\/imgproc\imgproc.hpp>
#include<iostream>
#include<string>
#include<fstream>
#include"get_rec_pos.h"
using namespace std;
using namespace cv;





using std::vector;
using cv::Mat;




void Image_To_Video(int frame_start, int frame_end, VideoCapture video);
int main()
{

	VideoCapture video("tracking.mp4");
	Mat img;
	ofstream pos("pos.txt");
	//表头
	pos << "x"<< "\t" << "y" << "\t" << "width" 
		<< "\t" << "height" <<"\t" <<"center.x"<<"\t "<<"center.y"<<"\n";
	
	for (unsigned i = 0; i < video.get(CAP_PROP_FRAME_COUNT); i++)
	{
		video.read(img);
		Rect Pos = get_rec_pos(img);
		rectangle(img, Pos, Scalar(255, 0, 0));
		imshow("test", img);
		waitKey(10);
		cout << i << endl;
		pos << Pos.x << "\t" << Pos.y << "\t" << Pos.width << "\t" 
			<< Pos.height << "\t" << Pos.x+Pos.width/2 <<"\t"<<Pos.y+Pos.height/2<< "\n";
		
	}
	cout << "done" <<"\a"<< endl;
	return 0;
}

void Image_To_Video(int frame_start,int frame_end,VideoCapture video)   //输入应该是起始和结束帧以及原视频对象
{
	string video_name = "data_cut1.avi";   //视频名字
	int hight = video.get(CAP_PROP_FRAME_HEIGHT);   //宽和高保持不变
	int wed = video.get(CAP_PROP_FRAME_WIDTH);
	VideoWriter writer;             //
	writer = VideoWriter(video_name, -1, video.get(CAP_PROP_FPS), Size(wed,hight), 0);
	//写入的对象，保持原来的帧率和大小。
	//writer.open(video_name,-1, video.get(CAP_PROP_FPS), Size(wed, hight), true);  
	Mat image;
	for (unsigned i = 0; i < video.get(CAP_PROP_FRAME_COUNT); i++)
	{
		cout << i << endl;
		video.read(image);
		imshow(" ", image);
		waitKey(20);
		if (i >= frame_start&&i < frame_end)
		{
			/*imshow("test", image);
			waitKey(10);*/
			writer.write(image);
		}
		if (i == frame_end)
		{
			cout << "transform task was done!" << endl;
			break;
		}
			
	}
	
}



