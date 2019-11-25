#ifndef SOLVER_H
#define SOLVER_H

#include <iostream>
#include <map>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace std;

class Clause {
    public:
        Clause(vector<int>& lits);
        ~Clause();
        const vector<int>& getLits();
        unsigned int size(); //returns number of literals in clause
        friend ostream& operator<<(ostream& os, const Clause& c);
        unsigned int watched1; //index of first watched literal in lits
        unsigned int watched2;
    private:
        const vector<int> lits; //absolute value of int is the variable name. If value < 0, indicates negation of the variable
};

class VarAssignment {
    public:
        VarAssignment();
        ~VarAssignment();
        void setAssignment(bool tVal, int lvl, int stp, int ant);
        bool truthVal;
        int level; //-1 if not assigned truthVal
        unsigned int step;
        unsigned int antecedent; //clause number that determined truth value of the Var
};

// Abstract base class for decision heuristic that guesses a new variable to propagate on
class Decider {
    public:
        Decider(vector<Clause>& f);
        ~Decider();
       virtual void stepCounter() = 0; 
       virtual void updateDecider(Clause& newClause) = 0;
    protected:
       unsigned int counter;
};

class Vsids : public Decider {
    public:
        Vsids(vector<Clause>& f);
        ~Vsids();
       void stepCounter(); 
       void updateDecider(Clause& newClause);
    private:
        unordered_map<int, float> vsidsScores; //decision heuristic score for each literal in original formula f
        map<int, float> vsidsMap; //partially ordered decision heuristic scores. Does not necessarily include all literals in f, but only assigned ones
};

map<int, unordered_set<unsigned int>> initWatchLists(vector<Clause>& f);
int initialCheck(vector<Clause>& f, vector<VarAssignment>& a, map<int, unordered_set<unsigned int>> watchLists, int level, unsigned int& numAssigned);
int bcp(vector<Clause>& f, vector<VarAssignment>& a, queue<int> q, map<int, unordered_set<unsigned int>> watchLists, 
        int& level, int& step, unsigned int& numAssigned);
void setAssignment(vector<VarAssignment>& a, int var, int truthVal, int level, int step, int antecedent, unsigned int& numAssigned);

//Returns 1 if formula f is satisfiable, 0 otherwise
vector<int> CDCL(vector<Clause>& f, const unsigned int numVars, int& sat);

#endif
