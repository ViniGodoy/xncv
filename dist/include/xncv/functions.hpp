/******************************************************************************
*
* COPYRIGHT Vinícius G. Mendonça ALL RIGHTS RESERVED.
*
* This software cannot be copied, stored, distributed without
* Vinícius G.Mendonça prior authorization.
*
* This file was made available on https://github.com/ViniGodoy/xncv and it
* is free to be restributed or used under Creative Commons license 2.5 br:
* http://creativecommons.org/licenses/by-sa/2.5/br/
*
*******************************************************************************/

#if !defined(__FUNCTIONS_HPP__)
#define __FUNCTIONS_HPP__

#include <xnCppWrapper.h>
#include <opencv2\core\core.hpp>

namespace xncv
{	
	//Image functions
	cv::Mat captureRGB(const xn::ImageGenerator& generator);
	cv::Mat captureBGR(const xn::ImageGenerator& generator);
	 
	//Depth functions
	cv::Mat captureDepth(const xn::DepthGenerator& generator);	
	cv::Mat cvtDepthTo8UDist(const cv::Mat &mat, int zRes=0);
	cv::Mat cvtDepthTo8UHist(const cv::Mat &mat, const cv::Mat& hist);

	//Histogram functions
	cv::Mat calcDepthHist(const cv::Mat& depth, const xn::DepthGenerator& generator);
	cv::Mat histogramImage(const cv::Mat& histogram, ushort height=640, bool cropRight=false, bool cropLeft=false);

	cv::Point worldToProjective(const XnPoint3D& point, const xn::DepthGenerator& depth);
	XnPoint3D projectiveToWorld(const cv::Point& point, XnFloat z, const xn::DepthGenerator& depth);
	std::ostream& operator<<(std::ostream& output, const XnPoint3D& p);

	//Fast iterators
	template <typename T, typename Function>
	void forEach(cv::Mat& mat, Function f, bool continuosOptimization = true)
	{
		int rows = mat.isContinuous() && continuosOptimization ? 1 : mat.rows;
		int cols = mat.isContinuous() && continuosOptimization ? mat.rows*mat.cols : mat.cols;

		for(cv::Point p(0,0); p.y < rows; ++p.y)
		{			
			T* row = mat.ptr<T>(p.y);
			for(p.x = 0; p.x < cols; ++p.x)
				f(const_cast<const cv::Point&>(p), row[p.x]);
		}
	}

	template <typename T, typename Function>
	void forEach(const cv::Mat& mat, Function f, bool continuosOptimization = true)
	{			 
		int rows = mat.isContinuous() && continuosOptimization ? 1 : mat.rows;
		int cols = mat.isContinuous() && continuosOptimization ? mat.rows*mat.cols : mat.cols;

		for(cv::Point p(0,0); p.y < rows; ++p.y)
		{			
			const T* elem = mat.ptr<T>(p.y);
			for(p.x = 0; p.x < cols; ++p.x)
				f(const_cast<const cv::Point&>(p), elem[p.x]);
		}
	}

	//Other functions
	template <typename T>
	T otsu(cv::Mat& histogram, int total)
	{
		double sum = 0;
		for (int t=0 ; t < histogram.rows; t++) 
			sum += t * static_cast<T>(histogram.ptr<T>(t)[0]);

		double sumB = 0;
		int wB = 0;
		int wF = 0;

		double varMax = 0;
		double threshold = 0;

		for (int t=0 ; t < histogram.rows; t++) 
		{
		   wB += histogram.ptr<T>(t)[0];     // Weight Background
		   if (wB == 0) continue;

		   wF = total - wB;                 // Weight Foreground
		   if (wF == 0) break;

		   sumB += t * static_cast<double>(histogram.ptr<T>(t)[0]);

		   double mB = sumB / wB;            // Mean Background
		   double mF = (sum - sumB) / wF;    // Mean Foreground

		   // Calculate Between Class Variance		   
		   double varBetween = wB * wF * (mB - mF) * (mB - mF);

		   // Check if new maximum found
		   if (varBetween > varMax) 
		   {
			  varMax = varBetween;
			  threshold = t;
		   }
		}
		return static_cast<T>(threshold);
	}	
};

#endif