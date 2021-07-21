#include <tigcclib.h>
#include "extgraph.h"
#include "physics.h"
#include "graphics.h"

uint8_t EIGHT_sprite[7] = {
	0b00111000,
	0b01000100,
	0b11010110,
	0b11000110,
	0b11010110,
	0b01000100,
	0b00111000
};

uint8_t CUE_BALL_light_sprite[7] = {
	0b00111000,
	0b01111100,
	0b11111110,
	0b11111110,
	0b11111110,
	0b01111100,
	0b00111000
};

uint8_t CUE_BALL_dark_sprite[7] = {
	0b00111000,
	0b01000100,
	0b10000010,
	0b10000010,
	0b10000010,
	0b01000100,
	0b00111000
};

uint8_t STRIPED_light_sprite[7] = {
	0b00111000,
	0b01010100,
	0b11010110,
	0b11010110,
	0b11010110,
	0b01010100,
	0b00111000
};

uint8_t STRIPED_dark_sprite[7] = {
	0b00111000,
	0b01101100,
	0b10101010,
	0b10101010,
	0b10101010,
	0b01101100,
	0b00111000
};

uint8_t path_sprite[5] = {
	0b00000000,
	0b00100000,
	0b01110000,
	0b00100000,
	0b00000000
};

void *light_plane;
void *dark_plane;
void *gray_buffer;

void *tmp_plane;

uint16_t rack_data_x[15] = {
	0x5800,
	0x5D32,
	0x5D32,
	0x6264,
	0x6264,
	0x6264,
	0x6796,
	0x6796,
	0x6796,
	0x6796,
	0x6CC8,
	0x6CC8,
	0x6CC8,
	0x6CC8,
	0x6CC8
};

uint16_t rack_data_y[15] = {
	0x2000,
	0x1D00,
	0x2300,
	0x1A00,
	0x2000,
	0x2600,
	0x1700,
	0x1D00,
	0x2300,
	0x2900,
	0x1400,
	0x1A00,
	0x2000,
	0x2600,
	0x2C00
};

uint16_t sin_func(uint16_t angle){
	angle %= 1024;

	if(angle < 256){
		return sin_table[angle];
	} else if(angle == 256){
		return 0x100;
	} else if(angle <= 512){
		return sin_table[512 - angle];
	} else if(angle < 768){
		return -(uint16_t) sin_table[angle - 512];
	} else if(angle == 768){
		return -0x100;
	} else {
		return -(uint16_t) sin_table[1024 - angle];
	}
}

uint16_t cos_func(uint16_t angle){
	return sin_func(angle + 256);
}

void draw_ball(int x, int y, ball_type type){
	uint8_t *light_sprite;
	uint8_t *dark_sprite;

	switch(type){
		case CUE_BALL:
			light_sprite = CUE_BALL_light_sprite;
			dark_sprite = CUE_BALL_dark_sprite;
			break;
		case STRIPED:
			light_sprite = STRIPED_light_sprite;
			dark_sprite = STRIPED_dark_sprite;
			break;
		case SOLID:
			light_sprite = SOLID_light_sprite;
			dark_sprite = SOLID_dark_sprite;
			break;
		case EIGHT:
			light_sprite = EIGHT_sprite;
			dark_sprite = EIGHT_sprite;
			break;
			
	}
	Sprite8_OR_R(x - 3, y - 3, 7, light_sprite, light_plane);
	Sprite8_OR_R(x - 3, y - 3, 7, dark_sprite, dark_plane);
}

void draw_balls(){
	int i;
	int pos_x;
	int pos_y;
	
	for(i = 0; i < NUM_BALLS; i++){
		if(!pool_balls[i].sunk){
			pos_x = (pool_balls[i].pos_x>>8) + 16;
			pos_y = (pool_balls[i].pos_y>>8) + 18;

			draw_ball(pos_x, pos_y, pool_balls[i].type);
		}
	}
}

