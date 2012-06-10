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

#include "VideoSource.hpp"
#include "functions.hpp"
#include "exceptions.hpp"

void xncv::VideoSource::init(const std::string& file)
{
	isFile = !file.empty();
	if (context.Init() != XN_STATUS_OK) throw std::runtime_error("Unable to init context");

	if (isFile)
	{
		if (context.OpenFileRecording(file.c_str(), player) != XN_STATUS_OK)
			throw UnableToOpenFileException(file);
		if (context.FindExistingNode(XN_NODE_TYPE_IMAGE, imgGen) != XN_STATUS_OK)
			throw UnableToInitGenerator("Unable to init image generator.");
		if (context.FindExistingNode(XN_NODE_TYPE_DEPTH, depthGen) != XN_STATUS_OK)
			throw UnableToInitGenerator("Unable to depth generator.");
		return;
	}

	if (imgGen.Create(context) != XN_STATUS_OK) throw UnableToInitGenerator("Unable to init image generator.");
	if (depthGen.Create(context) != XN_STATUS_OK) throw UnableToInitGenerator("Unable to depth generator.");
}

xncv::VideoSource::VideoSource()
{
	init("");
}

xncv::VideoSource::VideoSource(const std::string& file, bool repeat)
{
	init(file);
	setRepeat(repeat);
}

void xncv::VideoSource::start()
{
	if (context.StartGeneratingAll() != XN_STATUS_OK)
		throw new GeneratorError("Unable to start generating data!");
}

void xncv::VideoSource::stop()
{
	if (context.StopGeneratingAll() != XN_STATUS_OK && !isFile)
		throw new GeneratorError("Unable to stop generating data!");
}

void xncv::VideoSource::update()
{
	if (context.WaitAndUpdateAll() != XN_STATUS_OK)
		throw new GeneratorError("Unable to update data from generators!");
}

cv::Mat xncv::VideoSource::captureBGR(bool clone) const
{
	return clone ? xncv::captureBGR(imgGen).clone() : xncv::captureBGR(imgGen);
}

cv::Mat xncv::VideoSource::captureDepth(bool clone) const
{
	return clone ? xncv::captureDepth(depthGen).clone() : xncv::captureDepth(depthGen);
}

cv::Mat xncv::VideoSource::calcDepthHist() const
{
	return calcDepthHist(captureDepth());
}

cv::Mat xncv::VideoSource::calcDepthHist(const cv::Mat& depth) const
{
	return xncv::calcDepthHist(depth, depthGen);
}

cv::Point xncv::VideoSource::worldToProjective(const XnPoint3D& point)
{
	return xncv::worldToProjective(point, depthGen);
}

XnPoint3D xncv::VideoSource::projectiveToWorld(const cv::Point& point, XnFloat z)
{
	if (z < 0.0f) z = captureDepth().ptr<ushort>(point.y)[point.x];
	return xncv::projectiveToWorld(point, z, depthGen);
}

void xncv::VideoSource::seek(XnInt32 frame, XnPlayerSeekOrigin origin)
{
	//Command is ignored for the input device.
	if (!isFile)
		return;

	if (imgGen.IsValid())
	{
		int status = player.SeekToFrame(imgGen.GetName(), frame, origin) != XN_STATUS_OK;
		if (status != XN_STATUS_OK)
		{
			if (origin == XN_PLAYER_SEEK_SET)
				throw xncv::FrameSkipException("Unable to seek image to frame", frame, status);
			else
				throw xncv::FrameSkipException("Unable to jump image to frame", frame, status);
		}
	}
}

void xncv::VideoSource::first()
{
	seek(0, XN_PLAYER_SEEK_SET);
}

void xncv::VideoSource::jump(int frames)
{
	seek(frames, XN_PLAYER_SEEK_CUR);
}

void xncv::VideoSource::goTo(int frame)
{
	seek(frame, XN_PLAYER_SEEK_SET);
}

void xncv::VideoSource::last()
{
	seek(size(), XN_PLAYER_SEEK_SET);
}

int xncv::VideoSource::currentFrame() const
{
	if (!isFile)
		return -1;

	XnUInt32 nFrame = 0;
	XnStatus nRetVal = player.TellFrame(imgGen.GetName(), nFrame);
	if (nRetVal != XN_STATUS_OK)
		throw new xncv::VideoSourceException("Unable to read the current frame");
	return static_cast<int>(nFrame);
}

int xncv::VideoSource::size() const
{
	if (!isFile)
		return -1;

	XnUInt32 nFrames = 0;
	XnStatus nRetVal = player.GetNumFrames(imgGen.GetName(), nFrames);
	if (nRetVal != XN_STATUS_OK)
		throw new xncv::VideoSourceException("Unable to read the number of frames");

	return static_cast<int>(nFrames);
}

xncv::VideoSource::~VideoSource()
{
	imgGen.Release();
	depthGen.Release();
	player.Release();
}