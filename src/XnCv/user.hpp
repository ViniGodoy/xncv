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

#if !defined(__USER_HPP__)
#define __USER_HPP__

#include "functions.hpp"
#include <vector>
#include <map>

namespace xncv
{
	const XnUInt16 MAX_LIMBS=16;

	struct JointInfo
	{
		XnSkeletonJoint type;		
		cv::Point pos;
	};

	struct Limb
	{
		JointInfo joint1;
		JointInfo joint2;
		XnConfidence confidence;
	};

	class User
	{
		private:
			XnUserID id;			
			xn::UserGenerator* userGen;

		public:
			User(XnUserID userId, xn::UserGenerator* generator);
			~User();

			XnUserID getId() const;

			bool isCalibrating() const;
			bool isCalibrated() const;
			bool isTracking() const;
			bool setTracking(bool tracking=true);			

			bool abortCalibration();
			bool requestCalibration(bool force=true);
			bool saveCalibration(const std::string& fileName);
			bool loadCalibration(const std::string& fileName);

			bool getJoint(XnSkeletonJoint type, XnSkeletonJointTransformation& transform) const;
			std::map<XnSkeletonJoint, XnSkeletonJointTransformation> getJoints() const;
			std::vector<Limb> getLimbs(const xn::DepthGenerator& depthGen) const;
	};
}
#endif