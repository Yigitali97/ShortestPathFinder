#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include "GraphTypes.h"

class LoginScreen {
public:
    LoginScreen(sf::RenderWindow& win, sf::Font& font)
        : m_win(win), m_font(font),
          m_heading(font, "", 34), m_hint(font, "", 12), m_errText(font, "", 14),
          m_userLabel(font, "", 13), m_passLabel(font, "", 13),
          m_userDisp(font, "", 17), m_passDisp(font, "", 17),
          m_loginLbl(font, "", 16), m_exitLbl(font, "", 16)
    {
        const float W = static_cast<float>(win.getSize().x);
        const float H = static_cast<float>(win.getSize().y);

        constexpr float PW = 440.f, PH = 380.f;
        float px = (W - PW) / 2.f, py = (H - PH) / 2.f;

        m_panel.setSize({PW, PH});
        m_panel.setPosition({px, py});
        m_panel.setFillColor(sf::Color(38, 38, 55));
        m_panel.setOutlineThickness(2.f);
        m_panel.setOutlineColor(sf::Color(70, 110, 200));

        m_heading = sf::Text(font, "Login", 34);
        m_heading.setFillColor(sf::Color(80, 190, 255));
        m_heading.setStyle(sf::Text::Bold);
        centreX(m_heading, px + PW / 2.f, py + 32.f);

        m_hint = sf::Text(font, "Hint: admin / 1234", 12);
        m_hint.setFillColor(sf::Color(90, 100, 140));
        centreX(m_hint, px + PW / 2.f, py + 75.f);

        m_errText = sf::Text(font, "", 14);
        m_errText.setFillColor(sf::Color(255, 80, 80));
        m_errText.setPosition({px + 30.f, py + PH - 55.f});

        setupField(m_userBox, m_userLabel, m_userDisp, "Username", px+30.f, py+108.f, PW-60.f);
        setupField(m_passBox, m_passLabel, m_passDisp, "Password", px+30.f, py+205.f, PW-60.f);

        setupBtn(m_loginBtn, m_loginLbl, "Login", px+30.f,       py+PH-75.f, 170.f, sf::Color(45,110,210));
        setupBtn(m_exitBtn,  m_exitLbl,  "Exit",  px+PW-200.f,   py+PH-75.f, 170.f, sf::Color(150,45,45));
    }

    void handleEvent(const sf::Event& ev, AppState& state) {
        // mouse clicks
        if (const auto* mb = ev.getIf<sf::Event::MouseButtonPressed>()) {
            if (mb->button == sf::Mouse::Button::Left) {
                sf::Vector2f p(static_cast<float>(mb->position.x),
                               static_cast<float>(mb->position.y));
                if      (m_userBox.getGlobalBounds().contains(p))  m_focus = 0;
                else if (m_passBox.getGlobalBounds().contains(p))  m_focus = 1;
                else if (m_loginBtn.getGlobalBounds().contains(p)) tryLogin(state);
                else if (m_exitBtn.getGlobalBounds().contains(p))  m_win.close();
            }
        }
        // key presses
        if (const auto* kp = ev.getIf<sf::Event::KeyPressed>()) {
            if (kp->code == sf::Keyboard::Key::Tab)   m_focus ^= 1;
            if (kp->code == sf::Keyboard::Key::Enter) tryLogin(state);
        }
        // text input
        if (const auto* te = ev.getIf<sf::Event::TextEntered>()) {
            std::string& s = (m_focus == 0) ? m_username : m_password;
            uint32_t c = te->unicode;
            if (c == 8  && !s.empty()) s.pop_back();
            else if (c == 13) tryLogin(state);
            else if (c >= 32 && c < 127 && s.size() < 32) s += static_cast<char>(c);
        }
        refreshDisplays();
    }

