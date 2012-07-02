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

#if !defined(__USER_TRACKER_HPP__)
#define __USER_TRACKER_HPP__

#include "videosource.hpp"
#include "user.hpp"

namespace xncv
{
	class UserTracker
	{
		private:			
			xn::UserGenerator userGen;

			XnCallbackHandle calibrationHandler;
			XnCallbackHandle userHandler;

		public:
			UserTracker(VideoSource& source, XnSkeletonProfile profile=XN_SKEL_PROFILE_ALL);
			~UserTracker();

			std::vector<XnSkeletonJoint> getActiveJoints() const;
			bool setJointActive(XnSkeletonJoint joint, bool active=true);
			bool setProfile(XnSkeletonProfile profile);

			bool hasUser(XnUserID id);
			User getUser(XnUserID id);
			std::vector<User> getUsers();
	};

	void drawLimbs(cv::Mat& image, const std::vector<xncv::Limb>& limbs, float confidenceThreshold=0.5f, unsigned char color=0);
	std::vector<xncv::User> filterClosest(const std::vector<xncv::User>& users);
}
#endif 