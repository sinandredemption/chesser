Chesser
=========
Chesser is an open source chess engine derived from Winglet. It is an *almost* master level engine, but it is weak in playing the endgame. It is estimated to be around 400-600 elo points stronger than WingletX. Internally, it uses Bitboard based piece encoding, and uses Magic Move generation with Pradu Kannan's source. It has a fairly basic evaluation function, and uses PVS-based search with null move pruning and history heuristics.

**Version**
-----------
This technically is version 2.1 named "Chesser SS", and is dedicated to Sven Schule, who helped me throughly analyse the engine. There are of course many other people to thank: Matthew Brades, Pradu Kannan (magic move generation source), Stef Luijten (author of Winglet and "Creating a chess engine in 99 steps", that taught me chess programming), and of course chessprogramming.org

**Compiling**
--------------
You have the full source code. The source code has Windows dependant code but it shouldn't be too difficult to switch to Linux or even full compatibility. If you've managed to do compile it on Linux, do drop me a line at my email below.
Compiling it should be simple enough. I have successfully compiled it on Microsoft Visual C++ and G++ 4.8.1 on Dev C++.

**Protocol**
---------
Chesser can communitcate with GUIs that support Winboard Protocol e.g. XBoard,
Arena etc.
