#ifndef SOLVER_H
#define SOLVER_H

#include <vector>
#include <map>
#include <unordered_map>

using namespace std;

class Clause {
    public:
        Clause(vector<int>& lits);
        ~Clause();
    private:
        vector<int> lits; //absolute value of int is the variable name. If value < 0, indicates negation of the variable
        unsigned int watched1; //index of first watched literal in lits
        unsigned int watched2;
};

class VarAssignment {
    public:
        VarAssignment();
        ~VarAssignment();
    private:
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

//Returns 1 if formula f is satisfiable, 0 otherwise
int CDCL(vector<Clause>& f);

#endif
