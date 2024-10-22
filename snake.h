#ifndef SNAKE_H
#define SNAKE_H

#include <ncurses.h>
#include <unistd.h>
#include <string.h>

#include "matrix.h"

class Snake {
public:
    Snake() : m_field(m_height, m_width) {
        init_ncurses();
        init_game();
    }

    ~Snake() {
        deinit_ncurses();
        deinit_game();
    }

   void play() {
        render();

        int count = 0; // change this from counter to timer
        int max_count = 25;
        while (m_state != ENDING) {
            usleep(1000 * 250 / max_count);

            switch(m_state) {
                case INITIAL:
                    get_user_input();
                    break;
                case TWISTING:
                    get_user_input();
                    break;
                case MOVING:
                    move_snake_forward();
                    break;
                case PAUSE:
                    get_user_input(); // ?
                    break;
                case GAME_OVER:
                    get_user_input();
                    break;
                case ENDING:
                    break;
            }

            if (m_state == TWISTING && ++count == max_count) {
                m_state = MOVING;
                count = 0;
            }

            render();
        }
    }

private:
    std::string m_block = "[+]";

    struct Point {
        int x{ 0 };
        int y{ 0 };
    };

    enum States {
        INITIAL,
        TWISTING, // handling user input (wasd)
        MOVING, // snake moves one block forward
        PAUSE,
        GAME_OVER,
        ENDING,
    } m_state{ INITIAL };

    enum Direction {
        UP,
        DOWN,
        LEFT,
        RIGHT,
    } m_dir{ UP }, m_next_dir{ UP };

    int m_height{ 15 };
    int m_width{ 20 };
    Matrix m_field;

    Point* m_snake;
    int m_size{ 0 };
    int m_max_size{ 10 };

    void reset_game() {
        m_dir = UP;
        m_next_dir = UP;

        m_snake[4].x = 7;
        m_snake[4].y = 7;

        m_snake[3].x = 8;
        m_snake[3].y = 7;

        m_snake[2].x = 9;
        m_snake[2].y = 7;

        m_snake[1].x = 10;
        m_snake[1].y = 7;

        m_snake[0].x = 10;
        m_snake[0].y = 6;

        m_size = 5;
    }

    void init_game() {
        m_snake = new Point[m_max_size]{};

        // snake grows from behind
        m_snake[4].x = 7;
        m_snake[4].y = 7;

        m_snake[3].x = 8;
        m_snake[3].y = 7;

        m_snake[2].x = 9;
        m_snake[2].y = 7;

        m_snake[1].x = 10;
        m_snake[1].y = 7;

        m_snake[0].x = 10;
        m_snake[0].y = 6;

        m_size = 5;
    }

    void deinit_game() {
        if (m_snake != nullptr) {
            delete[] m_snake;
        }
    }

    void get_user_input() {
        char key = getch();

        if (key == 'q') {
            m_state = ENDING;
        } else if (key == 'r') {
            reset_game();
            m_state = INITIAL;
        } else if (m_state == INITIAL) {
            if (key == '\n') {
                m_state = TWISTING;
            }
        } else if (m_state == TWISTING) {
            if (key == 'w' || key == 'a' || key == 's' || key == 'd') {
                twist_snake(key);
            }
        }
    }
    
    void twist_snake(char key) {
        if (m_dir != m_next_dir) { // not first twist on this step
            return;
        }

        if (key == 'w') {
            twist_snake_upward();
        } else if (key == 'a') {
            twist_snake_left();
        } else if (key == 's') {
            twist_snake_down();
        } else if (key == 'd') {
            twist_snake_right();
        }
    }

    void twist_snake_upward() {
        if (m_dir != DOWN) {
            m_next_dir = UP;
        }
    }

    void twist_snake_left() {
        if (m_dir != RIGHT) {
            m_next_dir = LEFT;
        }
    }

    void twist_snake_down() {
        if (m_dir != UP) {
            m_next_dir = DOWN;
        }
    }

    void twist_snake_right() {
        if (m_dir != LEFT) {
            m_next_dir = RIGHT;
        }
    }

    void move_snake_forward() {
        for (int i = m_size - 1; i > 0; --i) {
            m_snake[i].x = m_snake[i - 1].x;
            m_snake[i].y = m_snake[i - 1].y;
        }

        Point& head = m_snake[0];

        if (m_next_dir == UP) {
            head.y -= 1;
        } else if (m_next_dir == DOWN) {
            head.y += 1;
        } else if (m_next_dir == LEFT) {
            head.x -= 1;
        } else if (m_next_dir == RIGHT) {
            head.x += 1;
        }

        m_dir = m_next_dir;

        if (head.x == 0 || head.x == m_width - 1 || head.y == 0 || head.y == m_height - 1) {
            m_state = GAME_OVER;
        } else {
            for (int i = 1; i < m_size; ++i) {
                if (head.x == m_snake[i].x && head.y == m_snake[i].y) {
                    m_state = GAME_OVER;
                    return;
                }
            }

            m_state = TWISTING;
        }
    }

    void end_game() {
        
    }

    void add_snake_to_field() {
        for (int i = 0; i < m_size; ++i) {
            m_field(m_snake[i].y, m_snake[i].x) = 1;
        }
    }

    void remove_snake_from_field() {
        for (int i = 0; i < m_size; ++i) {
            m_field(m_snake[i].y, m_snake[i].x) = 0;
        }
    }

    void render() {
        if (m_state != INITIAL) { add_snake_to_field(); }

        WINDOW *game_window = newwin(m_height + 2, 3 * m_width + 2, 0, 0);

        refresh();
        box(game_window, 0, 0);

        if (m_state == INITIAL) {
            mvwprintw(game_window, 1, 2, "[ENTER]      to start");
            mvwprintw(game_window, 2, 2, "[W][A][S][D] to move");
            mvwprintw(game_window, 3, 2, "[R]          to restart");
            mvwprintw(game_window, 4, 2, "[Q]          to quit");
            mvwprintw(game_window, get_center_y(), get_center_x(7), "[START]"); 
        // } else if (m_state == GAME_OVER) {
        //     mvwprintw(game_window, get_center_y(), get_center_x(11), "[GAME_OVER]");
        } else if (m_state == PAUSE) {
            mvwprintw(game_window, get_center_y(), get_center_x(7), "[PAUSE]");
        } else {
            for (int i = 0; i < m_height; ++i) {
                for (int j = 0; j < m_width; ++j) {
                    if (m_field(i, j) == 1) {
                        mvwprintw(game_window, i + 1, 3 * j + 1, "[+]");
                    } else {
                        mvwprintw(game_window, i + 1, 3 * j + 1, "   ");
                    }
                }
            }

            if (m_state == GAME_OVER) {
                mvwprintw(game_window, get_center_y(), get_center_x(11), "[GAME_OVER]");
            }
        }

        wrefresh(game_window);

        delwin(game_window);

        if (m_state != INITIAL) { remove_snake_from_field(); }
    }

    int get_center_x(int length) {
        return (m_width * m_block.size() + 2 + 1 - length) / 2;
    }

    int get_center_y() {
        return m_height / 2 + 1;
    }

    void init_ncurses() {
        initscr();
        nodelay(stdscr, TRUE);
        curs_set(0);
        cbreak();
        keypad(stdscr, TRUE);
        noecho();
    }

    void deinit_ncurses() {
        endwin();
    }
};

#endif