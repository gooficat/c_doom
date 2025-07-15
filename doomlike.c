#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <math.h>
#include <stdio.h>

#define WIDTH 640
#define HEIGHT 480

SDL_Window* window;
SDL_Surface* surface;

SDL_GameController* controller;

uint32_t* pixels;
bool keys[256];
bool gamepad_buttons[24];

void pix(int x, int y, uint32_t c) {
	if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) return;
	pixels[x + WIDTH * y] = c;
}

#define PLAYER_SPEED 0.2

typedef struct {
	double x, y;
} vec2_t;

typedef struct {
	double x, y, z;
} vec3_t;

typedef struct {
	vec3_t pos;
	vec2_t rot;
	vec3_t vel;
} player_t;

vec2_t LEFTSTICKAXIS;

player_t p;


SDL_GameController* findController() {
	for (int i = 0; i < SDL_NumJoysticks(); i++) {
		if (SDL_IsGameController(i))
			return SDL_GameControllerOpen(i);
	}
}

void init() {
	controller = findController();
	
	p.pos = (vec3_t){70, 20, -110};
}

void update(float dt) {
	p.vel.x = 0;
	p.vel.y = 0;
	
	if (controller) {
		p.vel.x = LEFTSTICKAXIS.x / 32767;
		p.vel.y = LEFTSTICKAXIS.y / 32767;
		
	}
	else {
		if (keys[SDL_SCANCODE_W])
			p.vel.y --;
		if (keys[SDL_SCANCODE_S])
			p.vel.y ++;
		if (keys[SDL_SCANCODE_A])
			p.vel.x ++;
		if (keys[SDL_SCANCODE_D])
			p.vel.x --;
	}
	
	p.pos.x += p.vel.x * PLAYER_SPEED * dt;
	p.pos.y += p.vel.y * PLAYER_SPEED * dt;
	

	if (keys[SDL_SCANCODE_SPACE])
		p.pos.z += PLAYER_SPEED * dt;
	if (keys[SDL_SCANCODE_LSHIFT])
		p.pos.z -= PLAYER_SPEED * dt;
	if (keys[SDL_SCANCODE_Q])
		p.rot.y -= 0.002 * dt;
	if (keys[SDL_SCANCODE_E])
		p.rot.y += 0.002 * dt;
	p.rot.y = fmod(p.rot.y, M_PI*2);
}

typedef struct {
	vec2_t a, b;
} wall_t;

typedef struct {
	wall_t walls[12];
	size_t wall_count;
	int top, bottom;
} sector_t;

typedef struct {
	vec3_t a, b;
} rquad_t;

wall_t w1 = {
	.a = {40, 0},
	.b = {10, 100}
};
wall_t w2 = {
	.a = {40, 0},
	.b = {290, 100}
};


void drawWall(vec2_t a, vec2_t b) {
	pix(a.x, a.y, 0xffffffff);
	pix(b.x, b.y, 0xff00ffff);
}

double cs, sn;

vec3_t transform(vec3_t v) {
	v.x -= p.pos.x;
	v.y -= p.pos.y;
	v.z -= p.pos.z;
	
	return (vec3_t){
		v.x * cs - v.y * sn,
		v.y * cs + v.x * sn,
		v.z
	};
}

vec2_t screenSpace(vec3_t v) {
	return (vec2_t){
		v.x * 200 / v.y + WIDTH/2,
		v.z * 200 / v.y + HEIGHT/2
	};
}

void render(float dt) {
	cs = cos(p.rot.y);
	sn = sin(p.rot.y);
	rquad_t t1, t2;
	t1.a = transform((vec3_t){w1.a.x, w1.a.y, 0});
	t1.b = transform((vec3_t){w1.b.x, w1.b.y, 0});
	
	wall_t w1;
	w1.a = screenSpace(t1.a);
	w1.b = screenSpace(t1.b);
	
	drawWall(w1.a, w1.b);
	
}


int main() {
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER);
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
			switch (e.type) {
				case SDL_QUIT:
					quit = true;
					break;
				case SDL_KEYDOWN:
					keys[e.key.keysym.scancode] = true;
					break;
				case SDL_KEYUP:
					keys[e.key.keysym.scancode] = false;
					break;

				case SDL_CONTROLLERDEVICEADDED:
					if (!controller) {
						controller = SDL_GameControllerOpen(e.cdevice.which);
					}
					break;
				case SDL_CONTROLLERDEVICEREMOVED:
					if (controller && e.cdevice.which == SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(controller))) {
						SDL_GameControllerClose(controller);
						controller = findController();
					}
					break;
				case SDL_CONTROLLERBUTTONDOWN:
					if (controller && e.cdevice.which == SDL_JoystickInstanceID(
						SDL_GameControllerGetJoystick(controller))) {
							gamepad_buttons[e.cbutton.button] = true;
						}
					break;
				case SDL_CONTROLLERBUTTONUP:
					if (controller && e.cdevice.which == SDL_JoystickInstanceID(
						SDL_GameControllerGetJoystick(controller))) {
							gamepad_buttons[e.cbutton.button] = false;
						}
					break;
				case SDL_CONTROLLERAXISMOTION:
					switch (e.jaxis.type) {
						case SDL_CONTROLLER_AXIS_LEFTX:
							LEFTSTICKAXIS.x = e.jaxis.value;
							break;
						case SDL_CONTROLLER_AXIS_LEFTY:
							LEFTSTICKAXIS.y = e.jaxis.value;
							break;
					}
					break;
			}
		}
		
		
		
		update(deltaTime);
		SDL_LockSurface(surface);
		memset(pixels, 0xff333333, WIDTH * HEIGHT * sizeof(uint32_t));
		render(deltaTime);
		SDL_UnlockSurface(surface);
		SDL_UpdateWindowSurface(window);
	}
	
	SDL_DestroyWindow(window);
	SDL_Quit();
}