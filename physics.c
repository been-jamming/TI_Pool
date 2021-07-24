//TI Pool
//Ben Jones - 7/17/2021

//Pool ball physics using fixed point arithmetic

#include "physics.h"

pool_ball pool_balls[NUM_BALLS];

uint32_t sign_extend(uint16_t x){
	if(x&0x8000){
		return 0xFFFF0000 | x;
	}

	return x;
}

//DON'T ROUND
//A moment of silence for the hour I wasted looking for that bug
uint32_t sign_shift_round8(uint32_t x){
	if(x&0x80000000){
		return (x>>8) | 0xFF000000;
	} else {
		return x>>8;
	}
}

//From wikipedia
uint32_t int_sqrt(uint32_t s){
	uint32_t x0;
	uint32_t x1;

	x0 = s>>1;
	if(x0){
		x1 = (x0 + s/x0)>>1;
		while(x1 < x0){
			x0 = x1;
			x1 = (x0 + s/x0)>>1;
		}
		return x0;
	} else {
		return s;
	}
}

static unsigned char pool_ball_collision(int id0, int id1, uint16_t *normal_x, uint16_t *normal_y, uint16_t *vp_x, uint16_t *vp_y){
	uint16_t diff_x016;
	uint16_t diff_y016;
	uint32_t diff_x0;
	uint32_t diff_y0;
	uint32_t diff_x1;
	uint32_t diff_y1;
	uint32_t squared_dist;

	diff_x016 = pool_balls[id0].pos_x - pool_balls[id1].pos_x;
	diff_y016 = pool_balls[id0].pos_y - pool_balls[id1].pos_y;
	if(!(diff_x016&0x8000) && diff_x016 > 1536){
		return 0;
	}
	if((diff_x016&0x8000) && diff_x016 < 64000){
		return 0;
	}
	if(!(diff_y016&0x8000) && diff_y016 > 1536){
		return 0;
	}
	if((diff_y016&0x8000) && diff_y016 < 64000){
		return 0;
	}

	//Determine if the pool balls are colliding
	diff_x0 = sign_extend(diff_x016);
	diff_y0 = sign_extend(diff_y016);
	squared_dist = ((uint32_t) (diff_x0*diff_x0)) + ((uint32_t) (diff_y0*diff_y0));
	//36 (the sum of the radii, squared)
	if(squared_dist >= 2359296){
		return 0;
	}

	//Determine if the pool balls are moving towards each other
	diff_x1 = sign_shift_round8(pool_balls[id0].vel_x - pool_balls[id1].vel_x);
	diff_y1 = sign_shift_round8(pool_balls[id0].vel_y - pool_balls[id1].vel_y);

	if(!((((uint32_t) (diff_x0*diff_x1)) + ((uint32_t) (diff_y0*diff_y1)))&0x80000000)){
		return 0;
	}

	*normal_x = diff_x0;
	*normal_y = diff_y0;
	*vp_x = diff_x1;
	*vp_y = diff_y1;

	return 1;
}

static void pool_ball_handle_collision(int id0, int id1){
	uint16_t normal_x;
	uint16_t normal_y;
	uint16_t vp_x;
	uint16_t vp_y;
	uint32_t normal_x32;
	uint32_t normal_y32;
	uint32_t vp_x32;
	uint32_t vp_y32;
	uint32_t d;
	uint32_t norm_size;
	uint32_t change_x32;
	uint32_t change_y32;
	uint16_t change_x;
	uint16_t change_y;

	if(pool_ball_collision(id0, id1, &normal_x, &normal_y, &vp_x, &vp_y)){
		normal_x32 = sign_extend(normal_x);
		normal_y32 = sign_extend(normal_y);
		vp_x32 = sign_extend(vp_x);
		vp_y32 = sign_extend(vp_y);
		norm_size = ((uint32_t) (normal_x32*normal_x32)) + ((uint32_t) (normal_y32*normal_y32));
		d = ((uint32_t) (normal_x32*vp_x32)) + ((uint32_t) (normal_y32*vp_y32));

		change_x32 = sign_shift_round8(d)*sign_extend(normal_x);
		if(change_x32&0x80000000){
			change_x32 = (~change_x32) + 1;
			change_x32 /= sign_shift_round8(norm_size);
			change_x = (~change_x32) + 1;
		} else {
			change_x = change_x32/sign_shift_round8(norm_size);
		}

		change_y32 = sign_shift_round8(d)*sign_extend(normal_y);
		if(change_y32&0x80000000){
			change_y32 = (~change_y32) + 1;
			change_y32 /= sign_shift_round8(norm_size);
			change_y = (~change_y32) + 1;
		} else {
			change_y = change_y32/sign_shift_round8(norm_size);
		}

		pool_balls[id0].vel_x -= sign_extend(change_x)<<8;
		pool_balls[id0].vel_y -= sign_extend(change_y)<<8;
		pool_balls[id1].vel_x += sign_extend(change_x)<<8;
		pool_balls[id1].vel_y += sign_extend(change_y)<<8;
	}
}

