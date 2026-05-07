#include "GraphCanvas.h"
#include <cmath>
#include <algorithm>
#include <queue>

// ─────────────────────────────────────────────────────────────────────────────
// local helpers  (SFML 3: FloatRect uses .position/.size, setPosition takes Vector2f)
// ─────────────────────────────────────────────────────────────────────────────

static float vlen(sf::Vector2f v) { return std::sqrt(v.x*v.x + v.y*v.y); }

static void centreText(sf::Text& t, float cx, float cy) {
    sf::FloatRect b = t.getLocalBounds();
    t.setOrigin({b.position.x + b.size.x / 2.f, b.position.y + b.size.y / 2.f});
    t.setPosition({cx, cy});
}

static void centreTextX(sf::Text& t, float cx, float y) {
    sf::FloatRect b = t.getLocalBounds();
    t.setOrigin({b.position.x + b.size.x / 2.f, b.position.y});
    t.setPosition({cx, y});
}

// ─────────────────────────────────────────────────────────────────────────────
// constructor
// ─────────────────────────────────────────────────────────────────────────────

GraphCanvas::GraphCanvas(sf::RenderWindow& win, sf::Font& font)
    : m_win(win), m_font(font)
{}

// ─────────────────────────────────────────────────────────────────────────────
// event handling
// ─────────────────────────────────────────────────────────────────────────────

