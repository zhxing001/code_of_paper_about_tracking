#include<opencv2/core/core.hpp>
#include<opencv2\highgui\highgui.hpp>
#include<opencv2\/imgproc\imgproc.hpp>
#include<utility>

#define  m_min(a,b)   ((a<b)?a:b) 
#define  m_max(a,b)   ((a>b)?a:b) 

using std::vector;
using cv::Mat;
using std::pair;

vector<pair<double, int>> sort_index(vector<double> &vec);
bool sort_pair(pair<double, int> &a, pair<double, int> &b);

vector<double>  Sum_row(Mat &img);
vector<double>  Sum_col(Mat &img);
cv::Rect get_rec_pos(Mat &img);


cv::Rect get_rec_pos(Mat &img)
{
	vector<Mat>  channels;
	split(img, channels);
	cv::split(img, channels);
	Mat r_y;
	r_y = channels[2] - channels[1];   //红色通道和蓝色通道做差
	auto sum_row = Sum_row(r_y);
	auto sum_col = Sum_col(r_y);
	vector<pair<double, int>> sort_row = sort_index(sum_row);
	vector<pair<double, int>> sort_col = sort_index(sum_col);

	int row_s = m_min((sort_row.end() - 1)->second, (sort_row.end() - 2)->second);
	int row_l = m_max((sort_row.end() - 1)->second, (sort_row.end() - 2)->second);
	int col_s = m_min((sort_col.end() - 1)->second, (sort_col.end() - 2)->second);
	int col_l = m_max((sort_col.end() - 1)->second, (sort_col.end() - 2)->second);
	vector<cv::Point2i> Pos;
	Pos.push_back(cv::Point2i(row_s, col_s));
	Pos.push_back(cv::Point2i(row_l, col_l));

	//注意坐标和行列刚好是相反的
	cv::Rect res(col_s, row_s, col_l - col_s, row_l - row_s);    
	return res;
	
}



vector<pair<double, int>> sort_index(vector<double> &vec)
{
	vector<pair<double, int>> res;
	for (int i = 0; i < vec.size(); i++)
	{
		res.push_back(std::make_pair(vec[i], i));
	}
	sort(res.begin(), res.end(), sort_pair);
	return res;

}

//排序规则
bool sort_pair(pair<double, int> &a, pair<double, int> &b)
{
	if (a.first <= b.first)
		return true;
	else
		return false;
}


vector<double>  Sum_row(Mat &img)
{
	int col_num = img.cols;
	int row_num = img.rows;
	vector<double> sum_row(row_num, 0);

	for (int i = 0; i < row_num; i++)
	{
		for (int j = 0; j < col_num; j++)
		{
			sum_row[i] += img.at<uchar>(i, j);
		}
	}
	return sum_row;
}

vector<double>  Sum_col(Mat &img)
{
	int col_num = img.cols;
	int row_num = img.rows;
	vector<double> sum_col(col_num, 0);

	for (int i = 0; i < col_num; i++)
	{
		for (int j = 0; j < row_num; j++)
		{
			sum_col[i] += img.at<uchar>(j, i);
		}
	}
	return sum_col;
}