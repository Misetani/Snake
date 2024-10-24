#ifndef SNAKE_H
#define SNAKE_H

#include <ncurses.h>
#include <iostream>
#include <cstdio>
#include <string>
#include <chrono>
#include <thread>

#include "matrix.h"
#include "rng.h"

class Snake {
private:
    const int c_ticks{ 25 }; // number of ticks between each shift
    const int c_speed{ 250 }; // time between each shift in ms

    const std::chrono::milliseconds c_tick_duration{ c_speed / c_ticks };

    RNG rng;

    struct Point {
        int x{ 0 };
        int y{ 0 };
    };

    enum States {
        INITIAL,
        TWISTING,
        PAUSE,
        GAME_OVER,
        ENDING,
    } m_state{ INITIAL };

    enum Direction {
        UP,
        DOWN,
        LEFT,
        RIGHT,
    } m_dir{ RIGHT }, m_next_dir{ RIGHT };

    int m_level{ 0 };

    int m_height{ 15 };
    int m_width{ 20 };
    Matrix m_field;

    Point* m_snake;
    int m_size{ 0 };
    int m_max_size{ 4 };

    int apple_x{ 0 };
    int apple_y{ 0 };

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

        int count = 0;
        while (m_state != ENDING) {
            std::this_thread::sleep_for(c_tick_duration);

            get_user_input();

            if (m_state == TWISTING && ++count == c_ticks) {
                move_snake_forward();
                count = 0;
            }

            render();
        }
    }

private:
    void terminate() {
        deinit_game();
        deinit_ncurses();
        exit(1);
    }

    void spawn_an_apple() {
        apple_x = rng.rand(0, m_width - 1);
        apple_y = rng.rand(0, m_height - 1);

        if (m_field(apple_y, apple_x) == 1) {
            spawn_an_apple();
        } else {
            for (int i = 0; i < m_size; ++i) {
                if (m_snake[i].x == apple_x && m_snake[i].y == apple_y) {
                    spawn_an_apple();
                }
            }
        }
    }

    void reset_game() {
        // load level
        FILE* file = fopen("./levels/level_0.txt", "r");

        if (file == nullptr) {
            std::cerr << "Error opening file!" << std::endl;
            terminate();
        }

        for (int i = 0; i < m_height; ++i) {
            for (int j = 0; j < m_width; ++j) {
                if(fscanf(file, "%d", &m_field(i, j)) != 1) {
                    std::cerr << "Error reading data!" << std::endl;
                    fclose(file);
                    terminate();
                }
            }
        }

        fclose(file);

        // create snake
        m_dir = RIGHT;
        m_next_dir = RIGHT;

        m_snake[2].x = 5;
        m_snake[2].y = 7;

        m_snake[1].x = 6;
        m_snake[1].y = 7;

        m_snake[0].x = 7;
        m_snake[0].y = 7;

        m_size = 3;

        // An apple
        spawn_an_apple();
    }

    void init_game() {
        m_snake = new Point[m_max_size]{};

        reset_game();
    }

    void deinit_game() {
        delete[] m_snake;
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
        } else if (m_state != GAME_OVER && key == 'p') {
            m_state = (m_state != PAUSE) ? PAUSE : TWISTING;
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

        switch (key) {
            case 'w': twist_snake_upward(); break;
            case 'a': twist_snake_left(); break;
            case 's': twist_snake_down(); break;
            case 'd': twist_snake_right(); break;
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

    Point* get_snake_copy() {
        Point* snake = new Point[m_size]{};

        for (int i = 0; i < m_size; ++i) {
            snake[i].x = m_snake[i].x;
            snake[i].y = m_snake[i].y;
        }

        return snake;
    }

    bool is_snake_dead() {
        Point& head = m_snake[0];

        // did snake crash with the playing field border?
        if (head.x < 0 || head.x >= m_width || head.y < 0 || head.y >= m_height) {
            return true;
        }
        else {
            for (int i = 0; i < m_size; ++i) {
                // did snake bite itself?
                if ((i >= 1 && head.x == m_snake[i].x && head.y == m_snake[i].y) ||
                    // or did it collide with some obstacle on the field?
                    (m_field(m_snake[i].y, m_snake[i].x)) == 1) {

                    return true;
                }
            }
        }

        return false;
    }

    void move_snake_forward() {
        Point* old_snake = get_snake_copy();

        Point tail = m_snake[m_size - 1];

        // move each snake block forward to the head
        for (int i = m_size - 1; i > 0; --i) {
            m_snake[i].x = m_snake[i - 1].x;
            m_snake[i].y = m_snake[i - 1].y;
        }

        Point& head = m_snake[0];

        // move head to the chosen direction
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

        if (is_snake_dead()) {
            delete[] m_snake;
            m_snake = old_snake;

            m_state = GAME_OVER;
        } else {
            m_state = TWISTING;
        }

        // Eating an apple?
        if (head.x == apple_x && head.y == apple_y) {
            if (m_size == m_max_size) {
                Point* new_snake = new Point[2 * m_max_size];

                for (int i = 0; i < m_size; ++i) {
                    new_snake[i] = m_snake[i]; // can I?
                }

                delete[] m_snake;

                m_snake = new_snake;

                m_max_size *= 2;
            }

            m_snake[m_size].x = tail.x;
            m_snake[m_size].y = tail.y;
            ++m_size;

            spawn_an_apple();
        }
    }

    void render() {
        int height = m_height + 2;
        int width = 3 * m_width + 2;
        WINDOW *game_window = newwin(height, width, 0, 0);

        refresh();
        box(game_window, 0, 0);

        if (m_state == INITIAL) {
            mvwprintw(game_window, 1, 2, "[ENTER]      to start");
            mvwprintw(game_window, 2, 2, "[W][A][S][D] to move");
            mvwprintw(game_window, 3, 2, "[R]          to restart");
            mvwprintw(game_window, 4, 2, "[Q]          to quit");
            mvwprintw(game_window, height / 2, (width + 1 - 7) / 2, "[START]");
        } else {
            for (int i = 0; i < m_height; ++i) {
                for (int j = 0; j < m_width; ++j) {
                    if (m_field(i, j) == 1) {
                        mvwprintw(game_window, i + 1, 3 * j + 1, "[ ]");
                    } else {
                        mvwprintw(game_window, i + 1, 3 * j + 1, "   ");
                    }
                }
            }

            for (int i = 0; i < m_size; ++i) {
                mvwprintw(game_window, m_snake[i].y + 1, 3 * m_snake[i].x + 1, "[-]");
            }

            mvwprintw(game_window, apple_y + 1, 3 * apple_x + 1, "[+]");

            if (m_state == GAME_OVER) {
                mvwprintw(game_window, height / 2, (width + 1 - 11) / 2, "[GAME_OVER]");
            } else if (m_state == PAUSE) {
                mvwprintw(game_window, height / 2, (width + 1 - 7) / 2, "[PAUSE]");
            }
        }

        wrefresh(game_window);

        delwin(game_window);
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