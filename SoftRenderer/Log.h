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