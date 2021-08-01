#include <tigcclib.h>

//Save some memory by defining the solid sprites as aliases
#define SOLID_light_sprite CUE_BALL_dark_sprite
#define SOLID_dark_sprite CUE_BALL_light_sprite
//The ghost sprite
#define ghost_sprite CUE_BALL_dark_sprite

extern uint8_t EIGHT_sprite[7];
extern uint8_t CUE_BALL_light_sprite[7];
extern uint8_t CUE_BALL_dark_sprite[7];
extern uint8_t STRIPED_light_sprite[7];
extern uint8_t STRIPED_dark_sprite[7];
extern uint8_t path_sprite[5];
extern void *light_plane;
extern void *dark_plane;
extern void *gray_buffer;
extern void *tmp_plane;
extern uint16_t rack_data_x[15];
extern uint16_t rack_data_y[15];
uint16_t sin_func(uint16_t angle);
uint16_t cos_func(uint16_t angle);
void draw_ball(int x, int y, ball_type type);
void draw_balls();
void clear_top();
void draw_table();
void clear_table();
uint16_t get_collision_time(uint16_t ax, uint16_t ay, uint16_t px, uint16_t py, uint16_t rx, uint16_t ry);
uint16_t get_first_collision_time(uint16_t ax, uint16_t ay, uint16_t rx, uint16_t ry);
void render_cue_ball_path(uint16_t angle);
void draw_power_bar(unsigned char power);