void draw_table(){
	FastOutlineRect_R(light_plane, 8, 10, 152, 91, A_NORMAL);
	FastOutlineRect_R(dark_plane, 8, 10, 152, 91, A_NORMAL);

	//The pockets
	FastOutlinedCircle_DRAW_R(light_plane, 16, 18, 5);
	FastOutlinedCircle_DRAW_R(dark_plane, 16, 18, 5);
	FloodFill_noshade_R(16, 18, tmp_plane, light_plane);
	FloodFill_noshade_R(16, 18, tmp_plane, dark_plane);

	FastOutlinedCircle_DRAW_R(light_plane, 16, 83, 5);
	FastOutlinedCircle_DRAW_R(dark_plane, 16, 83, 5);
	FloodFill_noshade_R(16, 83, tmp_plane, light_plane);
	FloodFill_noshade_R(16, 83, tmp_plane, dark_plane);

	FastOutlinedCircle_DRAW_R(light_plane, 144, 18, 5);
	FastOutlinedCircle_DRAW_R(dark_plane, 144, 18, 5);
	FloodFill_noshade_R(144, 18, tmp_plane, light_plane);
	FloodFill_noshade_R(144, 18, tmp_plane, dark_plane);

	FastOutlinedCircle_DRAW_R(light_plane, 144, 83, 5);
	FastOutlinedCircle_DRAW_R(dark_plane, 144, 83, 5);
	FloodFill_noshade_R(144, 83, tmp_plane, light_plane);
	FloodFill_noshade_R(144, 83, tmp_plane, dark_plane);

	FastOutlinedCircle_DRAW_R(light_plane, 80, 83, 5);
	FastOutlinedCircle_DRAW_R(dark_plane, 80, 83, 5);
	FloodFill_noshade_R(80, 83, tmp_plane, light_plane);
	FloodFill_noshade_R(80, 83, tmp_plane, dark_plane);

	FastOutlinedCircle_DRAW_R(light_plane, 80, 18, 5);
	FastOutlinedCircle_DRAW_R(dark_plane, 80, 18, 5);
	FloodFill_noshade_R(80, 18, tmp_plane, light_plane);
	FloodFill_noshade_R(80, 18, tmp_plane, dark_plane);

	//The inside of the table
	FastFilledRect_Erase_R(light_plane, 16, 18, 144, 83);
	FastFilledRect_Erase_R(dark_plane, 16, 18, 144, 83);
	FastOutlineRect_R(light_plane, 16, 18, 144, 83, A_NORMAL);
	FastOutlineRect_R(dark_plane, 16, 18, 144, 83, A_NORMAL);
}

void clear_table(){
	FastFilledRect_Erase_R(light_plane, 16, 18, 144, 83);
	FastFilledRect_Erase_R(dark_plane, 16, 18, 144, 83);
	FastOutlineRect_R(light_plane, 16, 18, 144, 83, A_NORMAL);
	FastOutlineRect_R(dark_plane, 16, 18, 144, 83, A_NORMAL);
}

uint16_t get_collision_time(uint16_t ax, uint16_t ay, uint16_t px, uint16_t py, uint16_t rx, uint16_t ry){
	uint32_t diff_x;
	uint32_t diff_y;
	uint32_t rx32;
	uint32_t ry32;
	uint32_t a;
	uint32_t b;
	uint32_t c;
	uint32_t discriminant;
	uint32_t solution;

	diff_x = sign_extend(px - ax);
	diff_y = sign_extend(py - ay);
	rx32 = sign_extend(rx);
	ry32 = sign_extend(ry);

	a = ((uint32_t) (rx32*rx32)) + ((uint32_t) (ry32*ry32));
	b = (((uint32_t) (diff_x*rx32)) + ((uint32_t) (diff_y*ry32)))<<1;
	c = ((uint32_t) (diff_x*diff_x)) + ((uint32_t) (diff_y*diff_y)) - ((uint32_t) 36<<16);
	discriminant = ((uint32_t) (sign_shift_round8(b)*sign_shift_round8(b))) - (((uint32_t) (sign_shift_round8(a)*sign_shift_round8(c)))<<2);
	if(discriminant&0x80000000){
		return 0;
	}

	solution = b - (int_sqrt(discriminant)<<8);
	if(a&0x80000000){
		if(solution&0x80000000){
			a = ~a + 1;
			solution = ~solution + 1;
			return solution/(sign_shift_round8(a)<<1);
		} else {
			return 0;
		}
	} else {
		if(solution&0x80000000){
			return 0;
		} else {
			return solution/(sign_shift_round8(a)<<1);
		}
	}
}

