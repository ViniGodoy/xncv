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

#if !defined(__SKELETON_IO_HPP__)
#define __SKELETON_IO_HPP__

#include <stdexcept>
#include "user.hpp"
#include <fstream>

namespace xncv
{
	class VideoSource;

	const unsigned MAGIC = 0x76436E58;
	const unsigned short VERSION = 1;
	
	struct ProjectiveJoint
	{		
		cv::Point position;
		float fConfidence;
	};

	class UserInformation
	{
		friend class SkeletonReader;

		private:
			XnUserID id;
			std::map<XnSkeletonJoint, XnSkeletonJointTransformation> worldJoints;
			std::map<XnSkeletonJoint, ProjectiveJoint> projectiveJoints;			

		public:
			UserInformation(XnUserID uid);
			XnUserID getId() const;
						
			const std::map<XnSkeletonJoint, XnSkeletonJointTransformation>& getJoints() const;
			std::vector<Limb> getLimbs() const;
	};

	class SkeletonReader
	{
		private:			
			std::map<int, std::vector<UserInformation> > users;
			const std::vector<UserInformation> empty;
			int frames;

		public:
			SkeletonReader();			
			void open(const std::string& fileName);

			int frameCount() const;
			const std::vector<UserInformation>& getUsers(int frame) const;
	};

	class SkeletonWriter
	{	
		private:
			std::fstream writer;
			xn::DepthGenerator* depthGen;
			int frame;

		public:
			SkeletonWriter(VideoSource& generator);
			void open(const std::string& fileName);
			bool isOpen() const;
			inline void beginFrame() {}
			void operator << (const User& user);
			void operator << (const std::vector<User>& users);
			void endFrame();
			void close();
			~SkeletonWriter();
	};
};

#endif
