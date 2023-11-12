
#define is_down(b) input->buttons[b].is_down
#define set_is_down(b) input->buttons[b].is_down = false;
#define pressed(b) (input->buttons[b].is_down && input->buttons[b].changed)
#define released(b) (!input->buttons[b].is_down && input->buttons[b].changed)

float player_1_p, player_1_dp, player_2_p, player_2_dp; //dp = velocity
float arena_half_size_x = 85, arena_half_size_y = 45;
float player_half_size_x = 2.5, player_half_size_y = 12;
float ball_p_x, ball_p_y, ball_dp_x = 200, ball_dp_y, ball_half_size = 1;
float player_1_ddp = 0.f; //acceleration
float player_2_ddp = 0.f;

int player_1_score = 0;
int player_2_score = 0;

internal void simulate_player(float *p, float *dp, float ddp, float dt) {

	ddp -= *dp * 10.f;

	*p = *p + *dp * dt + ddp * dt * dt * .5f;
	*dp = *dp + ddp * dt;

	if (*p + player_half_size_y > arena_half_size_y) {
		*p = arena_half_size_y - player_half_size_y;
		*dp *= 0;
	}
	else if (*p - player_half_size_y < -arena_half_size_y) {
		*p = -arena_half_size_y + player_half_size_y;
		*dp *= 0;
	}
}

internal bool aabb_vs_aabb( float p1x, float p1y, float hs1x, float hs1y, 
					float p2x, float p2y, float hs2x, float hs2y) {
	return (p1x + hs1x > p2x - hs2x &&
		p1x - hs1x < p2x + hs2x &&
		p1y + hs1x > p2y - hs2y &&
		p1y + hs1x < p2y + hs2y);
}

enum Gamemode {
	GM_MENU,
	GM_DIFFICULTY,
	GM_GAMEPLAY,
};

Gamemode current_gamemode;
int hot_button;
int difficulty = 1;
bool enemy_is_ai;
bool scored;
bool useCursor;
float ai_speed = 0;
float ai_detection = 0;
float xPos = 0;
float yPos = 0;

