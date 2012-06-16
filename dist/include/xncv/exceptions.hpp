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

#if !defined(__EXCEPTIONS_HPP__)
#define __EXCEPTIONS_HPP__

#include <stdexcept>

namespace xncv
{
	class Exception : public std::logic_error
	{
		private:
			XnStatus status;

		public:
            Exception(const char* what, int _status=XN_STATUS_OK) : logic_error(what), status(_status) {}
            Exception(const std::string& what, int _status=XN_STATUS_OK) : logic_error(what), status(_status) {}
			XnStatus getStatus() { return status; }
	};

	class UnableToOpenFileException : public Exception
	{
		private:
			std::string filename;
		public:
			UnableToOpenFileException(const std::string& file) : Exception(std::string("Unable to open file: " + file)), filename(file) {}
			const std::string& getFileName() const { return filename; }
	};

	class UnableToInitGenerator : public Exception
	{
		public:
			UnableToInitGenerator(const char* what) : Exception(what) {}
			UnableToInitGenerator(const std::string& what) : Exception(what) {}
	};

	class GeneratorError : public Exception
	{
		public:
			GeneratorError(const char* what) : Exception(what) {}
			GeneratorError(const std::string& what) : Exception(what) {}
	};

	class VideoSourceException : public Exception
	{
		public:
			VideoSourceException(const char* what, XnStatus status=XN_STATUS_OK) : Exception(what, status) {}
			VideoSourceException(const std::string& what, XnStatus status=XN_STATUS_OK) : Exception(what, status) {}
	};

	class FrameSkipException : public VideoSourceException
	{
		private:
			int frame;					

		public:
            FrameSkipException(const char* what, int frameNumber, int status) : VideoSourceException(what, status), frame(frameNumber) {}
            FrameSkipException(const std::string& what, int frameNumber, int status) : VideoSourceException(what, status), frame(frameNumber) {}
			int getFrame() { return frame; }
	};

	class NoCapabilityException : public Exception
	{
		public:
			NoCapabilityException(const char* what, XnStatus status=XN_STATUS_OK) : Exception(what, status) {}
			NoCapabilityException(const std::string& what, XnStatus status=XN_STATUS_OK) : Exception(what, status) {}
	};

	class UserNotFoundException : public Exception
	{
		private:
			XnUserID id;
		public:
			UserNotFoundException(XnUserID _id) : id(_id), Exception("User not found") {}			
			XnUserID getId() { return id; }
	};
}

#endif
