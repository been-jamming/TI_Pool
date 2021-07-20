#include <tigcclib.h>

#define NUM_BALLS 16
#define CUE_BALL_ID 15

typedef enum{
	CUE_BALL = 1,
	STRIPED = 2,
	SOLID = 3,
	EIGHT = 4
} ball_type;

typedef struct{
	uint16_t pos_x;
	uint16_t pos_y;
	uint32_t vel_x;
	uint32_t vel_y;
	ball_type type;
	unsigned char sunk;
} pool_ball;

extern pool_ball pool_balls[NUM_BALLS];

unsigned char do_physics();

uint32_t sign_extend(uint16_t x);

uint32_t sign_shift_round8(uint32_t x);
uint32_t int_sqrt(uint32_t s);

extern uint8_t sin_table[256];