internal void simulate_game(Input *input, float dt) {
	draw_rect(0, 0, arena_half_size_x, arena_half_size_y, 0x506670);
	draw_arena_border(arena_half_size_x, arena_half_size_y, 0x000000);

	if (current_gamemode == GM_GAMEPLAY) {
		if (scored == false) {
			ball_p_x += ball_dp_x * dt;
			ball_p_y += ball_dp_y * dt;
		}

		if (aabb_vs_aabb(ball_p_x, ball_p_y, ball_half_size, ball_half_size, 80, player_1_p, player_half_size_x, player_half_size_y)) {
			ball_p_x = 80 - player_half_size_x - ball_half_size;
			ball_dp_x *= -1;
			ball_dp_y = (ball_p_y - player_1_p) * 2;
		}
		else if (aabb_vs_aabb(ball_p_x, ball_p_y, ball_half_size, ball_half_size, -80, player_2_p, player_half_size_x, player_half_size_y)) {
			ball_p_x = -80 + player_half_size_x + ball_half_size;
			ball_dp_x *= -1;
			ball_dp_y = (ball_p_y - player_2_p) * 2;
		}

		if (ball_p_y + ball_half_size > arena_half_size_y) {
			ball_p_y = arena_half_size_y - ball_half_size;
			ball_dp_y *= -1;
		}
		else if (ball_p_y - ball_half_size < -arena_half_size_y) {
			ball_p_y = -arena_half_size_y + ball_half_size;
			ball_dp_y *= -1;
		}

		if (ball_p_x + ball_half_size > arena_half_size_x) {
			ball_p_x = 0;
			ball_p_y = 0;
			ball_dp_x *= -1;
			ball_dp_y = 0;
			player_1_score++;
			scored = true;
		}
		else if (ball_p_x - ball_half_size < -arena_half_size_x) {
			ball_p_x = 0;
			ball_p_y = 0;
			ball_dp_x *= -1;
			ball_dp_y = 0;
			player_2_score++;
			scored = true;
		}
		if (scored == true) {
			if (pressed(BUTTON_ENTER)) {
				scored = false;
			}
		}

		draw_number(player_1_score, -10, 40, 1.f, 0xbbffbb);
		draw_number(player_2_score, 10, 40, 1.f, 0xbbffbb);

		//Rendering
		draw_rect(ball_p_x, ball_p_y, ball_half_size, ball_half_size, 0xFFFFFF);//ball
		draw_rect(80, player_1_p, player_half_size_x, player_half_size_y, 0x00FFFF);//player
		draw_rect(-80, player_2_p, player_half_size_x, player_half_size_y, 0x00FFFF);//Enemy

	}
	else if(current_gamemode == GM_MENU) {
		if (pressed(BUTTON_LEFT) || pressed(BUTTON_RIGHT)) {
			hot_button = !hot_button;
		}

		if (pressed(BUTTON_ENTER)) {
			enemy_is_ai = hot_button? 0:1;
			if (enemy_is_ai) {
				current_gamemode = GM_DIFFICULTY;
			}
			else {
				current_gamemode = GM_GAMEPLAY;

			}
		}
		if (hot_button == 0) {
			draw_text("SINGLE PLAYER", -80, -10, 1, 0xffffff);
			draw_text("MULTIPLAYER", 20, -10, 1, 0xaaaaaa);
		}
		else {
			draw_text("SINGLE PLAYER", -80, -10, 1, 0xaaaaaa);
			draw_text("MULTIPLAYER", 20, -10, 1, 0xffffff);
		}

		draw_text("PONG", 0, 10, 1, 0x000000);
	}
	else { //Difficulty Menu
		if (pressed(BUTTON_UP)) {
			if (difficulty == 1) {
				difficulty = 3;
			}
			else {
				difficulty--;
			}
		}
		else if (pressed(BUTTON_DOWN)) {
			if (difficulty == 3) {
				difficulty = 1;
			}
			else {
				difficulty++;
			}
		}

		if (pressed(BUTTON_ENTER)) {
			if (difficulty == 1) {
				ai_speed = 200;
				ai_detection = 300;
			}
			else if(difficulty == 2) {
				ai_speed = 600;
				ai_detection = 900;

			}
			else {
				ai_speed = 1300;
				ai_detection = 1300;
			}
			current_gamemode = GM_GAMEPLAY;
		}

		if (difficulty == 1) {
			draw_text("EASY", -10, 20, 1, 0xffffff);
			draw_text("MEDIUM", -10, 0, 1, 0xaaaaaa);
			draw_text("HARD", -10, -20, 1, 0xaaaaaa);
		}
		else if (difficulty == 2) {
			draw_text("EASY", -10, 20, 1, 0xaaaaaa);
			draw_text("MEDIUM", -10, 0, 1, 0xffffff);
			draw_text("HARD", -10, -20, 1, 0xaaaaaa);
		}
		else {
			draw_text("EASY", -10, 20, 1, 0xaaaaaa);
			draw_text("MEDIUM", -10, 0, 1, 0xaaaaaa);
			draw_text("HARD", -10, -20, 1, 0xffffff);
		}
		draw_text("DIFFICULTY", -25, 40, 1, 0xffffff);
	}

}
internal void processPlayer(Input* input, float dt) {
	if (current_gamemode == GM_GAMEPLAY) {

		if (!enemy_is_ai) {
			if (is_down(BUTTON_UP)) player_1_ddp = 2000;
			if (is_down(BUTTON_DOWN)) player_1_ddp = -2000;
		}
		else {
			player_1_ddp = (ball_p_y - player_1_p + 10.f) * 100;
			if (player_1_ddp > ai_detection) player_1_ddp = ai_speed;
			if (player_1_ddp < -ai_detection) player_1_ddp = -ai_speed;
		}
		if (is_down(BUTTON_W)) {
			player_2_ddp = 2000;
			set_is_down(BUTTON_W);
		
		}
		if (is_down(BUTTON_S)) {
			player_2_ddp = -2000;
			set_is_down(BUTTON_S);
		}
		simulate_player(&player_1_p, &player_1_dp, player_1_ddp, dt);
		simulate_player(&player_2_p, &player_2_dp, player_2_ddp, dt);
	}
}