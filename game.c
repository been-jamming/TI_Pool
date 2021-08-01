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

uint16_t all_targets = 0x7FEF;
uint16_t ball8_target = 0;
uint16_t stripes_targets = 0;
uint16_t solids_targets = 0;

void clear_and_draw_table(){
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
}

void center_text(int x, int y, char *str, int font){
	int width;

	width = DrawStrWidth(str, font);
	light_plane = GrayDBufGetHiddenPlane(LIGHT_PLANE);
	dark_plane = GrayDBufGetHiddenPlane(DARK_PLANE);
	GrayDrawStr2B(x - width/2, y, str, A_NORMAL, light_plane, dark_plane);
	light_plane = GrayDBufGetActivePlane(LIGHT_PLANE);
	dark_plane = GrayDBufGetActivePlane(DARK_PLANE);
	GrayDrawStr2B(x - width/2, y, str, A_NORMAL, light_plane, dark_plane);
}

void draw_title(){
	FontSetSys(F_8x10);
	center_text(80, 22, "TI Pool", F_8x10);
	FontSetSys(F_4x6);
	center_text(40, 93, "Ben Jones - 7/17/2021", F_4x6);
	FontSetSys(F_6x8);
}

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

	while(_keytest(RR_ENTER)){
		//pass
	}

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

void select_cue_ball_placement(){
	int frame = 0;
	int i = 0;
	uint16_t diff_x;
	uint16_t diff_y;
	uint32_t dist;
	char *message = "Redo Selection";

	FontSetSys(F_6x8);
	if((global_game_state.turn && global_game_state.player1_human) || (!global_game_state.turn && global_game_state.player0_human)){
		clear_top();
		center_text(80, 1, "Select cue ball placement", F_6x8);
		pool_balls[CUE_BALL_ID].pos_x = 0x2000;
		pool_balls[CUE_BALL_ID].pos_y = 0x2000;
		pool_balls[CUE_BALL_ID].vel_x = 0;
		pool_balls[CUE_BALL_ID].vel_y = 0;

		while(1){
			if(do_update){
				do_update = 0;
				frame = (frame + 1)%26;
				if(_keytest(RR_ENTER)){
					for(i = 0; i < CUE_BALL_ID; i++){
						diff_x = pool_balls[i].pos_x - pool_balls[CUE_BALL_ID].pos_x;
						diff_y = pool_balls[i].pos_y - pool_balls[CUE_BALL_ID].pos_y;
						dist = sign_extend(diff_x)*sign_extend(diff_x) + sign_extend(diff_y)*sign_extend(diff_y);
						if(dist < (36UL<<16)){
							do_menu("Invalid Ball Placement", &message, 1);
							while(_keytest(RR_ENTER)){
								while(!do_update){
									//pass
								}
								do_update = 0;
							}
							clear_and_draw_table();
							center_text(80, 1, "Select cue ball placement", F_6x8);
							break;
						}
					}
					if(i == CUE_BALL_ID){
						while(_keytest(RR_ENTER)){
							while(!do_update){
								//pass
							}
							do_update = 0;
						}
						break;
					}
				}

				if(_keytest(RR_LEFT) && ((pool_balls[CUE_BALL_ID].pos_x>>8) > 3)){
					pool_balls[CUE_BALL_ID].pos_x -= 256;
				}
				if(_keytest(RR_RIGHT) && (pool_balls[CUE_BALL_ID].pos_x < 0x7D00U)){
					pool_balls[CUE_BALL_ID].pos_x += 256;
				}
				if(_keytest(RR_UP) && ((pool_balls[CUE_BALL_ID].pos_y>>8) > 3)){
					pool_balls[CUE_BALL_ID].pos_y -= 256;
				}
				if(_keytest(RR_DOWN) && (pool_balls[CUE_BALL_ID].pos_y < 0x3D00U)){
					pool_balls[CUE_BALL_ID].pos_y += 256;
				}

				light_plane = GrayDBufGetHiddenPlane(LIGHT_PLANE);
				dark_plane = GrayDBufGetHiddenPlane(DARK_PLANE);

				clear_table();
				draw_balls();

				if(frame < 13){
					draw_ball((pool_balls[CUE_BALL_ID].pos_x>>8) + 16, (pool_balls[CUE_BALL_ID].pos_y>>8) + 18, CUE_BALL);
				}

				GrayDBufToggle();
			}
		}
		pool_balls[CUE_BALL_ID].sunk = 0;
	} else {
		pool_balls[CUE_BALL_ID].vel_x = 0;
		pool_balls[CUE_BALL_ID].vel_y = 0;
		do{
			pool_balls[CUE_BALL_ID].pos_x = rand()%0x7A00U + 0x0300;
			pool_balls[CUE_BALL_ID].pos_y = rand()%0x3A00U + 0x0300;
			for(i = 0; i < CUE_BALL_ID; i++){
				diff_x = pool_balls[i].pos_x - pool_balls[CUE_BALL_ID].pos_x;
				diff_y = pool_balls[i].pos_y - pool_balls[CUE_BALL_ID].pos_y;
				dist = sign_extend(diff_x)*sign_extend(diff_x) + sign_extend(diff_y)*sign_extend(diff_y);
				if(dist < (36UL<<16)){
					break;
				}
			}
		} while(i < CUE_BALL_ID);
		pool_balls[CUE_BALL_ID].sunk = 0;
	}
}

