## Chalk 

---

#### About

A zippy SAT Solver for boolean satisfiability problems. Makes use of the
Clause Driven Conflict Learning (CDCL) algorithm internally. Uses the 
2-Watched Literal scheme for efficient constraint propagation, and the 
Variable State Independent Decaying Sum (VSIDS) heuristic to reduce 
the amount of backtracking.

Chalk must be fed a file containing CNF boolean formulas in the [DIMACS format](http://www.satcompetition.org/2009/format-benchmarks2009.html).
For any given formula that is satisfiable, CDCL-SAT outputs 'sat', followed by 
a whitespace separated list of literals that serve as a satisfiable truth assignment 
for the variables in the formula. If a given formula is unsatisfiable, it simply
outputs 'unsat'.

The solver has been extensively tested with various benchmarks, and outputs 
results in a reasonable time for >97% of them. Some of these benchmarks 
can be found in this repository. 

---

#### Setup

To build, run make:

    `make`

Or:

    `make solver`

---

#### Run

To run the solver with any file in the DIMACS CNF Format:

    `./solver filename`

---

#### Testing and Benchmarks

There are multiple folders in the `benchmarks/benchmarks` folder, including the following: 

* bench1: The easiest benchmark set holds 27 sat cnf and 21 unsat cnf.
          This folder has two sub-folders: one is sat and another is unsat.
          The 27 sat cnf are in the sat folder, and the 21 unsat cnf are
          in the unsat folder.

* bench2: Kind of harder than bench1, and also has two sub-folders: one 
          (named sat) contains 11 sat cnf and another one (named unsat)
          contains 14 unsat cnf. 

* bench3: The hardest benchmarks. It only contains five unsat cnf, and 
          does not have a sub-folder.

* groups: You can build a folder for each group and put their 
          folders in the 'groups' folder. For example, now we have one
          group, called "group".

*How to test ?*

Enter one of "groups" folders, e.g. "group".
You can see three files named 'bench1.sh', 'bench2.sh' and 'bench3.sh'.
Each of them is for testing the benchmarks in the corresponding folder. 
For example, when we enter the group1 folder, and type 'sh bench1.sh', the screen 
will show how many benchmarks the solver can pass.

You may notice there are five files: name1-5. Each of them is responsible
for recording cnf names:
- name1 lists all the sat cnf in bench1 
- name2 lists all the unsat cnf in bench1
- name3 lists all the sat cnf in bench2
- name4 lists all the unsat cnf in bench2
- name5 lists all the unsat cnf in bench3

You should be able to run the solver with the benchmarks scripts and
see all the tests passing, without needing to make any changes to the
script.

*How to apply more benchmarks?*

More benchmarks can be found [here](http://www.satcompetition.org/). To 
run the benchmarks:

1. Create a new folder in the `benchmarks/benchmarks` directory.
2. Create two sub-folders, `sat` and `unsat`.
3. Put your new cnf files in the appropriate sat or unsat folder
4. Update the corresponding name file, e.g., name1, since the test script
   will read the names from the file
5. Copy an existing test script, e.g. bench1, and modify the file paths in
   the script to point to your new folder.
6. Run the new test script.
