# Frozen Lake: Bellman Equation Approximation via Monte Carlo Heat Maps

C++17 implementation of value estimation and policy iteration on stochastic grid environments (Frozen Lake). The project uses Monte Carlo sampling to approximate the Bellman Expectation Equation and renders value distributions directly in the terminal using state heat maps.

## Overview & Methodology

* **Bellman Expectation Approximation:** Solves state-value distributions $V(s)$ over non-deterministic grid transitions via Monte Carlo rollouts:
  $$V(s) = \mathbb{E}\left[ R_{t+1} + \gamma V(S_{t+1}) \mid S_t = s \right]$$
* **Terminal Heat Maps:** Visualizes state values and value-density gradients directly in standard output to track policy convergence and risk boundaries around environmental hazards (holes).
* **High-Performance C++ Architecture:** Zero external dependencies, header-only structure using strict C++17 primitives for deterministic execution and low memory footprint.

## Project Structure

```text
.
├── agent.hpp     # Agent policy execution, Monte Carlo rollouts, and value updates
├── cell.hpp      # Grid state representations, transitions, and reward mappings
├── lake.hpp      # Lake environment state, grid layout, and step dynamics
├── path.hpp      # Value evaluation routines, trajectory tracking, and heat map rendering
├── main.cpp      # Simulation runner and environmental parameters
├── .gitignore    # Build artifact exclusions
└── README.md     # Project documentation and theoretical background