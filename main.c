#include <stdio.h>
#include <graph.h>
#include <dos.h>
#include <conio.h>

#define KEYPORT 0x60
#define CONPORT 0x61
#define PICPORT 0x20

#define DOWNSPRITE "dant3.spr"

enum KEYCODES {
	KESC   = 0x12,
	KLEFT  = 0x4B,
	KDOWN  = 0x50,
	KUP    = 0x48,
	KRIGHT = 0x4D
};

enum BOOL {
	false,
	true
};

struct Vector2 {
	int x;
	int y;
};

struct Rectangle {
	struct Vector2 pos;
	int width;
	int height;
};

volatile char keys[128] = {0};

void interrupt (*old_handler)(void);

void interrupt new_handler() {
	unsigned char scancode;
	unsigned char status = 0;
	
	status = inp(CONPORT);
	scancode = inp(KEYPORT);
	outp(CONPORT, status | 0x80);
	outp(CONPORT, status);
	
	if(scancode & 0x80) {
		keys[scancode & 0x7F] = 0;
	} else {
		keys[scancode] = 1;
	}
	outp(PICPORT, 0x20);
}



void drawImage(char* image, int posX, int posY, int size) {
	int sprSize = size * size;
	FILE* outbin;
	
    outbin = fopen(image, "rb");

    if (outbin != NULL) {
    	int i = 0;
		int color = 0;
        while((color = fgetc(outbin)) != EOF) {
        _setcolor(color);
        _setpixel(i % size + posX, i / size + posY);
		i++;
        }
		fclose(outbin);
    }
	

}

struct Vector2 getPlayerVel() {
	struct Vector2 vel;
	
	if(keys[KLEFT]) vel.x = -2;
	else if(keys[KRIGHT]) vel.x = 2;
	else vel.x = 0;
	if(keys[KUP]) vel.y = -2;
	else if(keys[KDOWN]) vel.y = 2;
	else vel.y = 0;
	
	return vel;
}

int checkCollision(struct Rectangle* Rect1, struct Rectangle* Rect2) {
	if(
		Rect1->pos.x < Rect2->pos.x + Rect2->width &&
		Rect1->pos.x + Rect1->width > Rect2->pos.x &&
		Rect1->pos.y < Rect2->pos.y + Rect2->height &&
		Rect1->pos.y + Rect1->height > Rect2->pos.y
	) {
		return true;
	} else {
		return false;
	}
}

int loadMap(char mapData[240], char* fileName) {
	FILE* mapFile = fopen(fileName, "r");
	int i = 0;
	
	if(mapFile != NULL) {
		int data = fgetc(mapFile);
		for(i; i < 240; i++) {
			if(data == EOF) {
				mapData[i] = EOF;
				return 1;
				break;
			} else {
				mapData[i] = data;
			}
		}
	}
	return 0;
}

void drawMap(char mapData[240]) {
	int i = 0;
	for(i; i < 240; i++) {
		int x = i % 16;
		int y = i / 16;
		if(mapData[i] == 1) _rectangle(_GFILLINTERIOR, x * 16, y * 16, x * 16 + 15, y * 16 + 15);
	}
}

int main() {
	unsigned long tick = 0;
    unsigned long lastTick = 0;
	
	struct Vector2 playerPos = {320 / 2, 200 / 2};
	int playerDir = 0;
	static char playerSprs[4][512];
	int wall = 0;
	int i = 0;
	char ball[512];
	
	
	struct Rectangle walls[4] = {
   		{{0,   0}, 16, 200},
   		{{0,   0}, 320, 16},
    	{{304, 0}, 16, 320},
    	{{0, 184}, 320, 16}
	};
	
	
	if(_setvideomode(_MRES16COLOR) == 0) {
		printf("no available Video modes!!");
		return 0;
	}
	
	old_handler = _dos_getvect(0x09);
    _dos_setvect(0x09, new_handler);
	
	drawImage("sprites/dant3.spr", 0, 0, 16);
	_getimage(0,0,15,15, playerSprs[0]);
	drawImage("sprites/dant4.spr", 0, 0, 16);
	_getimage(0,0,15,15, playerSprs[1]);
	drawImage("sprites/dant5.spr", 0, 0, 16);
	_getimage(0,0,15,15, playerSprs[2]);
	drawImage("sprites/dant6.spr", 0, 0, 16);
	_getimage(0,0,15,15, playerSprs[3]);
	
	
	
	_clearscreen(_GCLEARSCREEN);
	
	drawImage("sprites/ball.spr", 0, 0, 16);
	_getimage(0,0,15,15, ball);
	
	for(i; i < 20; i++) {
		if( i < 12 ) {
			_putimage(0, i * 16, ball, _GPSET);
			_putimage(304, i * 16, ball, _GPSET);
			
		}
		_putimage(i * 16, 0, ball, _GPSET);
		_putimage(i * 16, 184, ball, _GPSET);
	}
	
	_putimage(playerPos.x, playerPos.y, playerSprs[0], _GPSET);
	
	while(!keys[KESC]) {
		
		lastTick = tick;
		tick = *(unsigned long far *)MK_FP(0x40,0x6c);
		
		if(tick - lastTick >= 1) { 
			struct Vector2 pVelocity;
			struct Vector2 touching = {false, false};
			int i = 0;
			struct Rectangle playerColX = {{0,0}, 16, 16};
			struct Rectangle playerColY = {{0,0}, 16, 16};
			
			pVelocity = getPlayerVel();
			
			playerColX.pos.x = playerPos.x + pVelocity.x;
			playerColX.pos.y = playerPos.y;
			playerColY.pos.x = playerPos.x;
			playerColY.pos.y = playerPos.y + pVelocity.y;
			
			if(pVelocity.x != 0 || pVelocity.y != 0) {
				_setcolor(0);
				_rectangle(_GFILLINTERIOR,playerPos.x,playerPos.y,playerPos.x + 15,playerPos.y + 15);
				for(i; i < 4; i++) {
					if(checkCollision(&playerColX, &walls[i])) {
						touching.x = true;
					}
					if(checkCollision(&playerColY, &walls[i])) {
						touching.y = true;
					}
				} 
				
				playerPos.x += !touching.x * pVelocity.x ;
				playerPos.y += !touching.y * pVelocity.y;
				
				if(pVelocity.y < 0) playerDir = 3;
				else if(pVelocity.x < 0) playerDir = 2;
				else if(pVelocity.x > 0) playerDir = 1;
				else if(pVelocity.y > 0) playerDir = 0;

				_putimage(playerPos.x, playerPos.y, playerSprs[playerDir], _GPSET);
			}
		}
	}
	
	_dos_setvect(0x09, old_handler);
	_setvideomode(_DEFAULTMODE);
	return 0;
}
