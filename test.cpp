#include <SFML/Graphics.hpp>
#include <iostream>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <vector>
#include <cmath>

using namespace sf;
using namespace std;

// ---------------- TIMEZONE ----------------
struct TimeZone
{
    string name;
    int hourOffset;
    int minOffset;
};

vector<TimeZone> zones = {
    {"Nepal", 5, 45},
    {"India", 5, 30},
    {"UTC", 0, 0},
    {"UK", 0, 0}};

// ---------------- STOPWATCH ----------------
class Stopwatch
{
    Clock clock;
    bool running = false;
    Time elapsedBefore;

public:
    void toggle()
    {
        if (running)
        {
            elapsedBefore += clock.getElapsedTime();
            running = false;
        }
        else
        {
            clock.restart();
            running = true;
        }
    }

    void reset()
    {
        elapsedBefore = Time::Zero;
        clock.restart();
        running = false;
    }

    Time getTime()
    {
        if (running)
            return elapsedBefore + clock.getElapsedTime();
        return elapsedBefore;
    }
};

// ---------------- FORMAT ----------------
string formatTime(TimeZone tz, bool stopwatchMode, Time swTime)
{
    time_t now = time(nullptr);
    tm *local = gmtime(&now);

    local->tm_hour += tz.hourOffset;
    local->tm_min += tz.minOffset;
    mktime(local);

    stringstream ss;

    if (!stopwatchMode)
    {
        ss << setfill('0')
           << setw(2) << local->tm_hour << ":"
           << setw(2) << local->tm_min << ":"
           << setw(2) << local->tm_sec;
    }
    else
    {
        int total = (int)swTime.asSeconds();
        int h = total / 3600;
        int m = (total % 3600) / 60;
        int s = total % 60;

        ss << setfill('0')
           << setw(2) << h << ":"
           << setw(2) << m << ":"
           << setw(2) << s;
    }

    return ss.str();
}

// ---------------- MAIN ----------------
int main()
{
    RenderWindow window(VideoMode({900, 600}), "Clock System");

    Stopwatch stopwatch;
    int tzIndex = 0;
    bool stopwatchMode = false;

    Font font;
    if (!font.openFromFile("C:/Windows/Fonts/arial.ttf"))
    {
        cout << "Font missing!\n";
        return -1;
    }

    // -------- ANALOG CLOCK (LEFT SIDE) --------
    Vector2f center(250.f, 300.f);
    float radius = 200.f;

    CircleShape clockFace(200);
    clockFace.setFillColor(Color::Transparent);
    clockFace.setOutlineThickness(5);
    clockFace.setOutlineColor(Color::White);
    clockFace.setPosition({center.x - 200, center.y - 200});

    // -------- DIGITAL (RIGHT SIDE) --------
    Text digitalText(font, "");
    digitalText.setCharacterSize(32);
    digitalText.setFillColor(Color::Green);
    digitalText.setPosition({550.f, 150.f});

    Text infoText(font, "");
    infoText.setCharacterSize(18);
    infoText.setFillColor(Color::Yellow);
    infoText.setPosition({550.f, 300.f});

    while (window.isOpen())
    {

        while (auto event = window.pollEvent())
        {

            if (event->is<Event::Closed>())
                window.close();

            if (event->is<Event::KeyPressed>())
            {
                auto key = event->getIf<Event::KeyPressed>()->code;

                if (key == Keyboard::Key::Escape)
                    window.close();

                if (key == Keyboard::Key::T)
                    tzIndex = (tzIndex + 1) % zones.size();

                if (key == Keyboard::Key::S)
                    stopwatch.toggle();

                if (key == Keyboard::Key::R)
                    stopwatch.reset();

                if (key == Keyboard::Key::Space)
                    stopwatchMode = !stopwatchMode;
            }
        }

        window.clear(Color::Black);

        // -------- TIME --------
        Time swTime = stopwatch.getTime();
        string timeStr = formatTime(zones[tzIndex], stopwatchMode, swTime);

        digitalText.setString(timeStr + "\n[" + zones[tzIndex].name + "]");

        time_t now = time(nullptr);
        tm *lt = gmtime(&now);

        lt->tm_hour += zones[tzIndex].hourOffset;
        lt->tm_min += zones[tzIndex].minOffset;
        mktime(lt);

        float sec = lt->tm_sec * 6;
        float min = lt->tm_min * 6 + lt->tm_sec * 0.1f;
        float hr = lt->tm_hour * 30 + lt->tm_min * 0.5f;

        // -------- CLOCK NUMBERS --------
        for (int i = 1; i <= 12; i++)
        {
            float angle = i * 30 * 3.14159265f / 180.f;

            float x = center.x + cos(angle - 3.14159265f / 2) * (radius - 30);
            float y = center.y + sin(angle - 3.14159265f / 2) * (radius - 30);

            Text num(font, to_string(i));
            num.setCharacterSize(20);
            num.setFillColor(Color::White);
            num.setPosition({x - 10, y - 10});

            window.draw(num);
        }

        // -------- HANDS --------
        auto drawHand = [&](float angle, float length, Color color, float thick)
        {
            RectangleShape line({length, thick});
            line.setFillColor(color);
            line.setOrigin({0.f, thick / 2});
            line.setPosition(center);
            line.setRotation(sf::degrees(angle - 90));
            window.draw(line);
        };

        drawHand(hr, 80, Color::White, 6);
        drawHand(min, 120, Color::Cyan, 4);
        drawHand(sec, 150, Color::Red, 2);

        // -------- CENTER DOT --------
        CircleShape dot(5);
        dot.setFillColor(Color::White);
        dot.setPosition({center.x - 5, center.y - 5});

        // -------- STOPWATCH CONTROLS (UNDER DIGITAL) --------
        string controls =
            "S: Start/Stop\n"
            "R: Reset\n"
            "T: Timezone\n"
            "SPACE: Mode";

        infoText.setString(controls);

        // -------- DRAW --------
        window.draw(clockFace);
        window.draw(dot);

        window.draw(digitalText);
        window.draw(infoText);

        window.display();
    }

    return 0;
}