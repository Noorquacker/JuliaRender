#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <complex>
#include <time.h>
#include <inttypes.h>
#include <SDL2/SDL_thread.h>
#include <unistd.h>
#include "ocl_render.h"
#include "CL/opencl.hpp"
#include "util.h"
#include <fstream>

using namespace std;


rgb hsv2rgb(hsv in) {
	double      hh, p, q, t, ff;
	long        i;
	rgb         out;
	if(in.s <= 0.0) {       // < is bogus, just shuts up warnings
		out.r = in.v;
		out.g = in.v;
		out.b = in.v;
		return out;
	}
	hh = in.h;
	if(hh >= 360.0) hh = 0.0;
	hh /= 60.0;
	i = (long)hh;
	ff = hh - i;
	p = in.v * (1.0 - in.s);
	q = in.v * (1.0 - (in.s * ff));
	t = in.v * (1.0 - (in.s * (1.0 - ff)));
	switch(i) {
		case 0:
			out.r = in.v;
			out.g = t;
			out.b = p;
		break;
		case 1:
			out.r = q;
			out.g = in.v;
			out.b = p;
		break;
		case 2:
			out.r = p;
			out.g = in.v;
			out.b = t;
		break;

		case 3:
			out.r = p;
			out.g = q;
			out.b = in.v;
		break;
		case 4:
			out.r = t;
			out.g = p;
			out.b = in.v;
		break;
		case 5:
		default:
			out.r = in.v;
			out.g = p;
			out.b = q;
		break;
	}
	return out;
}
//the rest is all my work though

void set_pixelBuffer(Uint32* pixelBuffer, int x, int y, Uint32 pixel) {
	Uint32 *target_pixel = (Uint32*)((Uint8*)pixelBuffer + y*1000*sizeof(Uint32) + x*sizeof(Uint32));
	*target_pixel = pixel;
}


template<class T> int test_julia(complex<T> z, complex<T> c, int i, int imax) {
	if(i == imax) {
		return imax;
	}
	complex<T> a = pow(c, 2) + z;
	if(abs(a) > 2) {
		return i;
	}
	else {
		return test_julia<T>(z, a, i+1, imax);
	}
}

template<class T> int test_mandelbrot(complex<T> z, complex<T> c, int i, int imax) {
	if(i == imax) {
		return imax;
	}
	else if(i == 0) {
		//If you want very weird results, remove this
		//Might've discovered a new field in math from buggy code idk
		z = 0;
	}
	complex<T> a = pow(z, 2) + c;
	if(abs(a) > 2) {
		return i;
	}
	else {
		return test_mandelbrot(a, c, i+1, imax);
	}
}

int threadedMandelbrot(void* settingsPt) {
	//cast settingsPt to threadSettings* and then dereference to make it a threadSettings
	threadSettings settings = *((threadSettings*)settingsPt);
	//run every nth line, where n = THREADS
	int (*testFloat)(complex<float>, complex<float>, int, int);
	int (*testDouble)(complex<double>, complex<double>, int, int);
	int (*testLong)(complex<long double>, complex<long double>, int, int);
	if(settings.mandelbrotSelect) {
		testFloat = &test_mandelbrot<float>;
		testDouble = &test_mandelbrot<double>;
		testLong = &test_mandelbrot<long double>;
	}
	else {
		testFloat = &test_julia<float>;
		testDouble = &test_julia<double>;
		testLong = &test_julia<long double>;
	}
	for(int y = settings.threadID; y <= 999; y += settings.threadTotal) {
		for(int x = 0; x <= 999; x++) {
			int mandel;
			//switch based on precision setting
			if(settings.precision == 0) {
				float scale = (float)settings.scale;
				float oX = (float)settings.offsetX, oY = (float)settings.offsetY;
				//I have no clue how this image coordinate calculation works anymore
				complex<float> c(scale*(x-499.)/250., scale*((y-499.)/-250.));
				c += complex<float>(oX/250., oY/-250);
				mandel = testFloat((complex<float>)settings.juliaInit,c,0,settings.iter);
			} else if (settings.precision == 1) {
				double scale = (double)settings.scale;
				double oX = (double)settings.offsetX, oY = (double)settings.offsetY;
				complex<double> c(scale*(x-499.)/250., scale*((y-499.)/-250.));
				c += complex<double>(oX/250., oY/-250);
				mandel = testDouble((complex<double>)settings.juliaInit,c,0,settings.iter);
			} else {
				long double scale = (long double)settings.scale;
				long double oX = (long double)settings.offsetX, oY = (long double)settings.offsetY;
				complex<long double> c(scale*(x-499.)/250., scale*((y-499.)/-250.));
				c += complex<long double>(oX/250., oY/-250.);
				mandel = testLong(settings.juliaInit,c,0,settings.iter);
			}
			//julia output 2 hsv
			hsv c2;
			c2.s = 1.0;
			c2.v = mandel == settings.iter ? 0 : 1;
			//c2.h = mandel/(float)settings.iter * 360;
			c2.h = fmod(mandel/50. * 360., 360.);
			//hsv 2 rgb
			rgb c1 = hsv2rgb(c2);
			int color = ((int)(255.*c1.r) << 16) + ((int)(255.*c1.g) << 8) + (int)(255.*c1.b);
			set_pixelBuffer(settings.pixelBuffer, x, y, color);
		}
	}
	return 0;
}

