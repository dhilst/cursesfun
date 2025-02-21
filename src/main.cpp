#include <cassert>
#include <climits>
#include <functional>
#include <iostream>
#include <ncurses.h>
#include <ostream>
#include <random>

namespace draw {
static void point(int x, int y, char symbol)
{
    mvaddch(y, x, symbol);
}

enum Color : int {
    BLACK = 1,
    RED,
    GREEN,
    YELLOW,
    BLUE,
    MAGENTA,
    CYAN,
    WHITE,
    MAX,
};

static void initColors()
{
    // Define color pairs (index, foreground, background)
    init_pair(draw::Color::BLACK, COLOR_BLACK, COLOR_BLACK);
    init_pair(draw::Color::RED, COLOR_RED, COLOR_BLACK);
    init_pair(draw::Color::GREEN, COLOR_GREEN, COLOR_BLACK);
    init_pair(draw::Color::YELLOW, COLOR_YELLOW, COLOR_BLACK);
    init_pair(draw::Color::BLUE, COLOR_BLUE, COLOR_BLACK);
    init_pair(draw::Color::MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(draw::Color::CYAN, COLOR_CYAN, COLOR_BLACK);
    init_pair(draw::Color::WHITE, COLOR_WHITE, COLOR_BLACK);
}


static void color(Color c, auto draw)
{
    attron(COLOR_PAIR(c) | A_BOLD);
    draw();
    attroff(COLOR_PAIR(c)| A_BOLD);
}

template <typename T>
static void draw(std::vector<T> vec, auto color = Color::RED)
{
    for (auto& val : vec) {
        val.draw();
    }
}

};

namespace forces {

template <typename T>
static void gravity(T& obj)
{
    obj.move(obj.x, obj.y + 2);
};


};

namespace nums {
int random(int min, int max) {
    assert(max > 0);
    return (std::rand() % max + 1) + min;
}
};


template <typename T>
struct Velocity {
    T x, y;

    Velocity apply(auto a, auto b, auto dt)
    {
        return Velocity(x + a * dt, y + b * dt);
    }
};


struct Particle {
    int x = 0;
    int y = 0;
    Velocity<float> v{.x=1.1, .y=1.2};
    draw::Color c;
    char ch = char(nums::random(65, 64 + 24));

    Particle()
    {
        x = nums::random(0, COLS-1);
        y = nums::random(0, LINES-1);
        v.x = 1 + static_cast<float>(x)/50;
        v.y = 1 + static_cast<float>(y)/50;
        c = draw::Color(int(nums::random(0, draw::Color::MAX)));
    }

    void draw()
    {
        draw::color(c, [&](){
            draw::point(x, y, ch);
        });
    }

    void erase()
    {
        draw::point(x, y, ' ');
    }

    void move(int newX, int newY)
    {
        if (newX < 0 ||
            newX > COLS - 1) {
            v.x *= -1;
        }

        if (newY < 0 ||
            newY > LINES - 1) {
            v.y *= -1;
        }

        auto erased = false;
        if (newX >= 0 &&
            newX < COLS &&
            newX != x) {
            erase();
            erased = true;
            x = newX;
        }
            
        if (newY >= 0 &&
            newY < LINES &&
            newY != y) {
            if (!erased) {
                erase();
            }
            y = newY;
        }

    }


    void update(auto dt) 
    {
        move(x + v.x * dt, y + v.y * dt);
        // forces::gravity(*this);
    }
};


int main() {
    initscr();
    noecho();
    curs_set(0);
    // Start color functionality
    start_color();

    // Define color pairs (index, foreground, background)
    draw::initColors();

    int iteration = 0;
    std::vector<Particle> p(100);

    refresh();

    timeout(100);
    auto dt = 1;
    int count = 0;

    auto drawAll = [&p]() {
        for (auto& p : p) {
            p.draw();
        };
    };

    auto updateAll= [&p](auto dt) {
        for (auto& p : p) {
            p.update(dt);
        };
    };


    while (true) {
        draw::color(draw::Color::GREEN, [&]() {
            mvprintw(LINES - 1, 0, "Iteration: %d", iteration);
        });
        drawAll();
        refresh();
        napms(dt*16);
        updateAll(dt);
        iteration++;
    }

    endwin();
    return 0;
}