void do_ai_move(uint16_t targets){
	uint32_t direc_x;
	uint32_t direc_y;
	uint32_t vel_squared;
	uint16_t vel;
	int depth;

	ai_get_best_shot(targets, &depth, &direc_x, &direc_y, &vel_squared);
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
	if(!global_game_state.player0_human){
		pool_balls[i].vel_x = 512ULL*256;
		pool_balls[i].vel_y = rand()/2;
	} else {
		pool_balls[i].vel_x = 0;
		pool_balls[i].vel_y = 0;
	}
	pool_balls[i].sunk = 0;
	pool_balls[i].type = CUE_BALL;
}

void do_new_game_menu(){
	char *entries[4] = {"Player 1: human", "Player 2: human", "AI level: 1", "Start"};
	unsigned char player0_human = 1;
	unsigned char player1_human = 1;
	int ai_level = 0;
	int selection = -2;

	global_game_state.player0_human = 1;
	global_game_state.player1_human = 1;
	global_game_state.player0_targets = all_targets;
	global_game_state.player1_targets = all_targets;
	global_game_state.ai_level = 0;
	global_game_state.turn = 0;
	while(selection != 3){
		selection = do_menu("Game Setup", entries, sizeof(entries)/sizeof(entries[0]));
		switch(selection){
			case 0:
				player0_human = !player0_human;
				if(player0_human){
					entries[0] = "Player 1: human";
					global_game_state.player0_human = 1;
				} else {
					entries[0] = "Player 1: CPU";
					global_game_state.player0_human = 0;
				}
				break;
			case 1:
				player1_human = !player1_human;
				if(player1_human){
					entries[1] = "Player 2: human";
					global_game_state.player1_human = 1;
				} else {
					entries[1] = "Player 2: CPU";
					global_game_state.player1_human = 0;
				}
				break;
			case 2:
				ai_level = (ai_level + 1)%3;
				if(ai_level == 0){
					entries[2] = "AI level: 1";
					global_game_state.ai_level = 0;
				} else if(ai_level == 1){
					entries[2] = "AI level: 2";
					global_game_state.ai_level = 1;
				} else {
					entries[2] = "AI level: 3";
					global_game_state.ai_level = 2;
				}
				break;
		}
	}
}

void display_win(unsigned char win_team){
	char *messages[] = {"Player 1 Wins!", "Player 2 Wins!"};
	char *entry = "Continue";

	do_menu(messages[win_team], &entry, 1);
}

void do_main_menu(){
	char *main_entries[] = {"New Game", "Load Game", "Exit"};
	int selection;
	char *continue_text = "Continue";
	char *save_name;
	FILE *save_file;

	draw_title();
	do{
		selection = do_menu("Select Option", main_entries, sizeof(main_entries)/sizeof(main_entries[0]));
		if(selection == 0){
			do_new_game_menu();
			initialize_balls();
			global_game_state.prev_targets_mask = 0xFFFF;
			clear_and_draw_table();
			if(global_game_state.player0_human){
				select_cue_ball_path();
				select_power();
				pool_balls[NUM_BALLS - 1].vel_x += (uint32_t) global_power*16*sign_extend(cos_func(global_angle));
				pool_balls[NUM_BALLS - 1].vel_y += (uint32_t) global_power*16*sign_extend(sin_func(global_angle));
			}
			return;
		} else if(selection == 1){
			save_name = do_text_entry("Enter Save Name");
			if(save_name){
				save_file = fopen(save_name, "r");
				if(!save_file){
					do_menu("Failed to Open File", &continue_text, 1);
					free(save_name);
				} else {
					fread(&global_game_state, 1, sizeof(global_game_state), save_file);
					fclose(save_file);
					free(save_name);
					clear_and_draw_table();
					break;
				}
			}
		} else if(selection == 2){
			SetIntVec(AUTO_INT_5, old_int_5);
			GrayOff();
			free(tmp_plane);
			free(gray_buffer);
			exit(0);
		}
	} while(selection == 1 || selection != -1);

	while(_keytest(RR_ENTER)){
		while(!do_update){
			//pass
		}
		do_update = 0;
	}
}

unsigned char do_pause_menu(){
	char *pause_entries[] = {"Resume", "Save", "Exit"};
	int selection;
	char *save_name;
	char *continue_text = "Continue";
	FILE *save_file;

	do{
		selection = do_menu("Pause", pause_entries, sizeof(pause_entries)/sizeof(pause_entries[0]));
		if(selection == 2){
			clear_and_draw_table();
			return 1;
		} else if(selection == 1){
			save_name = do_text_entry("Enter Save Name");
			if(save_name){
				save_file = fopen(save_name, "w");
				if(!save_file){
					do_menu("Failed to Save File", &continue_text, 1);
					free(save_name);
				}
				fwrite(&global_game_state, sizeof(global_game_state), 1, save_file);
				fclose(save_file);
				free(save_name);
			}
		}
	} while(selection != 0);

	clear_and_draw_table();
	draw_balls();
}