uint16_t get_first_collision_time(uint16_t ax, uint16_t ay, uint16_t rx, uint16_t ry){
	int i;
	uint16_t output = -1;
	uint16_t t;
	uint32_t t32;

	for(i = 0; i < NUM_BALLS - 1; i++){
		if(!pool_balls[i].sunk){
			t32 = get_collision_time(ax, ay, pool_balls[i].pos_x, pool_balls[i].pos_y, rx, ry);
			if(t32 > 0 && t32 < output){
				output = t32;
			}
		}
	}

	if(rx > 0 && !(rx&0x8000)){
		t32 = (sign_extend(32000 - ax)<<8)/rx;
		if(t32 < output){
			output = t32;
		}
	}
	if(rx&0x8000){
		t32 = (sign_extend(ax - 768)<<8)/(-rx);
		if(t32 < output){
			output = t32;
		}
	}
	
	if(ry > 0 && !(ry&0x8000)){
		t32 = (sign_extend(16000 - ay)<<8)/ry;
		if(t32 < output){
			output = t32;
		}
	}
	if(ry&0x8000){
		t32 = (sign_extend(ay - 768)<<8)/(-ry);
		if(t32 < output){
			output = t32;
		}
	}

	return output;
}

void render_cue_ball_path(uint16_t angle){
	uint16_t collision_time;
	uint16_t t;
	uint16_t pos_x;
	uint16_t pos_y;
	uint16_t dot_pos_x;
	uint16_t dot_pos_y;
	uint16_t s;
	uint16_t c;

	s = sin_func(angle);
	c = cos_func(angle);

	pos_x = pool_balls[NUM_BALLS - 1].pos_x;
	pos_y = pool_balls[NUM_BALLS - 1].pos_y;

	collision_time = get_first_collision_time(pos_x, pos_y, c, s);

	if(collision_time >= 1536){
		for(t = 1536; t < collision_time - 1536; t += 1536){
			dot_pos_x = sign_shift_round8(sign_extend(c)*sign_extend(t)) + pos_x;
			dot_pos_y = sign_shift_round8(sign_extend(s)*sign_extend(t)) + pos_y;
			Sprite8_OR_R((dot_pos_x>>8) + 14, (dot_pos_y>>8) + 16, 5, path_sprite, light_plane);
			Sprite8_OR_R((dot_pos_x>>8) + 14, (dot_pos_y>>8) + 16, 5, path_sprite, dark_plane);
		}
	}

	pos_x += sign_shift_round8(sign_extend(c)*sign_extend(collision_time));
	pos_y += sign_shift_round8(sign_extend(s)*sign_extend(collision_time));

	if((pos_x>>8) > 0){
		if((pos_y>>8) > 0){
			Sprite8_OR_R((pos_x>>8) + 13, (pos_y>>8) + 15, 7, ghost_sprite, light_plane);
			Sprite8_OR_R((pos_x>>8) + 13, (pos_y>>8) + 15, 7, ghost_sprite, dark_plane);
		} else {
			Sprite8_OR_R((pos_x>>8) + 13, (pos_y>>8) + 16, 7, ghost_sprite, light_plane);
			Sprite8_OR_R((pos_x>>8) + 13, (pos_y>>8) + 16, 7, ghost_sprite, dark_plane);
		}
	} else {
		if((pos_y>>8) > 0){
			Sprite8_OR_R((pos_x>>8) + 14, (pos_y>>8) + 15, 7, ghost_sprite, light_plane);
			Sprite8_OR_R((pos_x>>8) + 14, (pos_y>>8) + 15, 7, ghost_sprite, dark_plane);
		} else {
			Sprite8_OR_R((pos_x>>8) + 14, (pos_y>>8) + 16, 7, ghost_sprite, light_plane);
			Sprite8_OR_R((pos_x>>8) + 14, (pos_y>>8) + 16, 7, ghost_sprite, dark_plane);
		}
	}
}

//Power is 0 through 31
void draw_power_bar(unsigned char power){
	FastFilledRect_Erase_R(light_plane, 3, 3, 33, 7);
	FastFilledRect_Erase_R(dark_plane, 3, 3, 33, 7);

	FastFilledRect_Draw_R(dark_plane, 2, 3, power + 2, 7);
	if(power >= 16){
		FastFilledRect_Draw_R(light_plane, 18, 3, power + 2, 7);
	}
}

