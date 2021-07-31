unsigned char early_collision(int exception0, int exception1, uint32_t pos_x, uint32_t pos_y, uint32_t direc_x, uint32_t direc_y, uint32_t *a_out);
void ai_get_shot_recursive(uint16_t balls_mask, int depth, uint32_t pos_x, uint32_t pos_y, uint32_t direc_x, uint32_t direc_y, uint32_t vel_squared, int ball, int *out_depth, uint32_t *out_direc_x, uint32_t *out_direc_y, uint32_t *out_vel_squared);
void ai_get_best_shot(uint16_t targets, int *out_depth, uint32_t *out_direc_x, uint32_t *out_direc_y, uint32_t *out_vel_squared);
