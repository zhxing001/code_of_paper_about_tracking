/*******************************************************************************
* Created by Qiang Wang on 16/7.24
* Copyright 2016 Qiang Wang.  [wangqiang2015-at-ia.ac.cn]
* Licensed under the Simplified BSD License
*******************************************************************************/



#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>

#include "kcf.hpp"
#include "getROI_Mouse.h"
#include "get_rec_pos.h"       //和别人的比较的一个函数，检测矩形框的
#include "TemplateTracking.h"

using namespace cv;
using namespace std;

std::vector<cv::Rect>   GetGroundtruth(std::string txt_file);
std::vector<double>   PrecisionCalculate(std::vector<cv::Rect> groundtruth_rect, 
				      std::vector<cv::Rect> result_rect);



cv::Rect split_line(string &line);   //分离字符串函数声明
									 //读取groundtruth信息



vector<cv::Rect> read_groundtruth(const string &groundtruth_txt, int &num_of_line)    //const常量才可以由字符串隐式转换
{
	vector<cv::Rect> groundtruth;            // vector<rect>  用来存groundtruth
	ifstream groundtruth_file;               // 文件对象
	groundtruth_file.open(groundtruth_txt);      //打开txt文件
	string line;         //当前行
	Rect rect_tmp;       //每一行搞成一个rect
	while (getline(groundtruth_file, line))
	{
		rect_tmp = split_line(line);    //分解字符串为RECT
		groundtruth.push_back(rect_tmp);    //压入vector
		num_of_line++;
	}
	groundtruth_file.close();
	return groundtruth;
}

cv::Rect split_line(string &line)
{
	double pos[8];            //八个点
	int index = 0;              //点的索引
	string tmp;               //暂存的string，来转换为double
	tmp.clear();              //清零
	for (auto l : line)          //遍历字符串，这里面是一个比较简单的字符串根据特定字符分离的一个算法
	{
		if (l == ',')
		{
			pos[index] = stod(tmp);
			index++;
			tmp.clear();     //一定要记得清零
		}
		else
		{
			tmp += l;
		}
	}
	pos[index] = stod(tmp);    //处理最后一个
							   //四个点，对应矩形的四个点

							   /* 我后来发现标注的点并不是遵循这样的规律，不一定一开始是左上角的点，这取决于当时标注的
							   人先从哪个点开始点的，所以应该来使用坐标之间的大小关系来确定到底是哪个点
							   cv::Point2f up_left(pos[0],pos[1]);
							   cv::Point2f up_right(pos[2],pos[3]);
							   cv::Point2f down_right(pos[4],pos[5]);
							   cv::Point2f down_left(pos[6],pos[7]);
							   //cout<<up_left<<" "<<up_right<<" "<<down_right<<" "<<down_left<<endl;


							   int x=round(up_left.x);
							   int y=round(up_left.y);
							   int weidth=round(down_right.x-up_left.x);
							   int height=round(down_right.y-up_left.y);
							   cv::Rect res(x,y,weidth,height);
							   */

	double xmin = min(pos[0], min(min(pos[2], pos[4]), pos[6]));
	double ymin = min(pos[1], min(min(pos[3], pos[5]), pos[7]));
	double xmax = max(pos[0], max(max(pos[2], pos[4]), pos[6]));
	double ymax = max(pos[1], max(max(pos[3], pos[5]), pos[7]));

	cv::Rect res(xmin, ymin, xmax - xmin, ymax - ymin);
	//cout<<res<<endl;

	return res;
}

// 读取list列表的信息
vector<string> read_list(const string &list_name)
{
	vector<string> list_mes;
	ifstream list(list_name);
	string line;
	while (getline(list, line))   //读取list列表信息
	{
		//cout<<line<<endl;
		list_mes.push_back(line);
	}
	list.close();
	return list_mes;
}


void setROI_4(cv::Rect &rect)
{
	if (rect.width % 4 == 1)   rect.width -= 1;
	else if (rect.width % 4 == 2) { rect.x -= 1; rect.width += 2; }
	else if (rect.width % 4 == 3)   rect.width += 1;

	if (rect.height % 4 == 1)   rect.height -= 1;
	else if (rect.height % 4 == 2) { rect.y -= 1; rect.height += 2; }
	else if (rect.height % 4 == 3)   rect.height += 1;
}




