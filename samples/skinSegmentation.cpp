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

#include <iostream>
#include <xncv\xncv.hpp>

/**
 * This tutorial shows how to use xncv::forEach to do skin segmentation over the OpenNI input device.
 */
int main(int argc, char* argv[])
{
	try
	{
		//Create and starts the video source from the device.
		xncv::VideoSource source();
		source.start();

		cv::namedWindow("Video");
		cv::namedWindow("Skin segmented");

		// Main loop
		bool running = true;
		while (running)
		{
			source.update();

			//Reads and displays the RGB image. Notice that the image is converted
			//to BGR, since it's opencv default format.
			cv::Mat video = source.captureBGR();
			cv::imshow("Video", video);

			//Calculate the skin segmentation using a simple RGB range
			cv::Mat skin = cv::Mat(video.rows, video.cols, CV_8U);
			xncv::forEach<cv::Vec3b>(video, [&skin](const cv::Point& p, const cv::Vec3b& pixel)
			{
				float rgbsum = pixel[0] + pixel[1] + pixel[2];
				float r = 100.0*pixel[2] / rgbsum;
				float g = 100.0*pixel[1] / rgbsum;
				skin.ptr<uchar>(p.y)[p.x] = ((r >= 38 && r <= 55) && (g >= 25 && g <= 38)) ? 255 : 0;
			});
			cv::imshow("Skin segmented", skin);

			//Waits for user input
			if (cv::waitKey(1) == 27)
				running = false;
		}
	}
	catch (xncv::Exception& e)
	{
		//Show error and leave
		std::cout << "Some problems have occurred:" << std::endl;
		std::cout << e.what() << std::endl;
		return 2;
	}

	return 0;
}