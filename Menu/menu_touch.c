#include "XPT2046.h"
#include "menu_touch.h"
#include <math.h>


void menu_touch_init(){
	XPT2046_Init();  //Programmer has to provide this function
}

void menu_get_touch_coordinates(uint16_t* X, uint16_t* Y){
	get_touch_coordinates(X, Y);	//by user
}

uint8_t menu_touch_pressed(){
	return XPT2046_pressed();	//by user
}

touch_gesture menu_touch_gesture(uint16_t* x, uint16_t* y){
	uint16_t X1, Y1, X2, Y2;
	int16_t DX, DY;
	if(menu_touch_pressed()){
		menu_get_touch_coordinates(&X1, &Y1);
		while(menu_touch_pressed()){
			menu_get_touch_coordinates(&X2, &Y2);
		}
		
		DX = (X2-X1);
		DY = (Y2-Y1);
		if(fabs(DX) > 60 || fabs(DY) > 60){
			if(fabs(DX) > fabs(DY)){
				if(DX > 60){
					return TOUCH_RIGHT;
				}
				if(DX < (-60)){
					return TOUCH_LEFT;
				}
			}
			if(fabs(DX) < fabs(DY)){
				if(DY > 60){
					return TOUCH_DOWN;
				}
				if(DY < (-60)){
					return TOUCH_UP;
				}
			}
		}
		else{
			*x = X2;
			*y = Y2;
			return TOUCH_CLICK;
		}		
	}
	return TOUCH_NONE;
}