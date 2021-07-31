#include <tigcclib.h>
#include "extgraph.h"
#include "physics.h"

#define MAX_VELOCITY (512UL*256UL)

static uint16_t shot_positions_x[6] = {
	0x0000,
	0x0000,
	0x4000,
	0x4000,
	0x7FFF,
	0x7FFF
};

static uint16_t shot_positions_y[6] = {
	0x0000,
	0x4000,
	0x0000,
	0x4000,
	0x0000,
	0x4000
};

unsigned char early_collision(int exception0, int exception1, uint32_t pos_x, uint32_t pos_y, uint32_t direc_x, uint32_t direc_y, uint32_t *a_out){
	int i;
	uint32_t diff_x;
	uint32_t diff_y;
	uint32_t a;
	uint32_t b;
	uint32_t c;
	uint64_t disc;

	a = sign_shift_round8(direc_x)*sign_shift_round8(direc_x) + sign_shift_round8(direc_y)*sign_shift_round8(direc_y);
	*a_out = a;
	for(i = 0; i < NUM_BALLS; i++){
		if(i == exception0 || i == exception1 || pool_balls[i].sunk){
			continue;
		}

		diff_x = pos_x - (sign_extend(pool_balls[i].pos_x)<<8);
		diff_y = pos_y - (sign_extend(pool_balls[i].pos_y)<<8);

		b = 2*(sign_shift_round8(direc_x)*sign_shift_round8(diff_x) + sign_shift_round8(direc_y)*sign_shift_round8(diff_y));
		c = sign_shift_round8(diff_x)*sign_shift_round8(diff_x) + sign_shift_round8(diff_y)*sign_shift_round8(diff_y) - (36UL<<16);
		disc = sign_extend64(b)*sign_extend64(b) - 4*sign_extend64(a)*sign_extend64(c);

		if(disc&0x8000000000000000ULL){
			continue;
		}

		if((a + b + c)&0x80000000){
			return 1;
		}

		if((b&0x80000000) && (!((2*a + b)&0x80000000))){
			return 1;
		}
	}

	return 0;
}

void ai_get_shot_recursive(uint16_t balls_mask, int depth, uint32_t pos_x, uint32_t pos_y, uint32_t direc_x, uint32_t direc_y, uint32_t vel_squared, int ball, int *out_depth, uint32_t *out_direc_x, uint32_t *out_direc_y, uint32_t *out_vel_squared){
	int i;
	uint32_t intersect_x;
	uint32_t intersect_y;
	uint32_t new_direc_x;
	uint32_t new_direc_y;
	uint32_t new_vel_squared;
	int test_depth;
	uint32_t test_direc_x;
	uint32_t test_direc_y;
	uint32_t test_vel_squared;
	uint32_t dist;
	uint32_t dot;
	unsigned char found = 0;

	*out_depth = 100;
	*out_vel_squared = 0xFFFFFFFF;

	if(!depth){
		*out_depth = -1;
		return;
	}

	intersect_x = pos_x - 6*direc_x;
	intersect_y = pos_y - 6*direc_y;

	for(i = CUE_BALL_ID; i >= 0; i--){
		if((balls_mask&(1U<<i)) || pool_balls[i].sunk){
			continue;
		}

		new_direc_x = intersect_x - (sign_extend(pool_balls[i].pos_x)<<8);
		new_direc_y = intersect_y - (sign_extend(pool_balls[i].pos_y)<<8);

		if((sign_shift_round8(new_direc_x)*sign_shift_round8(direc_x) + sign_shift_round8(new_direc_y)*sign_shift_round8(direc_y))&0x80000000){
			continue;
		}
		if(early_collision(i, ball, sign_extend(pool_balls[i].pos_x)<<8, sign_extend(pool_balls[i].pos_y)<<8, new_direc_x, new_direc_y, &dist)){
			continue;
		}
		dist = int_sqrt(dist);

		if(!dist){
			continue;
		}

		if(new_direc_x&0x80000000){
			new_direc_x = -(((uint32_t) -new_direc_x)/dist);
		} else {
			new_direc_x /= dist;
		}

		if(new_direc_y&0x80000000){
			new_direc_y = -(((uint32_t) -new_direc_y)/dist);
		} else {
			new_direc_y /= dist;
		}

		dot = new_direc_x*sign_shift_round8(direc_x) + new_direc_y*sign_shift_round8(direc_y);
		if(!(dot&0xFFFFFF00)){
			continue;
		}
		new_vel_squared = (((vel_squared<<8)/(sign_shift_round8(dot)*sign_shift_round8(dot)))<<8) + FRICTION*(dist>>3);
		if(new_vel_squared > (4UL<<16)){
			continue;
		}
		if(i == CUE_BALL_ID){
			*out_depth = 1;
			*out_direc_x = new_direc_x<<8;
			*out_direc_y = new_direc_y<<8;
			*out_vel_squared = new_vel_squared;
			return;
		} else {
			ai_get_shot_recursive(balls_mask | (1U<<i), depth - 1, sign_extend(pool_balls[i].pos_x)<<8, sign_extend(pool_balls[i].pos_y)<<8, new_direc_x<<8, new_direc_y<<8, new_vel_squared, i, &test_depth, &test_direc_x, &test_direc_y, &test_vel_squared);
			if(test_depth != -1 && (test_depth < *out_depth - 1 || (test_depth == *out_depth - 1 && test_vel_squared < *out_vel_squared))){
				*out_depth = test_depth + 1;
				*out_vel_squared = test_vel_squared;
				*out_direc_x = test_direc_x;
				*out_direc_y = test_direc_y;
				found = 1;
			}
		}
	}

	if(!found){
		*out_depth = -1;
	}
}

