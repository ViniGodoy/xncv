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
 * This tutorial shows how to control .oni video exhibition
 */
int main(int argc, char* argv[])
{
	std::string filename;

	if (argc > 1)
		filename = argv[1];

	try
	{
		//Create and starts the video source from a .oni file
		//To read from the input hardware do not provide a filename (you may
		//also provide a blank ("") filename).
		xncv::VideoSource source(filename);
		source.start();

		cv::namedWindow("Video");
		cv::namedWindow("Depth");

		// Main loop
		bool running = true;
		while (running)
		{
			source.update();

			//Reads and displays the RGB image. Notice that the image is converted
			//to BGR, since it's opencv default format. The true parameter indicates
			//that we want a copy of the source image, not just a pointer to it.			
			cv::Mat video = source.captureBGR(true);

			//Draws the red filled circle indicating that the video is recording			
			if (source.isRecording())
				cv::circle(video, cv::Point(600,15), 8, cv::Scalar(0,0,255,255), -1);

			//Shows the image
			cv::imshow("Video", video);

			//Reads the depth map and calculates it's histogram distributed image
			cv::Mat dm = source.captureDepth(); //Depth map as a ushort Mat.
			cv::Mat hist = source.calcDepthHist();  //Depth map histogram
			cv::Mat histImg = dm = xncv::cvtDepthTo8UHist(dm, hist);
			cv::imshow("Depth", histImg);

			//Waits for user input
			//ESC - Leave application
			//P - Prints current frame.
			//R - Start/stop recording
			char key = cv::waitKey(1);
			if (key == 27)
				running = false;
			else if (key == 'p' || key == 'P')
			{
				cv::imwrite("video.jpg", source.captureBGR());
				cv::imwrite("depth.jpg", dm);
				cv::waitKey(500);
			}
			else if (!source.fromFile() && (key == 'r' || key == 'R'))
			{				
				if (source.isRecording())
					source.stopRecording();
				else
					//Starts recording. This will only work if it's not 
					//a file being played.
					source.startRecording("recordSample");
			}
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