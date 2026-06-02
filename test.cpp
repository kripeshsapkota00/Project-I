#include <SFML/Graphics.hpp>
#include <iostream>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <vector>
#include <cmath>

using namespace sf;
using namespace std;

// ============================================================
// TIMEZONE: stores name and how many hours/minutes ahead of UTC
// ============================================================
struct TimeZone
{
    string name;
    int hourOffset;
    int minOffset;
};

// List of timezones we support
vector<TimeZone> timezones = {
    {"Nepal", 5, 45},
    {"India", 5, 30},
    {"UTC", 0, 0},
    {"UK", 0, 0}};

// ============================================================
// STOPWATCH: tracks elapsed time, can be started/stopped/reset
// ============================================================
class Stopwatch
{
    Clock clock;
    bool running = false;
    Time savedTime; // time accumulated before the last pause

public:
    // Start if stopped, pause if running
    void toggle()
    {
        if (running)
        {
            savedTime += clock.getElapsedTime(); // save progress
            running = false;
        }
        else
        {
            clock.restart(); // start counting fresh
            running = true;
        }
    }

    // Reset everything to zero
    void reset()
    {
        savedTime = Time::Zero;
        clock.restart();
        running = false;
    }

    // Get total elapsed time
    Time getElapsed()
    {
        if (running)
            return savedTime + clock.getElapsedTime();
        return savedTime;
    }
};

// ============================================================
// FORMAT TIME: returns a "HH:MM:SS" string
// If stopwatchMode is true, formats stopwatch time instead
// ============================================================
string formatTime(TimeZone tz, bool stopwatchMode, Time stopwatchTime)
{
    if (stopwatchMode)
    {
        // Format stopwatch as HH:MM:SS
        int totalSeconds = (int)stopwatchTime.asSeconds();
        int h = totalSeconds / 3600;
        int m = (totalSeconds % 3600) / 60;
        int s = totalSeconds % 60;

        stringstream ss;
        ss << setfill('0')
           << setw(2) << h << ":"
           << setw(2) << m << ":"
           << setw(2) << s;
        return ss.str();
    }

    // Get current UTC time and apply timezone offset
    time_t now = time(nullptr);
    tm *localTime = gmtime(&now);
    localTime->tm_hour += tz.hourOffset;
    localTime->tm_min += tz.minOffset;
    mktime(localTime); // fix overflow (e.g. 63 minutes → 1h 3m)

    stringstream ss;
    ss << setfill('0')
       << setw(2) << localTime->tm_hour << ":"
       << setw(2) << localTime->tm_min << ":"
       << setw(2) << localTime->tm_sec;
    return ss.str();
}

// ============================================================
// DRAW HAND: draws a clock hand as a rotated rectangle
// angle = degrees, length = how long the hand is
// ============================================================
void drawHand(RenderWindow &window, Vector2f center,
              float angle, float length, Color color, float thickness)
{
    RectangleShape hand({length, thickness});
    hand.setFillColor(color);
    hand.setOrigin({0.f, thickness / 2}); // rotate from the base
    hand.setPosition(center);
    hand.setRotation(sf::degrees(angle - 90)); // -90 so 0° points up
    window.draw(hand);
}

// ============================================================
// MAIN
// ============================================================
int main()
{
    RenderWindow window(VideoMode({900, 600}), "Clock System");

    Stopwatch stopwatch;
    int selectedZone = 0;       // index into timezones[]
    bool stopwatchMode = false; // false = show clock, true = show stopwatch

    // Load font
    Font font;
    if (!font.openFromFile("C:/Windows/Fonts/arial.ttf"))
    {
        cout << "Could not load font!\n";
        return -1;
    }

    // Analog clock settings
    Vector2f clockCenter(250.f, 300.f);
    float clockRadius = 200.f;

    // Draw the clock circle (outline only)
    CircleShape clockFace(clockRadius);
    clockFace.setFillColor(Color::Transparent);
    clockFace.setOutlineThickness(5);
    clockFace.setOutlineColor(Color::White);
    clockFace.setPosition({clockCenter.x - clockRadius, clockCenter.y - clockRadius});

    // Digital time display (right side)
    Text digitalText(font, "");
    digitalText.setCharacterSize(32);
    digitalText.setFillColor(Color::Green);
    digitalText.setPosition({550.f, 150.f});

    // Controls hint text
    Text controlsText(font, "");
    controlsText.setCharacterSize(18);
    controlsText.setFillColor(Color::Yellow);
    controlsText.setPosition({550.f, 300.f});
    controlsText.setString("S: Start/Stop\nR: Reset\nT: Timezone\nSPACE: Mode");

    // ============================================================
    // MAIN LOOP
    // ============================================================
    while (window.isOpen())
    {

        // --- Handle input events ---
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
                    selectedZone = (selectedZone + 1) % timezones.size(); // cycle timezones

                if (key == Keyboard::Key::S)
                    stopwatch.toggle();

                if (key == Keyboard::Key::R)
                    stopwatch.reset();

                if (key == Keyboard::Key::Space)
                    stopwatchMode = !stopwatchMode; // toggle between clock and stopwatch
            }
        }

        // --- Get current time for this frame ---
        time_t now = time(nullptr);
        tm *localTime = gmtime(&now);
        localTime->tm_hour += timezones[selectedZone].hourOffset;
        localTime->tm_min += timezones[selectedZone].minOffset;
        mktime(localTime);

        // Calculate hand angles (in degrees)
        // Seconds: each second = 6 degrees
        // Minutes: each minute = 6 degrees + small nudge from seconds
        // Hours:   each hour   = 30 degrees + small nudge from minutes
        float secondAngle = localTime->tm_sec * 6.f;
        float minuteAngle = localTime->tm_min * 6.f + localTime->tm_sec * 0.1f;
        float hourAngle = (localTime->tm_hour % 12) * 30.f + localTime->tm_min * 0.5f;

        // Update digital display
        Time stopwatchTime = stopwatch.getElapsed();
        string timeString = formatTime(timezones[selectedZone], stopwatchMode, stopwatchTime);
        digitalText.setString(timeString + "\n[" + timezones[selectedZone].name + "]");

        // --- Draw everything ---
        window.clear(Color::Black);

        // Draw clock face outline
        window.draw(clockFace);

        // Draw hour numbers (1–12)
        for (int i = 1; i <= 12; i++)
        {
            float angleRad = i * 30.f * 3.14159265f / 180.f;
            float x = clockCenter.x + cos(angleRad - 3.14159265f / 2) * (clockRadius - 30);
            float y = clockCenter.y + sin(angleRad - 3.14159265f / 2) * (clockRadius - 30);

            Text numLabel(font, to_string(i));
            numLabel.setCharacterSize(20);
            numLabel.setFillColor(Color::White);
            numLabel.setPosition({x - 10, y - 10});
            window.draw(numLabel);
        }

        // Draw the three clock hands
        drawHand(window, clockCenter, hourAngle, 80, Color::White, 6);   // hour
        drawHand(window, clockCenter, minuteAngle, 120, Color::Cyan, 4); // minute
        drawHand(window, clockCenter, secondAngle, 150, Color::Red, 2);  // second

        // Draw center dot
        CircleShape centerDot(5);
        centerDot.setFillColor(Color::White);
        centerDot.setPosition({clockCenter.x - 5, clockCenter.y - 5});
        window.draw(centerDot);

        // Draw digital text and controls
        window.draw(digitalText);
        window.draw(controlsText);

        window.display();
    }

    return 0;
}