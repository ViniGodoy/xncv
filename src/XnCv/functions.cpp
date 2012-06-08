#include "functions.hpp"
#include <opencv2\imgproc\imgproc.hpp>

//Private declarations
cv::Mat cvtDepth8UDistance(const cv::Mat& mat);

cv::Mat xncv::captureRGB(const xn::ImageGenerator& generator)
{
	xn::ImageMetaData meta;
	generator.GetMetaData(meta);
	return cv::Mat(meta.YRes(), meta.XRes(),cv::DataType<cv::Vec3b>::type, (void*)meta.RGB24Data());
}

cv::Mat xncv::captureBGR(const xn::ImageGenerator& generator)
{
	//Transforms it in a BGR cv::Mat
	cv::Mat img;
	cv::cvtColor(captureRGB(generator), img, CV_BGR2RGB);
	return img;
}

cv::Mat xncv::captureDepth(const xn::DepthGenerator& generator)
{
	xn::DepthMetaData meta;
	generator.GetMetaData(meta);
	return cv::Mat(meta.YRes(), meta.XRes(), cv::DataType<ushort>::type, (void*)meta.Data());
}

cv::Mat xncv::cvtDepthTo8UDist(const cv::Mat &mat, int zRes)
{
	//Calculate the maximum value
	if (zRes == 0)
		forEach<ushort>(mat, [&zRes](const cv::Point p, const ushort& elem) {
			if (elem > zRes) zRes = elem;
		});

	//Re-scale the matrix to fit in 255 colors.
	cv::Mat other(mat.rows, mat.cols, CV_8U);
	double scale = 255.0 / zRes;
	forEach<uchar>(other, [&mat, &zRes, &scale](const cv::Point p, uchar& elem) {
		const ushort& depth = mat.ptr<ushort>(p.y)[p.x];
		elem = depth == 0 ? 0 : static_cast<uchar>(255 -  depth * scale);
	});

	return other;
}

cv::Mat xncv::cvtDepthTo8UHist(const cv::Mat &mat, const cv::Mat& histogram)
{	
	cv::Mat result(mat.rows, mat.cols, CV_8U);

	//If the histogram is empty, returns an empty image
	if (histogram.empty())
		return result;
	
	//Calculate the accumulated histogram	
	cv::Mat accum = histogram.clone();
	accum.ptr<float>(0)[0] = 0;
	for (int i = 1; i < accum.rows; ++i)
		accum.ptr<float>(i)[0] += accum.ptr<float>(i-1)[0]; 

	//If there's only black pixels, return an empty image
	float count = accum.ptr<float>(accum.rows-1)[0];
	if (count == 0)
		return result;
	
	//Scale to 0..255 range
	forEach<float>(accum, [count](const cv::Point& p, float& elem) {
		elem = 256.0f * (1.0f - elem / count);
	});
				
	//Create the final image	
	forEach<uchar>(result, [&accum, mat](const cv::Point& p, uchar& elem)
	{
		const float &depth = mat.ptr<ushort>(p.y)[p.x];
		elem = static_cast<uchar>(accum.ptr<float>(static_cast<int>(depth))[0]);
	});

	return result;
}

cv::Mat xncv::calcDepthHist(const cv::Mat& depth, const xn::DepthGenerator& generator)
{
	xn::DepthMetaData meta;
	generator.GetMetaData(meta);

	int channels[] = {0};
	int histSize[] = {static_cast<int>(meta.ZRes())};
	float hranges[] = {0.0f, static_cast<float>(meta.ZRes()-1)};
	const float *ranges[] = {hranges};

	cv::Mat hist;
	cv::calcHist(&depth, 1, channels, cv::Mat(), hist, 1, histSize, ranges);
	return hist;
}

cv::Mat xncv::histogramImage(const cv::Mat& histogram, ushort height, bool cropRight, bool cropLeft)
{
	double max = 0, min = 0;
	cv::minMaxLoc(histogram, &min, &max);
	int lastCol = histogram.rows;
	int firstCol = 0;

	if (cropRight) while (histogram.ptr<float>(lastCol-1)[0] == 0) --lastCol;
	if (cropLeft) while (histogram.ptr<float>(firstCol)[0] == 0) ++firstCol;
	int cols = lastCol-firstCol;

	cv::Mat histImg = cv::Mat(height, cols, CV_8U, cv::Scalar(255));	

	for (int h = 0; h < cols; h++) 
	{						
		float bin = histogram.ptr<float>(h+firstCol)[0];
		int intensity = static_cast<int>(bin*(height*0.99-1)/max);		
		cv::line(histImg, 
			cv::Point(h, height),
			cv::Point(h, height-intensity),
			cv::Scalar::all(0));
	}
	return histImg;
}

cv::Point xncv::worldToProjective(const XnPoint3D& point, const xn::DepthGenerator& depth)
{
	XnPoint3D p;
	depth.ConvertRealWorldToProjective(1, &point, &p);
	return cv::Point(static_cast<int>(p.X), static_cast<int>(p.Y));
}

XnPoint3D xncv::projectiveToWorld(const cv::Point& point, XnFloat z, const xn::DepthGenerator& depth)
{
	XnPoint3D p;
	p.X = static_cast<XnFloat>(point.x);
	p.Y = static_cast<XnFloat>(point.y);
	p.Z = static_cast<XnFloat>(z);
	XnPoint3D p2;

	depth.ConvertProjectiveToRealWorld(1, &p, &p2);
	return p2;
}

std::ostream& xncv::operator<<(std::ostream& output, const XnVector3D& p) 
{
    return (output << "[" <<  p.X << ", " << p.Y <<", " << p.Z << "]");
}