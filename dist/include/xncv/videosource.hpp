#if !defined(__VIDEOSOURCE_HPP__)
#define __VIDEOSOURCE_HPP__

#include <string>
#include <opencv2\opencv.hpp>
#include <XnCppWrapper.h>

namespace xncv
{
	class VideoSource
	{
		private:
			xn::Player player;
			xn::Context context;		
			xn::ImageGenerator imgGen;
			xn::DepthGenerator depthGen;
			bool isFile;

			void init(const std::string& file);
		public:
			VideoSource();
			VideoSource(const std::string& file);

			void start();
			void stop();
			void update();

			cv::Mat captureBGR(bool clone=false) const;
			cv::Mat captureDepth(bool clone=false) const;

			cv::Mat calcDepthHist() const;
			cv::Mat calcDepthHist(const cv::Mat& depth) const;

			cv::Point worldToProjective(const XnPoint3D& point);
			XnPoint3D projectiveToWorld(const cv::Point& point, XnFloat z=-1.0f);			

			xn::Context& getXnContext() { return context; }
			xn::Player& getXnPlayer() { return player; }
			xn::ImageGenerator& getXnImageGenerator() { return imgGen; }
			xn::DepthGenerator& getXnDepthGenerator() { return depthGen; }			
	};
}
#endif