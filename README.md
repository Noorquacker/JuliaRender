# JuliaRender
Mandelbrot and Julia set renderer I wrote to learn multithreading in C/C++

Started in the Fall Break of my junior year of high school, a fresh, sweet pre-pandemic October 2019.

## How to use

- Arrow keys move you around the place
- Using `o` and `l` will zoom you out/in respectively
- Pressing `y` changes the precision level from `float` to `double` and `long double`. Check terminal output to see which precision you're at, or just notice the lag as you switch to `long double`
- Press `h` to switch between rendering the Mandelbrot set and rendering a Julia set
- While in a Julia set, the keypad keys 8, 4, 2, and 6 will change a complex number `juliaInit`. This is some "seed" number for choosing which Julia set to view. Very nice results.
- Use `i` and `k` to increase/decrease the amount of iterations, respectively. When reaching the max number of iterations, the code draws a pixel as black to let you know that it's hit its max.

Have fun with this. This was just to teach myself how to multithread intense algorithms in C and how to use SDL2. This was originally written in C, until I discovered C++ templates and immediately jumped on the bandwagon.
