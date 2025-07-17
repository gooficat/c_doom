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

#define PLAYER_SPEED 0.2
typedef struct {
	double vx, vy;
	double x, y, z,
	a, l;
} player_t;
player_t p;

typedef struct {
	int x1, y1,
		x2, y2;
	uint32_t color;
} wall_t;

typedef struct {
	int index, size;
	int z1, z2;
	int x, y;
	int d;
	int c1, c2;
	int surf[WIDTH];
	int surface;
} sector_t;

wall_t walls[30];
sector_t sectors[30];
int wall_count = 16;
int sector_count = 4;

int loadSectors[]=
{//wall start, wall end, z1 height, z2 height, bottom color, top color
 0,  4, 0, 40, 2,3, //sector 1
 4,  8, 0, 40, 4,5, //sector 2
 8, 12, 0, 40, 6,7, //sector 3
 12,16, 0, 40, 0,1, //sector 4
};

int loadWalls[]=
{//x1,y1, x2,y2, color
  0, 0, 32, 0, 0,
 32, 0, 32,32, 1,
 32,32,  0,32, 0,
  0,32,  0, 0, 1,

 64, 0, 96, 0, 2,
 96, 0, 96,32, 3,
 96,32, 64,32, 2,
 64,32, 64, 0, 3,

 64, 64, 96, 64, 4,
 96, 64, 96, 96, 5,
 96, 96, 64, 96, 4,
 64, 96, 64, 64, 5,

  0, 64, 32, 64, 6,
 32, 64, 32, 96, 7,
 32, 96,  0, 96, 6,
  0, 96,  0, 64, 7,
};

uint32_t colors[8] = {
	0xffffffff,
	0xff0000ff,
	0xff00ffff,
	0xffff0000,
	0xff00ff00,
	0xffffff00,
	0xffff00ff,
	0xaaffffaa
};

void init() {	
	p.x = 70;
	p.y = -110;
	p.z = 20;
	p.a = 0;
	p.l = 0;
	
	int v1 = 0,
		v2 = 0;
	
	for (int s = 0; s < sector_count; s++) {
		sectors[s].index = loadSectors[v1+0];
		sectors[s].size = loadSectors[v1+1];
		sectors[s].z1 = loadSectors[v1+2];
		sectors[s].z2 = loadSectors[v1+3] - loadSectors[v1+2];
		sectors[s].c1 = colors[loadSectors[v1+4]+1];
		sectors[s].c2 = colors[loadSectors[v1+5]+1];
		v1 += 6;
		
		for (int w = sectors[s].index; w < sectors[s].size; w++) {
			walls[w].x1 = loadWalls[v2+0];
			walls[w].y1 = loadWalls[v2+1];
			walls[w].x2 = loadWalls[v2+2];
			walls[w].y2 = loadWalls[v2+3];
			
			walls[w].color = colors[loadWalls[v2+4]+1];
			v2 += 5;
		}
	}
}

void clip(int* x1, int* y1, int* z1, int x2, int y2, int z2) {
	float da = *y1,
		  db =  y2;
	float d = da - db;if(!d)d=1;
	float s = da / (da - db);
	
	*x1 = *x1 + s * (x2 - (*x1));
	*y1 = *y1 + s * (y2 - (*y1)); if(!*y1)*y1=1;
	*z1 = *z1 + s * (z2 - (*z1));
}

void drawWall(int x1, int x2, int b1, int b2, int t1, int t2, uint32_t color, int sector_id) {

	int dyb = b2 - b1;
	int dyt = t2 - t1;
	int dx = x2 - x1; if(!dx)dx=1;
	int xs = x1;
	
	if(x1<1)x1=1;
	if(x2<1)x1=1;
	if(x1>WIDTH-1)x1=WIDTH-1;
	if(x2>WIDTH-1)x2=WIDTH-1;
	
	for (int x = x1; x < x2; x++) {
		int y1 = dyb * (x - xs + 0.5) / dx + b1;
		int y2 = dyt * (x - xs + 0.5) / dx + t1;

		if(y1<1)y1=1;
		if(y2<1)y1=1;
		if(y1>HEIGHT-1)y1=HEIGHT-1;
		if(y2>HEIGHT-1)y2=HEIGHT-1;
		
		if (sectors[sector_id].surface == 1) {
			sectors[sector_id].surf[x] = y1;
			continue;
		}
		if (sectors[sector_id].surface == 2) {
			sectors[sector_id].surf[x] = y2;
			continue;
		}
		if (sectors[sector_id].surface == -1) {
			for (int y = sectors[sector_id].surf[x]; y < y1; y++) {
				pix(x, y, sectors[sector_id].c2);
			}
		}
		if (sectors[sector_id].surface == -2) {
			for (int y = y1; y < sectors[sector_id].surf[x]; y++) {
				pix(x, y, sectors[sector_id].c2);
			}
		}
		

		for (int y = y1; y < y2; y++) {
			pix(x, y, color);
		}
	}
}

int distance(int x1, int y1, int x2, int y2) {
	return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
	
}

