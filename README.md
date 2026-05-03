# Shortest Path Finder — Dijkstra's Algorithm

An interactive C++ desktop application that lets you build a weighted graph and
watch Dijkstra's algorithm find the shortest path step by step, with full color
animation.

Built with **C++17** and **SFML 3**.

---

## Application flow

```
Splash Screen (4 s) → Login → Graph Editor → Run Algorithm → View Result
```

| Screen | What happens |
|---|---|
| **Splash** | Animated title and progress bar. Advances automatically after 4 seconds. |
| **Login** | Enter credentials to unlock the editor. |
| **Graph Editor** | Build your graph — add nodes, connect them with weighted edges, pick start/end. |
| **Algorithm** | Press R. Each step animates at 0.75 s intervals with color-coded nodes. |
| **Result** | Shortest path highlighted in green. Banner shows total distance and the node sequence. Path stays visible until you reset or run again. |
| **Exit** | Confirm with Y / N before closing. |

---

## Login credentials

| Username | Password |
|---|---|
| admin | 1234 |
| user | pass |
| guest | guest |

---

## Build instructions

### 1 — Install dependencies

**macOS (Homebrew)**
```bash
brew install sfml cmake
```

**Ubuntu / Debian**
```bash
sudo apt update
sudo apt install libsfml-dev cmake build-essential
```

**Windows**
1. Download SFML 3 from https://sfml-dev.org
2. Install CMake from https://cmake.org/download
3. Extract SFML and note the path (e.g. `C:\SFML-3.0.2`)
4. Pass it to CMake: `cmake .. -DSFML_DIR="C:\SFML-3.0.2\lib\cmake\SFML"`

> **Note:** This project requires **SFML 3** (not SFML 2). `brew install sfml`
> installs SFML 3 by default on current Homebrew.

---

### 2 — Build

```bash
git clone <repo-url>          # or just cd into the folder
cd ShortestPathFinder

mkdir build
cd build

cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

---

### 3 — Run

```bash
# macOS / Linux  (from inside the build/ folder)
./ShortestPathFinder

# Windows
ShortestPathFinder.exe
```

> **No text visible?** Place any `.ttf` font file at
> `ShortestPathFinder/assets/font.ttf` and rebuild. The app automatically
> searches common system font paths, but falls back to this file if none are
> found.

---

## How to use the Graph Editor

Follow these steps in order:

### Step 1 — Place nodes
Press **N** to enter Add Node mode, then **click** anywhere on the dark canvas
to place a node. Each node is numbered automatically (0, 1, 2 …).
Repeat until you have enough nodes.

### Step 2 — Connect nodes with edges
Press **E** to enter Add Edge mode.
- Click the **first** node — it turns purple.
- Click the **second** node.
- A dialog appears asking for the **edge weight** — type a positive number and
  press **Enter**. Press **Esc** to cancel.

Clicking the same pair again overwrites the weight.

### Step 3 — Set start and end nodes
- Press **S**, then click a node → it turns **green** (start).
- Press **D**, then click a node → it turns **red** (end).

### Step 4 — Run the algorithm
Press **R** (or click *Run Dijkstra* in the toolbar).

Watch the animation:
- **Bright blue** = node currently being processed.
- **Gold** = already visited.
- **d=N** labels show the current known shortest distance for each node.

The animation advances every 0.75 seconds automatically.

### Step 5 — Read the result
When the animation finishes, the shortest path turns **green** and a banner at
the top shows:

```
Shortest Distance: 15    Path: 0 → 2 → 4 → 7
```

The path stays highlighted so you can study it while continuing to edit.

### Step 6 — Reset or run again
- Press **C** (or click *Reset*) to clear everything and start over.
- Press **R** again on the same graph to re-run with new start/end nodes.

---

## Keyboard shortcuts

| Key | Action |
|---|---|
| **N** | Add Node mode — next click places a node |
| **E** | Add Edge mode — click two nodes, then type weight |
| **S** | Set Start — click a node to mark it green |
| **D** | Set End (Destination) — click a node to mark it red |
| **R** | Run Dijkstra's algorithm |
| **C** | Clear / reset the entire graph |
| **Esc** | Open exit confirmation dialog |

All actions are also available as buttons in the left toolbar.

---

## Color reference

### During algorithm animation

| Color | Node state |
|---|---|
| Steel blue | Unvisited (distance unknown) |
| Bright blue | Currently being relaxed |
| Gold / yellow | Visited (shortest distance finalized) |
| Green | On the final shortest path |
| Green outline | Start node |
| Red outline | End node |

### Edge colors

| Color | Meaning |
|---|---|
| Gray | Normal edge |
| Green (thick) | Edge on the shortest path |

---

## Project structure

```
ShortestPathFinder/
├── CMakeLists.txt             — SFML 3 build config
├── main.cpp                   — window creation, state machine loop
├── include/
│   ├── GraphTypes.h           — GraphNode, GraphEdge, AlgoStep, enums
│   ├── SplashScreen.h         — animated splash screen (header-only class)
│   ├── LoginScreen.h          — login form with credential check (header-only)
│   └── GraphCanvas.h          — graph editor + Dijkstra visualizer (header)
└── src/
    └── GraphCanvas.cpp        — full implementation (~900 lines)
```

### State machine

```
SPLASH → LOGIN → GRAPH_EDIT ←→ WEIGHT_INPUT
                     ↓
               ALGO_RUNNING
                     ↓
               SHOW_RESULT → GRAPH_EDIT
                     ↓
               EXIT_CONFIRM → (close)
```

### Dijkstra implementation notes

- All steps are **pre-computed** in one pass when you press R, stored as
  `AlgoStep` snapshots (dist[], visited[], parent[]).
- The render loop **replays** those snapshots at 0.75 s intervals — no blocking
  `sleep()` calls.
- The path is reconstructed by following `parent[]` from end → start, then
  reversed.
- Graph is **undirected** — every edge is traversable in both directions.

---

## Troubleshooting

| Problem | Fix |
|---|---|
| `SFML not found` during cmake | Run `brew install sfml` (macOS) or install `libsfml-dev` |
| App opens but shows no text | Add a `.ttf` font to `assets/font.ttf` and rebuild |
| Window title shows `?` characters | Use only ASCII in the title (already fixed in current code) |
| Path disappears after clicking | Fixed — path now persists until Reset or re-run |
| Node placed on top of another | Nodes must be at least 2× radius apart; move your click |
# ShortestPathFinder