int main() {
	SDL_Window* window = NULL;
	SDL_Surface* surface = NULL;
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL couldn't initialize\n");
		return 1;
	}
	window = SDL_CreateWindow("JuliaRender", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1000, 1000, SDL_WINDOW_SHOWN);
	if(window == NULL) {
		printf("cant make window\n");
		return 1;
	}
	surface = SDL_GetWindowSurface(window);
	if(surface == NULL) {
		printf("cant get surface\n");
		return 1;
	}
	int THREADS = sysconf(_SC_NPROCESSORS_ONLN);
	printf("Detected %i threads\n", THREADS);

	
	// OpenCL go brrrrr
	bool cpu = false;
#ifdef __OPENCL_H
	cl::Device cl_device = easy_device(0);
	cl::Context context({cl_device});
	cl::Program::Sources sources;
	ifstream ifs("kernel_src.ocl");
	string src = slurp(ifs);
	sources.push_back({src.c_str(), src.length()});
	cl::Program program({context, src});
	if(program.build({cl_device}) != CL_SUCCESS){
		cpu = true;
		cerr << "***OpenCL compile error***\n" << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(cl_device) << endl;
	}
	cl::CommandQueue queue(context, cl_device);
	cl::Kernel render_kernel = cl::Kernel(program, "not_main");
	
	// wtf is a buffer???
	cl::Buffer settings_buffer(context, CL_MEM_READ_WRITE, sizeof(clSettings));
	cl::Buffer output_buffer(context, CL_MEM_READ_WRITE, sizeof(Uint32) * 1000000);
	
	int cl_threads = 32; // Idk why but everything is 32 core?
