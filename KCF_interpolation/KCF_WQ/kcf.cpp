/*******************************************************************************
* Created by Qiang Wang on 16/7.24
* Copyright 2016 Qiang Wang.  [wangqiang2015-at-ia.ac.cn]
* Licensed under the Simplified BSD License
*******************************************************************************/

#include "kcf.hpp"
//这是一个构造函数
KCF::KCF(std::string kernel_type, std::string feature_type) {
  padding_ = 1;
  lambda_ = 1e-4;
  output_sigma_factor_ = 0.1;

  if (strcmp(feature_type.c_str(), "gray") == 0) {
    interp_factor_ = 0.2;

    kernel_sigma_ = 0.2;

    kernel_poly_a_ = 1;
    kernel_poly_b_ = 7;

    features_gray_ = true;
    cell_size_ = 1;
  } else if (strcmp(feature_type.c_str(), "hog") == 0) {
    interp_factor_ = 0.02;

    kernel_sigma_ = 0.5;

    kernel_poly_a_ = 1;
    kernel_poly_b_ = 9;

    features_hog_ = true;
    features_hog_orientations_ = 9;
    cell_size_ = 4;
  }

  kernel_type_ = kernel_type;
}

//初始化函数
void KCF::Init(cv::Mat image, cv::Rect rect_init) {
  result_rect_ = rect_init;

  pos_ = cv::Point(rect_init.x + cvFloor((float)(rect_init.width)/2.),
		           rect_init.y + cvFloor((float)(rect_init.height)/2.));
  target_sz_ = rect_init.size();

  resize_image_ = std::sqrt(target_sz_.area()) >= 100;
  if (resize_image_) {
    pos_ = FloorPointScale(pos_, 0.5);
    target_sz_ = FloorSizeScale(target_sz_, 0.5);
  }

  window_sz_ = FloorSizeScale(target_sz_, 1 + padding_);

  float output_sigma = std::sqrt(float(target_sz_.area())) * output_sigma_factor_ / cell_size_;

  cv::dft(GaussianShapedLabels(output_sigma, FloorSizeScale(window_sz_, 1. / cell_size_)),
		  yf_, DFT_COMPLEX_OUTPUT);

  cos_window_ = CalculateHann(yf_.size());

  cv::Mat patch = GetSubwindow(image, pos_, window_sz_);

  Learn(patch, 1.);
}