int main()
{
	printf("this is a kcf test code!!!\n");
	vector<string> list = read_list("C:\\Users\\zhxing\\Desktop\\code_of_paper_about_tracking//vot2015//list.txt");
	for (int i = 0; i < list.size(); i++)
	{

		cout << "this is the VOT tracking test!!" << endl;
		cout << "and this is " << list[i] << endl;

		//保存跟踪结果
		ofstream res_ground("results//" + list[i] + "_res_ground.txt");
		ofstream res_kcf("results//" + list[i] + "_res_kcf.txt");
		ofstream res_kcf_inter("results//" + list[i] + "_res_kcf_interpolation.txt");
		ofstream ave_fps("results//" + list[i] + "_avefps.txt");

		//表头信息
		res_ground<< "frame\tx\ty\twidth\theight\n";
		res_kcf << "frame\tx\ty\twidth\theight\n";
		res_kcf_inter << "frame\tx\ty\twidth\theight\n";
		ave_fps<< "frame\tave_fps\n";

		string path = "C:\\Users\\zhxing\\Desktop\\code_of_paper_about_tracking//vot2015//" + list[i] + "//";
		
		int num_of_line = 0;

		//读取groundtruth信息
		vector<cv::Rect> groundtruth = read_groundtruth(path + "groundtruth.txt", num_of_line);


		// ground_truth信息存起来
		int index = 1;
		for (auto gg : groundtruth)
		{
			setROI_4(gg);
			res_ground << index++ << "\t" << gg.x << "\t" << gg.y << "\t" << gg.width << "\t" << gg.height << "\n";
		}
		res_ground.close();      //关闭txt文件


	    //跟踪结果保存的vector
		vector<cv::Rect> track_res;
		setROI_4(groundtruth[0]);
		cout <<"groundtruth[0]\t" <<groundtruth[0] << endl;

		track_res.push_back(groundtruth[0]);

		//读取第一张图片信息
		string zeros8 = "00000000";
		cv::Mat img = imread(path + "00000001.jpg");
		cout << "the first one image_size\t" << img.size() << endl;
		cout << "the first roi size is\t" << groundtruth[0] << endl;
		imshow("img", img);
		double all_time = 0;


		std::string kernel_type = "gaussian";  //gaussian polynomial linear  用什么核,可选多项式和高斯，如果是其他的就不用核函数
		std::string feature_type = "hog";      //hog gray fhog             用什么特征,这里hog默认用的是fhog

		// 高斯核，hog特征，i是插值的，不带i是不插值的。
		KCF kcf_tracker_i(kernel_type, feature_type);    //这是写好的一个类。
		KCF kcf_tracker(kernel_type, feature_type);    //这是写好的一个类。


		kcf_tracker_i.Init(img, groundtruth[0]);
		kcf_tracker.Init(img, groundtruth[0]);

		double start, end;
		Rect Rect_kcf_i, Rect_kcf;
		
		for (int j = 2; j < num_of_line; j++)
		{
			cout << "frame\t" << j << endl;
			string img_name = zeros8 + std::to_string(j);
			string img_path = path + string(img_name.end() - 8, img_name.end()) + ".jpg";
			cv::Mat frame = imread(img_path);      //读取当前的照片
			start= static_cast<double>(getTickCount());
			Rect_kcf = kcf_tracker.Update1(frame);
			Rect_kcf_i = kcf_tracker_i.Update(frame); 
			rectangle(frame, Rect_kcf, Scalar(0, 0, 255));
			rectangle(frame, Rect_kcf_i, Scalar(0, 255, 0));

			imshow("test", frame);
			waitKey(10);
		}

		destroyAllWindows();

	
		







	}

	//
	//


	////测试用
	////for (unsigned i = 0; i < video_num;i++)
	////{
	//   // video.read(frame);
	//   // imshow("视频", frame);
	//   // waitKey(10);
	////}
	//ofstream pos("pos_kcf2.txt");       //存结果
	//ofstream pos_tt("pos_tempTrack.txt");
	////表头
	////pos << "x" << "\t" << "y" << "\t" << "width"
	////	<< "\t" << "height" << "\t" << "center.x" << "\t " << "center.y" << "\n";
	//Rect TemTrackingRes;
	//Mat model;
	//Rect TrackBox = groundtruth_rect[0];

	//for (unsigned frame = 0; frame < video_num; ++frame)   //遍历图片序列
	//{


	//	video.read(image);
	//	/*Mat img_cut;
	//	img_cut = image(Range(215, 380),Range(175, 460));
	//	imwrite(".\\截图\\" + to_string(frame) + ".jpg", img_cut);*/
	//	//image = cv::imread(image_files[frame]);
	//	//imwrite("test.jpg", image);
	//	tic = getTickCount();
	//	
	//		if (frame == 0)
	//		{    //第一帧直接初始化并把
	//			cv::String frame1 = "frame1";
	//			imshow(frame1, image);
	//			
	//			Mat img_gray;
	//			cvtColor(image, img_gray, CV_RGB2GRAY);
	//			
	//			
	//			//model = img_gray(TrackBox);       //获取模板
	//			//imshow("model", model);
	//		

	//			//groundtruth_rect[0] = get_rec_pos(image);
	//			//groundtruth_rect[0] = GetRoi(frame1, image);

	//			//这是我新加的一个函数，选择结束时按ESC,会返回一个REC的对象可以初始化跟踪器的初始位置
	//			//按理说封装成一个类比较好，但是没有封装成功，主要原因是鼠标回掉函数不能作为类成员函数，声名
	//			//成友元也不能访问成员（按理说是可以访问的），着急这测试效果，（在这个项目中不封装也不是很影响）
	//			//所以就先写成头文件了。
	//			cout << "跟踪框大小" << groundtruth_rect[0] << endl;
	//			kcf_tracker.Init(image, groundtruth_rect[0]);
	//			kcf_trackerG.Init(image, groundtruth_rect[0]);
	//			result_rect.push_back(groundtruth_rect[0]);     //0-index
	//			result_rectG.push_back(groundtruth_rect[0]);
	//		}
	//		//result_rect存的是程序跟踪到的位置
	//		else
	//		{
	//			//tracking(image, model, TrackBox);
	//			//imshow("model", model);
	//			result_rect.push_back(kcf_tracker.Update(image));
	//			result_rectG.push_back(kcf_trackerG.Update1(image));
	//		}
	//		
	//		if (show_visualization) {
	//			cv::putText(image, to_string(frame + 1), cv::Point(20, 40), 6, 1,
	//				cv::Scalar(0, 255, 255), 2);
	//			//cv::rectangle(image, groundtruth_rect[frame], cv::Scalar(0, 255, 0), 2);
	//			cv::rectangle(image, TrackBox, cv::Scalar(0, 255, 0), 1);
	//			//cv::circle(image, Point(TrackBox.x + 0.5*TrackBox.width, TrackBox.y + 0.5*TrackBox.height), 1, Scalar(0, 255, 0));
	//			cv::rectangle(image, result_rect[frame], cv::Scalar(0, 0, 255), 1);
	//			cv::rectangle(image, result_rectG[frame], cv::Scalar(0, 255,255), 1);
	//			//cv::circle(image, kcf_tracker.getPos(), 3, Scalar(0, 0, 255), -1);  //把目标的中心点画出来
	//			cv::imshow("tracking", image);

	//			//imwrite(to_string(frame) + ".jpg", image);
	//			//结果存入txt
	//			//Rect Pos_kcf = kcf_tracker.getRec();
	//			//pos << Pos_kcf.x +0.5*Pos_kcf.width<< "\t" << Pos_kcf.y+0.5*Pos_kcf.height << "\n";
	//			pos_tt << TrackBox.x + 0.5*TrackBox.width << "\t" << TrackBox.y + 0.5*TrackBox.height << "\n";
	//			//	<< Pos.height << "\t" << Pos.x + Pos.width / 2 << "\t" << Pos.y + Pos.height / 2 << "\n";
	//			
	//			cv::imwrite(".\\image1\\" + std::to_string(frame) + ".jpg", image);
	//			char key = cv::waitKey(1);
	//			if (key == 27 || key == 'q' || key == 'Q')
	//				break;
	//			

	//	}
	//}
	//	time = time / double(getTickFrequency());
	//	double fps = double(result_rect.size()) / time;
	//	cout << "帧率: ----------" << fps << endl;
	//	/*std::vector<double> precisions = PrecisionCalculate(groundtruth_rect,
	//														result_rect);
	//	std::cout<<"%12s - Precision (20px) : %1.3f, FPS : %4.2f\n"<<
	//		   video_base_path.c_str()<<" "<<precisions[20]<<"  "<< fps<<endl;*/
	//	cv::destroyAllWindows();

	
		
	
	return 0;
}

