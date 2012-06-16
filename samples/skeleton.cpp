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
 * This tutorial shows how to draw the user skeleton.
 */
int main(int argc, char* argv[])
{
	try
	{
		//Create and starts the video source from a .oni file
		//To read from the input hardware do not provide a filename (you may
		//also provide a blank ("") filename).
		xncv::VideoSource source;

		//Associate an user tracker to this video source.
		xncv::UserTracker tracker(source, XN_SKEL_PROFILE_UPPER);

		source.start();

		cv::namedWindow("Depth");

		// Main loop
		bool running = true;
		while (running)
		{
			source.update();

			//Reads the depth map and calculates it's histogram distributed image
			cv::Mat dm = source.captureDepth(); //Depth map as a ushort Mat.
			cv::Mat hist = source.calcDepthHist();  //Depth map histogram
			cv::Mat histImg = dm = xncv::cvtDepthTo8UHist(dm, hist);

			//Draws the skeleton over histImg
			std::vector<xncv::User> users = tracker.getUsers();
			for (unsigned i = 0; i < users.size(); ++i)
			{
				//If already tracking user, draws it.
				if (users[i].isTracking())
				{
					std::vector<xncv::Limb> limbs = users[i].getLimbs(source.getXnDepthGenerator());
					xncv::drawLimbs(limbs, histImg);
				}
				else
				{
					//Otherwise, it's a new user. Track him.
					users[i].setTracking();
				}
			}

			cv::imshow("Depth", histImg);

			//Waits for user input
			//ESC - Leave application
			//R - Records current frame.
			char key = cv::waitKey(1);
			if (key == 27)
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