//更新函数，返回的是一个Rect
cv::Rect KCF::Update(cv::Mat image) {
  cv::Mat patch = GetSubwindow(image, pos_, window_sz_);    //子窗口
  std::vector<cv::Mat> z = GetFeatures(patch);     //获取窗口内特征，这里根据参数的不同获取的特征也不一样，分别是9维，36维和31维（还没有很懂）
  std::vector<cv::Mat> zf_vector(z.size());      //这么多维特征
  for (unsigned int i = 0; i < z.size(); ++i)
    cv::dft(z[i], zf_vector[i], DFT_COMPLEX_OUTPUT);   //离散傅里叶变换

  /*cout << "特征维数:" << zf_vector.size() <<"     特征尺寸："<<zf_vector[0].size()<< endl;*/
  cv::Mat kzf;
  if (strcmp(kernel_type_.c_str(), "gaussian") == 0)     //kernel_type_.c_str()这个成员函数是把string转换为字符数组
    kzf = GaussianCorrelation(zf_vector, model_xf_);
  else if (strcmp(kernel_type_.c_str(), "polynomial") == 0)
    kzf = PolynomialCorrelation(zf_vector,model_xf_);
  else
    kzf = LinearCorrelation(zf_vector,model_xf_);       
  //这三个函数都是根据不同的定义去获得不同的kzf即核函数，特征的处理主要就在这里进行

  cv::Mat response;
  cv::Mat responseFFT;        //响应图
  responseFFT = ComplexMul(model_alphaf_, kzf);
 


  cv::Mat responseSZ = cv::Mat::zeros(Size(80, 120), responseFFT.type());

  cv::Mat res;
  responseFFT(Rect(0, 0, 10, 15)).copyTo(responseSZ(Rect(0, 0, 10, 15)));
  responseFFT(Rect(10, 0, 10, 15)).copyTo(responseSZ(Rect(70, 0, 10, 15)));
  responseFFT(Rect(0, 15, 10, 15)).copyTo(responseSZ(Rect(0, 105, 10, 15)));
  responseFFT(Rect(10, 15, 10, 15)).copyTo(responseSZ(Rect(70, 105, 10, 15)));
  /*vector<Mat> responseFFTRI;
  split(responseSZ, responseFFTRI);*/
  //cout <<"实部"<< responseFFTRI[0] << endl;
  //cout << "虚部" << responseFFTRI[1] << endl;
  cv::idft(responseSZ, res, cv::DFT_SCALE | cv::DFT_REAL_OUTPUT); // Applying IDFT


  cv::Point maxLoc;

  cv::minMaxLoc(res, NULL, NULL, NULL, &maxLoc);  //只返回最大值的位置就好了


  if ((maxLoc.x + 1) > (res.cols / 2))
	  maxLoc.x = maxLoc.x - res.cols;
  if ((maxLoc.y + 1) > (res.rows / 2))
	  maxLoc.y = maxLoc.y - res.rows;
  //这个没有进行移位操作，所以在这里做，这里画个图就清楚了。

  pos_.x +=  maxLoc.x;
  pos_.y += maxLoc.y;
  //hog特征图是等比例缩小的，缩小的系数就是cell_size这里在得到具体位置的时候要乘回来
  //这里得到的是一个偏移位置即相对位置，相对于上一帧中心的位置，在这里加上就能得到绝对坐标了（原图上的）
  result_rect_.x +=  maxLoc.x;
  result_rect_.y +=  maxLoc.y;

 
  //cv::idft(ComplexMul(model_alphaf_, kzf), response, cv::DFT_SCALE | cv::DFT_REAL_OUTPUT); // Applying IDFT
  //这个就是快速检测的公式，所有的主要循环到这里就结束了，下面是显示的一些东西
  //cout << response.size() << endl;
  //


  //cv::Point maxLoc;
 
  //cv::minMaxLoc(response, NULL, NULL, NULL, &maxLoc);  //只返回最大值的位置就好了
  //
  //maxLoctxt << (maxLoc+Point(1,1)).x << "\t"<< (maxLoc + Point(1, 1)).y<<"\n";

  //if ((maxLoc.x + 1) > (response.cols / 2))
  //  maxLoc.x = maxLoc.x - response.cols;
  //if ((maxLoc.y + 1) > (response.rows / 2))
  //  maxLoc.y = maxLoc.y - response.rows;
  ////这个没有进行移位操作，所以在这里做，这里画个图就清楚了。

  //pos_.x += cell_size_ * maxLoc.x; 
  //pos_.y += cell_size_ * maxLoc.y;
  ////hog特征图是等比例缩小的，缩小的系数就是cell_size这里在得到具体位置的时候要乘回来
  ////这里得到的是一个偏移位置即相对位置，相对于上一帧中心的位置，在这里加上就能得到绝对坐标了（原图上的）
  //result_rect_.x += cell_size_ * maxLoc.x;
  //result_rect_.y += cell_size_ * maxLoc.y;  
 
  patch = GetSubwindow(image, pos_, window_sz_);   //根据新的位置获取训练用的子窗口，因为没有尺度适应性，
  Learn(patch, interp_factor_);   //学习alpha模型

  return result_rect_;
}


cv::Rect KCF::Update1(cv::Mat image) {
	cv::Mat patch = GetSubwindow(image, pos_, window_sz_);    //子窗口
	std::vector<cv::Mat> z = GetFeatures(patch);     //获取窗口内特征，这里根据参数的不同获取的特征也不一样，分别是9维，36维和31维（还没有很懂）
	std::vector<cv::Mat> zf_vector(z.size());      //这么多维特征
	for (unsigned int i = 0; i < z.size(); ++i)
		cv::dft(z[i], zf_vector[i], DFT_COMPLEX_OUTPUT);   //离散傅里叶变换
														   /*cout << "特征维数:" << zf_vector.size() <<"     特征尺寸："<<zf_vector[0].size()<< endl;*/
	cv::Mat kzf;
	if (strcmp(kernel_type_.c_str(), "gaussian") == 0)     //kernel_type_.c_str()这个成员函数是把string转换为字符数组
		kzf = GaussianCorrelation(zf_vector, model_xf_);
	else if (strcmp(kernel_type_.c_str(), "polynomial") == 0)
		kzf = PolynomialCorrelation(zf_vector, model_xf_);
	else
		kzf = LinearCorrelation(zf_vector, model_xf_);
	//这三个函数都是根据不同的定义去获得不同的kzf即核函数，特征的处理主要就在这里进行

	cv::Mat response;        //响应图
	cv::idft(ComplexMul(model_alphaf_, kzf), response, cv::DFT_SCALE | cv::DFT_REAL_OUTPUT); // Applying IDFT
																							 //这个就是快速检测的公式，所有的主要循环到这里就结束了，下面是显示的一些东西

	cv::Point maxLoc;
	cv::minMaxLoc(response, NULL, NULL, NULL, &maxLoc);  //只返回最大值的位置就好了

	if ((maxLoc.x + 1) > (response.cols / 2))
		maxLoc.x = maxLoc.x - response.cols;
	if ((maxLoc.y + 1) > (response.rows / 2))
		maxLoc.y = maxLoc.y - response.rows;
	//这个没有进行移位操作，所以在这里做，这里画个图就清楚了。

	pos_.x += cell_size_ * maxLoc.x;
	pos_.y += cell_size_ * maxLoc.y;
	//hog特征图是等比例缩小的，缩小的系数就是cell_size这里在得到具体位置的时候要乘回来
	//这里得到的是一个偏移位置即相对位置，相对于上一帧中心的位置，在这里加上就能得到绝对坐标了（原图上的）
	result_rect_.x += cell_size_ * maxLoc.x;
	result_rect_.y += cell_size_ * maxLoc.y;

	patch = GetSubwindow(image, pos_, window_sz_);   //根据新的位置获取训练用的子窗口，因为没有尺度适应性，
	Learn(patch, interp_factor_);   //学习alpha模型

	return result_rect_;
}



