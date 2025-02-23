#include <algorithm>
#include <array>
#include <bits/ranges_algo.h>
#include <cassert>
#include <climits>
#include <cmath>
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
#include <chrono>

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
    init_pair(draw::Color::BLACK, COLOR_BLACK, COLOR_BLACK);
    init_pair(draw::Color::RED, COLOR_RED, COLOR_BLACK);
    init_pair(draw::Color::GREEN, COLOR_GREEN, COLOR_BLACK);
    init_pair(draw::Color::YELLOW, COLOR_YELLOW, COLOR_BLACK);
    init_pair(draw::Color::BLUE, COLOR_BLUE, COLOR_BLACK);
    init_pair(draw::Color::MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(draw::Color::CYAN, COLOR_CYAN, COLOR_BLACK);
    init_pair(draw::Color::WHITE, COLOR_WHITE, COLOR_BLACK);
}

static void point(int x, int y, char symbol, Color c)
{
    attron(COLOR_PAIR(c) | A_BOLD);
    mvaddch(y, x, symbol);
    attroff(COLOR_PAIR(c)| A_BOLD);
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

static void clear()
{
    ::clear();
}

static void circle(auto &&focus, auto radius = 10.0, char ch = '@',
                   draw::Color c = draw::Color::YELLOW)
{
    for (double theta = 0; theta < 2 * M_PI; theta += 0.1) {
        int x = focus.x + radius * cos(theta);
        int y = focus.y + radius * sin(theta);
        point(x, y, ch, c);
    }
};

static void line(auto start, auto end, char ch = 'u',
                 draw::Color c = draw::Color::GREEN)
{
    auto path = end - start;
    auto distance = path.mag();
    auto dir = path.norm();
    for (auto i = 0; i < distance; i += 1) {
        auto p = start + dir * i;
        point(p.x, p.y, ch, c);
    }
};

};

namespace phys {
// inclusive/exclusive
static int random(int min, int max) {
    assert(min >= 0);
    assert(max > 0);
    assert(min < max);
    auto now = std::chrono::high_resolution_clock::now();
    auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(
                     now.time_since_epoch())
                     .count();
    std::srand(static_cast<unsigned>(nanos));
    auto v = (std::rand() % (max - min)) + min;
    assert(v >= min);
    assert(v < max);
    return v;
}

template <typename T>
struct Vec2 {
    T x, y;

    Vec2 operator-() const {
        return Vec2{-x, -y};
    }

    Vec2 operator+(const Vec2& other) const {
        return Vec2{x + other.x, y + other.y};
    }

    Vec2 operator-(const Vec2& other) const {
        return Vec2{x - other.x, y - other.y};
    }

    Vec2 operator*(const T k) const {
        return Vec2{k * x, k * y};
    }

    T mag() const {
        return static_cast<T>(std::sqrt(x * x + y * y));
    }

    Vec2 norm() const {
        auto m = mag();
        return Vec2{x/m, y/m};
    };
};

constexpr auto applyForce(const auto& focus, const auto& obj, double mag = 400.0, const char ch = '0')
-> decltype(auto)
{
    draw::color(draw::Color::RED, [&](){
        draw::point(focus.x, focus.y, ch);
    });
    
    auto direction = Vec2{
        focus.x - obj.p.x,
        focus.y - obj.p.y,
    };
    auto force = Vec2{
        direction.norm().x * mag / obj.mass,
        direction.norm().y * mag / obj.mass,
    };
    return force;
};

struct Body {
    constexpr static auto track = false;
    // Position,  Velocity and Acceleration
    Vec2<double> p{.x=0, .y=0};
    Vec2<double> oldp{};
    Vec2<double> v{.x=0, .y=0};
    double mass = 10;
    draw::Color c;
    char ch = char(phys::random(65, 64 + 24));
    
    Body(int x, int y)
    {
        p.x = x;
        p.y = y;
        c = draw::Color(phys::random(draw::Color::MIN+1, draw::Color::MAX));
        oldp = p;
    }

    Body()
    {
        p.x = phys::random(0, COLS-1);
        p.y = phys::random(0, LINES-1);
        c = draw::Color(phys::random(draw::Color::MIN+1, draw::Color::MAX));
        oldp = p;
    }

    void draw()
    {
        draw::color(c, [&](){
            draw::point(p.x, p.y, ch);
        });
    }

    void erase()
    {
        if (track) {
            draw::color(c, [&](){
                draw::point(p.x, p.y, '.');
            });
        } else {
            draw::point(p.x, p.y, ' ');
        }
    }

    void move(const auto& newP)
    {
        if (newP.x < 0 ||
            newP.x > COLS - 1) {
            //v.x *= -0.5;
            //p.x = oldp.x;
        }

        if (newP.y < 0 ||
            newP.y > LINES - 1) {
            //v.y *= -0.5;
            //p.y = oldp.y;
        }

        funcs::CallOnce once;
        if (newP.x >= 0 &&
            newP.x < COLS &&
            newP.x != p.x) {
            once([&](){ erase(); });
            p.x = newP.x;
        }
            
        if (newP.y >= 0 &&
            newP.y < LINES &&
            newP.y != p.y) {
            once([&](){ erase(); });
            p.y = newP.y;
        }
    }

    void update(auto dt) 
    {
        const auto G = 1000;
        auto acc = applyForce(Vec2{
            COLS*.5,
            LINES*.4,
        }, *this, G*3, '^');
        acc = acc + applyForce(Vec2{
            COLS*.4,
            LINES*.5,
        }, *this, G*3, '<');
        acc = acc + applyForce(Vec2{
            COLS*.6,
            LINES*.5,
        }, *this, G*3, '>');
        acc = acc + applyForce(Vec2{
            COLS*.5,
            LINES*.6,
        }, *this, G*3, 'v');
        acc = acc + applyForce(Vec2{
            COLS*.5,
            LINES*.5,
        }, *this, G*5, '.');
        // v = Vec2 {
        //     (p.x - oldp.x) * 200.0 + acc.x * 2.0,
        //     (p.y - oldp.y) * 200.0 + acc.y * 2.0,
        // };
        // oldp = p;
        v = Vec2 {
            v.x + acc.x,
            v.y + acc.y,
        };
        auto newP = Vec2 {
            p.x + v.x * dt * dt,
            p.y + v.y * dt * dt,
        };
        move(newP);
        draw::str(30, LINES - 2, "Pos: %f %f", p.x, p.y);
        draw::str(60, LINES - 2, "Vel: %f %f", v.x, v.y);
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
    std::vector<Body> p(50);

    refresh();

    double dt = 1.0/60.0; // 60 fps
    int count = 0;
    draw::clear();
    
    draw::str(0, LINES - 1, "World: %dx%d", COLS, LINES); 
    auto focus = Vec2{COLS*.75, LINES*.75};
    draw::circle(focus, 10.0); 
    draw::circle(focus, 5.0, 'x', draw::Color::BLUE); 
    draw::circle(focus, 1.0, 'X', draw::Color::RED); 

    // draw a line
    auto start = Vec2{1.0, 1.0};
    auto end = Vec2{80.0, 10.0};
    draw::line(start, end);

    while (true) {
        draw::color(draw::Color::RED, [&]() {
            draw::str(0, LINES - 2, "Iteration: %d", iteration); 
        });
        funcs::each(p, &Body::update, dt);
        funcs::each(p, &Body::draw);
        draw::refr(); // update the screen
        draw::sleep(dt);
        iteration++;
    }

    endwin();
    return 0;
}
