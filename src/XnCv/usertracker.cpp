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

#include "usertracker.hpp"
#include "exceptions.hpp"

using namespace std;

// Callback: New user was detected
void XN_CALLBACK_TYPE onNewUser(xn::UserGenerator& generator, XnUserID nId, void* pCookie)
{
    generator.GetSkeletonCap().RequestCalibration(nId, TRUE);
}

// Callback: An existing user was lost
void XN_CALLBACK_TYPE onLostUser(xn::UserGenerator& generator, XnUserID nId, void* pCookie)
{
}

void XN_CALLBACK_TYPE onCalibrationComplete(xn::SkeletonCapability& capability, XnUserID nId, XnCalibrationStatus eStatus, void* pCookie)
{
    if (eStatus == XN_CALIBRATION_STATUS_OK)
	{
        capability.StartTracking(nId);
		return;
	}

    // If the calibration was aborted, do not retry
    if(eStatus==XN_CALIBRATION_STATUS_MANUAL_ABORT)
        return;

	//Otherwise, retry
    capability.RequestCalibration(nId, TRUE);
}

xncv::UserTracker::UserTracker(VideoSource& source, XnSkeletonProfile profile)
{
	if (source.fromFile())
		throw xncv::NoCapabilityException("Cannot generate skeleton from files!");

	XnStatus status = userGen.Create(source.getXnContext());
	if (status != XN_STATUS_OK)
		throw xncv::UnableToInitGenerator("Unable to init User Generator!");

	if (!userGen.IsCapabilitySupported(XN_CAPABILITY_SKELETON))
	{
		userGen.Release();
		throw xncv::NoCapabilityException("Generator does not suport skeleton capability!");
	}
	userGen.GetSkeletonCap().SetSkeletonProfile(profile);
	userGen.RegisterUserCallbacks(&onNewUser, &onLostUser, nullptr, userHandler);
	userGen.GetSkeletonCap().RegisterToCalibrationComplete(&onCalibrationComplete, nullptr, calibrationHandler);

	userGen.StartGenerating();
}

xncv::UserTracker::~UserTracker()
{
	userGen.UnregisterUserCallbacks(userHandler);
	userGen.GetSkeletonCap().UnregisterFromCalibrationComplete(calibrationHandler);
	userGen.Release();
}

std::vector<XnSkeletonJoint> xncv::UserTracker::getActiveJoints() const
{
	XnSkeletonJoint joints[25];
	XnUInt16 numJoints = 25;
	userGen.GetSkeletonCap().EnumerateActiveJoints(joints, numJoints);
	vector<XnSkeletonJoint> jointVector(numJoints);
	for (int i = 0; i < numJoints; ++i)
		jointVector.push_back(joints[i]);
	return jointVector;
}

bool xncv::UserTracker::setJointActive(XnSkeletonJoint joint, bool active)
{
	return userGen.GetSkeletonCap().SetJointActive(joint, active) == XN_STATUS_OK;
}

bool xncv::UserTracker::setProfile(XnSkeletonProfile profile)
{
	return userGen.GetSkeletonCap().SetSkeletonProfile(profile) == XN_STATUS_OK;
}

bool xncv::UserTracker::hasUser(XnUserID id)
{
	XnUserID ids[5];
	XnUInt16 numIds = 5;
	userGen.GetUsers(ids, numIds);

	for (int i = 0; i < numIds; ++i)
		if (ids[i] == id)
			return true;
	return false;
}

xncv::User xncv::UserTracker::getUser(XnUserID id)
{
	if (!hasUser(id))
		throw UserNotFoundException(id);
	return User(id, &userGen);
}

vector<xncv::User> xncv::UserTracker::getUsers()
{
	XnUserID ids[5];
	XnUInt16 numIds = 5;
	userGen.GetUsers(ids, numIds);

	vector<User> users;
	for (int i = 0; i < numIds; ++i)
		users.push_back(User(ids[i], &userGen));
	return users;
}

void xncv::drawLimbs(cv::Mat& image, const vector<xncv::Limb>& limbs, float confidenceThreshold, unsigned char color)
{
	std::for_each(limbs.begin(), limbs.end(), [&image, confidenceThreshold, color](const xncv::Limb& limb)
	{
		cv::line(image, limb.joint1.pos, limb.joint2.pos,
			cv::Scalar::all(color), 
			limb.confidence < confidenceThreshold ? 1 : 2);
		cv::circle(image, limb.joint1.pos, 3, cv::Scalar::all(color), -1);
		cv::circle(image, limb.joint2.pos, 3, cv::Scalar::all(color), -1);
	});
}

std::vector<xncv::User> xncv::filterClosest(const std::vector<xncv::User>& users)
{
	float closestDist = -1;
	int index = -1;

	for (unsigned i = 0; i < users.size(); ++i)
	{
		if (users[i].isCalibrating() || !users[i].isTracking())
			continue;

		auto joints = users[i].getJoints();
		if (joints.size() == 0)
			continue;

		//Test if it's closest than the previous one
		XnVector3D CoM = users[i].getCenterOfMass();
		float dist = CoM.X * CoM.X + CoM.Y * CoM.Y + CoM.Z * CoM.Z;
		if (index == -1 || dist  < closestDist)
		{
			index = i;
			closestDist = dist;
		}
	}

	//Returns the closest user.
	std::vector<xncv::User> result;
	if (index != -1)
		result.push_back(users[index]);
	return result;
}