void KCF::Learn(cv::Mat &patch, float lr) {

  std::vector<cv::Mat> x = GetFeatures(patch);

  std::vector<cv::Mat> xf(x.size());

  for (unsigned int i = 0; i < x.size(); i++)
    cv::dft(x[i], xf[i], DFT_COMPLEX_OUTPUT);

  cv::Mat kf;
  if (strcmp(kernel_type_.c_str(), "gaussian") == 0)
    kf = GaussianCorrelation(xf, xf);
  else if(strcmp(kernel_type_.c_str(), "polynomial") == 0)
    kf = PolynomialCorrelation(xf, xf);
  else
    kf = LinearCorrelation(xf, xf);

  cv::Mat alphaf = ComplexDiv(yf_, kf + cv::Scalar(lambda_, 0));

  if (lr > 0.99) {               //学习率不能高于1，小于1都是好的
    model_alphaf_ = alphaf;
    model_xf_.clear();
    for (unsigned int i = 0; i < xf.size(); ++i)
      model_xf_.push_back(xf[i]);
  } else {
    model_alphaf_ = (1.0 - lr) * model_alphaf_ + lr * alphaf;
    for (unsigned int i = 0; i < xf.size(); ++i)
      model_xf_[i] = (1.0 - lr) * model_xf_[i] + lr * xf[i];
  }
}

cv::Mat KCF::CreateGaussian1D(int n, float sigma) {
  cv::Mat kernel(n, 1, CV_32F);
  float* cf = kernel.ptr<float>();

  double sigmaX = sigma > 0 ? sigma : ((n - 1)*0.5 - 1)*0.3 + 0.8;
  double scale2X = -0.5 / (sigmaX*sigmaX);

  for (int i = 0; i < n; ++i) {
    double x = i - floor(n / 2) + 1;
    double t = std::exp(scale2X * x * x);
    cf[i] = (float)t;
  }

  return kernel;
}

cv::Mat KCF::CreateGaussian2D(cv::Size sz, float sigma) {
  cv::Mat a = CreateGaussian1D(sz.height, sigma);
  cv::Mat b = CreateGaussian1D(sz.width, sigma);
  return a * b.t();
}

void CircShift(cv::Mat &x, cv::Size k) {
  int cx, cy;
  if (k.width < 0)
    cx = -k.width;
  else
    cx = x.cols - k.width;

  if (k.height < 0)
    cy = -k.height;
  else
    cy = x.rows - k.height;

  cv::Mat q0(x, cv::Rect(0, 0, cx, cy));   // Top-Left - Create a ROI per quadrant
  cv::Mat q1(x, cv::Rect(cx, 0, x.cols - cx, cy));  // Top-Right
  cv::Mat q2(x, cv::Rect(0, cy, cx, x.rows -cy));  // Bottom-Left
  cv::Mat q3(x, cv::Rect(cx, cy, x.cols -cx, x.rows-cy)); // Bottom-Right

  cv::Mat tmp1, tmp2;                           // swap quadrants (Top-Left with Bottom-Right)
  cv::hconcat(q3, q2, tmp1);
  cv::hconcat(q1, q0, tmp2);
  cv::vconcat(tmp1, tmp2, x);

}

