#ifndef SOLVER_H
#define SOLVER_H

#include <iostream>
#include <map>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <tuple>
#include <utility>
#include <vector>

namespace solver {

using namespace std;

class VarAssignment {
    public:
        VarAssignment();
        ~VarAssignment();
        void setAssignment(bool tVal, int lvl, int stp, unsigned int ant);
        void unsetAssignment();
        friend ostream& operator<<(ostream& os, const VarAssignment& v);
        bool truthVal;
        int level; // -1 if not assigned truthVal
        unsigned int step;
        unsigned int antecedent; // Clause number that determined truth value of the Var
};

class Clause {
    public:
        Clause(vector<int> lits);
        ~Clause();
        const vector<int>& getLits();
        unsigned int size(); // Returns number of literals in clause
        friend ostream& operator<<(ostream& os, const Clause& c);
        unsigned int watched1; // Index of first watched literal in lits
        unsigned int watched2;
    private:
        // Absolute value of int is the variable name. If value < 0, indicates negation of the variable
        const vector<int> lits;
};

// Abstract base class for decision heuristic that guesses a new variable to propagate on
class Decider {
    public:
        Decider(vector<Clause>& f);
        ~Decider();
        virtual void stepCounter() = 0;
        virtual void update(Clause& newClause) = 0;
        virtual int decide(const vector<VarAssignment>& a) = 0;
    protected:
       unsigned int counter;
};

// Variable State Independent Decay Heuristic
class Vsids : public Decider {
    public:
        Vsids(vector<Clause>& f);
        ~Vsids();
        void stepCounter();
        void update(Clause& newClause);
        int decide(const vector<VarAssignment>& a);
        void addToContention(int var);
    private:
        unordered_map<int, float> vsidsScores; // Decision heuristic score for each literal in original formula f
        // Partially ordered decision heuristic scores. Does not necessarily include all literals in f, but only 
        // unassigned ones
        multimap<float, int> vsidsMap;
};

// 2-Watched literal scheme. Map from literal to list of clause numbers in which it is watched
map<int, unordered_set<unsigned int>> initWatchLists(vector<Clause>& f);

// Sets truth assignments for all Clauses of size 1, and propagates new assignments
int initialCheck(vector<Clause>& f, vector<VarAssignment>& a, map<int, 
                 unordered_set<unsigned int>>& watchLists, int level, unsigned int& numAssigned);

// Boolean Constant Propagation: Given new truth assignments in queue q, looks at all clauses in f to 
// determine all new truth assignments that can be deduced
tuple<int, unsigned int, int> bcp(vector<Clause>& f, vector<VarAssignment>& a, queue<int> q, 
                                  map<int, unordered_set<unsigned int>>& watchLists,
                                  int& level, int& step, unsigned int& numAssigned);

// Given a conflicting clause, determines first Unique Implication Point (UIP), 
// returns a clause at that point with the new information learnt
pair<int, Clause> analyzeConflict(vector<Clause>& f, vector<VarAssignment>& a, unsigned int clauseNum);

// Given formulas 1 -2 3, and 5 2 3, with conflictVar == 2, returns 1 5 3
void resolve(vector<int>& lits, const vector<int>& lits2, int conflictVar);

// Add watched literals for Clause c to watchLists
void addToWatchLists(map<int, unordered_set<unsigned int>>& watchLists, Clause& c, unsigned int clauseNum);

// Returns max step assigned at the backtracking level. Unsets all variables at higher levels
unsigned int backtrack(vector<VarAssignment>& a, Vsids& vsids, const int newLevel, unsigned int& numAssigned);

// Returns 1 and a satisfying assignment if formula f is satisfiable, 0 or a negative number otherwise
pair<int, vector<int>> CDCL(vector<Clause>& f, const unsigned int numVars);

}

#endif
