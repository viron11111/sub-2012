#include <cstdio>
#include <stdexcept>

#include <boost/foreach.hpp>

#include "Blob.h"

using namespace cv;

Blob::Blob(IOImages* ioimages, float minContour, float maxContour, float maxPerimeter)
{
	Mat dbg_temp = ioimages->dbg.clone();
	std::vector<std::vector<cv::Point> > contours;
	std::vector<cv::Vec4i> hierarchy;
	findContours(dbg_temp,contours,hierarchy,RETR_CCOMP,CHAIN_APPROX_SIMPLE);

	BOOST_FOREACH(const std::vector<cv::Point> &contour, contours) {
		float area_holder = fabs(contourArea(Mat(contour)));
		double perimeter_holder = arcLength(Mat(contour), true);

		if(area_holder < minContour || area_holder > maxContour
				|| perimeter_holder > maxPerimeter /*|| !isContourConvex(Mat(contour))*/)
			continue;

		cv::Point2f center_holder;
		float radius_holder;
		minEnclosingCircle(Mat(contour),center_holder,radius_holder);

		if(center_holder.x == 0 || center_holder.y == 0)
			continue; // ???

		circle(ioimages->res,center_holder,2,CV_RGB(0,255,255),-1,8,0);
		BlobData bdata;
		bdata.perimeter = (float)perimeter_holder;
		bdata.area = area_holder;
		bdata.centroid.x = (int)center_holder.x;
		bdata.centroid.y = (int)center_holder.y;
		bdata.radius = radius_holder;

		// Check for color of blob
		Mat tempHSV; cvtColor(ioimages->prcd,tempHSV,CV_BGR2HSV);
		std::vector<Mat> channelsHSV(ioimages->src.channels()); split(tempHSV,channelsHSV);
		Mat tempMask = Mat::zeros(ioimages->src.rows, ioimages->src.cols, CV_8UC1);
			drawContours(tempMask, std::vector<std::vector<cv::Point> >(1,contour), 0, Scalar(255), CV_FILLED, 1, vector<Vec4i>(), 5);
		bdata.hue = mean(channelsHSV[0], tempMask)[0];

		data.push_back(bdata);
	}

	sort(data.begin(), data.end());
	reverse(data.begin(),data.end());
	if(data.size() > 3)
		data.resize(3);
}

void Blob::drawResult(IOImages* ioimages, std::string objectName)
{
	// Sort the data array by hue using our custom comparitor
	sort(data.begin(),data.end(),compareBlobData);

	BOOST_FOREACH(const BlobData &item, data) {
		Scalar color = CV_RGB(255, 255, 255);
		if(objectName == "buoy/red") // red
			color = CV_RGB(255,100,0);
		else if(objectName == "buoy/yellow") // yellow
			color = CV_RGB(230,230,0);
		else if(objectName == "buoy/green") // green
			color = CV_RGB(0,200,0);
		else if(objectName == "hedge")	// green
			color = CV_RGB(0,200,0);

		circle(ioimages->res,item.centroid,(int)item.radius,color,2,8,0);
		std::ostringstream os; os << "Area: " << (int)item.area;
		putText(ioimages->res,os.str().c_str(),Point(item.centroid.x-30,item.centroid.y-10),FONT_HERSHEY_DUPLEX,0.4,CV_RGB(0,0,0),1.5);
	}

}

bool Blob::compareBlobData(BlobData a, BlobData b)
{
	return a.hue < b.hue;
}
