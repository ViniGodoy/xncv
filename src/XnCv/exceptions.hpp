#if !defined(__EXCEPTIONS_HPP__)
#define __EXCEPTIONS_HPP__

#include <stdexcept>

namespace xncv
{
	class Exception : public std::logic_error
	{
		public:
			Exception(const char* what) : logic_error(what) {}
			Exception(const std::string& what) : logic_error(what) {}			
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
}

#endif