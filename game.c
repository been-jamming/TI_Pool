#define USE_TI89
#define SAVE_SCREEN

#include <tigcclib.h>
#include "extgraph.h"
#include "physics.h"
#include "graphics.h"
#include "ai.h"

volatile unsigned char quit;
INT_HANDLER old_int_5;
volatile unsigned char do_update;
uint16_t global_angle;
unsigned char global_power;

void select_cue_ball_path(){
	unsigned char prev_f1_pressed = 0;
	unsigned char f1_pressed;
	unsigned char precision = 10;

	global_angle = 0;
	do_update = 1;

	light_plane = GrayDBufGetHiddenPlane(LIGHT_PLANE);
	dark_plane = GrayDBufGetHiddenPlane(DARK_PLANE);
	FastFilledRect_Erase_R(light_plane, 2, 2, 34, 8);
	FastFilledRect_Erase_R(dark_plane, 2, 2, 34, 8);
	GrayDBufToggle();
	light_plane = GrayDBufGetHiddenPlane(LIGHT_PLANE);
	dark_plane = GrayDBufGetHiddenPlane(DARK_PLANE);
	FastFilledRect_Erase_R(light_plane, 2, 2, 34, 8);
	FastFilledRect_Erase_R(dark_plane, 2, 2, 34, 8);
	while(1){
		if(do_update){
			do_update = 0;

			f1_pressed = _keytest(RR_F1);
			if(precision == 10 && f1_pressed && !prev_f1_pressed){
				precision = 1;
			} else if(precision == 1 && f1_pressed && !prev_f1_pressed){
				precision = 10;
			}

			if(_keytest(RR_LEFT)){
				global_angle = (global_angle - precision)&0x03FF;
			}
			if(_keytest(RR_RIGHT)){
				global_angle = (global_angle + precision)&0x03FF;
			}
			if(_keytest(RR_ENTER)){
				break;
			}
			light_plane = GrayDBufGetHiddenPlane(LIGHT_PLANE);
			dark_plane = GrayDBufGetHiddenPlane(DARK_PLANE);
			clear_table();
			draw_balls();
			render_cue_ball_path(global_angle);
			GrayDBufToggle();

			prev_f1_pressed = f1_pressed;
		}
	}

	while(_keytest(RR_ENTER)){
		//pass
	}
}

void select_power(){
	unsigned char increasing = 1;

	light_plane = GrayDBufGetHiddenPlane(LIGHT_PLANE);
	dark_plane = GrayDBufGetHiddenPlane(DARK_PLANE);
	FastOutlineRect_R(light_plane, 2, 2, 34, 8, A_NORMAL);
	FastOutlineRect_R(dark_plane, 2, 2, 34, 8, A_NORMAL);
	GrayDBufToggle();
	light_plane = GrayDBufGetHiddenPlane(LIGHT_PLANE);
	dark_plane = GrayDBufGetHiddenPlane(DARK_PLANE);
	FastOutlineRect_R(light_plane, 2, 2, 34, 8, A_NORMAL);
	FastOutlineRect_R(dark_plane, 2, 2, 34, 8, A_NORMAL);
	global_power = 0;
	while(1){
		if(do_update){
			do_update = 0;
			if(_keytest(RR_ENTER)){
				break;
			}
			if(global_power == 0 && !increasing){
				increasing = 1;
			}
			if(global_power == 31 && increasing){
				increasing = 0;
			}
			if(increasing){
				global_power++;
			}
			if(!increasing){
				global_power--;
			}
			light_plane = GrayDBufGetHiddenPlane(LIGHT_PLANE);
			dark_plane = GrayDBufGetHiddenPlane(DARK_PLANE);
			draw_power_bar(global_power);
			GrayDBufToggle();
		}
	}
}

void do_ai_move(){
	uint32_t direc_x;
	uint32_t direc_y;
	uint32_t vel_squared;
	uint16_t vel;
	int depth;
	unsigned char result;

	result = ai_get_best_shot(&direc_x, &direc_y, &vel_squared, 2, &depth);
	vel = 512UL;
	dark_plane = GrayDBufGetHiddenPlane(DARK_PLANE);
	PortSet(dark_plane, 239, 127);
	printf("%d: %lx, %lx", result, direc_x, direc_y);
	GrayDBufToggle();
	ngetchx();
	//vel = int_sqrt(vel_squared);
	pool_balls[CUE_BALL_ID].vel_x += sign_shift_round8(direc_x)*vel;
	pool_balls[CUE_BALL_ID].vel_y += sign_shift_round8(direc_y)*vel;
}

DEFINE_INT_HANDLER(update){
	ExecuteHandler(old_int_5);
	do_update = 1;
}

void initialize_balls(){
	int i;

	for(i = 0; i < NUM_BALLS - 1; i++){
		pool_balls[i].pos_x = rack_data_x[i];
		pool_balls[i].pos_y = rack_data_y[i];
		pool_balls[i].vel_x = 0;
		pool_balls[i].vel_y = 0;
		pool_balls[i].sunk = 0;
		if(i != 4 && (i&1)){
			pool_balls[i].type = SOLID;
		} else if(i != 4){
			pool_balls[i].type = STRIPED;
		} else if(i == 4){
			pool_balls[i].type = EIGHT;
		}
	}

	pool_balls[i].pos_x = 0x2000;
	pool_balls[i].pos_y = 0x2000;
	pool_balls[i].vel_x = 512ULL*256;
	pool_balls[i].vel_y = 0;
	pool_balls[i].sunk = 0;
	pool_balls[i].type = CUE_BALL;
}

void _main(){
	unsigned char physics_result;

	quit = 0;
	do_update = 1;

	gray_buffer = malloc(GRAYDBUFFER_SIZE);
	tmp_plane = malloc(LCD_SIZE);
	FontSetSys(F_4x6);
	GrayOn();
	GrayDBufInit(gray_buffer);
	light_plane = GrayDBufGetHiddenPlane(LIGHT_PLANE);
	dark_plane = GrayDBufGetHiddenPlane(DARK_PLANE);
	memset(light_plane, 0, LCD_SIZE);
	memset(dark_plane, 0, LCD_SIZE);
	draw_table(light_plane, dark_plane);
	GrayDBufToggle();
	light_plane = GrayDBufGetHiddenPlane(LIGHT_PLANE);
	dark_plane = GrayDBufGetHiddenPlane(DARK_PLANE);
	memset(light_plane, 0, LCD_SIZE);
	memset(dark_plane, 0, LCD_SIZE);
	draw_table(light_plane, dark_plane);
	initialize_balls();
	old_int_5 = GetIntVec(AUTO_INT_5);
	SetIntVec(AUTO_INT_5, update);
	while(!quit){
		if(do_update){
			do_update = 0;
			if(_keytest(RR_ESC)){
				break;
			}
			light_plane = GrayDBufGetHiddenPlane(LIGHT_PLANE);
			dark_plane = GrayDBufGetHiddenPlane(DARK_PLANE);
			clear_table();
			draw_balls();
			physics_result = do_physics();
			GrayDBufToggle();

			if(physics_result){
				do_ai_move();
				/*
				select_cue_ball_path();
				select_power();
				pool_balls[NUM_BALLS - 1].vel_x += (uint32_t) global_power*16*sign_extend(cos_func(global_angle));
				pool_balls[NUM_BALLS - 1].vel_y += (uint32_t) global_power*16*sign_extend(sin_func(global_angle));
				*/
			}
		}
	}
	SetIntVec(AUTO_INT_5, old_int_5);
	GrayOff();
	free(tmp_plane);
	free(gray_buffer);
}

