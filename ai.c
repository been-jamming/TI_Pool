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

static unsigned char check_other_collisions(unsigned char exception_id, uint32_t ax, uint32_t ay, uint32_t rx, uint32_t ry, uint32_t *norm_squared){
	uint32_t a;
	uint32_t b;
	uint32_t c;
	uint32_t discriminant;
	uint32_t diff_x;
	uint32_t diff_y;
	int i;

	for(i = 0; i < NUM_BALLS; i++){
		if(i == exception_id){
			continue;
		}
		diff_x = sign_extend(pool_balls[i].pos_x) - ax;
		diff_y = sign_extend(pool_balls[i].pos_y) - ay;
		a = ((uint32_t) (sign_shift_round8(rx)*sign_shift_round8(rx))) + ((uint32_t) (sign_shift_round8(ry)*sign_shift_round8(ry)));
		b = (((uint32_t) (sign_shift_round8(diff_x)*sign_shift_round8(rx))) + ((uint32_t) (sign_shift_round8(diff_y)*sign_shift_round8(ry))))<<1;
		if(b&0x80000000){
			continue;
		}
		c = ((uint32_t) (sign_shift_round8(diff_x)*sign_shift_round8(diff_x))) + ((uint32_t) (sign_shift_round8(diff_y)*sign_shift_round8(diff_y))) - ((uint32_t) 36<<16);
		if((a&0x80000000) == ((a - b + c)&0x80000000) && (a&0x80000000) != ((b - (a<<1))&0x80000000)){
			continue;
		}
		discriminant = ((uint32_t) (sign_shift_round8(b)*sign_shift_round8(b))) - (((uint32_t) (sign_shift_round8(a)*sign_shift_round8(c)))<<2);
		if(discriminant&0x80000000){
			continue;
		}
		return 1;
	}

	*norm_squared = a;
	return 0;
}

int ai_get_shot_recursive(uint32_t ball_mask, uint32_t pos_x, uint32_t pos_y, uint32_t direc_x, uint32_t direc_y, uint32_t vel_squared, int depth, uint32_t *new_pos_x, uint32_t *new_pos_y, uint32_t *new_direc_x, uint32_t *new_direc_y, uint32_t *new_vel_squared){
	int i;
	int test_depth;
	unsigned char path_found = 0;
	uint32_t intersect_x;
	uint32_t intersect_y;
	uint32_t next_pos_x;
	uint32_t next_pos_y;
	uint32_t rx;
	uint32_t ry;
	uint32_t norm_squared;
	uint32_t norm;
	uint32_t normed_x;
	uint32_t normed_y;
	uint32_t dot;
	uint32_t new_vel;

	uint32_t test_new_pos_x;
	uint32_t test_new_pos_y;
	uint32_t test_new_direc_x;
	uint32_t test_new_direc_y;
	uint32_t test_new_vel_squared;

	if(!depth){
		return 0;
	}

	*new_vel_squared = UINT32_MAX;

	intersect_x = pos_x - 6*direc_x;
	intersect_y = pos_y - 6*direc_y;

	//First we check the cue ball
	for(i = NUM_BALLS - 1; i >= 0; i--){
		if(pool_balls[i].sunk || ((1U<<i)&ball_mask)){
			continue;
		}
		next_pos_x = sign_extend(pool_balls[i].pos_x)<<8;
		next_pos_y = sign_extend(pool_balls[i].pos_y)<<8;
		rx = intersect_x - next_pos_x;
		ry = intersect_y - next_pos_y;
		dot = sign_shift_round8(rx)*sign_shift_round8(direc_x) + sign_shift_round8(ry)*sign_shift_round8(direc_y);
		if(dot&0x80000000){
			continue;
		}
		if(check_other_collisions(i, next_pos_x, next_pos_y, rx, ry, &norm_squared)){
			continue;
		}
		norm = int_sqrt(norm_squared);
		if(!norm){
			return 0;
		}
		if(rx&0x80000000){
			normed_x = -((-rx)/norm);
		} else {
			normed_x = rx/norm;
		}
		if(ry&0x80000000){
			normed_y = -((-ry)/norm);
		} else {
			normed_y = ry/norm;
		}
		dot = normed_x*direc_x + normed_y*direc_y;
		dot = sign_shift_round8(dot)*sign_shift_round8(dot);
		if(!dot){
			return 0;
		}
		new_vel = ((vel_squared/dot)<<8) + 2*norm*FRICTION;
		if(new_vel > MAX_VELOCITY){
			continue;
		}
		if(i == CUE_BALL_ID){
			*new_pos_x = next_pos_x;
			*new_pos_y = next_pos_y;
			*new_direc_x = normed_x<<8;
			*new_direc_y = normed_y<<8;
			*new_vel_squared = new_vel;

			return 1;
		} else {
			test_depth = ai_get_shot_recursive(ball_mask | (1U<<i), next_pos_x, next_pos_y, normed_x<<8, normed_y<<8, new_vel, depth - 1, &test_new_pos_x, &test_new_pos_y, &test_new_direc_x, &test_new_direc_y, &test_new_vel_squared);
			if(test_depth && (test_depth < depth - 1 || (test_depth == depth - 1 && test_new_vel_squared < *new_vel_squared))){
				depth = test_depth + 1;
				*new_pos_x = test_new_pos_x;
				*new_pos_y = test_new_pos_y;
				*new_direc_x = test_new_direc_x;
				*new_direc_y = test_new_direc_y;
				*new_vel_squared = test_new_vel_squared;
				path_found = 1;
			}
		}
	}

	if(!path_found){
		return 0;
	} else {
		return depth;
	}
}