cv::Mat KCF::GaussianShapedLabels(float sigma, cv::Size sz) {
  cv::Mat labels = CreateGaussian2D(sz, sigma);
  cv::Size shift_temp = cv::Size(-cvFloor(sz.width * (1./2)), -cvFloor(sz.height * (1./2)));
  shift_temp.width += 1;

  shift_temp.height += 1;
  CircShift(labels, shift_temp);
  
  return labels;
}

cv::Mat KCF::CalculateHann(cv::Size sz) {
  cv::Mat temp1(Size(sz.width, 1), CV_32FC1);
  cv::Mat temp2(Size(sz.height, 1), CV_32FC1);
  for (int i = 0; i < sz.width; ++i)
    temp1.at<float>(0, i) = 0.5*(1 - cos(2 * PI * i / (sz.width - 1)));
  for (int i = 0; i < sz.height; ++i)
    temp2.at<float>(0, i) = 0.5*(1 - cos(2 * PI * i / (sz.height - 1)));
  return temp2.t() * temp1;
 
}

cv::Mat KCF::GetSubwindow(const cv::Mat &frame, cv::Point centerCoor, cv::Size sz) 
{   //获取子窗口的函数，给定中心点和尺寸
	//frame 原图   centerCoor 中心点    sz 尺寸
  cv::Mat subWindow;
  cv::Point lefttop(min(frame.cols - 2, max(-sz.width+1, centerCoor.x - cvFloor(float(sz.width) / 2.0) + 1)),
		  	  	  	min(frame.rows - 2, max(-sz.height+1, centerCoor.y - cvFloor(float(sz.height) / 2.0) + 1)));
  //算左上角的坐标要避免出界，这个是保证不出下右下边界，虽然没有很看懂为什么这么设计，但是确实是可以避免左上角出右下界，
  //这个处理实际上是避免超过frame[frame.cols - 2][frame.rows - 2]这个点，这个点是右下的倒数第二个点
  //但是这个也是容易越界的，主要是越过（0，0）这个点
  
  cv::Point rightbottom(lefttop.x + sz.width, lefttop.y + sz.height);
  //右下角坐标,在左上角的基础上加上sz就行了，这个就容易超界了，所以要处理下

  cv::Rect border(-min(lefttop.x, 0), -min(lefttop.y, 0),
		  max(rightbottom.x - (frame.cols - 1), 0), max(rightbottom.y - (frame.rows - 1), 0));
  //这个是算出越界到底越了多少，用在下面的填充
  cv::Point lefttopLimit(max(lefttop.x, 0), max(lefttop.y, 0));    //保证不要大于小于（0，0）点
  cv::Point rightbottomLimit(min(rightbottom.x, frame.cols - 1), min(rightbottom.y, frame.rows - 1));
               //保证不要大于右下角的点
  cv::Rect roiRect(lefttopLimit, rightbottomLimit);  //用Point初始化Rect的时候会自动转换为Rect格式的数据，这里提供的是左上和右下点坐标
  
  frame(roiRect).copyTo(subWindow);       //如果没有越界的话这样直接拷贝

  if (border != cv::Rect(0,0,0,0))      //这里是处理越界的情况,扩大边界，
    cv::copyMakeBorder(subWindow, subWindow, border.y, border.height, border.x, border.width, cv::BORDER_REPLICATE);
  
  return subWindow;      //子窗口
}



std::vector<cv::Mat> KCF::GetFeatures(cv::Mat patch) {
  cv::Mat x;
  std::vector<Mat> x_vector;
  if (features_hog_) {
    if (patch.channels() == 3)
      cv::cvtColor(patch, patch, CV_BGR2GRAY);    //灰度化
    patch.convertTo(patch, CV_32FC1, 1.0 / 255);  //归一化

    x_vector = f_hog_.extract(patch);   //算f_hog，所以这个是重点，相比CSK来说

    for (unsigned int i = 0; i < x_vector.size(); ++i)
      x_vector[i] = x_vector[i].mul(cos_window_);    //点乘，opencv竟然自带了点乘，这个过程是加窗
  }

  if (features_gray_)   //这个是用灰度信息的话，就直接加窗就行了
  {
    if(patch.channels() == 3)
      cv::cvtColor(patch, patch, CV_BGR2GRAY);
    patch.convertTo(x, CV_32FC1, 1.0 / 255);    //归一化
    x = x - cv::mean(x).val[0];   //所有都减去均值，这里把中心移到0位置
    x = x.mul(cos_window_);     //加窗
    x_vector.push_back(x);
  }

  return x_vector;
}


