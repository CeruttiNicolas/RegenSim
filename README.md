# PROMETHEUS

**P**arallelized **R**egenerative c**O**oling **M**odeler and **E**ngine **T**hermo-**H**ydraulic **E**xplicit **U**nsteady **S**imulator

A GPU-accelerated 3D thermal solver for the transient analysis of regenerative cooling channels in liquid-propellant rocket engines, written in C++/CUDA.

Originally developed to analyze the *Efesto* (Hephaestus) rocket engine, this solver pays homage to its origins through its name. Just as the Titan Prometheus stole the flames from the forge of Hephaestus to give them to humankind, PROMETHEUS steals the extreme heat from the engine walls.

## 1. Introduction
Regenerative cooling is a critical part of modern liquid rocket engine design, where the propellant is circulated through the walls of the engine to move heat away from the thrust chamber, maintaining the structure within safe operating temperatures. Insufficient cooling of the engine can lead to catastrophic failures, and physical trial-and-error is prohibitely expensive; thus, simulating the complex interaction between coolant flow and heat transfer is crucial.
