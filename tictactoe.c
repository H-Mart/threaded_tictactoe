#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define FALSE 0
#define TRUE 1
#define NUM_SPACES 9
#define NO_MOVE 0
#define O_PLAYER 1
#define X_PLAYER 2
#define NUM_WIN_CONDITIONS 8

typedef struct {
    int current_player;
    int game_over;
    int board[NUM_SPACES];
} GameState;

typedef struct {
    pthread_t thread_id;
    int player_name;
} PlayerData;

typedef struct {
    int valid_moves[9];
    int num_valid;
} ValidMoves;

int winning_combos[NUM_WIN_CONDITIONS][3] = {{0, 1, 2}, {3, 4, 5}, {6, 7, 8}, {0, 3, 6}, {1, 4, 7}, {2, 5, 8}, {0, 4, 8}, {2, 4, 6}};

GameState global_game_state;

pthread_mutex_t signal_mutex;

char* get_name(int id) {
    if (id == O_PLAYER) {
        return "O";
    } else if (id == X_PLAYER) {
        return "X";
    } else {
        return " ";
    }
}

int get_next_player(int curr_id) {
    return (curr_id == O_PLAYER ? X_PLAYER : O_PLAYER);
}

int getRandom(int rangeLow, int rangeHigh) {
    double myRand = rand() / (1.0 + RAND_MAX);
    int range = rangeHigh - rangeLow + 1;
    int myRand_scaled = (myRand * range) + rangeLow;
    return myRand_scaled;
}

ValidMoves get_valid_moves() {
    ValidMoves valid_moves;

    for (int i = 0; i < NUM_SPACES; i++) {
        if (global_game_state.board[i] == NO_MOVE) {
            valid_moves.valid_moves[valid_moves.num_valid++] = i;
        }
    }

    return valid_moves;
}

void init_game() {
    for (int i = 0; i < NUM_SPACES; i++) {
        global_game_state.board[i] = NO_MOVE;
    }
    global_game_state.current_player = getRandom(O_PLAYER, X_PLAYER);
    global_game_state.game_over = FALSE;
}

void setup_time_seed() {
    struct timeval time;
    gettimeofday(&time, NULL);
    // srand for windows instead
    srand((unsigned int)time.tv_usec);
}

void make_move(int position, int player_name) {
    global_game_state.board[position] = player_name;
}

int check_winner() {
    for (int i = 0; i < NUM_WIN_CONDITIONS; i++) {
        int pos_one = winning_combos[i][0];
        int pos_two = winning_combos[i][1];
        int pos_three = winning_combos[i][2];
        // first condition is to check if the first position is not placed yet
        // second and third are checking to make sure all three are equal
        if (global_game_state.board[pos_one] > 0 &&
            global_game_state.board[pos_one] == global_game_state.board[pos_two] &&
            global_game_state.board[pos_two] == global_game_state.board[pos_three]) {
            return global_game_state.board[pos_one];
        }
    }
    return 0;
}

void print_board() {
    printf(" %s | %s | %s\n", get_name(global_game_state.board[0]), get_name(global_game_state.board[1]), get_name(global_game_state.board[2]));
    printf("---|---|---\n");
    printf(" %s | %s | %s\n", get_name(global_game_state.board[3]), get_name(global_game_state.board[4]), get_name(global_game_state.board[5]));
    printf("---|---|---\n");
    printf(" %s | %s | %s\n\n", get_name(global_game_state.board[6]), get_name(global_game_state.board[7]), get_name(global_game_state.board[8]));
}

void take_turn(PlayerData* player_data) {
    pthread_mutex_lock(&signal_mutex);

    ValidMoves valid = get_valid_moves();

    print_board();

    if (valid.num_valid == 0) {
        printf("GAME OVER - TIE\n");
        global_game_state.game_over = TRUE;
        return;
    }

    int move = getRandom(0, valid.num_valid - 1);
    make_move(valid.valid_moves[move], player_data->player_name);
    int winner = check_winner();

    if (winner == O_PLAYER) {
        print_board();
        printf("GAME OVER - O WINS\n");
        global_game_state.game_over = TRUE;

    } else if (winner == X_PLAYER) {
        print_board();
        printf("GAME OVER - X WINS\n");
        global_game_state.game_over = TRUE;
    }

    global_game_state.current_player = get_next_player(player_data->player_name);
    
    pthread_mutex_unlock(&signal_mutex);
}

void* play_game(void* data) {
    PlayerData* player_data = (PlayerData*)data;

    while (!global_game_state.game_over) {
        if (global_game_state.current_player == player_data->player_name) {
            take_turn(player_data);
        }
        sleep(.5);
    }
}

int main(int argc, char* argv) {
    setup_time_seed();
    init_game();

    pthread_mutex_init(&signal_mutex, NULL);

    PlayerData player_o = {0, O_PLAYER};
    PlayerData player_x = {0, X_PLAYER};
    pthread_create(&player_o.thread_id, NULL, play_game, (void*)&player_o);
    pthread_create(&player_x.thread_id, NULL, play_game, (void*)&player_x);
    pthread_join(player_o.thread_id, NULL);
    pthread_join(player_x.thread_id, NULL);
}