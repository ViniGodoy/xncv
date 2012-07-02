#include "skeletonio.hpp"
#include "exceptions.hpp"
#include "videosource.hpp"

//-----------------------------------------------------------------------------
//Auxiliary read/write functions
//-----------------------------------------------------------------------------
void write(std::fstream& writer, unsigned char value) {	writer.write((char*)&value, sizeof(unsigned char)); }
void write(std::fstream& writer, unsigned short value) { writer.write((char*)&value, sizeof(unsigned short)); }
void write(std::fstream& writer, unsigned int value) { writer.write((char*)&value, sizeof(unsigned int)); }
void write(std::fstream& writer, unsigned long value) { writer.write((char*)&value, sizeof(unsigned long)); }
void write(std::fstream& writer, bool value) { writer.write((char*)&value, sizeof(bool)); }
void write(std::fstream& writer, float value) { writer.write((char*)&value, sizeof(float)); }
void write(std::fstream& writer, double value) { writer.write((char*)&value, sizeof(double)); }
void write(std::fstream& writer, char value) {	writer.write((char*)&value, sizeof(char)); }
void write(std::fstream& writer, short value) { writer.write((char*)&value, sizeof(short)); }
void write(std::fstream& writer, int value) { writer.write((char*)&value, sizeof(int)); }
void write(std::fstream& writer, long value) { writer.write((char*)&value, sizeof(long)); }

void read(std::fstream& writer, unsigned char &value) {	writer.read((char*)&value, sizeof(unsigned char)); }
void read(std::fstream& writer, unsigned short &value) { writer.read((char*)&value, sizeof(unsigned short)); }
void read(std::fstream& writer, unsigned int &value) { writer.read((char*)&value, sizeof(unsigned int)); }
void read(std::fstream& writer, unsigned long &value) { writer.read((char*)&value, sizeof(unsigned long)); }
void read(std::fstream& writer, bool &value) { writer.read((char*)&value, sizeof(bool)); }
void read(std::fstream& writer, float &value) { writer.read((char*)&value, sizeof(float)); }
void read(std::fstream& writer, double &value) { writer.read((char*)&value, sizeof(double)); }
void read(std::fstream& writer, char &value) {	writer.read((char*)&value, sizeof(char)); }
void read(std::fstream& writer, short &value) { writer.read((char*)&value, sizeof(short)); }
void read(std::fstream& writer, int &value) { writer.read((char*)&value, sizeof(int)); }
void read(std::fstream& writer, long &value) { writer.read((char*)&value, sizeof(long)); }

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
xncv::SkeletonReader::SkeletonReader() : frames(0)
{
}

void xncv::SkeletonReader::open(const std::string& filename)
{
	using namespace std;

	fstream reader;
	reader.open(filename, fstream::in | fstream::binary);

	unsigned magic;
	read(reader, magic);
	if (magic != MAGIC)
		throw new xncv::IOException("Invalid file", filename);

	unsigned short version;
	read(reader, version);
	if (version > VERSION)
		throw new xncv::IOException("Version bigger than expected. Please update your xncv library.", filename);


	while (!reader.eof())
	{
		int frame=0;
		read(reader, frame);
		if (frame > frames) frames = frame;

		XnUserID userId=0;
		read(reader, userId);
		UserInformation userInfo(userId);

		unsigned short jointsSize=0;
		read(reader, jointsSize);
		for (int i = 0; i < jointsSize; ++i)
		{
			//Joint type
			unsigned short type=0;
			read(reader, type);
			XnSkeletonJoint jointType = static_cast<XnSkeletonJoint>(type);

			//World position
			XnSkeletonJointTransformation transform;
			read(reader, transform.position.position.X);
			read(reader, transform.position.position.Y);
			read(reader, transform.position.position.Z);
			read(reader, transform.position.fConfidence);

			//World orientation
			for (int i = 0; i < 9; ++i)
				read(reader, transform.orientation.orientation.elements[i]);
			read(reader, transform.orientation.fConfidence);

			//Projective position
			ProjectiveJoint projectiveJoint;
			read(reader, projectiveJoint.position.x);
			read(reader, projectiveJoint.position.y);
			projectiveJoint.fConfidence = transform.position.fConfidence;

			userInfo.worldJoints[jointType] = transform;
			userInfo.projectiveJoints[jointType] = projectiveJoint;			
		}
		users[frame].push_back(userInfo);
	}
}

int xncv::SkeletonReader::frameCount() const
{
	return frames;
}

const std::vector<xncv::UserInformation>& xncv::SkeletonReader::getUsers(int frame) const
{
	if (frame < 0 || frame >= frameCount())
		return empty;

	return users.find(frame) == users.end() ? empty : users.at(frame);
}

//-----------------------------------------------------------------------------
//Skeleton writer
//-----------------------------------------------------------------------------
xncv::SkeletonWriter::SkeletonWriter(xncv::VideoSource& videoSource)
	: writer(), depthGen(&(videoSource.getXnDepthGenerator())), frame(0)
{
	writer.exceptions(std::fstream::failbit | std::fstream::badbit);
}

void xncv::SkeletonWriter::open(const std::string& fileName)
{
	writer.open(fileName, std::fstream::out | std::fstream::trunc | std::fstream::binary);
	frame = 0;
	write(writer, MAGIC);
	write(writer, VERSION); //File version
}

bool xncv::SkeletonWriter::isOpen() const
{
	return writer.is_open();
}

void xncv::SkeletonWriter::operator<<(const User& user)
{
	if (!isOpen() || !user.isTracking() || user.isCalibrating())
		return;

	write(writer, frame);
	write (writer, user.getId());
	auto joints = user.getJoints();
	unsigned short jointsSize = static_cast<unsigned short>(joints.size());
	write(writer, jointsSize);	
	for (auto it = joints.cbegin(); it != joints.cend(); ++it)
	{
		//Joint type
		write(writer, static_cast<unsigned short>(it->first));

		//World position
		write(writer, it->second.position.position.X);
		write(writer, it->second.position.position.Y);
		write(writer, it->second.position.position.Z);
		write(writer, it->second.position.fConfidence);

		//World orientation
		for (int i = 0; i < 9; ++i)
			write(writer, it->second.orientation.orientation.elements[i]);
		write(writer, it->second.orientation.fConfidence);

		//Projective position
		cv::Point projectivePos = xncv::worldToProjective(it->second.position.position, *depthGen);
		write(writer, projectivePos.x);
		write(writer, projectivePos.y);
	}
}

void xncv::SkeletonWriter::operator<<(const std::vector<User>& users)
{
	for (unsigned i = 0; i < users.size(); ++i)
		(*this) << users[i];
}

void xncv::SkeletonWriter::endFrame()
{
	if (!isOpen()) return;
	++frame;
	writer.flush();
}

void xncv::SkeletonWriter::close()
{
	if (!isOpen()) return;
	writer.flush();
	writer.close();
}

xncv::SkeletonWriter::~SkeletonWriter()
{
	close();
}