void GraphCanvas::handleEvent(const sf::Event& ev, AppState& state) {
    if (!m_buttonsBuilt) { buildButtons(); m_buttonsBuilt = true; }

    // ── EXIT_CONFIRM ──────────────────────────────────────────────────────────
    if (state == AppState::EXIT_CONFIRM) {
        if (const auto* kp = ev.getIf<sf::Event::KeyPressed>()) {
            if (kp->code == sf::Keyboard::Key::Y) m_win.close();
            if (kp->code == sf::Keyboard::Key::N ||
                kp->code == sf::Keyboard::Key::Escape)
                state = AppState::GRAPH_EDIT;
        }
        if (const auto* mb = ev.getIf<sf::Event::MouseButtonPressed>()) {
            float W = static_cast<float>(m_win.getSize().x);
            float H = static_cast<float>(m_win.getSize().y);
            sf::Vector2f p(static_cast<float>(mb->position.x),
                           static_cast<float>(mb->position.y));
            if (sf::FloatRect({W/2.f-170.f, H/2.f+28.f}, {150.f,40.f}).contains(p))
                m_win.close();
            if (sf::FloatRect({W/2.f+20.f, H/2.f+28.f}, {150.f,40.f}).contains(p))
                state = AppState::GRAPH_EDIT;
        }
        return;
    }

    // ── WEIGHT_INPUT ──────────────────────────────────────────────────────────
    if (state == AppState::WEIGHT_INPUT) {
        if (const auto* te = ev.getIf<sf::Event::TextEntered>()) {
            uint32_t c = te->unicode;
            if (c == 8 && !m_weightStr.empty()) {
                m_weightStr.pop_back();
            } else if ((c == 13 || c == '\n') && !m_weightStr.empty()) {
                int w = std::stoi(m_weightStr);
                if (w < 1) w = 1;
                addEdge(m_edgeFirst, m_edgeSecond, w);
                m_edgeFirst = m_edgeSecond = -1;
                m_weightStr.clear();
                m_mode = EditMode::ADD_EDGE;
                state  = AppState::GRAPH_EDIT;
                m_statusMsg = "Edge added. Click first node for next edge.";
            } else if (c == 27) {
                m_edgeFirst = m_edgeSecond = -1;
                m_weightStr.clear();
                m_mode = EditMode::ADD_EDGE;
                state  = AppState::GRAPH_EDIT;
                m_statusMsg = "Edge creation cancelled. Click first node to try again.";
            } else if (c >= '0' && c <= '9' && m_weightStr.size() < 5) {
                m_weightStr += static_cast<char>(c);
            }
        }
        return;
    }

    // ── SHOW_RESULT ───────────────────────────────────────────────────────────
    if (state == AppState::SHOW_RESULT) {
        bool dismiss = ev.is<sf::Event::MouseButtonPressed>() ||
                       (ev.getIf<sf::Event::KeyPressed>() &&
                        ev.getIf<sf::Event::KeyPressed>()->code == sf::Keyboard::Key::Space);
        if (dismiss) {
            state = AppState::GRAPH_EDIT;
            m_statusMsg = "Path displayed. Edit graph or run again.";
        }
        return;
    }

    // ── toolbar clicks ────────────────────────────────────────────────────────
    if (const auto* mb = ev.getIf<sf::Event::MouseButtonPressed>()) {
        if (mb->button == sf::Mouse::Button::Left) {
            sf::Vector2f p(static_cast<float>(mb->position.x),
                           static_cast<float>(mb->position.y));

            for (auto& btn : m_buttons) {
                if (btn.rect.getGlobalBounds().contains(p)) {
                    handleToolbarClick(btn.id, state);
                    return;
                }
            }

            // canvas click
            if (inCanvas(p.x, p.y)) {
                switch (m_mode) {

                case EditMode::ADD_NODE:
                    addNode(p.x, p.y);
                    break;

                case EditMode::ADD_EDGE: {
                    int idx = nodeAt(p.x, p.y);
                    if (idx < 0) break;
                    if (m_edgeFirst < 0) {
                        m_edgeFirst = idx;
                        m_statusMsg = "Node " + std::to_string(idx) +
                                      " selected. Click the second node.";
                    } else if (idx != m_edgeFirst) {
                        m_edgeSecond = idx;
                        m_weightStr.clear();
                        state = AppState::WEIGHT_INPUT;
                        m_statusMsg = "Type the edge weight, then press Enter.";
                    }
                    break;
                }

                case EditMode::SELECT_START: {
                    int idx = nodeAt(p.x, p.y);
                    if (idx >= 0) {
                        m_startId   = idx;
                        m_mode      = EditMode::NONE;
                        m_statusMsg = "Start node set to " + std::to_string(idx) + ".";
                    }
                    break;
                }

                case EditMode::SELECT_END: {
                    int idx = nodeAt(p.x, p.y);
                    if (idx >= 0) {
                        m_endId     = idx;
                        m_mode      = EditMode::NONE;
                        m_statusMsg = "End node set to " + std::to_string(idx) + ".";
                    }
                    break;
                }

                default: break;
                }
            }
        }
    }

    // ── keyboard shortcuts ────────────────────────────────────────────────────
    if (const auto* kp = ev.getIf<sf::Event::KeyPressed>()) {
        if (state != AppState::GRAPH_EDIT) return;
        switch (kp->code) {
        case sf::Keyboard::Key::N:
            m_mode = EditMode::ADD_NODE;
            m_statusMsg = "Add Node: click anywhere on the canvas.";
            break;
        case sf::Keyboard::Key::E:
            m_mode = EditMode::ADD_EDGE; m_edgeFirst = -1;
            m_statusMsg = "Add Edge: click the first node.";
            break;
        case sf::Keyboard::Key::S:
            m_mode = EditMode::SELECT_START;
            m_statusMsg = "Click a node to set it as the START.";
            break;
        case sf::Keyboard::Key::D:
            m_mode = EditMode::SELECT_END;
            m_statusMsg = "Click a node to set it as the END.";
            break;
        case sf::Keyboard::Key::R:
            if (m_startId >= 0 && m_endId >= 0 && !m_nodes.empty()) {
                runDijkstra();
                state = AppState::ALGO_RUNNING;
                m_statusMsg = m_steps.empty() ? "No path." : m_steps[0].message;
            } else {
                m_statusMsg = "Set both Start and End nodes before running!";
            }
            break;
        case sf::Keyboard::Key::C:
            resetGraph();
            break;
        case sf::Keyboard::Key::Escape:
            state = AppState::EXIT_CONFIRM;
            break;
        default: break;
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// toolbar click dispatcher
// ─────────────────────────────────────────────────────────────────────────────

void GraphCanvas::handleToolbarClick(const std::string& id, AppState& state) {
    if      (id == "addnode") {
        if (m_mode == EditMode::ADD_NODE) {
            m_mode = EditMode::NONE;
            m_statusMsg = "Add Node mode disabled.";
        } else {
            m_mode = EditMode::ADD_NODE;
            m_statusMsg = "Add Node: click canvas.";
        }
    }
    else if (id == "addedge") {
        if (m_mode == EditMode::ADD_EDGE) {
            m_mode = EditMode::NONE;
            m_edgeFirst = -1;
            m_statusMsg = "Add Edge mode disabled.";
        } else {
            m_mode = EditMode::ADD_EDGE;
            m_edgeFirst = -1;
            m_statusMsg = "Add Edge: click first node.";
        }
    }
    else if (id == "start")   { m_mode = EditMode::SELECT_START; m_statusMsg = "Click a node to set as START."; }
    else if (id == "end")     { m_mode = EditMode::SELECT_END;   m_statusMsg = "Click a node to set as END."; }
    else if (id == "run") {
        if (m_startId >= 0 && m_endId >= 0 && !m_nodes.empty()) {
            runDijkstra(); state = AppState::ALGO_RUNNING;
            m_statusMsg = m_steps.empty() ? "No path." : m_steps[0].message;
        } else { m_statusMsg = "Set Start and End nodes first!"; }
    }
    else if (id == "reset") { resetGraph(); state = AppState::GRAPH_EDIT; }
    else if (id == "exit")  { state = AppState::EXIT_CONFIRM; }
}

// ─────────────────────────────────────────────────────────────────────────────
// update
// ─────────────────────────────────────────────────────────────────────────────

void GraphCanvas::update(float dt, AppState& state) {
    if (state != AppState::ALGO_RUNNING) return;
    m_stepTimer += dt;
    if (m_stepTimer < m_stepDelay) return;
    m_stepTimer -= m_stepDelay;
    m_stepIdx++;
    if (m_stepIdx >= static_cast<int>(m_steps.size())) {
        m_stepIdx   = static_cast<int>(m_steps.size()) - 1;
        state       = AppState::SHOW_RESULT;
        m_statusMsg = m_steps.back().message;
    } else {
        m_statusMsg = m_steps[m_stepIdx].message;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// render
// ─────────────────────────────────────────────────────────────────────────────

void GraphCanvas::render(AppState& state) {
    if (!m_buttonsBuilt) { buildButtons(); m_buttonsBuilt = true; }

    const float W = static_cast<float>(m_win.getSize().x);
    const float H = static_cast<float>(m_win.getSize().y);

    // canvas background
    sf::RectangleShape canvasBg({W - TOOLBAR_W, H - STATUS_H});
    canvasBg.setPosition({TOOLBAR_W, 0.f});
    canvasBg.setFillColor(sf::Color(20, 20, 32));
    m_win.draw(canvasBg);

    // subtle grid
    sf::Color gridCol(30, 30, 45);
    for (float x = TOOLBAR_W; x < W; x += 40.f) {
        sf::RectangleShape line({1.f, H - STATUS_H});
        line.setPosition({x, 0.f}); line.setFillColor(gridCol);
        m_win.draw(line);
    }
    for (float y = 0.f; y < H - STATUS_H; y += 40.f) {
        sf::RectangleShape line({W - TOOLBAR_W, 1.f});
        line.setPosition({TOOLBAR_W, y}); line.setFillColor(gridCol);
        m_win.draw(line);
    }

    const AlgoStep* step = nullptr;
    if ((state == AppState::ALGO_RUNNING || state == AppState::SHOW_RESULT) &&
        m_stepIdx < static_cast<int>(m_steps.size()))
        step = &m_steps[m_stepIdx];

    // draw edges
    for (int i = 0; i < static_cast<int>(m_edges.size()); i++) {
        const auto& e = m_edges[i];
        sf::Vector2f a(m_nodes[e.u].x, m_nodes[e.u].y);
        sf::Vector2f b(m_nodes[e.v].x, m_nodes[e.v].y);
        bool hasResult = (state == AppState::SHOW_RESULT || state == AppState::GRAPH_EDIT) && !m_path.empty();
        bool onPath = hasResult && m_pathEdgeSet.count(i);
        drawEdgeLine(a, b, onPath ? 5.f : 2.5f,
                     onPath ? sf::Color(50,220,100) : sf::Color(100,110,145));
        drawEdgeWeight((a + b) / 2.f, e.weight, onPath);
    }

    // preview line while selecting second node for edge
    if (m_mode == EditMode::ADD_EDGE && m_edgeFirst >= 0) {
        sf::Vector2f a(m_nodes[m_edgeFirst].x, m_nodes[m_edgeFirst].y);
        sf::Vector2i mi = sf::Mouse::getPosition(m_win);
        sf::Vector2f mouse(static_cast<float>(mi.x), static_cast<float>(mi.y));
        drawEdgeLine(a, mouse, 1.5f, sf::Color(180,180,255,100));
    }

    // draw nodes
    for (int i = 0; i < static_cast<int>(m_nodes.size()); i++) {
        const auto& n = m_nodes[i];
        sf::Color fill, outline;
        int distVal = -2;

        bool hasResult2 = (state == AppState::SHOW_RESULT || state == AppState::GRAPH_EDIT) && !m_path.empty();
        bool onPath = hasResult2 && std::find(m_path.begin(), m_path.end(), i) != m_path.end();

        if (onPath) {
            fill = sf::Color(40,210,90); outline = sf::Color(180,255,200);
        } else if (i == m_startId && !(step && step->currentNode == i)) {
            fill = sf::Color(0,170,90); outline = sf::Color(0,240,130);
        } else if (i == m_endId && !(step && step->currentNode == i)) {
            fill = sf::Color(200,45,45); outline = sf::Color(255,90,90);
        } else if (step) {
            distVal = (step->dist[i] >= INF) ? -1 : step->dist[i];
            if (step->currentNode == i) {
                fill = sf::Color(30,144,255); outline = sf::Color(160,220,255);
            } else if (step->visited[i]) {
                fill = sf::Color(195,160,0); outline = sf::Color(255,215,0);
            } else {
                fill = sf::Color(55,75,115); outline = sf::Color(90,120,175);
            }
        } else if (i == m_edgeFirst) {
            fill = sf::Color(140,90,220); outline = sf::Color(200,160,255);
        } else {
            fill = sf::Color(55,95,155); outline = sf::Color(90,145,215);
        }
        drawNode(n, fill, outline, distVal);
    }

    drawToolbar(state);
    drawStatusBar(state);
    if (state == AppState::WEIGHT_INPUT) drawWeightDialog();
    if (state == AppState::SHOW_RESULT ||
        ((state == AppState::GRAPH_EDIT || state == AppState::SHOW_RESULT) && !m_path.empty()))
        drawResultBanner();
    if (state == AppState::EXIT_CONFIRM) drawExitDialog(state);
}

// ─────────────────────────────────────────────────────────────────────────────
// graph operations
// ─────────────────────────────────────────────────────────────────────────────

void GraphCanvas::addNode(float x, float y) {
    for (auto& n : m_nodes) {
        float dx = n.x - x, dy = n.y - y;
        if (std::sqrt(dx*dx + dy*dy) < NODE_RADIUS * 2.6f) {
            m_statusMsg = "Too close to an existing node!"; return;
        }
    }
    m_nodes.push_back({m_nextId++, x, y});
    m_statusMsg = "Node " + std::to_string(m_nextId - 1) + " placed.";
}

void GraphCanvas::addEdge(int u, int v, int w) {
    for (auto& e : m_edges) {
        if ((e.u==u && e.v==v) || (e.u==v && e.v==u)) {
            e.weight = w;
            m_statusMsg = "Edge weight updated to " + std::to_string(w) + ".";
            return;
        }
    }
    m_edges.push_back({u, v, w});
}

void GraphCanvas::resetGraph() {
    m_nodes.clear(); m_edges.clear();
    m_steps.clear(); m_path.clear(); m_pathEdgeSet.clear();
    m_nextId=0; m_startId=-1; m_endId=-1;
    m_edgeFirst=-1; m_mode=EditMode::NONE; m_stepIdx=0;
    m_statusMsg = "Graph cleared. Start adding nodes!";
}

// ─────────────────────────────────────────────────────────────────────────────
// Dijkstra
// ─────────────────────────────────────────────────────────────────────────────

void GraphCanvas::runDijkstra() {
    int n = static_cast<int>(m_nodes.size());
    m_steps.clear(); m_path.clear(); m_pathEdgeSet.clear();
    m_stepIdx=0; m_stepTimer=0.f; m_finalDist=INF;

    std::vector<int>  dist(n, INF);
    std::vector<bool> visited(n, false);
    std::vector<int>  parent(n, -1);
    dist[m_startId] = 0;

    auto snap = [&](int cur, const std::string& msg) {
        m_steps.push_back({cur, dist, visited, parent, msg});
    };
    snap(-1, "Initialised — source node " + std::to_string(m_startId) + " has distance 0.");

    using pii = std::pair<int,int>;
    std::priority_queue<pii, std::vector<pii>, std::greater<pii>> pq;
    pq.push({0, m_startId});

    while (!pq.empty()) {
        auto [d, u] = pq.top(); pq.pop();
        if (visited[u]) continue;
        visited[u] = true;

        snap(u, "Visiting node " + std::to_string(u) +
                  "  (dist = " + std::to_string(d) + ").");
        if (u == m_endId) break;

        for (auto& e : m_edges) {
            int v = -1;
            if (e.u == u) v = e.v;
            else if (e.v == u) v = e.u;
            if (v < 0 || visited[v]) continue;

            if (dist[u] + e.weight < dist[v]) {
                dist[v]   = dist[u] + e.weight;
                parent[v] = u;
                pq.push({dist[v], v});
                snap(u, "Relaxed " + std::to_string(u) + " -> " +
                          std::to_string(v) + "   new dist = " + std::to_string(dist[v]) + ".");
            }
        }
    }

    m_finalDist = dist[m_endId];
    if (m_finalDist >= INF) {
        snap(-1, "No path exists between node " + std::to_string(m_startId) +
                  " and node " + std::to_string(m_endId) + ".");
    } else {
        buildPath(parent, m_endId);
        snap(-1, "Done!  Shortest distance = " + std::to_string(m_finalDist) +
                  ".  Press Space or click to view result.");
    }
}

void GraphCanvas::buildPath(const std::vector<int>& parent, int dst) {
    for (int v = dst; v != -1; v = parent[v]) m_path.push_back(v);
    std::reverse(m_path.begin(), m_path.end());
    for (int k = 0; k+1 < static_cast<int>(m_path.size()); k++) {
        int a = m_path[k], b = m_path[k+1];
        for (int i = 0; i < static_cast<int>(m_edges.size()); i++)
            if ((m_edges[i].u==a && m_edges[i].v==b) ||
                (m_edges[i].u==b && m_edges[i].v==a))
                m_pathEdgeSet.insert(i);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// drawing helpers
// ─────────────────────────────────────────────────────────────────────────────

void GraphCanvas::drawEdgeLine(sf::Vector2f a, sf::Vector2f b,
                               float thick, sf::Color col) {
    sf::Vector2f dir = b - a;
    float len = vlen(dir);
    if (len < 1.f) return;
    float angle = std::atan2(dir.y, dir.x) * 180.f / 3.14159265f;
    sf::RectangleShape line({len, thick});
    line.setOrigin({0.f, thick / 2.f});
    line.setPosition(a);
    line.setRotation(sf::degrees(angle));
    line.setFillColor(col);
    m_win.draw(line);
}

void GraphCanvas::drawEdgeWeight(sf::Vector2f mid, int w, bool highlight) {
    std::string s = std::to_string(w);
    float bw = 28.f + s.size() * 6.f;
    sf::RectangleShape bg({bw, 20.f});
    bg.setOrigin({bw/2.f, 10.f}); bg.setPosition(mid);
    bg.setFillColor(sf::Color(28,28,42,230));
    bg.setOutlineThickness(1.f);
    bg.setOutlineColor(highlight ? sf::Color(50,210,100) : sf::Color(75,80,115));
    m_win.draw(bg);

    sf::Text t(m_font, s, 12);
    t.setFillColor(highlight ? sf::Color(100,255,150) : sf::Color(210,210,240));
    centreText(t, mid.x, mid.y);
    m_win.draw(t);
}

void GraphCanvas::drawNode(const GraphNode& n, sf::Color fill,
                           sf::Color outline, int distVal) {
    sf::CircleShape shadow(NODE_RADIUS + 3.f);
    shadow.setOrigin({NODE_RADIUS+3.f, NODE_RADIUS+3.f});
    shadow.setPosition({n.x+3.f, n.y+3.f});
    shadow.setFillColor(sf::Color(0,0,0,70));
    m_win.draw(shadow);

    sf::CircleShape circ(NODE_RADIUS);
    circ.setOrigin({NODE_RADIUS, NODE_RADIUS});
    circ.setPosition({n.x, n.y});
    circ.setFillColor(fill);
    circ.setOutlineThickness(3.f);
    circ.setOutlineColor(outline);
    m_win.draw(circ);

    sf::Text id(m_font, std::to_string(n.id), 17);
    id.setFillColor(sf::Color::White);
    id.setStyle(sf::Text::Bold);
    centreText(id, n.x, n.y);
    m_win.draw(id);

    if (distVal >= 0) {
        sf::Text dt(m_font, "d=" + std::to_string(distVal), 11);
        dt.setFillColor(sf::Color(255,225,80));
        centreTextX(dt, n.x, n.y + NODE_RADIUS + 5.f);
        m_win.draw(dt);
    } else if (distVal == -1) {
        sf::Text dt(m_font, "d=inf", 11);
        dt.setFillColor(sf::Color(180,180,200));
        centreTextX(dt, n.x, n.y + NODE_RADIUS + 5.f);
        m_win.draw(dt);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// toolbar
// ─────────────────────────────────────────────────────────────────────────────

void GraphCanvas::drawToolbar(AppState state) {
    const float H = static_cast<float>(m_win.getSize().y);

    sf::RectangleShape bg({TOOLBAR_W, H});
    bg.setFillColor(sf::Color(32,32,48));
    bg.setOutlineThickness(1.f);
    bg.setOutlineColor(sf::Color(55,55,85));
    m_win.draw(bg);

    sf::Text title(m_font, "TOOLS", 12);
    title.setFillColor(sf::Color(100,110,155));
    title.setStyle(sf::Text::Bold);
    title.setPosition({12.f, 12.f});
    m_win.draw(title);

    sf::Vector2i mi = sf::Mouse::getPosition(m_win);
    sf::Vector2f mouse(static_cast<float>(mi.x), static_cast<float>(mi.y));

    for (auto& btn : m_buttons) {
        bool hover  = btn.rect.getGlobalBounds().contains(mouse);
        bool active = (btn.id=="addnode" && m_mode==EditMode::ADD_NODE) ||
                      (btn.id=="addedge" && m_mode==EditMode::ADD_EDGE) ||
                      (btn.id=="start"   && m_mode==EditMode::SELECT_START) ||
                      (btn.id=="end"     && m_mode==EditMode::SELECT_END);

        sf::Color col = btn.baseColor;
        if (active)     col = sf::Color(std::min(255,col.r+40),std::min(255,col.g+40),std::min(255,col.b+40));
        else if (hover) col = sf::Color(std::min(255,col.r+25),std::min(255,col.g+25),std::min(255,col.b+25));
        btn.rect.setFillColor(col);
        btn.rect.setOutlineThickness(active ? 2.f : 1.f);
        btn.rect.setOutlineColor(active ? sf::Color(200,220,255) : sf::Color(60,65,100));
        m_win.draw(btn.rect);
        {
            sf::FloatRect r = btn.rect.getGlobalBounds();
            sf::Text lbl(m_font, btn.labelText, 12);
            lbl.setFillColor(sf::Color::White);
            sf::FloatRect lb = lbl.getLocalBounds();
            lbl.setPosition({r.position.x + 8.f,
                             r.position.y + (r.size.y - lb.size.y) / 2.f - 3.f});
            m_win.draw(lbl);
        }
    }

    // legend during algorithm
    if (state == AppState::ALGO_RUNNING || state == AppState::SHOW_RESULT) {
        float ly = H * 0.60f;
        sf::Text legHdr(m_font, "LEGEND", 11);
        legHdr.setFillColor(sf::Color(95,100,145));
        legHdr.setStyle(sf::Text::Bold);
        legHdr.setPosition({12.f, ly - 6.f});
        m_win.draw(legHdr);
        ly += 18.f;

        struct LEntry { sf::Color col; const char* lbl; };
        const LEntry entries[] = {
            {sf::Color(55,95,155),   "Unvisited"},
            {sf::Color(30,144,255),  "Current"},
            {sf::Color(195,160,0),   "Visited"},
            {sf::Color(40,210,90),   "Shortest Path"},
            {sf::Color(0,170,90),    "Start Node"},
            {sf::Color(200,45,45),   "End Node"},
        };
        for (auto& e : entries) {
            sf::CircleShape dot(7.f);
            dot.setFillColor(e.col);
            dot.setPosition({12.f, ly});
            m_win.draw(dot);
            sf::Text lt(m_font, e.lbl, 11);
            lt.setFillColor(sf::Color(185,185,210));
            lt.setPosition({27.f, ly + 1.f});
            m_win.draw(lt);
            ly += 20.f;
        }
    }

    // keyboard hints
    float ky = H - STATUS_H - 10.f;
    const char* hints[] = {"N - Add Node","E - Add Edge","S - Set Start",
                            "D - Set End","R - Run","C - Clear","Esc - Exit"};
    for (int i = 6; i >= 0; i--) {
        ky -= 15.f;
        sf::Text ht(m_font, hints[i], 10);
        ht.setFillColor(sf::Color(75,80,115));
        ht.setPosition({10.f, ky});
        m_win.draw(ht);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// status bar
// ─────────────────────────────────────────────────────────────────────────────

void GraphCanvas::drawStatusBar(AppState state) {
    const float W = static_cast<float>(m_win.getSize().x);
    const float H = static_cast<float>(m_win.getSize().y);

    sf::RectangleShape bar({W - TOOLBAR_W, STATUS_H});
    bar.setPosition({TOOLBAR_W, H - STATUS_H});
    bar.setFillColor(sf::Color(28,28,42));
    bar.setOutlineThickness(1.f);
    bar.setOutlineColor(sf::Color(50,55,80));
    m_win.draw(bar);

    const char* modeName = "View";
    sf::Color modeCol(90,120,200);
    switch (m_mode) {
    case EditMode::ADD_NODE:     modeName="Add Node";  modeCol=sf::Color(80,130,220); break;
    case EditMode::ADD_EDGE:     modeName="Add Edge";  modeCol=sf::Color(120,80,200); break;
    case EditMode::SELECT_START: modeName="Set Start"; modeCol=sf::Color(0,160,80);   break;
    case EditMode::SELECT_END:   modeName="Set End";   modeCol=sf::Color(190,50,50);  break;
    default: break;
    }
    if (state==AppState::ALGO_RUNNING) { modeName="Running"; modeCol=sf::Color(80,160,255); }
    if (state==AppState::SHOW_RESULT)  { modeName="Result";  modeCol=sf::Color(40,200,90);  }

    sf::RectangleShape badge({90.f, 26.f});
    badge.setPosition({TOOLBAR_W + 8.f, H - STATUS_H + 11.f});
    badge.setFillColor(modeCol);
    m_win.draw(badge);

    sf::Text modeT(m_font, modeName, 12);
    modeT.setFillColor(sf::Color::White);
    modeT.setStyle(sf::Text::Bold);
    centreText(modeT, TOOLBAR_W + 8.f + 45.f, H - STATUS_H + 24.f);
    m_win.draw(modeT);

    sf::Text msg(m_font, m_statusMsg, 13);
    msg.setFillColor(sf::Color(200,200,220));
    msg.setPosition({TOOLBAR_W + 108.f, H - STATUS_H + 16.f});
    m_win.draw(msg);

    sf::Text cnt(m_font, "Nodes: " + std::to_string(m_nodes.size()) +
                          "   Edges: " + std::to_string(m_edges.size()), 11);
    cnt.setFillColor(sf::Color(100,105,145));
    sf::FloatRect cb = cnt.getLocalBounds();
    cnt.setOrigin({cb.position.x + cb.size.x, cb.position.y});
    cnt.setPosition({W - 10.f, H - STATUS_H + 17.f});
    m_win.draw(cnt);
}

// ─────────────────────────────────────────────────────────────────────────────
// weight input dialog
// ─────────────────────────────────────────────────────────────────────────────

void GraphCanvas::drawWeightDialog() {
    const float W = static_cast<float>(m_win.getSize().x);
    const float H = static_cast<float>(m_win.getSize().y);

    sf::RectangleShape dim({W, H});
    dim.setFillColor(sf::Color(0,0,0,145));
    m_win.draw(dim);

    sf::RectangleShape box({360.f, 170.f});
    box.setOrigin({180.f, 85.f}); box.setPosition({W/2.f, H/2.f});
    box.setFillColor(sf::Color(38,38,58));
    box.setOutlineThickness(2.f); box.setOutlineColor(sf::Color(90,150,255));
    m_win.draw(box);

    float cx = W/2.f, cy = H/2.f;
    auto txt = [&](const std::string& s, unsigned sz, sf::Color col, float y) {
        sf::Text t(m_font, s, sz); t.setFillColor(col);
        centreTextX(t, cx, y); m_win.draw(t);
    };
    txt("Enter Edge Weight", 20, sf::Color(90,180,255), cy - 72.f);
    txt("(digits only, positive integer)", 12, sf::Color(130,135,175), cy - 46.f);

    sf::RectangleShape field({230.f, 40.f});
    field.setOrigin({115.f, 20.f}); field.setPosition({cx, cy - 5.f});
    field.setFillColor(sf::Color(24,24,38));
    field.setOutlineThickness(2.f); field.setOutlineColor(sf::Color(90,150,255));
    m_win.draw(field);

    sf::Text val(m_font, m_weightStr.empty() ? "|" : m_weightStr + "|", 20);
    val.setFillColor(sf::Color::White);
    centreText(val, cx, cy - 5.f);
    m_win.draw(val);

    txt("Enter = confirm     Esc = cancel", 12, sf::Color(120,125,165), cy + 30.f);
}

// ─────────────────────────────────────────────────────────────────────────────
// result banner
// ─────────────────────────────────────────────────────────────────────────────

void GraphCanvas::drawResultBanner() {
    const float W = static_cast<float>(m_win.getSize().x);

    sf::RectangleShape panel({W - TOOLBAR_W - 20.f, 72.f});
    panel.setPosition({TOOLBAR_W + 10.f, 10.f});
    panel.setFillColor(sf::Color(22,45,28,235));
    panel.setOutlineThickness(2.f); panel.setOutlineColor(sf::Color(45,200,95));
    m_win.draw(panel);

    std::string pathStr;
    if (m_path.empty()) {
        pathStr = "No path found!";
    } else {
        for (int i = 0; i < static_cast<int>(m_path.size()); i++) {
            if (i > 0) pathStr += " -> ";
            pathStr += std::to_string(m_path[i]);
        }
    }

    sf::Text res(m_font, "Shortest Distance: " +
                          (m_finalDist >= INF ? "No path" : std::to_string(m_finalDist)) +
                          "    Path: " + pathStr, 17);
    res.setFillColor(sf::Color(90,255,145)); res.setStyle(sf::Text::Bold);
    res.setPosition({TOOLBAR_W + 20.f, 20.f});
    m_win.draw(res);

    sf::Text hint(m_font, "Path stays visible until you Reset (C) or run again (R).", 12);
    hint.setFillColor(sf::Color(130,200,145));
    hint.setPosition({TOOLBAR_W + 20.f, 52.f});
    m_win.draw(hint);
}

// ─────────────────────────────────────────────────────────────────────────────
// exit dialog
// ─────────────────────────────────────────────────────────────────────────────

void GraphCanvas::drawExitDialog(AppState& /*state*/) {
    const float W = static_cast<float>(m_win.getSize().x);
    const float H = static_cast<float>(m_win.getSize().y);

    sf::RectangleShape dim({W, H});
    dim.setFillColor(sf::Color(0,0,0,165));
    m_win.draw(dim);

    sf::RectangleShape box({380.f, 200.f});
    box.setOrigin({190.f, 100.f}); box.setPosition({W/2.f, H/2.f});
    box.setFillColor(sf::Color(38,30,42));
    box.setOutlineThickness(2.f); box.setOutlineColor(sf::Color(210,75,75));
    m_win.draw(box);

    float cx = W/2.f, cy = H/2.f;
    auto txt = [&](const std::string& s, unsigned sz, sf::Color col, float y) {
        sf::Text t(m_font, s, sz); t.setFillColor(col);
        centreTextX(t, cx, y); m_win.draw(t);
    };
    txt("Exit Application?",           22, sf::Color(255,95,95),   cy - 82.f);
    txt("All unsaved work will be lost.", 14, sf::Color(200,195,215), cy - 50.f);

    // Yes button
    sf::RectangleShape yBtn({150.f, 40.f});
    yBtn.setPosition({cx - 170.f, cy + 28.f});
    yBtn.setFillColor(sf::Color(180,45,45));
    m_win.draw(yBtn);
    sf::Text yt(m_font, "Yes  (Y)", 14);
    yt.setFillColor(sf::Color::White);
    centreText(yt, cx - 95.f, cy + 48.f);
    m_win.draw(yt);

    // No button
    sf::RectangleShape nBtn({150.f, 40.f});
    nBtn.setPosition({cx + 20.f, cy + 28.f});
    nBtn.setFillColor(sf::Color(45,100,45));
    m_win.draw(nBtn);
    sf::Text nt(m_font, "No  (N)", 14);
    nt.setFillColor(sf::Color::White);
    centreText(nt, cx + 95.f, cy + 48.f);
    m_win.draw(nt);
}

// ─────────────────────────────────────────────────────────────────────────────
// hit testing
// ─────────────────────────────────────────────────────────────────────────────

int GraphCanvas::nodeAt(float x, float y) const {
    for (int i = 0; i < static_cast<int>(m_nodes.size()); i++) {
        float dx = m_nodes[i].x - x, dy = m_nodes[i].y - y;
        if (std::sqrt(dx*dx + dy*dy) <= NODE_RADIUS + 5.f) return i;
    }
    return -1;
}

bool GraphCanvas::inCanvas(float x, float y) const {
    const float H = static_cast<float>(m_win.getSize().y);
    return x > TOOLBAR_W && y < H - STATUS_H;
}

// ─────────────────────────────────────────────────────────────────────────────
// toolbar buttons
// ─────────────────────────────────────────────────────────────────────────────

void GraphCanvas::buildButtons() {
    m_buttons.clear();
    struct Def { const char* id; const char* lbl; sf::Color col; };
    const Def defs[] = {
        {"addnode","Add Node  (N)",    sf::Color(45, 85,170)},
        {"addedge","Add Edge  (E)",    sf::Color(75, 55,165)},
        {"start",  "Set Start  (S)",   sf::Color(30,120, 75)},
        {"end",    "Set End  (D)",     sf::Color(155,40, 40)},
        {nullptr,  nullptr,            sf::Color()},
        {"run",    "Run Dijkstra (R)", sf::Color(55,120,210)},
        {nullptr,  nullptr,            sf::Color()},
        {"reset",  "Reset  (C)",       sf::Color(115,75, 25)},
        {"exit",   "Exit  (Esc)",      sf::Color(110,35, 35)},
    };

    const float bw=TOOLBAR_W-20.f, bh=38.f, bx=10.f;
    float by = 35.f;
    for (auto& d : defs) {
        if (!d.id) { by += 10.f; continue; }
        Btn btn;
        btn.id = d.id; btn.baseColor = d.col;
        btn.rect.setSize({bw, bh}); btn.rect.setPosition({bx, by});
        btn.rect.setFillColor(d.col);
        btn.labelText = d.lbl;
        m_buttons.push_back(btn);
        by += bh + 5.f;
    }
}
