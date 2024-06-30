#pragma once
#include <iostream>

//Defining pi
#define PI 3.14159265358979323846

//Defining the separation character
#ifdef _WIN32
	#define SEP_CHAR '\\'
#else
	#define SEP_CHAR '/'
#endif

//Define the name of the current file
#include <cstring>
#define SHORT_FILE (strrchr(__FILE__, SEP_CHAR) ? strrchr(__FILE__, SEP_CHAR) + 1 : __FILE__)

//Define error/warning macros
#define THROW_ERROR(message) \
{\
	std::cerr << "[RenderEngine Error] " << SHORT_FILE << " at line " << __LINE__ << " :" << std::endl; \
	std::cerr << message << std::endl; \
	std::cerr << std::endl; \
	throw std::runtime_error(message);\
}

#define WARN(message) \
{\
	std::cerr << "[RenderEngine Warning] " << __SHORT_FILE__ << " at line " << __LINE__ << " :" << std::endl;\
	std::cerr << "\t" << message << std::endl;\
}