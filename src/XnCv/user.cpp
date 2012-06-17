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

#include "user.hpp"

xncv::User::User(XnUserID userId, xn::UserGenerator* generator)
: id(userId), userGen(generator)
{
}

xncv::User::~User()
{
}

XnUserID xncv::User::getId() const
{
	return id;
}

bool xncv::User::isCalibrating() const
{
	return userGen->GetSkeletonCap().IsCalibrating(id) == TRUE;
}

bool xncv::User::isCalibrated() const
{
	return userGen->GetSkeletonCap().IsCalibrated(id) == TRUE;
}

bool xncv::User::setTracking(bool tracking)
{
	return (tracking ? userGen->GetSkeletonCap().StartTracking(id) :
					   userGen->GetSkeletonCap().StopTracking(id)) == XN_STATUS_OK;

}

bool xncv::User::isTracking() const
{
	return userGen->GetSkeletonCap().IsTracking(id) == TRUE;
}

bool xncv::User::abortCalibration()
{
	return userGen->GetSkeletonCap().AbortCalibration(id) == XN_STATUS_OK;
}

bool xncv::User::requestCalibration(bool force)
{
	return userGen->GetSkeletonCap().RequestCalibration(id, force) == XN_STATUS_OK;
}

bool xncv::User::saveCalibration(const std::string& fileName)
{
	return userGen->GetSkeletonCap().SaveCalibrationDataToFile(id, fileName.c_str()) == XN_STATUS_OK;
}

bool xncv::User::loadCalibration(const std::string& fileName)
{
	return userGen->GetSkeletonCap().LoadCalibrationDataFromFile(id, fileName.c_str()) == XN_STATUS_OK;
}

bool xncv::User::getJoint(XnSkeletonJoint type, XnSkeletonJointTransformation& transform) const
{
	return userGen->GetSkeletonCap().GetSkeletonJoint(id, type, transform) == XN_STATUS_OK;
}

std::map<XnSkeletonJoint, XnSkeletonJointTransformation> xncv::User::getJoints() const
{
	std::map<XnSkeletonJoint, XnSkeletonJointTransformation> jointMap;
	if (isTracking())
		return jointMap;

	XnSkeletonJoint joints[25];
	XnUInt16 numJoints = 25;
	userGen->GetSkeletonCap().EnumerateActiveJoints(joints, numJoints);

	for (int i = 0; i < numJoints; ++i)
	{
		XnSkeletonJointTransformation joint;
		if (getJoint(joints[i], joint))
			jointMap[joints[i]] = joint;
	}
	return jointMap;
}

std::vector<xncv::Limb> xncv::User::getLimbs(const xn::DepthGenerator& depthGen) const
{
    static XnSkeletonJoint LIMB_JOINTS[][2] =
    {
        { XN_SKEL_HEAD, XN_SKEL_NECK },
        { XN_SKEL_NECK, XN_SKEL_LEFT_SHOULDER },
        { XN_SKEL_LEFT_SHOULDER, XN_SKEL_LEFT_ELBOW },
        { XN_SKEL_LEFT_ELBOW, XN_SKEL_LEFT_HAND },
        { XN_SKEL_NECK, XN_SKEL_RIGHT_SHOULDER },
        { XN_SKEL_RIGHT_SHOULDER, XN_SKEL_RIGHT_ELBOW },
        { XN_SKEL_RIGHT_ELBOW, XN_SKEL_RIGHT_HAND },
        { XN_SKEL_LEFT_SHOULDER, XN_SKEL_TORSO },
        { XN_SKEL_RIGHT_SHOULDER, XN_SKEL_TORSO },
        { XN_SKEL_TORSO, XN_SKEL_LEFT_HIP },
        { XN_SKEL_LEFT_HIP, XN_SKEL_LEFT_KNEE },
        { XN_SKEL_LEFT_KNEE, XN_SKEL_LEFT_FOOT },
        { XN_SKEL_TORSO, XN_SKEL_RIGHT_HIP },
        { XN_SKEL_RIGHT_HIP, XN_SKEL_RIGHT_KNEE },
        { XN_SKEL_RIGHT_KNEE, XN_SKEL_RIGHT_FOOT },
        { XN_SKEL_LEFT_HIP, XN_SKEL_RIGHT_HIP },
    };

	std::vector<xncv::Limb> limbs;

	if (!isTracking())
		return limbs;

    XnSkeletonJointPosition joint1, joint2;
	for(XnUInt16 i=0; i < MAX_LIMBS; ++i)
    {
        if(userGen->GetSkeletonCap().GetSkeletonJointPosition(id, LIMB_JOINTS[i][0], joint1)!=XN_STATUS_OK)
            continue; // bad joint

        if(userGen->GetSkeletonCap().GetSkeletonJointPosition(id, LIMB_JOINTS[i][1], joint2)!=XN_STATUS_OK)
            continue; // bad joint

		Limb limb;
		limb.confidence = joint1.fConfidence < joint2.fConfidence ? joint1.fConfidence : joint2.fConfidence;
		limb.joint1.type = LIMB_JOINTS[i][0];
		limb.joint1.pos = xncv::worldToProjective(joint1.position, depthGen);
		limb.joint2.type = LIMB_JOINTS[i][1];
		limb.joint2.pos = xncv::worldToProjective(joint2.position, depthGen);
		limbs.push_back(limb);
    }

    return limbs;
}

XnVector3D xncv::User::getCenterOfMass() const
{
	XnPoint3D center = {0.0f, 0.0f, 0.0f};
	userGen->GetCoM(id, center);
	return center;
}