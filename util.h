#pragma once
#include <complex>
#include <SDL2/SDL.h>

struct threadSettings {
		int threadTotal;
		int threadID;
		SDL_Surface* surface;
		int iter;
		int precision;
		long double offsetX;
		long double offsetY;
		long double scale;
		std::complex<long double> juliaInit;
		Uint32* pixelBuffer;
		bool mandelbrotSelect;
};

// We can't use std::complex in this ;-;
// Wait can we?
// No
struct clSettings {
	int iter;
	double offsetX;
	double offsetY;
	double scale;
	double jiR;
	double jiI;
	bool mandelbrotSelect;
};

//yes this was all copied from stackoverflow
struct rgb {
		double r;       // a fraction between 0 and 1
		double g;       // a fraction between 0 and 1
		double b;       // a fraction between 0 and 1
};
struct hsv {
		double h;       // angle in degrees
		double s;       // a fraction between 0 and 1
		double v;       // a fraction between 0 and 1
};
