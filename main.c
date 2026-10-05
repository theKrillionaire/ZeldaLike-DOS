#include <stdio.h>
#include <graph.h>
#include <dos.h>
#include <conio.h>

#define KEYPORT 0x60
#define CONPORT 0x61
#define PICPORT 0x20

enum KEYCODES {
	KESC   = 1,
	KLEFT  = 75,
	KDOWN  = 80,
	KUP    = 72,
	KRIGHT = 77
};

struct Vector2 {
	int x;
	int y;
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

int main() {
	unsigned long tick = 0;
    unsigned long lastTick = 0;
	
	struct Vector2 playerPos;
	
	int playerDir = 0;
	
	static char playerSprs[4][512];
	
	playerPos.x = 320 / 2;
	playerPos.y = 200 / 2;
	
	
	if(_setvideomode(_MRES16COLOR) == 0) {
		printf("no available Video modes!!");
		return 0;
	}
	
	old_handler = _dos_getvect(0x09);
    _dos_setvect(0x09, new_handler);
	
	drawImage("dant3.spr", 0, 0, 16);
	_getimage(0,0,15,15, playerSprs[0]);
	drawImage("dant4.spr", 0, 0, 16);
	_getimage(0,0,15,15, playerSprs[1]);
	drawImage("dant5.spr", 0, 0, 16);
	_getimage(0,0,15,15, playerSprs[2]);
	drawImage("dant6.spr", 0, 0, 16);
	_getimage(0,0,15,15, playerSprs[3]);
	
	_clearscreen(_GCLEARSCREEN);
	
	_putimage(playerPos.x, playerPos.y, playerSprs[0], _GPSET);
	
	while(!keys[KESC]) {
		
		lastTick = tick;
		tick = *(unsigned long far *)MK_FP(0x40,0x6c);
		
		
		if(tick - lastTick >= 1) { 
			struct Vector2 pVelocity;
			
			pVelocity = getPlayerVel();
			
			if(pVelocity.x != 0 || pVelocity.y != 0) {
				_setcolor(0);
				_rectangle(_GFILLINTERIOR,playerPos.x,playerPos.y,playerPos.x + 16,playerPos.y + 16);
				
				playerPos.x += pVelocity.x;
				playerPos.y += pVelocity.y;
				
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
