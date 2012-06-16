#include "usertracker.hpp"
#include "exceptions.hpp"

using namespace std;

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
	userGen.StartGenerating();
}

xncv::UserTracker::~UserTracker()
{
	userGen.Release();
}

bool xncv::UserTracker::isCalibrationPoseRequired() const
{
	return userGen.GetSkeletonCap().NeedPoseForCalibration() == TRUE;
}

string xncv::UserTracker::getCalibrationPose() const
{
	XnChar calibrationPose[20];

	string pose;
	if (userGen.GetSkeletonCap().GetCalibrationPose(calibrationPose) == XN_STATUS_OK)
		pose.assign(calibrationPose);

	return pose;
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

void xncv::drawLimbs(const vector<xncv::Limb>& limbs, cv::Mat& image, float confidenceThreshold)
{
	std::for_each(limbs.begin(), limbs.end(), [&image, confidenceThreshold](const xncv::Limb& limb)
	{
		cv::line(image, limb.joint1.pos, limb.joint2.pos,
			cv::Scalar::all(0),
			limb.confidence < confidenceThreshold ? 1 : 2);
		cv::circle(image, limb.joint1.pos, 3, cv::Scalar::all(0), -1);
		cv::circle(image, limb.joint2.pos, 3, cv::Scalar::all(0), -1);
	});
}