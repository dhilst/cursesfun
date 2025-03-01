#include <algorithm>
#include <bits/ranges_algo.h>
#include <cassert>
#include <climits>
#include <cmath>
#include <concepts>
#include <functional>
#include <iostream>
#include <limits>
#include <ncurses.h>
#include <numeric>
#include <optional>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <ranges>
#include <vector>
#include <cstdlib>
#include <chrono>
#include <type_traits>

#include <gtest/gtest.h>


namespace funcs {

template <typename T>
std::optional<T> pop(T& container)
{
    if (container.size() == 0) {
        return std::nullopt;
    }
    auto last = std::move(container.back());
    container.pop_back();
    return last;
}

template <typename T>
std::ostream& operator<<(std::ostream& os, std::vector<T> vec)
{
    for (auto& v : vec) {
        os << v << "," ; 
    }
    return os;
}

std::optional<std::string> getenvopt(const char* name)
{
    const char* value = getenv(name);
    if (value == NULL) {
        return std::nullopt;
    }

    return std::make_optional(std::string(value));
}

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

auto parseTwoNumbers(const std::string& input)
{
    std::pair<int, int> output;
    char middle;
    std::istringstream sinput(input);
    sinput >> output.first;
    if (sinput.peek() != ' ')
        sinput >> middle;
    sinput >> output.second;
    return output;
}

TEST(parseTwoNumbers, BasicTtest) {
    EXPECT_EQ(parseTwoNumbers("1 2"), std::make_pair(1,2));
    EXPECT_EQ(parseTwoNumbers("1|2"), std::make_pair(1,2));
};

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

TEST(CallOnce, Basic) {
    CallOnce callOnce;
    auto i = 0;
    auto inc = [&i](){ i++; };
    callOnce(inc);
    callOnce(inc);
    EXPECT_EQ(i, 1);
    inc(); inc();
    EXPECT_EQ(i, 3);
}

}

namespace draw {

static void point(int x, int y, char symbol)
{
    mvaddch(y, x, symbol);
}

template <typename T>
static void draw(const T& ch)
{
}

static void erase(int x, int y, char ch = ' ')
{
    point(x, y, ch);
}

template <typename ...Args>
static void fmt(int x, int y, auto fmt, Args... args)
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

constexpr auto sink(const auto& focus, const auto& obj, double mag = 400.0, const char ch = '0')
-> decltype(auto)
{
    draw::point(focus.x, focus.y, ch, draw::Color::RED);
    
    auto diff = Vec2{
        focus.x - obj.p.x,
        focus.y - obj.p.y,
    };
    auto distance = diff.mag();
    auto direction = diff.norm();
    auto force = Vec2{
        direction.x * mag / distance * distance,
        direction.y * mag / distance * distance,
    };
    return force;
};

constexpr auto resistence(
    const auto& obj, double maxVelocity = 1.0,
    double loss = 0.1)
{
    auto velocity = obj.v.mag();
    if (velocity > maxVelocity) {
        return (-obj.v).norm() * velocity * loss;
    }

    return Vec2{0.0, 0.0};
}

constexpr auto source(const auto& focus, const auto& obj,
                      double mag = 400.0, double minDistance = 9.0, 
                      const char ch = '0', draw::Color c = draw::Color::BLUE)
{
    draw::point(focus.x, focus.y, ch, c);
    auto diff = Vec2{
        obj.p.x - focus.x,
        obj.p.y - focus.y,
    };

    auto distance = diff.mag();
    if (distance > minDistance) {
        return Vec2{0.0,0.0};
    }

    auto direction = diff.norm();
    auto force = Vec2{
        direction.x * mag / obj.mass,
        direction.y * mag / obj.mass,
    };
    return force;
}

struct Body {
    constexpr static auto track = false;
    // Position,  Velocity and Acceleration
    Vec2<double> p{.x=0, .y=0};
    Vec2<double> oldp{};
    Vec2<double> v{.x=0, .y=0};
    double mass = 1000;
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
        v.x = phys::random(COLS*.5, COLS);
        v.y = phys::random(LINES*.5, LINES);
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
        if (newP.x < 0) {
            v.x *= -0.3;
            //p.x = COLS-1;
        } else if (newP.x > COLS-1) {
            v.x *= -0.3;
            //p.x = 0;
        }

