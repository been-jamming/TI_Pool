#include <tigcclib.h>

#define FRICTION ((uint32_t) 1<<4)

#define NUM_BALLS 16
#define CUE_BALL_ID 15
#define EIGHT_BALL_ID 4

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

struct game_state{
	pool_ball balls[NUM_BALLS];
	unsigned char turn;
	unsigned char player0_human;
	unsigned char player1_human;
	unsigned char ai_level;
	uint16_t player0_targets;
	uint16_t player1_targets;
};

extern struct game_state global_game_state;
extern uint16_t all_targets;
extern uint16_t stripes_targets;
extern uint16_t solids_targets;
extern pool_ball *pool_balls;

unsigned char do_physics();

uint32_t sign_extend(uint16_t x);
uint64_t sign_extend64(uint32_t x);
uint32_t int_sqrt(uint32_t s);

uint32_t sign_shift_round8(uint32_t x);
uint32_t sign_shift_round12(uint32_t x);
uint32_t int_sqrt(uint32_t s);

extern uint8_t sin_table[256];

