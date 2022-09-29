// Copyright 2022 Alduinar & CO MIT

#include <ncurses.h>
#include <string.h>
#include <unistd.h>

#define DELAY 50000

typedef struct paddle {
    // paddle variables
    int x;
    int y;    /* y is the 'top' of the paddle */
    int len;
    int score;
} paddle_t;

typedef struct ball {
    /* ball variables */
    int x;
    int y;
    int next_x;
    int next_y;
    int x_vel;
    int y_vel;
} ball_t;

typedef struct dimensions {
    int x;
    int y;
} dimensions_t;

void draw_ball(ball_t *input);
void draw_paddle(paddle_t *paddle);
void draw_score1(paddle_t *inpt_paddle, dimensions_t *wall);
void draw_score2(paddle_t *inpt_paddle, dimensions_t *wall);
void paddle_collisions(ball_t *inpt_ball, paddle_t *inpt_paddle);
void paddle_pos1(paddle_t *pddl, dimensions_t *wall, char dir);
void paddle_pos2(paddle_t *pddl, dimensions_t *wall, char dir);

void draw_position(ball_t *input, dimensions_t *wall);
int score_player1(ball_t *usr_ball, dimensions_t *walls);
int score_player2(ball_t *usr_ball, dimensions_t *walls);
int check_endgame(void);

int wall_collisions(ball_t *usr_ball, dimensions_t *walls);
int kbdhit();

paddle_t usr_paddle1 = { 0 };
paddle_t usr_paddle2 = { 0 };

int main(int argc, char **argv) {
    // initialize curses
    initscr();
    noecho();
    curs_set(0);

    dimensions_t walls = { 0 };
    getmaxyx(stdscr, walls.y, walls.x); /* get dimensions */

    // paddle 1
    usr_paddle1.x = 2;
    usr_paddle1.y = 3;
    usr_paddle1.len = 7; usr_paddle1.score = 0;

    // paddle 2
    usr_paddle2.x = 78;
    usr_paddle2.y = 3;
    usr_paddle2.len = 7; usr_paddle2.score = 0;

    ball_t usr_ball = { 0 };

    usr_ball.x = walls.x / 2;
    usr_ball.y = walls.y / 2;
    usr_ball.next_x = 0;
    usr_ball.next_y = 0;
    usr_ball.x_vel = 1;
    usr_ball.y_vel = 1;

    // keypresses
    int keypress = 0;
    int run = 1;
    nodelay(stdscr, TRUE);
    scrollok(stdscr, TRUE);

    while (run) {
        while (kbdhit()) {
            getmaxyx(stdscr, walls.y, walls.x);
            clear(); /* clear screen of all printed chars */

            draw_ball(&usr_ball);

            draw_paddle(&usr_paddle1);
            draw_paddle(&usr_paddle2);

            draw_position(&usr_ball, &walls);

            draw_score1(&usr_paddle1, &walls);
            draw_score2(&usr_paddle1, &walls);
            refresh(); /* draw to term */
            usleep(DELAY);

            // set next position
            usr_ball.next_x = usr_ball.x + usr_ball.x_vel;
            usr_ball.next_y = usr_ball.y + usr_ball.y_vel;

            // check for collisions
            paddle_collisions(&usr_ball, &usr_paddle1);
            paddle_collisions(&usr_ball, &usr_paddle2);

            // check for scoring
            score_player1(&usr_ball, &walls);
            score_player2(&usr_ball, &walls);

            if (wall_collisions(&usr_ball, &walls)) {
                run = 0;
            }

            // this doesn't end game :(
            if (check_endgame()) {
                break;
            }
        }

        keypress = getch();

        switch (keypress) {
        // player 1
        case 'z':
        case 'a':
            paddle_pos1(&usr_paddle1, &walls, keypress);
            break;

        // player 2
        case 'k':
        case 'm':
            paddle_pos2(&usr_paddle2, &walls, keypress);
            break;

        case 'p': /* pause functionality */
            mvprintw(1, 0, "PAUSE - press any key to resume");
            while (getch() == ERR) {
                usleep(DELAY * 7);
            }
            break;

        case 'q':
            run = 0;
            break;
        }
    }
    endwin();

    printf("GAME OVER\n");
    printf("Player 1 Score: %d\n", usr_paddle1.score);
    printf("Player 2 Score: %d\n", usr_paddle2.score);

    return 0;
}

void paddle_pos1(paddle_t *pddl, dimensions_t *wall, char dir) {
    if (dir == 'z') { /* moving down */
        if (pddl->y + pddl->len + 1 <= wall->y)
            pddl->y++;
    } else {          /* moving up (must be 'm') */
        if (pddl->y - 1 >= 0)
            pddl->y--;
    }

    return;
}

void paddle_pos2(paddle_t *pddl, dimensions_t *wall, char dir) {
    if (dir == 'm') { /* moving down */
        if (pddl->y + pddl->len + 1 <= wall->y)
            pddl->y++;
    } else {          /* moving up (must be 'm') */
        if (pddl->y - 1 >= 0)
            pddl->y--;
    }
    return;
}

int wall_collisions(ball_t *usr_ball, dimensions_t *walls) {
    // check for X
    if (usr_ball->next_x >= walls->x || usr_ball->next_x < 0) {
        usr_ball->x_vel *= -1;
    } else {
        usr_ball->x += usr_ball->x_vel;
    }

    // check for Y
    if (usr_ball->next_y >= walls->y || usr_ball->next_y < 0) {
        usr_ball->y_vel *= -1;
    } else {
        usr_ball->y += usr_ball->y_vel;
    }

    return 0;
}

int score_player1(ball_t *usr_ball, dimensions_t *walls) {
    if (usr_ball->next_x == 0) {
        usr_paddle2.score++;
        return 1;
    }
    return 0;
}

int score_player2(ball_t *usr_ball, dimensions_t *walls) {
    if (usr_ball->next_x == 80) {
        usr_paddle1.score++;
        return 1;
    }
    return 0;
}


int check_endgame(void) {
    if (usr_paddle1.score || usr_paddle2.score == 2) {
        return 1;
    }
    return 0;
}

void paddle_collisions(ball_t *inpt_ball, paddle_t *inpt_paddle) {
    if (inpt_ball->next_x == inpt_paddle->x) {
        if (inpt_paddle->y <= inpt_ball->y &&
            inpt_ball->y <=
            inpt_paddle->y + inpt_paddle->len) {
            inpt_ball->x_vel *= -1;
        }
    }
    return;
}

void draw_ball(ball_t *input) {
    mvprintw(input->y, input->x, "O");
    return;
}

void draw_paddle(paddle_t *paddle) {
    int i;

    for (i = 0; i < paddle->len; i++)
        mvprintw(paddle->y + i, paddle->x, "|");

    return;
}

void draw_score1(paddle_t *inpt_paddle, dimensions_t *wall) {
    mvprintw(0, wall->x / 2 - 20, "Player 1 score: %d", usr_paddle1.score);

    return;
}

void draw_score2(paddle_t *inpt_paddle, dimensions_t *wall) {
    mvprintw(0, wall->x / 2, "Player 2 score: %d", usr_paddle2.score);

    return;
}

void draw_position(ball_t *input, dimensions_t *wall) {
    mvprintw(1, wall->x / 2, "x:%d", input->x);
    mvprintw(1, wall->x / 2 - 5, "y:%d", input->y);

    return;
}

int kbdhit() {
    int key = getch();

    if (key != ERR) {
        ungetch(key);
        return 0;
    } else {
        return 1;
    }
}