void pool_ball_handle_friction(int id){
	uint32_t vel_x_shifted;
	uint32_t vel_y_shifted;
	uint32_t vel;
	uint32_t normal_x;
	uint32_t normal_y;

	vel_x_shifted = sign_shift_round8(pool_balls[id].vel_x);
	vel_y_shifted = sign_shift_round8(pool_balls[id].vel_y);

	vel = ((uint32_t) (vel_x_shifted*vel_x_shifted)) + ((uint32_t) (vel_y_shifted*vel_y_shifted));
	if((vel<<8) < FRICTION*FRICTION){
		pool_balls[id].vel_x = 0;
		pool_balls[id].vel_y = 0;
	} else {
		vel = int_sqrt(vel);
		if(pool_balls[id].vel_x&0x80000000){
			normal_x = (~pool_balls[id].vel_x + 1)/vel;
			pool_balls[id].vel_x += (normal_x*FRICTION)>>4;
		} else {
			normal_x = pool_balls[id].vel_x/vel;
			pool_balls[id].vel_x -= (normal_x*FRICTION)>>4;
		}
		
		if(pool_balls[id].vel_y&0x80000000){
			normal_y = (~pool_balls[id].vel_y + 1)/vel;
			pool_balls[id].vel_y += (normal_y*FRICTION)>>4;
		} else {
			normal_y = pool_balls[id].vel_y/vel;
			pool_balls[id].vel_y -= (normal_y*FRICTION)>>4;
		}
	}
}

unsigned char do_physics(){
	int i;
	int j;
	uint16_t pre_pos_x;
	uint16_t pre_pos_y;
	unsigned char output = 1;

	for(i = 1; i < NUM_BALLS; i++){
		for(j = 0; j < i; j++){
			if(!pool_balls[i].sunk && !pool_balls[j].sunk){
				pool_ball_handle_collision(i, j);
			}
		}
	}

	for(i = 0; i < NUM_BALLS; i++){
		if(!pool_balls[i].sunk){
			//Bounce on overflow of pos_x and pos_y
			pre_pos_x = pool_balls[i].pos_x;
			pre_pos_y = pool_balls[i].pos_y;
			pool_balls[i].pos_x += sign_shift_round8(pool_balls[i].vel_x);
			pool_balls[i].pos_y += sign_shift_round8(pool_balls[i].vel_y);

			if(((pre_pos_x&0x8000) != (pool_balls[i].pos_x&0x8000)) || (pool_balls[i].pos_x >= 32000) || (pool_balls[i].pos_x <= 768)){
				if(pre_pos_y <= (5<<8) || pre_pos_y >= (60<<8)){
					pool_balls[i].sunk = 1;
				} else {
					pool_balls[i].pos_x = pre_pos_x;
					pool_balls[i].vel_x = (~pool_balls[i].vel_x) + 1;
				}
			}
			if(((pre_pos_y&0x8000) != (pool_balls[i].pos_y&0x8000)) || (pool_balls[i].pos_y >= 16000) || (pool_balls[i].pos_y <= 768)){
				if(pre_pos_x <= (5<<8) || (pre_pos_x >= (60<<8) && pre_pos_x <= (68<<8)) || pre_pos_x >= (123<<8)){
					pool_balls[i].sunk = 1;
				} else {
					pool_balls[i].pos_y = pre_pos_y;
					pool_balls[i].vel_y = (~pool_balls[i].vel_y) + 1;
				}
			}
			pool_ball_handle_friction(i);
			if(pool_balls[i].vel_x || pool_balls[i].vel_y){
				output = 0;
			}
		}
	}

	if(pool_balls[CUE_BALL_ID].sunk){
		pool_balls[CUE_BALL_ID].pos_x = 0x2000;
		pool_balls[CUE_BALL_ID].pos_y = 0x2000;
		pool_balls[CUE_BALL_ID].vel_x = 0;
		pool_balls[CUE_BALL_ID].vel_y = 0;
		pool_balls[CUE_BALL_ID].sunk = 0;
	}

	return output;
}

