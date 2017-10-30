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
using namespace cv;
using namespace std;

std::vector<cv::Rect>   GetGroundtruth(std::string txt_file);
std::vector<double>   PrecisionCalculate(std::vector<cv::Rect> groundtruth_rect, 
				      std::vector<cv::Rect> result_rect);

int main(/*int argc, char **argv*/) 
{


	//这个是我自己测试用的
	//Mat patch = imread("zx.jpg");
	//imshow("test ", patch);
	//FHoG fhog;
	//std::vector<Mat> x_vector;
	//cv::cvtColor(patch, patch, CV_BGR2GRAY);    //灰度化
	//patch.convertTo(patch, CV_32FC1, 1.0 / 255);  //归一化

	//x_vector = fhog.extract(patch,1);   //算
	//cout << "原图大小" <<patch.size()<< endl;
	//cout <<"特征维数"<< x_vector.size() << endl;
	//string x = "hog";
	//int i = 1;
	////cout << x_vector[0] << endl;
	//for (auto m : x_vector)
	//{
	//	
	//	cout << m.size() << endl;
	//	namedWindow(x + std::to_string(i),2);
	//	cout << x + std::to_string(i);
	//	imshow(x+std::to_string(i), m);
	//	i++;
	//}
	//
	//	
	//waitKey(0);
	//return 0;
	//以上是我测试用的


  //if (argc != 3) 
  //{
  //  std::cout << "Usage:" 
  //            << argv[0] << " video_base_path[./girl] Verbose[0/1]" << std::endl;
  //  return 0;
  //}
 /* cout << argv[1] << endl;*/
  
  
  //读图片的话用这个
  std::string video_base_path;
  video_base_path ="C:/Users/zhxing/Desktop/论文代码/KCF_WQ/KCF_WQ/david";
  std::string pattern_jpg = video_base_path+ "/image/*.jpg";
  cout << pattern_jpg << endl;
  std::string pattern_png = video_base_path+ "/image/*.png";
  std::vector<cv::String> image_files;
  cv::glob(pattern_jpg, image_files);
 
   //测试用

   //以上，测试用

  if (image_files.size() == 0)
    cv::glob(pattern_png, image_files);
  if (image_files.size() == 0) 
  {
    std::cout << "No image files[jpg png]" << std::endl;
    return 0;
  }
  std::string txt_base_path = video_base_path + "/groundtruth.txt";

  
  std::vector<cv::Rect> groundtruth_rect;
  groundtruth_rect = GetGroundtruth(txt_base_path);
 
//  for (auto i : groundtruth_rect)
//	  cout << i << endl;
//这里读取的gt信息根本就不对，我也不要这个了，我自己给吧

  groundtruth_rect[0] = Rect(330, 260, 85, 50);
  cv::Mat image;
  std::vector<cv::Rect> result_rect;
  int64 tic, toc;      //计算时间用的
  double time = 0;     
  bool show_visualization =true;

  std::string kernel_type = "gaussian";  //gaussian polynomial linear  用什么核
  std::string feature_type = "hog";      //hog gray fhog             用什么特征

  KCF kcf_tracker(kernel_type, feature_type);    //这是写好的一个类。
  cout << "test1" << endl;
  
  //读视频的话用这个可以。
  VideoCapture video("C:\\Users\\zhxing\\Desktop\\论文代码\\KCF_WQ\\KCF_WQ\\dat1.avi");
  Mat frame;
  unsigned video_num= video.get(CV_CAP_PROP_FRAME_COUNT);
  cout << "总帧数: "<<video_num << endl;
  
  
  //测试用
  //for (unsigned i = 0; i < video_num;i++)
  //{
	 // video.read(frame);
	 // imshow("视频", frame);
	 // waitKey(10);
  //}

  
  for (unsigned  frame = 0; frame < video_num; ++frame)   //遍历图片序列
  {
	  
	  video.read(image);
	  //image = cv::imread(image_files[frame]);
	 //imwrite("test.jpg",image)
	  tic = getTickCount();
      if (frame == 0) 
	  {    //第一帧直接初始化并把
		  cv::String frame1 = "frame1";
		  imshow(frame1, image);
		  groundtruth_rect[0]= GetRoi(frame1, image);   
		  //这是我新加的一个函数，选择结束时按ESC,会返回一个REC的对象可以初始化跟踪器的初始位置
		  //按理说封装成一个类比较好，但是没有封装成功，主要原因是鼠标回掉函数不能作为类成员函数，声名
		  //成友元也不能访问成员（按理说是可以访问的），着急这测试效果，（在这个项目中不封装也不是很影响）
		  //所以就先写成头文件了。
		  cout << "跟踪尺寸:  " << groundtruth_rect[0] << endl;
        kcf_tracker.Init(image, groundtruth_rect[0]);
        result_rect.push_back(groundtruth_rect[0]);     //0-index
      }
	  //result_rect存的是程序跟踪到的位置
	  else 
	  {
        result_rect.push_back(kcf_tracker.Update(image));
      }
      toc = cv::getTickCount() - tic;
      time += toc;

      if (show_visualization) {
        cv::putText(image, to_string(frame+1), cv::Point(20, 40), 6, 1,
                    cv::Scalar(0, 255, 255), 2);
        //cv::rectangle(image, groundtruth_rect[frame], cv::Scalar(0, 255, 0), 2);
        cv::rectangle(image, result_rect[frame], cv::Scalar(0, 255, 255), 2);
		
        cv::imshow(video_base_path, image);
      
        char key = cv::waitKey(1);
        if(key == 27 || key == 'q' || key == 'Q')
          break;
      }
    }
    time = time / double(getTickFrequency());
    double fps = double(result_rect.size()) / time;
	cout << "帧率: ----------" <<fps << endl;
    /*std::vector<double> precisions = PrecisionCalculate(groundtruth_rect,
                                                        result_rect);
    std::cout<<"%12s - Precision (20px) : %1.3f, FPS : %4.2f\n"<<
           video_base_path.c_str()<<" "<<precisions[20]<<"  "<< fps<<endl;*/
    cv::destroyAllWindows();
  
 
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