        if (newP.y < 0) {
            v.y *= -0.3;
            //p.y = LINES-1;
        } else if (newP.y > LINES-1) {
            v.y *= -0.3;
            //p.y = 0;
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
        const auto G = 100;
        Vec2<double> acc;
        acc = acc + sink(Vec2{
            COLS*.25,
            LINES*.25,
        }, *this, G, '0');
        v = Vec2 {
            v.x + acc.x,
            v.y + acc.y,
        };
        auto newP = Vec2 {
            p.x + v.x * dt * dt,
            p.y + v.y * dt * dt,
        };
        move(newP);
    }
};

};

namespace game {


template <typename T>
concept IDrawable = requires(T t) {
    { t.c } -> std::same_as<draw::Color&>;
    { t.sym } -> std::same_as<char&>;
};

struct Player final {
    double hp = 100.0;
    double armor = 0.0;
    unsigned x, y;
    
    struct Stats {
        double ing{0}, str{0}, dex{0}; // ing = inteligence because int is reserved
    };
};

std::string toString(const Player&)
{
    return "Player";
}
// static_assert(IGameObj<Player>);


// A 2D grid representing a world where we can push or pop stuff from the floor
template <IDrawable T>
// requires std::is_default_constructible_v<T>
class World {
    std::size_t m_columns, m_lines;

    struct HashKey {
        std::size_t operator ()(const std::pair<std::size_t, std::size_t>& p) const {
            auto h1 = std::hash<std::size_t>{}(p.first);
            auto h2 = std::hash<std::size_t>{}(p.second);
            return h1 ^ h2;
        }
    };
    std::unordered_map<std::pair<std::size_t, std::size_t>, std::vector<T>, HashKey> m_map;

    static auto make_index(auto x, auto y) -> decltype(auto)
    {
        return std::make_pair(x, y);
    }

    std::optional<std::vector<T>> idx(std::size_t x, std::size_t y) const
    {
        auto i = make_index(x, y);
        try {
            return m_map.at(i);
        } catch (const std::out_of_range& e) {
            return std::nullopt;
        }
    };

public:

    World(std::size_t columns, std::size_t lines)
    : m_columns(columns)
    , m_lines(lines)
    , m_map()
    {
    }

    std::optional<T> get(std::size_t x, std::size_t y) const
    {
        auto v = idx(x, y);

        if (v) {
            return v.value().back();
        }

        return std::nullopt;
    };

    std::optional<T&> pop(std::size_t x, std::size_t y)
    {
        static_assert(x < m_map.size());
        static_assert(m_map.size() > 0 && y < m_map[0].size());
        return funcs::pop(idx(x, y));
    };

    void push(std::size_t x, std::size_t y, T&& value)
    {
        auto&& floorStack = idx(x, y);
        if (!floorStack) {
            m_map.emplace(make_index(x, y), std::vector{std::move(value)});
        } else {
            floorStack.value().push_back(std::move(value));
        }
    }

    void draw() const
    {
        for (auto x = 0; x < m_columns; ++x) {
            for (auto y = 0; y < m_lines; ++y) {
                auto value = get(x, y);
                if (value) {
                    auto val = value.value();
                    draw::point(x, y, val.sym, val.c);
                } else {
                    draw::point(x, y, ' ');
                }
            }
        }
    }

};


};

int main()
{
    struct Trash {
        char sym;
        draw::Color c;
    };
    static_assert(game::IDrawable<Trash>);
    if (getenv("TEST") != NULL) {
        testing::InitGoogleTest();
        return RUN_ALL_TESTS();
    }
    using namespace phys;
    // Define color pairs (index, foreground, background)
    draw::init();

    int iteration = 0;
    game::Player player;

    game::World<Trash> world(COLS-1, LINES-1);
    // std::vector<Body> p(3);
    
    const Trash banana = {')', draw::Color::YELLOW};
    for (auto i = 0; i < 10; ++i) {
        auto x = phys::random(0, COLS-1);
        auto y = phys::random(0, LINES-1);
        world.push(x, y, Trash(banana));
    };


    double dt = 1.0/60.0; // 60 fps
    int count = 0;
    draw::clear();


    while (true) {
        draw::fmt(0, LINES - 1, "World: %dx%d", LINES, COLS); 
        world.draw();
        draw::refr();
        draw::sleep(dt);
    }

    endwin();
    return 0;
}