uint16_t get_targets_mask(){
	uint16_t output = 0;
	int i;

	for(i = 0; i < NUM_BALLS; i++){
		if(!pool_balls[i].sunk){
			output |= 1U<<i;
		}
	}

	return output;
}

void do_game_move(uint16_t targets){
	clear_top();
	FontSetSys(F_6x8);
	if(global_game_state.turn == 0){
		if(global_game_state.player0_targets == all_targets){
			center_text(90, 1, "Player 1", F_6x8);
		} else if(global_game_state.player0_targets == stripes_targets){
			center_text(90, 1, "Player 1: stripes", F_6x8);
		} else if(global_game_state.player0_targets == solids_targets){
			center_text(90, 1, "Player 1: solids", F_6x8);
		} else if(global_game_state.player0_targets == ball8_target){
			center_text(90, 1, "Player 1: 8 ball", F_6x8);
		}
	} else {
		if(global_game_state.player1_targets == all_targets){
			center_text(90, 1, "Player 2", F_6x8);
		} else if(global_game_state.player1_targets == stripes_targets){
			center_text(90, 1, "Player 2: stripes", F_6x8);
		} else if(global_game_state.player1_targets == solids_targets){
			center_text(90, 1, "Player 2: solids", F_6x8);
		} else if(global_game_state.player1_targets == ball8_target){
			center_text(90, 1, "Player 2: 8 ball", F_6x8);
		}
	}
	if((global_game_state.turn == 0 && global_game_state.player0_human) || (global_game_state.turn == 1 && global_game_state.player1_human)){
		select_cue_ball_path();
		select_power();
		pool_balls[NUM_BALLS - 1].vel_x += (uint32_t) global_power*16*sign_extend(cos_func(global_angle));
		pool_balls[NUM_BALLS - 1].vel_y += (uint32_t) global_power*16*sign_extend(sin_func(global_angle));
	} else {
		do_ai_move(targets);
	}
}

void _main(){
	unsigned char physics_result;
	uint16_t targets;
	uint16_t targets_mask;

	pool_balls = global_game_state.balls;
	randomize();
	do_update = 1;

	gray_buffer = malloc(GRAYDBUFFER_SIZE);
	tmp_plane = malloc(LCD_SIZE);
	FontSetSys(F_4x6);
	GrayOn();
	GrayDBufInit(gray_buffer);
	clear_and_draw_table();
	initialize_balls();
	old_int_5 = GetIntVec(AUTO_INT_5);
	SetIntVec(AUTO_INT_5, update);
	while(1){
		clear_and_draw_table();
		do_main_menu();
		while(1){
			if(do_update){
				do_update = 0;
				if(_keytest(RR_ESC)){
					while(_keytest(RR_ESC)){
						while(!do_update){
							//pass
						}
						do_update = 0;
					}
					if(do_pause_menu()){
						break;
					}
				}
				light_plane = GrayDBufGetHiddenPlane(LIGHT_PLANE);
				dark_plane = GrayDBufGetHiddenPlane(DARK_PLANE);
				clear_table();
				draw_balls();
				do_physics();
				physics_result = do_physics();
				GrayDBufToggle();

				if(physics_result){
					targets_mask = get_targets_mask();
					if(global_game_state.turn){
						targets = global_game_state.player1_targets;
					} else {
						targets = global_game_state.player0_targets;
					}
					if(targets != ball8_target && pool_balls[EIGHT_BALL_ID].sunk){
						display_win(!global_game_state.turn);
						break;
					} else if(targets == ball8_target && pool_balls[EIGHT_BALL_ID].sunk && !pool_balls[CUE_BALL_ID].sunk){
						display_win(global_game_state.turn);
						break;
					} else if(targets == ball8_target && pool_balls[EIGHT_BALL_ID].sunk && pool_balls[CUE_BALL_ID].sunk){
						display_win(!global_game_state.turn);
						break;
					}

					if(!(targets_mask^global_game_state.prev_targets_mask) || ((targets_mask^global_game_state.prev_targets_mask)&~targets)){
						global_game_state.turn = !global_game_state.turn;
					}

					if(pool_balls[CUE_BALL_ID].sunk){
						select_cue_ball_placement();
						targets_mask = get_targets_mask();
					}

					if(global_game_state.turn){
						targets = global_game_state.player1_targets;
						if(!(targets&targets_mask)){
							global_game_state.player1_targets = ball8_target;
							targets = ball8_target;
						}
					} else {
						targets = global_game_state.player0_targets;
						if(!(targets&targets_mask)){
							global_game_state.player0_targets = ball8_target;
							targets = ball8_target;
						}
					}

					do_game_move(targets);
					global_game_state.prev_targets_mask = targets_mask;
				}
			}
		}
	}
	SetIntVec(AUTO_INT_5, old_int_5);
	GrayOff();
	free(tmp_plane);
	free(gray_buffer);
}

