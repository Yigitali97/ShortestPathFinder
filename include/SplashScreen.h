#pragma once
#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>
#include <string>

class SplashScreen {
public:
    SplashScreen(sf::RenderWindow& win, sf::Font& font)
        : m_win(win), m_font(font),
          m_title   (font, "Shortest Path Finder", 54),
          m_subtitle(font, "Dijkstra's Algorithm  --  Interactive Visualization", 21),
          m_author  (font, "Developed by: Yigit Ali Toshboev, Abdulloh Kamoliddinov, Feruzbek Pirmatov", 17),
          m_course  (font, "Data Structures & Algorithms  |  2026", 15),
          m_loading (font, "Loading...", 13)
    {
        const float W = static_cast<float>(win.getSize().x);
        const float H = static_cast<float>(win.getSize().y);

        // colours and styles
        m_title.setFillColor(sf::Color(80, 190, 255));
        m_title.setStyle(sf::Text::Bold);
        m_subtitle.setFillColor(sf::Color(180, 200, 230));
        m_author.setFillColor(sf::Color(150, 160, 200));
        m_course.setFillColor(sf::Color(110, 120, 160));
        m_loading.setFillColor(sf::Color(120, 120, 160));

        centreX(m_title,    W, H * 0.28f);
        centreX(m_subtitle, W, H * 0.40f);
        centreX(m_author,   W, H * 0.55f);
        centreX(m_course,   W, H * 0.62f);
        centreX(m_loading,  W, H * 0.83f);

        float barW = W * 0.48f;
        float barX = (W - barW) / 2.f;
        float barY = H * 0.77f;
        m_barW = barW;

        m_barBg.setSize({barW, 14.f});
        m_barBg.setPosition({barX, barY});
        m_barBg.setFillColor(sf::Color(45, 45, 65));
        m_barBg.setOutlineThickness(1.f);
        m_barBg.setOutlineColor(sf::Color(80, 90, 130));

        m_bar.setSize({0.f, 14.f});
        m_bar.setPosition({barX, barY});
        m_bar.setFillColor(sf::Color(80, 190, 255));

        // decorative corner dots
        const std::vector<sf::Vector2f> pts = {
            {W*0.12f,H*0.18f},{W*0.88f,H*0.18f},
            {W*0.08f,H*0.72f},{W*0.92f,H*0.72f},{W*0.50f,H*0.10f}
        };
        for (auto& p : pts) {
            sf::CircleShape c(12.f);
            c.setOrigin({12.f, 12.f});
            c.setPosition(p);
            c.setFillColor(sf::Color(60, 100, 160, 160));
            c.setOutlineThickness(2.f);
            c.setOutlineColor(sf::Color(80, 160, 220, 160));
            m_decor.push_back(c);
        }
    }

    bool update(float dt) {
        m_elapsed += dt;
        float t = std::min(m_elapsed / m_duration, 1.f);
        m_bar.setSize({m_barW * t, 14.f});

        auto a = static_cast<uint8_t>(std::min(255.f, t * 2.5f * 255.f));
        setAlpha(m_title, a); setAlpha(m_subtitle, a);
        setAlpha(m_author, a); setAlpha(m_course, a);
        return m_elapsed >= m_duration;
    }

    void render() {
        sf::RectangleShape bg({static_cast<float>(m_win.getSize().x),
                               static_cast<float>(m_win.getSize().y)});
        bg.setFillColor(sf::Color(18, 18, 30));
        m_win.draw(bg);
        for (auto& c : m_decor) m_win.draw(c);
        m_win.draw(m_title); m_win.draw(m_subtitle);
        m_win.draw(m_author); m_win.draw(m_course);
        m_win.draw(m_barBg); m_win.draw(m_bar);
        m_win.draw(m_loading);
    }

private:
    sf::RenderWindow&            m_win;
    sf::Font&                    m_font;
    float                        m_elapsed  = 0.f;
    const float                  m_duration = 4.2f;
    float                        m_barW     = 0.f;

    sf::Text                     m_title, m_subtitle, m_author, m_course, m_loading;
    sf::RectangleShape           m_barBg, m_bar;
    std::vector<sf::CircleShape> m_decor;

    static void centreX(sf::Text& t, float W, float y) {
        sf::FloatRect b = t.getLocalBounds();
        t.setOrigin({b.position.x + b.size.x / 2.f, b.position.y});
        t.setPosition({W / 2.f, y});
    }
    static void setAlpha(sf::Text& t, uint8_t a) {
        auto c = t.getFillColor(); c.a = a; t.setFillColor(c);
    }
};
