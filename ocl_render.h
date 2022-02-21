#pragma once

#include <iostream>
#include <vector>
#include "CL/opencl.hpp"
#include "util.h"
#include <SDL2/SDL.h>
#include <complex>

cl::Device easy_device(int index=0);

void clThreadMgr(int THREADS, threadSettings* settings, cl::Device device, cl::Context context, Uint32* pixelBuffer, SDL_Surface* surface, int iter, long double offsetX, long double offsetY, long double scale, int precision, std::complex<long double> juliaInit, bool mandelbrotSelect);

std::string slurp(std::ifstream& in);
