#include "skeletonio.hpp"
#include "exceptions.hpp"

//-----------------------------------------------------------------------------
//User information
//-----------------------------------------------------------------------------
xncv::UserInformation::UserInformation(XnUserID uid) : id(uid)
{
}

XnUserID xncv::UserInformation::getId() const
{
	return id;
}

const std::map<XnSkeletonJoint, XnSkeletonJointTransformation>& xncv::UserInformation::getJoints() const
{
	return worldJoints;
}

std::vector<xncv::Limb> xncv::UserInformation::getLimbs() const
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
	for(XnUInt16 i=0; i < MAX_LIMBS; ++i)
    {
		auto joint1It = projectiveJoints.find(LIMB_JOINTS[i][0]);
		if(joint1It == projectiveJoints.cend())
            continue; // bad joint


		auto joint2It = projectiveJoints.find(LIMB_JOINTS[i][1]);
		if(joint2It == projectiveJoints.cend())
            continue; // bad joint
		ProjectiveJoint joint1 = joint1It->second;
		ProjectiveJoint joint2 = joint2It->second;

		Limb limb;
		limb.confidence = joint1.fConfidence < joint2.fConfidence ? joint1.fConfidence : joint2.fConfidence;
		limb.joint1.type = LIMB_JOINTS[i][0];
		limb.joint1.pos = joint1.position;
		limb.joint2.type = LIMB_JOINTS[i][1];
		limb.joint2.pos = joint2.position;
		limbs.push_back(limb);
    }

    return limbs;
}

//-----------------------------------------------------------------------------
//Skeleton reader
//-----------------------------------------------------------------------------
xncv::SkeletonReader::SkeletonReader()
{
}

void xncv::SkeletonReader::open(const std::string& filename)
{
	using namespace std;

	fstream reader;
	reader.open(filename, fstream::out | fstream::binary);

	unsigned magic;
	reader >> magic;
	if (magic != MAGIC)
		throw new xncv::IOException("Invalid file", filename);

	unsigned short version;
	reader >> version;
	if (version > VERSION)
		throw new xncv::IOException("Version bigger than expected. Please update your xncv library.", filename);


	while (!reader.eof())
	{
		int frame;
		reader >> frame;

		XnUserID userId;
		reader >> userId;
		UserInformation userInfo(userId);

		unsigned short jointsSize;
		reader >> jointsSize;
		for (int i = 0; i < jointsSize; ++i)
		{
			//Joint type
			unsigned short type;
			reader >> type;
			XnSkeletonJoint jointType = static_cast<XnSkeletonJoint>(type);

			//World position
			XnSkeletonJointTransformation transform;
			reader >> transform.position.position.X;
			reader >> transform.position.position.Y;
			reader >> transform.position.position.Z;
			reader >> transform.position.fConfidence;

			//World orientation
			for (int i = 0; i < 9; ++i)
				reader >> transform.orientation.orientation.elements[i];
			reader >> transform.orientation.fConfidence;

			//Projective position
			ProjectiveJoint projectiveJoint;
			reader >> projectiveJoint.position.x;
			reader >> projectiveJoint.position.y;
			projectiveJoint.fConfidence = transform.position.fConfidence;

			userInfo.worldJoints[jointType] = transform;
			userInfo.projectiveJoints[jointType] = projectiveJoint;
			users[frame].push_back(userInfo);
		}
	}
}

int xncv::SkeletonReader::frameCount() const
{
	return users.size();
}

const std::vector<xncv::UserInformation>& xncv::SkeletonReader::getUsers(int frame) const
{
	if (frame < 0 || frame >= frameCount())
		return empty;

	return users.at(frame);
}

//-----------------------------------------------------------------------------
//Skeleton writer
//-----------------------------------------------------------------------------
xncv::SkeletonWriter::SkeletonWriter(xn::DepthGenerator* generator)
	: writer(), depthGen(generator), frame(0)
{
	writer.exceptions(std::fstream::failbit | std::fstream::badbit);
}

void xncv::SkeletonWriter::open(const std::string& fileName)
{
	writer.open(fileName, std::fstream::in | std::fstream::trunc | std::fstream::binary);
	frame = 0;
	writer << MAGIC;
	writer << VERSION; //File version
}

void xncv::SkeletonWriter::operator<<(const User& user)
{
	if (!user.isTracking() || user.isCalibrating())
		return;

	writer << frame;
	writer << user.getId();
	auto joints = user.getJoints();
	unsigned short jointsSize = static_cast<unsigned short>(joints.size());
	writer << joints.size();
	for (auto it = joints.cbegin(); it != joints.cend(); ++it)
	{
		//Joint type
		writer << static_cast<unsigned short>(it->first);

		//World position
		writer << it->second.position.position.X;
		writer << it->second.position.position.Y;
		writer << it->second.position.position.Z;
		writer << it->second.position.fConfidence;

		//World orientation
		for (int i = 0; i < 9; ++i)
			writer << it->second.orientation.orientation.elements[i];
		writer << it->second.orientation.fConfidence;

		//Projective position
		cv::Point projectivePos = xncv::worldToProjective(it->second.position.position, *depthGen);
		writer << projectivePos.x;
		writer << projectivePos.y;
	}
}

void xncv::SkeletonWriter::operator<<(const std::vector<User>& users)
{
	for (unsigned i = 0; i < users.size(); ++i)
		(*this) << users[i];
}

void xncv::SkeletonWriter::endFrame()
{
	++frame;
}

void xncv::SkeletonWriter::close()
{
	writer.close();
}

xncv::SkeletonWriter::~SkeletonWriter()
{
	if (writer.is_open())
		writer.close();
}