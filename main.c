#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <math.h>
#include <stdio.h>

#define WIDTH 640
#define HEIGHT 480

SDL_Window* window;
SDL_Surface* surface;

uint32_t* pixels;
bool keys[256];

void pix(int x, int y, uint32_t c) {
	if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) return;
	pixels[x + WIDTH * y] = c;
}

#define PLAYER_SPEED 0.1

typedef struct {
	double x, y, z, a;
} player_t;

player_t p;

void init() {
	p.x = 70;
	p.y = -20;
	p.z = -110;
	p.a = 0;
}

void update(float dt) {
	if (keys[SDL_SCANCODE_W])
		p.z += PLAYER_SPEED * dt;
	if (keys[SDL_SCANCODE_S])
		p.z -= PLAYER_SPEED * dt;
	if (keys[SDL_SCANCODE_A])
		p.x -= PLAYER_SPEED * dt;
	if (keys[SDL_SCANCODE_D])
		p.x += PLAYER_SPEED * dt;
	if (keys[SDL_SCANCODE_SPACE])
		p.y -= PLAYER_SPEED * dt;
	if (keys[SDL_SCANCODE_LSHIFT])
		p.y += PLAYER_SPEED * dt;
	if (keys[SDL_SCANCODE_Q])
		p.a -= 0.002 * dt;
	if (keys[SDL_SCANCODE_E])
		p.a += 0.002 * dt;
	p.a = fmod(p.a, M_PI*2);
}

double a[4] = {
	40, 0, 10, 100
};
double b[4] = {
	40, 0, 290, 100
};

#define FOCAL 100
void project(double in[], player_t* p, int out[]) {
	double w[3] = {in[0] - p->x, in[1] - p->y, in[2] - p->z};
	double v[4];
	v[0] = w[0] * cos(p->a) - w[2] * sin(p->a);
	v[1] = w[1];
	v[2] = w[2] * cos(p->a) + w[0] * sin(p->a);
	v[3] = in[3] - p->y;
	
	
	out[0] = v[0] * FOCAL/v[2]+(WIDTH/2);
	out[1] = v[1] * FOCAL/v[2]+(HEIGHT/2);
	out[2] = v[3] * FOCAL/v[2]+(HEIGHT/2);
}

void drawWall(int x1, int x2, int b1, int b2, int t1, int t2) {
	int x,y;
	
	if (x1 > x2) {
		int q = x2;
		x2 = x1;
		x1 = q;
		
		q = b2;
		b2 = b1;
		b1 = q;

		q = t2;
		t2 = t1;
		t1 = q;		
	}
	
	int dyb = b2 - b1;
	int dyt = t2 - t1;
	int dx = x2 - x1; if (dx==0) dx=1;
	int xs = x1;
	
	for (x = x1; x < x2; x++) {
		int y1 = dyb * (x - xs + 0.5) / dx + b1;
		int y2 = dyt * (x - xs + 0.5) / dx + t1;
		for (y = y1; y < y2; y++) {
			pix(x, y, 0xffff00ff);
		}
	}
}


void render(float dt) {
	int x[3];
	int y[3];
	
	project(a, &p, x);
	project(b, &p, y);
	
	drawWall(x[0], y[0], x[1], y[1], x[2], y[2]);
}


int main() {
	SDL_Init(SDL_INIT_VIDEO);
	window = SDL_CreateWindow("", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
	surface = SDL_GetWindowSurface(window);
	pixels = (uint32_t*)surface->pixels;
	init();
	
	float frameTime = SDL_GetTicks();
	float lastFrameTime = frameTime;
	float deltaTime = 0;
	
	bool quit = false;
	SDL_Event e;
	while (!quit) {
		frameTime = SDL_GetTicks();
		deltaTime = frameTime - lastFrameTime;
		lastFrameTime = frameTime;
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT)
				quit = true;
			else if (e.type == SDL_KEYDOWN)
				keys[e.key.keysym.scancode] = true;
			else if (e.type == SDL_KEYUP)
				keys[e.key.keysym.scancode] = false;
		}
		update(deltaTime);
		SDL_LockSurface(surface);
		memset(pixels, 0xff000000, WIDTH * HEIGHT * sizeof(uint32_t));
		render(deltaTime);
		SDL_UnlockSurface(surface);
		SDL_UpdateWindowSurface(window);
	}
	
	SDL_DestroyWindow(window);
	SDL_Quit();
}