cv::Mat KCF::GaussianCorrelation(std::vector<cv::Mat> xf, std::vector<cv::Mat> yf) {
  int N = xf[0].size().area();   //这就是一个面积，也就是总的像素数
  double xx = 0, yy = 0; 

  std::vector<cv::Mat> xyf_vector(xf.size());
  cv::Mat xy(xf[0].size(), CV_32FC1, Scalar(0.0)), xyf, xy_temp;
  for (unsigned int i = 0; i < xf.size(); ++i)      //特征维数
  {
    xx += cv::norm(xf[i]) * cv::norm(xf[i]) / N;    //这里算的是二范数，是一个double。并除以N
    yy += cv::norm(yf[i]) * cv::norm(yf[i]) / N;
    cv::mulSpectrums(xf[i], yf[i], xyf, 0, true);   //傅里叶域点乘
    cv::idft(xyf, xy_temp, cv::DFT_SCALE | cv::DFT_REAL_OUTPUT);   // Applying IDFT，这里也除以N了，DFT_SCALE就是标志
    xy += xy_temp;
  }   //把所有层的特征都加起来，实际上是在复数域加起来

  float numel_xf = N * xf.size();
  cv::Mat k, kf;
  exp((-1 / (kernel_sigma_ * kernel_sigma_)) * max(0.0, (xx + yy - 2 * xy) / numel_xf), k);
  k.convertTo(k, CV_32FC1);
  cv::dft(k, kf, DFT_COMPLEX_OUTPUT);
  return kf;    //这整个过程和CSK里面的有细微差别，注意一些细节
}

cv::Mat KCF::PolynomialCorrelation(std::vector<cv::Mat> xf, std::vector<cv::Mat> yf) {
  std::vector<cv::Mat> xyf_vector(xf.size());
  cv::Mat xy(xf[0].size(), CV_32FC1, Scalar(0)), xyf, xy_temp;
  for (unsigned int i = 0; i < xf.size(); ++i) {
    cv::mulSpectrums(xf[i], yf[i], xyf, 0, true);
    cv::idft(xyf, xy_temp, cv::DFT_SCALE | cv::DFT_REAL_OUTPUT); // Applying IDFT
    xy += xy_temp;
  }
  float numel_xf = xf[0].size().area() * xf.size();
  cv::Mat k, kf;
  cv::pow(xy / numel_xf + kernel_poly_a_, kernel_poly_b_, k);
  k.convertTo(k, CV_32FC1);
  cv::dft(k, kf, DFT_COMPLEX_OUTPUT);
  return kf;
}

cv::Mat KCF::LinearCorrelation(std::vector<cv::Mat> xf, std::vector<cv::Mat> yf) {
  cv::Mat kf(xf[0].size(), CV_32FC2, cv::Scalar(0)), xyf;
  for (unsigned int i = 0; i < xf.size(); ++i) {
    cv::mulSpectrums(xf[i], yf[i], xyf, 0, true);
    kf += xyf;
  }
  float numel_xf = xf[0].size().area() * xf.size();
  return kf/numel_xf;
}

//这个应该是复数乘法
cv::Mat KCF::ComplexMul(const cv::Mat &x1, const cv::Mat &x2) {
  std::vector<cv::Mat> planes1;
  cv::split(x1, planes1);
  std::vector<cv::Mat> planes2;
  cv::split(x2, planes2);
  //通道分离
  std::vector<cv::Mat> complex(2); //两个mat放在vector容器中    
  complex[0] = planes1[0].mul(planes2[0]) - planes1[1].mul(planes2[1]);
  complex[1] = planes1[0].mul(planes2[1]) + planes1[1].mul(planes2[0]);
  //这其实算的是一个复数乘法 (a+bj)(c+dj)=(ac-bd+j(ad+bc))  就是这么个过程
  Mat result;
  cv::merge(complex, result);
  return result;
}

cv::Mat KCF::ComplexDiv(const cv::Mat &x1, const cv::Mat &x2) {
  std::vector<cv::Mat> planes1;
  cv::split(x1, planes1);
  std::vector<cv::Mat> planes2;
  cv::split(x2, planes2);
  std::vector<cv::Mat>complex(2);
  cv::Mat cc = planes2[0].mul(planes2[0]);
  cv::Mat dd = planes2[1].mul(planes2[1]);

  complex[0] = (planes1[0].mul(planes2[0]) + planes1[1].mul(planes2[1])) / (cc + dd);
  complex[1] = (-planes1[0].mul(planes2[1]) + planes1[1].mul(planes2[0])) / (cc + dd);
  cv::Mat result;
  cv::merge(complex, result);
  return result;
}