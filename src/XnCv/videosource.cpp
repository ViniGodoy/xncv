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

xncv::VideoSource::VideoSource(const std::string& file)
{
	init(file);
}

void xncv::VideoSource::start()
{
	if (context.StartGeneratingAll() != XN_STATUS_OK) throw new GeneratorError("Unable to start generating data!");
}

void xncv::VideoSource::stop()
{
	if (context.StopGeneratingAll() != XN_STATUS_OK) throw new GeneratorError("Unable to stop generating data!");
}

void xncv::VideoSource::update()
{
	if (context.WaitAndUpdateAll() != XN_STATUS_OK) throw new GeneratorError("Unable to update data from generators!");
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