    void render() {
        sf::RectangleShape bg({static_cast<float>(m_win.getSize().x),
                               static_cast<float>(m_win.getSize().y)});
        bg.setFillColor(sf::Color(18, 18, 30));
        m_win.draw(bg);
        m_win.draw(m_panel);
        m_win.draw(m_heading);
        m_win.draw(m_hint);

        auto drawField = [&](sf::RectangleShape& box, sf::Text& lbl,
                              sf::Text& disp, int idx) {
            box.setOutlineColor(m_focus == idx ? sf::Color(90,170,255)
                                               : sf::Color(70,70,105));
            box.setOutlineThickness(m_focus == idx ? 2.f : 1.f);
            m_win.draw(box); m_win.draw(lbl); m_win.draw(disp);
        };
        drawField(m_userBox, m_userLabel, m_userDisp, 0);
        drawField(m_passBox, m_passLabel, m_passDisp, 1);

        sf::Vector2i mi = sf::Mouse::getPosition(m_win);
        sf::Vector2f mouse(static_cast<float>(mi.x), static_cast<float>(mi.y));
        hoverBtn(m_loginBtn, mouse, sf::Color(65,130,235), sf::Color(45,110,210));
        hoverBtn(m_exitBtn,  mouse, sf::Color(180,60,60),  sf::Color(150,45,45));

        m_win.draw(m_loginBtn); m_win.draw(m_loginLbl);
        m_win.draw(m_exitBtn);  m_win.draw(m_exitLbl);
        m_win.draw(m_errText);
    }

private:
    sf::RenderWindow&  m_win;
    sf::Font&          m_font;

    sf::RectangleShape m_panel;
    sf::Text           m_heading, m_hint, m_errText;
    sf::RectangleShape m_userBox,  m_passBox;
    sf::Text           m_userLabel, m_passLabel;
    sf::Text           m_userDisp,  m_passDisp;
    sf::RectangleShape m_loginBtn, m_exitBtn;
    sf::Text           m_loginLbl, m_exitLbl;

    std::string m_username, m_password;
    int         m_focus = 0;

    static constexpr const char* CREDS[][2] = {
        {"admin","1234"}, {"user","pass"}, {"guest","guest"}
    };

    void setupField(sf::RectangleShape& box, sf::Text& lbl, sf::Text& disp,
                    const std::string& name, float x, float y, float w) {
        lbl = sf::Text(m_font, name, 13);
        lbl.setFillColor(sf::Color(160,165,200));
        lbl.setPosition({x, y - 20.f});

        box.setSize({w, 42.f}); box.setPosition({x, y});
        box.setFillColor(sf::Color(28,28,42));
        box.setOutlineThickness(1.f); box.setOutlineColor(sf::Color(70,70,105));

        disp = sf::Text(m_font, "", 17);
        disp.setFillColor(sf::Color::White);
        disp.setPosition({x + 10.f, y + 11.f});
    }

    void setupBtn(sf::RectangleShape& btn, sf::Text& lbl, const std::string& s,
                  float x, float y, float w, sf::Color col) {
        btn.setSize({w, 44.f}); btn.setPosition({x, y}); btn.setFillColor(col);

        lbl = sf::Text(m_font, s, 16);
        lbl.setFillColor(sf::Color::White);
        lbl.setStyle(sf::Text::Bold);
        sf::FloatRect b = lbl.getLocalBounds();
        lbl.setOrigin({b.position.x + b.size.x / 2.f, b.position.y + b.size.y / 2.f});
        lbl.setPosition({x + w / 2.f, y + 22.f});
    }

    static void centreX(sf::Text& t, float cx, float y) {
        sf::FloatRect b = t.getLocalBounds();
        t.setOrigin({b.position.x + b.size.x / 2.f, b.position.y});
        t.setPosition({cx, y});
    }

    static void hoverBtn(sf::RectangleShape& btn, sf::Vector2f mouse,
                         sf::Color hover, sf::Color normal) {
        btn.setFillColor(btn.getGlobalBounds().contains(mouse) ? hover : normal);
    }

    void refreshDisplays() {
        m_userDisp.setString(m_username + (m_focus == 0 ? "|" : ""));
        m_passDisp.setString(std::string(m_password.size(), '*') + (m_focus == 1 ? "|" : ""));
    }

    void tryLogin(AppState& state) {
        for (auto& c : CREDS)
            if (m_username == c[0] && m_password == c[1]) {
                state = AppState::GRAPH_EDIT; return;
            }
        m_errText.setString("Invalid username or password. Try again.");
        m_password.clear(); refreshDisplays();
    }
};
