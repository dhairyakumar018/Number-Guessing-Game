# MechStrike — Tactical Battle Simulator

A simultaneous-turn tactics game on an 8×8 grid: each round you and the enemy AI
declare actions **at the same time**, then both actions resolve together. Dodge
the shot the enemy is about to take, cut off its retreat, or fire at the cell it
is about to occupy.

The game ships in two editions with **identical rules**:

| Edition | File | Runs on |
|---|---|---|
| **Web** | `index.html` | Any modern browser — phone, tablet, desktop. No install, no server, no dependencies. |
| **Console** | `main.cpp` | Any terminal — pure standard-library C++, compiles on Windows / macOS / Linux. |

## Play the web edition

Open `index.html` in a browser. That's it — everything (styles, game logic,
statistics) lives in that one file, and results are saved on your device with
`localStorage`.

To host it online, enable GitHub Pages for this repository and the game will be
served at `https://<user>.github.io/<repo>/mechstrike/`.

- Move with the on-screen D-pad or <kbd>W</kbd><kbd>A</kbd><kbd>S</kbd><kbd>D</kbd> / arrow keys
- Fire with <kbd>F</kbd> or <kbd>Space</kbd>
- Flee with <kbd>Q</kbd> (asks to confirm — fleeing counts as a loss)
- Menu shortcuts: <kbd>1</kbd> new game, <kbd>2</kbd> statistics, <kbd>3</kbd> instructions

## Play the console edition

```sh
g++ main.cpp -o mechstrike
./mechstrike            # Windows: mechstrike.exe
```

Type a command each round, then press Enter: `W`/`A`/`S`/`D` move, `F` fires,
`Q` quits the battle. Results append to `stats.txt` next to the executable.

## Rules

- **8×8 battlefield** with 8 randomly placed obstacles. You (`P`, 100 HP,
  20 ATK) start top-left; the enemy (`E`, 100 HP, 15 ATK) starts bottom-right.
  Both have attack range 2, measured in Manhattan distance (steps, not
  diagonals).
- **Declare, then resolve.** Both robots choose blind, then movement happens
  first, and shots resolve **after** movement, against post-move positions.
- **Clash:** moving to the same cell — or swapping cells — bounces both robots
  back to where they were. Stepping into the cell the enemy just left is legal.
- **Firing is always allowed**, even out of range: the enemy may walk *into*
  range this round, and a well-predicted shot lands.
- **Mutual destruction is possible.** Damage applies after both hit checks, so
  both robots can die in the same round — a draw.
- Invalid moves (off the board, into an obstacle) cost nothing; the round is
  not used.

## Enemy AI

The enemy decides from the state at the **start** of the round — it cannot see
your declaration, just as you cannot see its:

1. If you are in its range, it **fires** at your position.
2. If its health is below 30%, it **retreats** (moves to maximize distance).
3. Otherwise it **advances** (moves to minimize distance).

## Statistics

Every finished battle is recorded as `WIN` / `LOSS` / `DRAW` plus the number of
rounds. The statistics screen shows totals, win rate, and average rounds — the
web edition stores records in your browser, the console edition in `stats.txt`.