void ai_get_best_shot(uint16_t targets, int *out_depth, uint32_t *out_direc_x, uint32_t *out_direc_y, uint32_t *out_vel_squared){
	int test_depth;
	uint32_t test_direc_x;
	uint32_t test_direc_y;
	uint32_t test_vel_squared;
	uint32_t direc_x;
	uint32_t direc_y;
	uint32_t dist;

	int i;
	int j;

	*out_depth = 4;
	*out_direc_x = rand()<<1;
	*out_direc_y = rand()<<1;
	*out_vel_squared = 1024UL<<10;

	for(j = 0; j < 6; j++){
		for(i = 0; i < CUE_BALL_ID; i++){
			if(pool_balls[i].sunk || !(targets&(1U<<i))){
				continue;
			}

			direc_x = (sign_extend(shot_positions_x[j])<<8) - (sign_extend(pool_balls[i].pos_x)<<8);
			direc_y = (sign_extend(shot_positions_y[j])<<8) - (sign_extend(pool_balls[i].pos_y)<<8);
			if(early_collision(i, -1, sign_extend(pool_balls[i].pos_x)<<8, sign_extend(pool_balls[i].pos_y)<<8, direc_x, direc_y, &dist)){
				continue;
			}
			dist = int_sqrt(dist);
			if(!dist){
				continue;
			}
			if(direc_x&0x80000000){
				direc_x = -(((uint32_t) -direc_x)/dist);
			} else {
				direc_x /= dist;
			}

			if(direc_y&0x80000000){
				direc_y = -(((uint32_t) -direc_y)/dist);
			} else {
				direc_y /= dist;
			}
			ai_get_shot_recursive(1U<<i, 4, sign_extend(pool_balls[i].pos_x)<<8, sign_extend(pool_balls[i].pos_y)<<8, direc_x<<8, direc_y<<8, FRICTION*(dist>>3) + 0x0800, i, &test_depth, &test_direc_x, &test_direc_y, &test_vel_squared);
			if(test_depth != -1 && (test_depth < *out_depth || (test_depth == *out_depth && test_vel_squared < *out_vel_squared))){
				*out_depth = test_depth;
				*out_vel_squared = test_vel_squared;
				*out_direc_x = test_direc_x;
				*out_direc_y = test_direc_y;
			}
		}
	}
}

