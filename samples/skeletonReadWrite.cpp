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

const char* FILENAME = "skeleton.skl";

//-----------------------------------------------------------------------------
//	Records skeleton information
//-----------------------------------------------------------------------------
void writeSkeleton(int argc, char* argv[])
{
	//Test if should record only the closest user
	bool closest = argc == 2 && strcmp(argv[1], "-closest");

	//Creates the video source using the device.
	xncv::VideoSource source;

	//Associate an user tracker to this video source.
	xncv::UserTracker tracker(source);

	//Creates the skeleton writer
	xncv::SkeletonWriter writer(source);
	writer.open(FILENAME);

	//Start the video device
	source.start();	
	cv::namedWindow("Depth");

	// Recording loop
	bool running = true;
	while (running)
	{
		source.update();

		//Reads the depth map and calculates it's histogram distributed image
		cv::Mat dm = source.captureDepth(); //Depth map as a ushort Mat.
		cv::Mat hist = source.calcDepthHist();  //Depth map histogram
		cv::Mat histImg = dm = xncv::cvtDepthTo8UHist(dm, hist);

		//Draws the skeleton over histImg
		std::vector<xncv::User> users =
			closest ? xncv::filterClosest(tracker.getUsers()) : tracker.getUsers();

		//Indicate that a new frame will be recorded
		writer.beginFrame();		

		//For each user in the frame.
		for (unsigned i = 0; i < users.size(); ++i)
		{
			//If already tracking user, draws it.
			std::vector<xncv::Limb> limbs = users[i].getLimbs(source.getXnDepthGenerator());
			xncv::drawLimbs(histImg, limbs);	

			//Also records the user information.
			writer << users[i];				
		}
		//Indicate that this frame recording is finished
		writer.endFrame();

		cv::imshow("Depth", histImg);

		//Waits for user input
		//ESC - Stop recording
		char key = cv::waitKey(1);
		if (key == 27)
			running = false;
	}
}

//-----------------------------------------------------------------------------
//	Reads skeleton information
//-----------------------------------------------------------------------------
void readSkeleton(int argc, char* argv[])
{
	//Loaded all recorded skeleton information
	xncv::SkeletonReader reader;
	reader.open(FILENAME);
	
	for (int frame = 0; frame < reader.frameCount(); frame++)
	{		
		cv::Mat area = cv::Mat(480, 640, cv::DataType<ushort>::type);
		auto users = reader.getUsers(frame);

		//Skip frames with no information
		if (users.size() == 0)
			continue;
		
		//Draw each user skeleton
		for (unsigned i = 0; i < users.size(); ++i)
			xncv::drawLimbs(area, users[i].getLimbs());

		cv::imshow("Depth", area);
		char key = cv::waitKey(33); //~30FPS
		if (key == 27) return;
	}
}

int main(int argc, char* argv[])
{
	try
	{		
		writeSkeleton(argc, argv);
		readSkeleton(argc, argv);
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