std::vector<cv::Rect> GetGroundtruth(std::string txt_file) 
{
  std::vector<cv::Rect> rect;
  ifstream gt;
  gt.open(txt_file.c_str());
  if (!gt.is_open())
    std::cout << "Ground truth file " << txt_file 
              << " can not be read" << std::endl;
  std::string line;
  int tmp1, tmp2, tmp3, tmp4;
  while (getline( gt, line)) {
    std::replace(line.begin(), line.end(), ',', ' ');
    stringstream ss;
    ss.str(line);
    ss >> tmp1 >> tmp2 >> tmp3 >> tmp4;
    rect.push_back( cv::Rect(--tmp1, --tmp2, tmp3, tmp4) ); //0-index
  }
  gt.close();
  return rect;
}


//这个是计算准确率的
std::vector<double> PrecisionCalculate(std::vector<cv::Rect>groundtruth_rect,
                                       std::vector<cv::Rect>result_rect) {
  int max_threshold = 50;
  std::vector<double> precisions(max_threshold + 1, 0);
  if (groundtruth_rect.size() != result_rect.size()) {
    int n = min(groundtruth_rect.size(), result_rect.size());
    groundtruth_rect.erase(groundtruth_rect.begin()+n, groundtruth_rect.end());
    result_rect.erase(result_rect.begin() + n, result_rect.end());
  }
  std::vector<double> distances;
  double distemp;
  for (unsigned int i = 0; i < result_rect.size(); ++i) {
    distemp = sqrt(double(pow((result_rect[i].x + result_rect[i].width / 2) -
              (groundtruth_rect[i].x + groundtruth_rect[i].width / 2), 2) +
                          pow((result_rect[i].y + result_rect[i].height / 2) -
              (groundtruth_rect[i].y + groundtruth_rect[i].height / 2), 2)));
    distances.push_back(distemp);
  }
  for (int i = 0; i <= max_threshold; ++i) {
    for (unsigned int j = 0; j < distances.size(); ++j) {
      if (distances[j] < double(i))
        precisions[i]++;
    }
    precisions[i] = precisions[i] / distances.size();
  }
  return precisions;
}

