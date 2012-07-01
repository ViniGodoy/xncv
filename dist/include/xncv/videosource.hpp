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

#if !defined(__VIDEOSOURCE_HPP__)
#define __VIDEOSOURCE_HPP__

#include <string>
#include <opencv2\opencv.hpp>
#include <XnCppWrapper.h>


namespace xncv
{
	enum DepthCompression {DEPTH_DONT_CAPTURE, DEPTH_NONE, DEPTH_EMB_TABLES_16Z};
	enum ImageCompression {IMG_DONT_CAPTURE, IMG_NONE, IMG_JPEG};

	class VideoSource
	{
		private:
			xn::Player player;
			xn::Context context;		
			xn::ImageGenerator imgGen;
			xn::DepthGenerator depthGen;
			xn::Recorder* recorder;

			bool isFile;

			void init(const std::string& file);
			void seek(XnInt32 frame, XnPlayerSeekOrigin origin);

			std::string fixFileName(const std::string fileName);
			void createRecorder(const std::string& fileName);
		public:
			VideoSource();
			VideoSource(const std::string& file);
			
			bool fromFile() const;

			//Display commands
			void start();
			void update();
			void stop();

			//Navigation commands			
			void first();
			void jump(int frames);
			void goTo(int frame);
			void last();

			int currentFrame() const;
			int size() const;

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
			
			bool startRecording(const std::string& filename, 
				ImageCompression imageCompression=IMG_JPEG, DepthCompression 
				depthCompression = DEPTH_EMB_TABLES_16Z);
			void stopRecording();
			bool isRecording() const;

			~VideoSource();
	};
}
#endif