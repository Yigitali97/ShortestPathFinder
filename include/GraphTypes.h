#pragma once
#include <vector>
#include <string>
#include <climits>

constexpr int   INF          = INT_MAX / 2;
constexpr float NODE_RADIUS  = 28.f;
constexpr float TOOLBAR_W    = 215.f;
constexpr float STATUS_H     = 48.f;

enum class AppState {
    SPLASH,
    LOGIN,
    GRAPH_EDIT,
    WEIGHT_INPUT,
    ALGO_RUNNING,
    SHOW_RESULT,
    EXIT_CONFIRM
};

enum class EditMode {
    NONE,
    ADD_NODE,
    ADD_EDGE,
    SELECT_START,
    SELECT_END
};

struct GraphNode {
    int   id;
    float x, y;
};

struct GraphEdge {
    int u, v, weight;
};

struct AlgoStep {
    int               currentNode = -1;
    std::vector<int>  dist;
    std::vector<bool> visited;
    std::vector<int>  parent;
    std::string       message;
};
