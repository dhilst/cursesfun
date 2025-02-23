#include <algorithm>
#include <array>
#include <bits/ranges_algo.h>
#include <cassert>
#include <climits>
#include <concepts>
#include <functional>
#include <iostream>
#include <ncurses.h>
#include <ostream>
#include <stdexcept>
#include <vector>
#include <ranges>
#include <vector>
#include <cstdlib>

namespace funcs {

template <typename T, template <typename> typename C = std::vector>
static constexpr auto map(C<T>& input, auto func, auto... args)
-> decltype(auto)
{
    C<decltype(func(std::declval<T>(), args...))> output(input.size());
    for (auto& val : input) {
        output.emplace_back(func(val, args...));
    }
    return output;
}

static constexpr int factorial(int n)
{
    return std::ranges::fold_left(
        std::views::iota(1, n+1),
        1,
        std::multiplies<>{});
};
static_assert(!(factorial(4) == 23));
static_assert(factorial(4) == 24);
static_assert(factorial(5) == 120);

static constexpr auto map2(const auto& input, auto func)
-> decltype(auto)
{
    return input
        | std::views::transform(func)
        | std::ranges::to<std::vector>();
}
static_assert(map2(std::vector<int>{1,2,3,4,5},
                   factorial)
    == std::vector<int>{1,2,6,24,120});

template <typename T, template <typename> typename C = std::vector,
    typename R, typename... Args>
static constexpr C<R> map(C<T>& input, R(T::*method)(Args...), Args... arg)
{
    constexpr C<R> output;
    output.reserve(input.size());
    for (auto& val : input) {
        output.emplace_back((val.*method)(arg...));
    }
    return output;
}


struct test {
    int v;
    int inc() {
        return v++;
    }
};

static consteval auto testMap(auto cb)
{
    std::vector<int> vec = {1,2,3};
    std::vector<int>value = vec
        | std::views::transform(cb)
        | std::ranges::to<std::vector>();

    return value;
};
static_assert(testMap([](auto i){ return i*2; }) == std::vector<int>{2,4,6});


template <typename T, typename ...Arg,  template <typename> typename C = std::vector>
static void each(C<T>& vec, void (T::*method)(Arg... arg), Arg... arg)
{
    for (auto& val : vec) {
        (val.*method)(arg...);
    }
}

class CallOnce {
    bool m_done = false;
public:
    using Type = std::function<void()>;
    void call(Type&& f) {
        if (m_done) {
            return;
        }

        m_done = true;
        f();
        return;
    }

    void operator()(Type&& f)
    {
        call(std::forward<Type>(f));
    }
};

}

namespace draw {
static void point(int x, int y, char symbol)
{
    mvaddch(y, x, symbol);
}

static void erase(int x, int y, char ch = ' ')
{
    point(x, y, ch);
}

template <typename ...Args>
static void str(int x, int y, auto fmt, Args... args)
{
    mvprintw(y, x, fmt, args...);
}

static void refr()
{
    refresh();
}

static void sleep(auto dt)
{
    napms(dt * 1000);
}

enum Color : int {
    MIN = 0,
    RED,
    GREEN,
    YELLOW,
    BLUE,
    MAGENTA,
    CYAN,
    WHITE,
    BLACK,
    MAX,
};

static void init()
{
    initscr();
    noecho();
    curs_set(0);
    // Start color functionality
    start_color();
    // Define color pairs (index, foreground, background)
    // init_pair(draw::Color::BLACK, COLOR_BLACK, COLOR_BLACK);
    init_pair(draw::Color::RED, COLOR_RED, COLOR_BLACK);
    init_pair(draw::Color::GREEN, COLOR_GREEN, COLOR_BLACK);
    init_pair(draw::Color::YELLOW, COLOR_YELLOW, COLOR_BLACK);
    init_pair(draw::Color::BLUE, COLOR_BLUE, COLOR_BLACK);
    init_pair(draw::Color::MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(draw::Color::CYAN, COLOR_CYAN, COLOR_BLACK);
    init_pair(draw::Color::WHITE, COLOR_WHITE, COLOR_BLACK);
}

static void clear()
{
    ::clear();
}


static void color(const Color c, const auto draw)
{
    if (!(c > Color::MIN && c < Color::MAX))
    {
        throw std::invalid_argument(std::format("Invalid color {}", static_cast<int>(c)));
    }
    attron(COLOR_PAIR(c) | A_BOLD);
    draw();
    attroff(COLOR_PAIR(c)| A_BOLD);
}

};



namespace phys {
// inclusive/exclusive
static int random(int min, int max) {
    assert(min >= 0);
    assert(max > 0);
    assert(min < max);
    auto v = (std::rand() % (max - min)) + min;
    assert(v >= min);
    assert(v < max);
    return v;
}

template <typename T>
struct Vec2 {
    T x, y;
};

const auto gravityMag = 9.81;

constexpr auto applyForce(const auto& force, const auto& mass)
{
    return Vec2{
        force.x / mass,
        force.y / mass
    };
};

struct Body {
    // Position,  Velocity and Acceleration
    Vec2<double> p{.x=0, .y=0};
    Vec2<double> v{.x=0, .y=0};
    double mass = 1;
    draw::Color c;
    char ch = char(phys::random(65, 64 + 24));

    Body()
    {
        p.x = phys::random(0, COLS-1);
        p.y = phys::random(0, LINES-1);
        c = draw::Color(phys::random(draw::Color::MIN+1, draw::Color::MAX));
    }

    void draw()
    {
        draw::color(c, [&](){
            draw::point(p.x, p.y, ch);
        });
    }

    void erase()
    {
        draw::point(p.x, p.y, ' ');
    }

    void move(int newX, int newY)
    {
        if (newX < 0 ||
            newX > COLS - 1) {
            v.x *= -0.5;
        }

        if (newY < 0 ||
            newY > LINES - 1) {
            v.y *= -0.5;
        }

        funcs::CallOnce once;
        if (newX >= 0 &&
            newX < COLS &&
            newX != p.x) {
            once([&](){ erase(); });
            p.x = newX;
        }
            
        if (newY >= 0 &&
            newY < LINES &&
            newY != p.y) {
            once([&](){ erase(); });
            p.y = newY;
        }
    }

    void update(auto dt) 
    {
        v = Vec2{
            v.x + 0,
            v.y + gravityMag,
        };
        auto newP = Vec2{
            p.x + v.x * dt,
            p.y + v.y * dt
        };
        move(newP.x, newP.y);
        draw::str(30, LINES - 2, "Pos: %f %f", p.x, p.y);
        draw::str(60, LINES - 2, "Vel: %f %f", v.x, v.y);
        draw::refr();
    }
};

};

template <typename T>
std::ostream& operator<<(std::ostream& os, std::vector<T> vec)
{
    for (auto& v : vec) {
        os << v << "," ; 
    }
    return os;
}

int main()
{
    using namespace phys;
    // Define color pairs (index, foreground, background)
    draw::init();

    int iteration = 0;
    std::vector<Body> p(10);

    refresh();

    double dt = 1.0/60.0; // 60 fps
    int count = 0;
    draw::clear();
    
        draw::str(0, LINES - 1, "World: %dx%d", COLS, LINES); 

    while (true) {
        draw::color(draw::Color::RED, [&]() {
            draw::str(0, LINES - 2, "Iteration: %d", iteration); 
        });
        funcs::each(p, &Body::draw);
        draw::refr();
        draw::sleep(dt);
        funcs::each(p, &Body::update, dt);

        iteration++;
    }

    endwin();
    return 0;
}