void update(float dt) {
	p.vx = 0;
	p.vy = 0;
	if (keys[SDL_SCANCODE_W])
		p.vy ++;
	if (keys[SDL_SCANCODE_S])
		p.vy --;
	if (keys[SDL_SCANCODE_A])
		p.vx --;
	if (keys[SDL_SCANCODE_D])
		p.vx ++;

	float cn = cos(p.a);
	float sn = sin(p.a);
	float nX = p.vx * cn + p.vy * sn;
	p.vy = -(p.vx * sn - p.vy * cn);
	p.vx = nX;
	
	p.x += p.vx * PLAYER_SPEED * dt;
	p.y += p.vy * PLAYER_SPEED * dt;

	if (keys[SDL_SCANCODE_SPACE])
		p.z -= PLAYER_SPEED * dt;
	if (keys[SDL_SCANCODE_LSHIFT])
		p.z += PLAYER_SPEED * dt;
	if (keys[SDL_SCANCODE_Q])
		p.a -= 0.002 * dt;
	if (keys[SDL_SCANCODE_E])
		p.a += 0.002 * dt;
	p.a = fmod(p.a, M_PI*2);
}

void render(float dt) {
	int wx[4], wy[4], wz[4];
	float cs = cos(p.a),
		  sn = sin(p.a);
	
	//sorting algorithm for the sectors
	for (int s = 0; s < sector_count - 1; s++) {
		for (int w = 0; w < sector_count - s - 1; w++) {
			if (sectors[w].d < sectors[w + 1].d) {
				sector_t st = sectors[w];
				sectors[w] = sectors[w+1];
				sectors[w+1] = st;
			}
		}
	}
	
	for (int s = 0; s < sector_count; s++) {
		sectors[s].d = 0;
		
		if (p.z < sectors[s].z1) sectors[s].surface = 1;
		else if (p.z > sectors[s].z2) sectors[s].surface = 2;
		else sectors[s].surface = 0;
		
		for (int i = 0; i < 2; i++) {
			for (int w = sectors[s].index; w < sectors[s].size; w++) {
				
				//transform around the player
				
				int x1 = walls[w].x1 - p.x,
					y1 = walls[w].y1 - p.y;
				
				int x2 = walls[w].x2 - p.x,
					y2 = walls[w].y2 -p.y;
				
				//if on the first draw, swap points
				if (!i) {
					int swp = x1;
					x1 = x2;
					x2 = swp;
					swp = y1;
					y1 = y2;
					y2 = swp;
				}
				
				wx[0] = x1 * cs - y1 * sn;
				wx[1] = x2 * cs - y2 * sn;
				wx[2] = wx[0];
				wx[3] = wx[4];
				
				wy[0] = y1 * cs + x1 * sn;
				wy[1] = y2 * cs + x2 * sn;
				wy[2] = wy[0];
				wy[3] = wy[1];
				
				sectors[s].d += distance(0, 0, (wx[0] + wx[1]) / 2, (wy[0] + wy[1]) / 2);
				
				wz[0] = sectors[s].z1 - p.z + ((p.l * wy[0])/32.0);
				wz[1] = sectors[s].z1 - p.z + ((p.l * wy[1])/32.0);
				wz[2] = wz[0] + sectors[s].z2;
				wz[3] = wz[1] + sectors[s].z2;
				
				//clip
				
				if (wy[0] < 1 && wy[1] < 1) continue;
				
				if (wy[0] < 1) {
					clip(&wx[0], &wy[0], &wz[0], wx[1], wy[1], wz[1]);
					clip(&wx[2], &wy[2], &wz[2], wx[3], wy[3], wz[3]);
				}
				
				if (wy[1] < 1) {
					clip(&wx[1], &wy[1], &wz[1], wx[0], wy[0], wz[0]);
					clip(&wx[3], &wy[3], &wz[3], wx[2], wy[2], wz[2]);
				}
				
				//screen space
				
				wx[0] = wx[0] * 200 / wy[0] + WIDTH/2;
				wy[0] = wz[0] * 200 / wy[0] + HEIGHT/2;
				
				wx[1] = wx[1] * 200 / wy[1] + WIDTH/2;
				wy[1] = wz[1] * 200 / wy[1] + HEIGHT/2;
				
				wx[2] = wx[2] * 200 / wy[2] + WIDTH/2;
				wy[2] = wz[2] * 200 / wy[2] + HEIGHT/2;
				
				wx[3] = wx[3] * 200 / wy[3] + WIDTH/2;
				wy[3] = wz[3] * 200 / wy[3] + HEIGHT/2;
				
				
				
				drawWall(wx[0], wx[1], wy[0], wy[1], wy[2], wy[3], walls[w].color, s);
				
			}
		}
		sectors[s].d /= (sectors[s].size - sectors[s].index);
		sectors[s].surface *= -1;
	}
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
			}
		}
		
		
		update(deltaTime);
		SDL_LockSurface(surface);
		memset(pixels, 0, WIDTH * HEIGHT * sizeof(uint32_t));
		render(deltaTime);
		SDL_UnlockSurface(surface);
		SDL_UpdateWindowSurface(window);
	}
	
	SDL_DestroyWindow(window);
	SDL_Quit();
}
