#include "ocl_render.h"
#include <iostream>
#include <vector>
#include "CL/opencl.hpp"
#include <fstream>

cl::Device easy_device(int index) {
	// First, you want to find OpenCL devices
	// But things are weird and you have to check for platforms first, which are basically just the drivers

	//get all platforms (drivers)
	std::vector<cl::Platform> all_platforms;
	cl::Platform::get(&all_platforms);
	if(all_platforms.size()==0){
		std::cout<<" No platforms found. Check OpenCL installation!\n";
		exit(1);
	}
	cl::Platform default_platform=all_platforms[index % all_platforms.size()];
	std::cout << "Using platform: "<<default_platform.getInfo<CL_PLATFORM_NAME>()<<"\n";

	//get default device of the default platform
	std::vector<cl::Device> all_devices;
	default_platform.getDevices(CL_DEVICE_TYPE_ALL, &all_devices);
	if(all_devices.size()==0){
		std::cout<<" No devices found. Check OpenCL installation!\n";
		exit(1);
	}
	cl::Device default_device=all_devices[0];
	std::cout<< "Using device: "<<default_device.getInfo<CL_DEVICE_NAME>()<<"\n";
	
	return default_device;
}

void clThreadMgr(int THREADS, threadSettings* settings, cl::Device device, cl::Context context, Uint32* pixelBuffer, SDL_Surface* surface, int iter, long double offsetX, long double offsetY, long double scale, int precision, std::complex<long double> juliaInit, bool mandelbrotSelect) {
	//cl::Context 
	std::cout << "nothing" << std::endl;
}

std::string slurp(std::ifstream& in) {
    std::ostringstream sstr;
    sstr << in.rdbuf();
    return sstr.str();
}
