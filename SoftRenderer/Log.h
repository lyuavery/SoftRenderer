#pragma once
#include <stdio.h>
#include <string>
#define XLogInfo(fmt, ...) { \
	fprintf(stderr, std::string("[INFO][File:%s][Line:%d]\n:").append(fmt).append("\n").c_str(), __FILE__, __LINE__, __VA_ARGS__); \
}

#define XLogWarning(fmt, ...) { \
	fprintf(stderr, std::string("[WARNING][File:%s][Line:%d]\n:").append(fmt).append("\n").c_str(), __FILE__, __LINE__, __VA_ARGS__); \
}

#define XLogError(fmt, ...) { \
	fprintf(stderr, std::string("[ERROR][File:%s][Line:%d]\n:").append(fmt).append("\n").c_str(), __FILE__, __LINE__, __VA_ARGS__); \
}

//
//namespace SR
//{
//	class Logger
//	{
//		static const int DEFAULT_RESERVE = 512;
//	private:
//	public:
//		void LogInfo(char* fmt, ...)
//		{
//			auto fd = stdout;
//			const char* tag = "INFO";
//
//			std::string extFmt("[");
//			extFmt.reserve(DEFAULT_RESERVE);
//			extFmt = extFmt + tag + "] " + __FILE__ + ": Line " + std::to_string(__LINE__) + ":\n" + fmt;
//			va_list args;
//			va_start(args, fmt);
//			vfprintf(fd, extFmt.c_str(), args);
//			va_end(args);
//		}
//
//		void LogWarning(char* fmt, ...)
//		{
//			auto fd = stdout;
//			const char* tag = "WARNING";
//
//			std::string extFmt("[");
//			extFmt.reserve(DEFAULT_RESERVE);
//			extFmt = extFmt + tag + "] " + __FILE__ + ": Line " + std::to_string(__LINE__) + ":\n" + fmt;
//			va_list args;
//			va_start(args, fmt);
//			vfprintf(fd, extFmt.c_str(), args);
//			va_end(args);
//		}
//
//		void LogError(char* fmt, ...)
//		{
//			auto fd = stderr;
//			const char* tag = "ERROR";
//
//			std::string extFmt("[");
//			extFmt.reserve(DEFAULT_RESERVE);
//			extFmt = extFmt + tag + "] " + __FILE__ + ": Line " + std::to_string(__LINE__) + ":\n" + fmt;
//			va_list args;
//			va_start(args, fmt);
//			vfprintf(fd, extFmt.c_str(), args);
//			va_end(args);
//		}
//	};
//
//}