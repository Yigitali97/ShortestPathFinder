#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <set>
#include <string>
#include "GraphTypes.h"

class GraphCanvas {
public:
    GraphCanvas(sf::RenderWindow& win, sf::Font& font);

    void handleEvent(const sf::Event& ev, AppState& state);
    void update(float dt, AppState& state);
    void render(AppState& state);

private:
    sf::RenderWindow& m_win;
    sf::Font&         m_font;

    std::vector<GraphNode> m_nodes;
    std::vector<GraphEdge> m_edges;
    int m_nextId      = 0;

    EditMode m_mode   = EditMode::NONE;
    int m_startId     = -1;
    int m_endId       = -1;
    int m_edgeFirst   = -1;
    int m_edgeSecond  = -1;
    std::string m_weightStr;

    std::vector<AlgoStep> m_steps;
    int   m_stepIdx   = 0;
    float m_stepTimer = 0.f;
    float m_stepDelay = 0.75f;
    int   m_finalDist = INF;
    std::vector<int> m_path;
    std::set<int>    m_pathEdgeSet;

    struct Btn {
        sf::RectangleShape rect;
        std::string        labelText; // sf::Text built on-the-fly in render (no default ctor in SFML3)
        std::string        id;
        sf::Color          baseColor;
    };
    std::vector<Btn> m_buttons;
    bool m_buttonsBuilt = false;

    std::string m_statusMsg = "Welcome! Choose a tool or press a hotkey.";

    int  nodeAt(float x, float y) const;
    bool inCanvas(float x, float y) const;

    void addNode(float x, float y);
    void addEdge(int u, int v, int w);
    void resetGraph();
    void runDijkstra();
    void buildPath(const std::vector<int>& parent, int dst);
    void handleToolbarClick(const std::string& id, AppState& state);

    void drawEdgeLine(sf::Vector2f a, sf::Vector2f b, float thick, sf::Color col);
    void drawEdgeWeight(sf::Vector2f mid, int w, bool highlight);
    void drawNode(const GraphNode& n, sf::Color fill, sf::Color outline, int distVal);
    void drawToolbar(AppState state);
    void drawStatusBar(AppState state);
    void drawWeightDialog();
    void drawResultBanner();
    void drawExitDialog(AppState& state);
    void buildButtons();
};
