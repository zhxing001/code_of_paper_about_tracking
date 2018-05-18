/*******************************************************************************
* Created by Qiang Wang on 16/7.24
* Copyright 2016 Qiang Wang.  [wangqiang2015-at-ia.ac.cn]
* Licensed under the Simplified BSD License
*******************************************************************************/

#ifndef SRC_KCF_HPP_
#define SRC_KCF_HPP_

#define PI 3.141592653589793

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <vector>
#include <string>
#include <cmath>
#include "fhog.hpp"

using namespace cv;
using namespace std;

class KCF
{
public:
  KCF(std::string kernel_type = "gaussian", std::string feature_type = "hog");   //构造函数，带默认参数
  void Init(cv::Mat image, cv::Rect rect_init);    //初始化函数
  cv::Rect Update(cv::Mat image);           //主要调用函数
  cv::Rect Update1(cv::Mat image);
  Point getPos()    //把pos也返回去，我用一下画下中心点
  {
	  return pos_;
  }
  cv::Rect getRec()
  {
	  return result_rect_;
  }
  int Num = 0;
  ofstream maxLoctxt;
protected:
  void Learn(cv::Mat &patch, float lr);         

  cv::Mat CreateGaussian1D(int n, float sigma);         //一维高斯

  cv::Mat CreateGaussian2D(cv::Size sz, float sigma);     //二维高斯
   
  cv::Mat GaussianShapedLabels(float sigma, cv::Size sz);      //高斯标签，y

  cv::Mat CalculateHann(cv::Size sz);           //汉明窗

  cv::Mat GetSubwindow(const cv::Mat &frame, cv::Point centraCoor, cv::Size sz);
                                                //子窗口
  std::vector<cv::Mat> GetFeatures(cv::Mat patch);    //获取个证

  cv::Mat GaussianCorrelation(std::vector<cv::Mat> xf, std::vector<cv::Mat> yf);   //高斯核

  cv::Mat PolynomialCorrelation(std::vector<cv::Mat> xf, std::vector<cv::Mat> yf);   //多项式核

  cv::Mat LinearCorrelation(std::vector<cv::Mat> xf, std::vector<cv::Mat> yf);  //线性核

  cv::Mat ComplexMul(const cv::Mat &x1, const cv::Mat &x2);    //复数乘法（element_wise）

  cv::Mat ComplexDiv(const cv::Mat &x1, const cv::Mat &x2);    //复数除法(element_wise)
 

  inline cv::Size FloorSizeScale(cv::Size sz, double scale_factor) {
            if (scale_factor > 0.9999 && scale_factor < 1.0001)
              return sz;
           return cv::Size(cvFloor(sz.width * scale_factor), 
                           cvFloor(sz.height * scale_factor));
         }

  inline cv::Point FloorPointScale(cv::Point p, double scale_factor) {
           if (scale_factor > 0.9999 && scale_factor < 1.0001)
              return p; 
           return cv::Point(cvFloor(p.x * scale_factor), 
                            cvFloor(p.y * scale_factor));
	 }
 //上面这两个像是在更新初读

private:
  float padding_ = 2;    
  float lambda_ = 1e-4;
  float output_sigma_factor_ = 0.1;          
  bool features_hog_ = false;   
  int features_hog_orientations_ = 9;    //9个方向
  bool features_gray_ = false;          
  float kernel_sigma_ = 0.2;         //核的sigma
  int kernel_poly_a_ = 1;
  int kernel_poly_b_ = 7;             //多项式系数
  std::string kernel_type_;           //核种类
  int cell_size_ = 1;                  
  float interp_factor_ = 0.075;       //插值系数
  cv::Rect result_rect_;              //这个是存矩形框的吧，左上角坐标外加长和宽，具体看类定义，
									//update返回的就是这个参数
  cv::Point pos_;
  cv::Size target_sz_;
  bool resize_image_ = false;
  cv::Size window_sz_;
  cv::Mat yf_;
  cv::Mat cos_window_;
  std::vector<cv::Mat> model_xf_;
  cv::Mat model_alphaf_;
  FHoG f_hog_;
};

#endif /* SRC_KCF_HPP_ */
