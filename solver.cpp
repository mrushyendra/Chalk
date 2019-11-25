#include "solver.h"
#include <cstdlib>
#include <queue>

//For Clauses with <2 literals, the Clause is not added to Watched Literals, so value of watched1 or watched2 is irrelevant
Clause::Clause(vector<int>& lits) : watched1(0), watched2(1), lits(lits){}

Clause::~Clause(){}

ostream& operator<<(ostream& os, const Clause& c){
    for(unsigned int i = 0; i < c.lits.size(); ++i){
        os << c.lits[i] << " ";
    }
    return os;
}

const vector<int>& Clause::getLits(){
    return this->lits;
}

unsigned int Clause::size(){
    return this->lits.size();
}

//level <0 indicates that variable has not been assigned
VarAssignment::VarAssignment() : truthVal(false), level(-1), step(0), antecedent(0){}

VarAssignment::~VarAssignment(){}

void VarAssignment::setAssignment(bool tVal, int lvl, int stp, int ant){
    this->truthVal = tVal;
    this->level = lvl;
    this->step = stp;
    this->antecedent = ant;
}

Decider::Decider(vector<Clause>& f) : counter(0) {}

Decider::~Decider(){}

Vsids::Vsids(vector<Clause>& f): Decider(f) {
    for(Clause& c : f){
       const vector<int>& ls = c.getLits();
       for(int lit : ls){
           vsidsScores[lit]++;
           vsidsMap[lit]++;
       }
    }
}

Vsids::~Vsids(){}

void Vsids::stepCounter(){
    this->counter++;
}

void Vsids::updateDecider(Clause& newClause){
}

vector<int> CDCL(vector<Clause>& f, const unsigned int numVars, int& sat){
    vector<VarAssignment> assignment(numVars + 1);
    unsigned int numAssigned = 0; //number of variables that solver has assigned
    unsigned int level = 0;
    Vsids vsids(f); //decision heuristic

    //map from variable to set of clauses in which it is watched. Only for clauses with >= 2 literals
    map<int, unordered_set<unsigned int>> watchLists = initWatchLists(f);

    sat = initialCheck(f, assignment, watchLists, level, numAssigned);
    if(sat < 0){ //initial check yielded conflict
        return vector<int>();
    }

    return vector<int>(5,0);
}

map<int, unordered_set<unsigned int>> initWatchLists(vector<Clause>& f){
    map<int, unordered_set<unsigned int>> watchLists;
    //Add clause number to set of clause numbers for each of its watched literals
    //Index of clause in f is its clause number
    for(unsigned int i = 0; i < f.size(); ++i){
        if(f[i].size() > 1){
            const vector<int>& lits = f[i].getLits();
            watchLists[lits[f[i].watched1]].insert(i);
            watchLists[lits[f[i].watched2]].insert(i);
        }
    }
    return watchLists;
}

//Determines values for literals in all clauses of size 1, and propagates. Returns -1 if conflict found, 0 otherwise
int initialCheck(vector<Clause>& f, vector<VarAssignment>& a, map<int, unordered_set<unsigned int>> watchLists, int level, unsigned int& numAssigned){
    queue<int> q;
    int step = 0;
    for(unsigned int i = 0; i < f.size(); ++i){
        if(f[i].size() == 1){
            int lit = f[i].getLits()[0];
            int var = abs(lit);
            int truthVal = lit > 0 ? true : false;
            bool prevAssigned = a[var].level >= 0 ? true : false;
            if(prevAssigned && (truthVal != a[var].truthVal)){
                return -1; //conflict
            } else {
                q.push(lit);
                setAssignment(a, var, truthVal, level, step, 0, numAssigned);
            }
        }
    }

    return bcp(f, a, q, watchLists, level, step, numAssigned);
}

//Boolean constant propagation
int bcp(vector<Clause>& f, vector<VarAssignment>& a, queue<int> q, map<int, unordered_set<unsigned int>> watchLists,
        int& level, int& step, unsigned int& numAssigned){
    while(!q.empty()){
        int propagatedLit = q.front();
        q.pop();

        auto it = watchLists[-propagatedLit].begin();
        while(it != watchLists[-propagatedLit].end()){
            unsigned int clauseNum = *it;
            const vector<int>& lits = f[clauseNum].getLits();
            bool unit = true;
            unsigned int replacementIdx = 0;

            //try to find a replacement watched literal for clause
            for(unsigned int i = 0; i < lits.size(); ++i){
                if(f[clauseNum].watched1 == i || f[clauseNum].watched2 == i){
                    continue;
                }

                int var = abs(lits[i]);
                int unsatVal = lits[i] > 0 ? false : true; //value needed for variable in order to make literal false
                if((a[var].level >= 0) && (a[var].truthVal == unsatVal)){
                    continue;
                } else { //found a replacement
                    replacementIdx = i;
                    unit = false;
                    break;
                }
            }

            if(unit){ //try to set the other watched literal to true
                //get the other watched literal
                int secondWatchedLit = (lits[f[clauseNum].watched1] == -propagatedLit) ? lits[f[clauseNum].watched2] : lits[f[clauseNum].watched1];
                int var = abs(secondWatchedLit);
                int newVal = secondWatchedLit > 0 ? true : false;
                step++;
                if(a[var].level >= 0){ //value was previously assigned
                    if(!(a[var].truthVal == newVal)){
                        return -1; //previously assigned value not equal to new truth value
                    }
                } else {
                    setAssignment(a, var, newVal, level, step, clauseNum, numAssigned);
                    q.push(secondWatchedLit);
                }
                ++it;
            } else { //replace watched literal
                it = watchLists[-propagatedLit].erase(it);
                watchLists[lits[replacementIdx]].insert(clauseNum);
                //choose which watchedLit in Clause to replace
                (lits[f[clauseNum].watched1] == -propagatedLit) ? f[clauseNum].watched1 = replacementIdx : f[clauseNum].watched2 = replacementIdx;
            }
        }
    }
    return 0;
}

void setAssignment(vector<VarAssignment>& a, int var, int truthVal, int level, int step, int antecedent, unsigned int& numAssigned){
    a[var].setAssignment(truthVal, level, step, antecedent);
    numAssigned++;
}


