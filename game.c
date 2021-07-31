//TI Pool
//Ben Jones - 7/17/2021

//8-ball pool with an AI

#define USE_TI89
#define SAVE_SCREEN

#include <tigcclib.h>
#include "extgraph.h"
#include "physics.h"
#include "graphics.h"
#include "ai.h"
#include "menu.h"

struct game_state global_game_state;
volatile unsigned char esc_pressed;
INT_HANDLER old_int_5;
volatile unsigned char do_update;
uint16_t global_angle;
unsigned char global_power;

uint16_t player0_targets;
uint16_t player1_targets;
uint16_t ball8_target = 0;
uint16_t stripes_targets = 0;
uint16_t solids_targets = 0;

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

	ai_get_best_shot(0xFFFF, &depth, &direc_x, &direc_y, &vel_squared);
	vel = int_sqrt(vel_squared);
	pool_balls[CUE_BALL_ID].vel_x += sign_shift_round8(direc_x)*vel;
	pool_balls[CUE_BALL_ID].vel_y += sign_shift_round8(direc_y)*vel;
}

DEFINE_INT_HANDLER(update){
	ExecuteHandler(old_int_5);
	do_update = 1;
}

void initialize_balls(){
	int i;

	solids_targets = 0;
	stripes_targets = 0;
	ball8_target = 0;
	for(i = 0; i < NUM_BALLS - 1; i++){
		pool_balls[i].pos_x = rack_data_x[i];
		pool_balls[i].pos_y = rack_data_y[i];
		pool_balls[i].vel_x = 0;
		pool_balls[i].vel_y = 0;
		pool_balls[i].sunk = 0;
		if(i != 4 && (i&1)){
			pool_balls[i].type = SOLID;
			solids_targets |= 1U<<i;
		} else if(i != 4){
			pool_balls[i].type = STRIPED;
			stripes_targets |= 1U<<i;
		} else if(i == 4){
			pool_balls[i].type = EIGHT;
			ball8_target |= 1U<<i;
		}
	}

	pool_balls[i].pos_x = 0x2000;
	pool_balls[i].pos_y = 0x2000;
	pool_balls[i].vel_x = 512ULL*256;
	pool_balls[i].vel_y = rand();
	pool_balls[i].sunk = 0;
	pool_balls[i].type = CUE_BALL;
}

void do_new_game_menu(){
	char *entries[4] = {"Player 1: human", "Player 2: human", "AI level: 1", "Start"};
	unsigned char player0_human = 1;
	unsigned char player1_human = 1;
	int ai_level = 0;
	int selection = -2;

	while(selection != 3){
		selection = do_menu("Game Setup", entries, sizeof(entries)/sizeof(entries[0]));
		switch(selection){
			case 0:
				player0_human = !player0_human;
				if(player0_human){
					entries[0] = "Player 1: human";
				} else {
					entries[0] = "Player 1: CPU";
				}
				break;
			case 1:
				player1_human = !player1_human;
				if(player1_human){
					entries[1] = "Player 2: human";
				} else {
					entries[1] = "Player 2: CPU";
				}
				break;
			case 2:
				ai_level = (ai_level + 1)%3;
				if(ai_level == 0){
					entries[2] = "AI level: 1";
				} else if(ai_level == 1){
					entries[2] = "AI level: 2";
				} else {
					entries[2] = "AI level: 3";
				}
				break;
		}
	}
}

void do_main_menu(){
	char *main_entries[] = {"New Game", "Exit"};
	int selection;

	selection = do_menu("Select Option", main_entries, sizeof(main_entries)/sizeof(main_entries[0]));
	if(selection == 0){
		do_new_game_menu();
	} else if(selection == 1){
		SetIntVec(AUTO_INT_5, old_int_5);
		GrayOff();
		free(tmp_plane);
		free(gray_buffer);
		exit(0);
	}
}

void _main(){
	unsigned char physics_result;

	pool_balls = global_game_state.balls;
	randomize();
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
	while(1){
		do_main_menu();
		light_plane = GrayDBufGetHiddenPlane(LIGHT_PLANE);
		dark_plane = GrayDBufGetHiddenPlane(DARK_PLANE);
		GrayClearScreen2B_R(light_plane, dark_plane);
		draw_table(light_plane, dark_plane);
		light_plane = GrayDBufGetActivePlane(LIGHT_PLANE);
		dark_plane = GrayDBufGetActivePlane(DARK_PLANE);
		GrayClearScreen2B_R(light_plane, dark_plane);
		draw_table(light_plane, dark_plane);
		initialize_balls();
		while(1){
			if(do_update){
				do_update = 0;
				if(_keytest(RR_ESC)){
					break;
				}
				light_plane = GrayDBufGetHiddenPlane(LIGHT_PLANE);
				dark_plane = GrayDBufGetHiddenPlane(DARK_PLANE);
				clear_table();
				draw_balls();
				do_physics();
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
	}
	SetIntVec(AUTO_INT_5, old_int_5);
	GrayOff();
	free(tmp_plane);
	free(gray_buffer);
}