#endif
	
	//malloc the pixelBuffer
	//I know this isn't needed anymore but whatever
	Uint32* pixelBuffer = (Uint32*)malloc(sizeof(Uint32) * 1000000);
	bool enable = true;
	SDL_Event e;
	int start;
	int iter = 50;
	long double offsetX = 0, offsetY = 0;
	bool mandelbrotSelect = true;
	complex<long double> juliaInit = 0;
	long double scale = 1;
	int precision = 0;
	while(enable) {
		start = SDL_GetTicks();
		//store pthreads to detect when they stop after creating them
		SDL_Thread* threads[THREADS];
		//threadsettings MUST be stored per thread. otherwise one thread loses its settings and outputs black
		threadSettings settings[THREADS];
		
		for(int i = 0; i < THREADS; i++) {
			settings[i].threadTotal = THREADS;
			settings[i].threadID = i;
			settings[i].surface = surface;
			settings[i].iter = iter;
			settings[i].offsetX = offsetX;
			settings[i].offsetY = offsetY;
			settings[i].scale = scale;
			settings[i].precision = precision;
			settings[i].juliaInit = juliaInit;
			settings[i].pixelBuffer = pixelBuffer;
			settings[i].mandelbrotSelect = mandelbrotSelect;
			
		}
		if(cpu) {
			for(int i = 0; i < THREADS; i++) {
				threads[i] = SDL_CreateThread(threadedMandelbrot, "JuliaRender worker thread", (void*)&settings[i]);
			}
			for(int i = 0; i < THREADS; i++) {
				SDL_WaitThread(threads[i], NULL);
			}
		}
		else {
			clSettings cl_settings;
			cl_settings.mandelbrotSelect = mandelbrotSelect;
			cl_settings.iter = iter;
			cl_settings.jiI = juliaInit.imag();
			cl_settings.jiR = juliaInit.real();
			cl_settings.offsetX = offsetX;
			cl_settings.offsetY = offsetY;
			cl_settings.scale = scale;
			queue.enqueueWriteBuffer(settings_buffer, CL_TRUE, 0, sizeof(clSettings), &cl_settings);
			
			render_kernel.setArg(0, settings_buffer);
			render_kernel.setArg(1, output_buffer);
			queue.enqueueNDRangeKernel(render_kernel, cl::NullRange, cl::NDRange(cl_threads), cl::NullRange);
			
			queue.finish();
			queue.enqueueReadBuffer(output_buffer, CL_TRUE, 0, sizeof(Uint32)*1000000, pixelBuffer);
		}
		
		//add crosshair
		set_pixelBuffer(pixelBuffer, 500, 500, 16777215);
		set_pixelBuffer(pixelBuffer, 501, 501, 16777215);
		set_pixelBuffer(pixelBuffer, 499, 499, 16777215);
		set_pixelBuffer(pixelBuffer, 501, 499, 16777215);
		set_pixelBuffer(pixelBuffer, 499, 501, 16777215);
		//copy pixel buffer to sdl's pixel buffer
		//I know I don't actually need a pixel buffer and it actually adds overhead, but if you're that concerned with the 4mb of ram then stop using chrome
		memcpy(surface->pixels, pixelBuffer, sizeof(pixelBuffer[0]) * 1000000);

		SDL_UpdateWindowSurface(window);
		while(SDL_PollEvent(&e) != 0) {
			if(e.type == SDL_QUIT) {
				enable = false;
			}
			else if(e.type == SDL_KEYDOWN) {
				offsetX += scale * ((int)(e.key.keysym.sym == SDLK_RIGHT) - (int)(e.key.keysym.sym == SDLK_LEFT));
				offsetY += -1. * scale * ((int)(e.key.keysym.sym == SDLK_UP) - (int)(e.key.keysym.sym == SDLK_DOWN)); //Y is inverted, remember?
				scale = pow(2,log2(scale)+(int)(e.key.keysym.sym == SDLK_o)-(int)(e.key.keysym.sym == SDLK_l));
				//precision += (int)(e.key.keysym.sym == SDLK_y);
				mandelbrotSelect = e.key.keysym.sym == SDLK_h ? !mandelbrotSelect : mandelbrotSelect;
				cpu = e.key.keysym.sym == SDLK_y ? !cpu : cpu;
				iter += (int)(e.key.keysym.sym == SDLK_i) - (int)(e.key.keysym.sym == SDLK_k);
				juliaInit += complex<long double>(0.01, 0.01) * complex<long double>(((int)e.key.keysym.sym == SDLK_KP_6) - ((int)e.key.keysym.sym == SDLK_KP_4),((int)e.key.keysym.sym == SDLK_KP_8) - ((int)e.key.keysym.sym == SDLK_KP_2));
			}
		}
		iter = iter < 0 ? 0 : iter;
		//precision = precision > 2 ? 0 : precision;
		if(log2(scale) < -45.) {
			precision = 2;
		}
		else if(log2(scale) < -15.) {
			precision = 1;
		}
		else {
			precision = 0;
		}
		long double duration = (SDL_GetTicks() - start);
		int fps = 1000/duration;
		long int zoom = 1./scale;
		if(zoom < 1) {
			scale = 1;
			zoom = 1;
		}
		printf("FPS: %i at %i iterations, %lix zoom, offset (%Lg,%Lg), frametime %Lgms, juliaInit %Lg %Lg, precision level %i, CPU mode %i\n", fps, iter, zoom, offsetX, offsetY, duration, real(juliaInit), imag(juliaInit), precision, cpu);
	}
	free((void*)pixelBuffer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