unsigned char ai_get_best_shot(uint32_t *direc_x, uint32_t *direc_y, uint32_t *vel_squared, int max_depth, int *depth){
	int i;
	int j;
	int result_depth;
	unsigned char output = 0;
	uint32_t pos_x;
	uint32_t pos_y;
	uint32_t req_direc_x;
	uint32_t req_direc_y;
	uint32_t squared_norm;
	uint16_t norm;
	uint32_t normed_direc_x;
	uint32_t normed_direc_y;
	uint32_t req_vel_squared;

	uint32_t result_pos_x;
	uint32_t result_pos_y;
	uint32_t result_direc_x;
	uint32_t result_direc_y;
	uint32_t result_vel_squared;

	*vel_squared = MAX_VELOCITY;
	*depth = max_depth;
	*direc_x = 0x10000;
	*direc_y = 0;

	for(i = 0; i < NUM_BALLS - 1; i++){
		for(j = 0; j < 6; j++){
			pos_x = sign_extend(pool_balls[i].pos_x)<<8;
			pos_y = sign_extend(pool_balls[i].pos_y)<<8;
			req_direc_x = (sign_extend(shot_positions_x[j])<<8) - pos_x;
			req_direc_y = (sign_extend(shot_positions_y[j])<<8) - pos_y;
			squared_norm = sign_shift_round8(req_direc_x)*sign_shift_round8(req_direc_x) + sign_shift_round8(req_direc_y)*sign_shift_round8(req_direc_y);
			norm = int_sqrt(squared_norm);
			if(req_direc_x&0x80000000){
				normed_direc_x = -((-req_direc_x)/norm);
			} else {
				normed_direc_x = req_direc_x/norm;
			}
			if(req_direc_y&0x80000000){
				normed_direc_y = -((-req_direc_y)/norm);
			} else {
				normed_direc_y = req_direc_y/norm;
			}
			req_vel_squared = 2*FRICTION*norm;
			result_depth = ai_get_shot_recursive(0, pos_x, pos_y, normed_direc_x, normed_direc_y, req_vel_squared, max_depth, &result_pos_x, &result_pos_y, &result_direc_x, &result_direc_y, &result_vel_squared);
			if(result_depth < max_depth || (result_depth == max_depth && result_vel_squared < *vel_squared)){
				max_depth = result_depth;
				*direc_x = result_direc_x;
				*direc_y = result_direc_y;
				*depth = max_depth;
				output = 1;
			}
		}
	}

	return output;
}

