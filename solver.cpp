#include "solver.h"
#include <algorithm>
#include <cstdlib>
#include <exception>
#include <queue>
#include <set>

namespace solver {

unsigned int numLitsAtLvl(const vector<int>& lits, int level, const vector<VarAssignment>& a);
void setAssignment(vector<VarAssignment>& a, int var, bool truthVal, int level, int& step, 
                   unsigned int antecedent, unsigned int& numAssigned);
void unsetAssignment(vector<VarAssignment>& a, int var, unsigned int& numAssigned);

// Level <0 indicates that variable has not been assigned
VarAssignment::VarAssignment() : truthVal(false), level(-1), step(0), antecedent(0){}

VarAssignment::~VarAssignment(){}

inline void VarAssignment::setAssignment(bool tVal, int lvl, int stp, unsigned int ant){
    this->truthVal = tVal;
    this->level = lvl;
    this->step = stp;
    this->antecedent = ant;
}

inline void VarAssignment:: unsetAssignment(){
    this->level = -1;
}

inline ostream& operator<<(ostream& os, const VarAssignment& v){
    os << "Val: " << v.truthVal << " Lvl: " << v.level << " Step: " << v.step << " Ant: " << v.antecedent << " ";
    return os;
}

// For Clauses with <2 literals, the Clause is not added to Watched Literals, so value of watched1 or watched2
// is irrelevant
Clause::Clause(vector<int> lits) : watched1(0), watched2(1), lits(lits){}

Clause::~Clause(){}

inline ostream& operator<<(ostream& os, const Clause& c){
    for(unsigned int i = 0; i < c.lits.size(); ++i){
        os << c.lits[i] << " ";
    }
    return os;
}

inline const vector<int>& Clause::getLits(){
    return this->lits;
}

inline unsigned int Clause::size(){
    return this->lits.size();
}

Decider::Decider(vector<Clause>& f) : counter(0) {}

Decider::~Decider(){}

Vsids::Vsids(vector<Clause>& f): Decider(f) {
    for(Clause& c : f){
        const vector<int>& ls = c.getLits();
        for(int lit : ls){
            vsidsScores[lit]++;
        }
    }
    for(auto it = vsidsScores.begin(); it != vsidsScores.end(); ++it){
        // Map from (ordered) scores to literals. Makes finding max score more efficient
        vsidsMap.insert(make_pair(it->second, it->first));
    }
}

Vsids::~Vsids(){}

inline void Vsids::stepCounter(){
    this->counter++;
}

// Should only update vsidsMap if already present
void Vsids::update(Clause& newClause){
    const vector<int>& lits = newClause.getLits();
    for(int lit : lits){
        float oldScore = this->vsidsScores[lit];
        float newScore = oldScore + 1;
        this->vsidsScores[lit] = newScore;

        // Update vsidsMap with new score for lit
        multimap<float, int>::iterator it = this->vsidsMap.find(oldScore);
        while(it != this->vsidsMap.end()){
            if((it->first == oldScore) && (it->second == lit)){
                it = this->vsidsMap.erase(it);
                this->vsidsMap.insert(make_pair(newScore, lit));
                break;
            } else if (it->first == oldScore){
                ++it;
                continue;
            } else { // Gone past 'oldScore' key
                break;
            }
        }
    }
}

int Vsids::decide(const vector<VarAssignment>& a){
    while(!vsidsMap.empty()){
       multimap<float, int>::iterator it = --vsidsMap.end(); // Get largest score
       int lit = it->second;
       int var = abs(lit);
       vsidsMap.erase(it);
       if(a[var].level >= 0){ // Variable has already been assigned
           continue;
       } else {
           return lit;
       }
    }
    return 0; // Should not occur, since Vsids::decide() is only called when assignment is partial
}

// Not just score (key), but also value needs to be examined
void Vsids::addToContention(int var){
    int score = this->vsidsScores[var];
    pair<multimap<float, int>::iterator, multimap<float, int>::iterator> ret = this->vsidsMap.equal_range(score);
    bool found = false;
    for(auto it = ret.first; it != ret.second; ++it){
        if(it->second == var){
            found = true; // No need to re-add score to vsidsMap
            break;
        }
    }
    if(!found){
        this->vsidsMap.insert(make_pair(score, var));
    }

    // Do the same for negation
    score = this->vsidsScores[-var];
    ret = this->vsidsMap.equal_range(score);
    found = false;
    for(auto it = ret.first; it != ret.second; ++it){
        if(it->second == -var){
            found = true; // No need to re-add score to vsidsMap
            break;
        }
    }
    if(!found){
        this->vsidsMap.insert(make_pair(score, -var));
    }
}

pair<int, vector<int>> CDCL(vector<Clause>& f, const unsigned int numVars){
    vector<VarAssignment> assignment(numVars + 1);
    unsigned int numAssigned = 0; // Number of variables that solver has assigned
    int level = 0;
    Vsids vsids(f); // Decision heuristic

    // Map from variable to set of clause numbers in which it is watched. Only for clauses with >= 2 literals
    map<int, unordered_set<unsigned int>> watchLists = initWatchLists(f);

    if(initialCheck(f, assignment, watchLists, level, numAssigned) < 0){ // See if initial check yields conflict
        return make_pair(0, vector<int>());
    }

    while(numAssigned < numVars){
        ++level;
        int step = 0;
        int guessedLit = vsids.decide(assignment);
        bool truthVal = guessedLit > 0 ? true : false;
        setAssignment(assignment, abs(guessedLit), truthVal, level, step, 0, numAssigned);

        queue<int> q(deque<int>{guessedLit});
        tuple<int, unsigned int, int> conflict; // (isConflict, clause number, conflicting variable) tuple

        while(get<0>(conflict = bcp(f, assignment, q, watchLists, level, step, numAssigned)) < 0){
            vsids.stepCounter();
            pair<int, Clause> newClause = analyzeConflict(f, assignment, get<1>(conflict));
            if(newClause.first < 0){
                return make_pair(-1, vector<int>());
            }

            // By construction, new clause is unit, so we will have to flip its value compared to last guess.
            // If there is another conflict, we will have to move up another level
            q = queue<int>();
            int maxStep = backtrack(assignment, vsids, newClause.first, numAssigned);
            step = maxStep + 1;
            level = newClause.first;
            int newClauseNum = f.size();

            const vector<int>& lits = newClause.second.getLits();
            bool watch1Set = false;
            bool watch2Set = false;
            // Set watch literals for new clause
            for(unsigned int i = 0; i < lits.size(); ++i){
                unsigned int var = abs(lits[i]);
                if(assignment[var].level >= 0){
                    bool isUnsat = lits[i] > 0 ? (assignment[var].truthVal == false) : (assignment[var].truthVal == true);
                    if(!isUnsat && !watch1Set){
                        watch1Set = true;
                        newClause.second.watched1 = i;
                    } else if(!isUnsat && !watch2Set){
                        watch2Set = true;
                        newClause.second.watched2 = i;
                    }
                } else {
                    if(!watch1Set){
                        watch1Set = true;
                        newClause.second.watched1 = i;
                    } else if(!watch2Set){
                        watch2Set = true;
                        newClause.second.watched2 = i;
                    }
                }
            }
            // Case where watched1 is updated to watched2, but watched2 is unchanged
            if(newClause.second.watched1 == newClause.second.watched2){
                newClause.second.watched2 = 0;
            }

            if(watch1Set && !watch2Set){ // Mathematically, new clause should be unit
                bool newVal = lits[newClause.second.watched1] > 0 ? true : false;
                setAssignment(assignment, abs(lits[newClause.second.watched1]), newVal, level, step, newClauseNum, numAssigned);
                q.push(lits[newClause.second.watched1]);               
            } else {
                throw "new clause should be unit";
            }

            f.push_back(newClause.second);
            addToWatchLists(watchLists, newClause.second, f.size()-1);
            vsids.update(newClause.second);
        }
    }

    // Since all variables are assigned, formula is satisfiable
    vector<int> satAssignment(assignment.size()-1, 0);
    for(unsigned int i = 1; i < assignment.size(); ++i){
        assignment[i].truthVal == true ? satAssignment[i-1] = i : satAssignment[i-1] = - static_cast<int>(i);
    }
    return make_pair(1, satAssignment);
}

map<int, unordered_set<unsigned int>> initWatchLists(vector<Clause>& f){
    map<int, unordered_set<unsigned int>> watchLists;
    // Add clause number to set of clause numbers for each of its watched literals
    // Index of clause in f is its clause number
    for(unsigned int i = 0; i < f.size(); ++i){
        if(f[i].size() > 1){
            const vector<int>& lits = f[i].getLits();
            watchLists[lits[f[i].watched1]].insert(i);
            watchLists[lits[f[i].watched2]].insert(i);
        }
    }
    return watchLists;
}

// Determines values for literals in all clauses of size 1, and propagates. Returns -1 if conflict found, 0 otherwise
int initialCheck(vector<Clause>& f, vector<VarAssignment>& a, map<int, unordered_set<unsigned int>>& watchLists, 
                 int level, unsigned int& numAssigned){
    queue<int> q;
    int step = 0;
    for(unsigned int i = 0; i < f.size(); ++i){
        if(f[i].size() == 1){
            int lit = f[i].getLits()[0];
            int var = abs(lit);
            int truthVal = lit > 0 ? true : false;
            bool prevAssigned = a[var].level >= 0 ? true : false;
            if(prevAssigned && (truthVal != a[var].truthVal)){
                return -1; // Conflict
            } else {
                q.push(lit);
                setAssignment(a, var, truthVal, level, step, 0, numAssigned);
            }
        }
    }
    return get<0>(bcp(f, a, q, watchLists, level, step, numAssigned));
}

// Boolean constant propagation
tuple<int, unsigned int, int> bcp(vector<Clause>& f, vector<VarAssignment>& a, queue<int> q, 
    map<int, unordered_set<unsigned int>>& watchLists, int& level, int& step, unsigned int& numAssigned){
    while(!q.empty()){
        int propagatedLit = q.front();
        q.pop();

        auto it = watchLists[-propagatedLit].begin();
        while(it != watchLists[-propagatedLit].end()){
            unsigned int clauseNum = *it;
            const vector<int>& lits = f[clauseNum].getLits();
            bool unit = true;
            unsigned int replacementIdx = 0;

            // Try to find a replacement watched literal for clause
            for(unsigned int i = 0; i < lits.size(); ++i){
                if(f[clauseNum].watched1 == i || f[clauseNum].watched2 == i){
                    continue;
                }

                int var = abs(lits[i]);
                int unsatVal = lits[i] > 0 ? false : true; // Value needed for variable in order to make literal false
                if((a[var].level >= 0) && (a[var].truthVal == unsatVal)){
                    continue;
                } else { // Found a replacement
                    replacementIdx = i;
                    unit = false;
                    break;
                }
            }

            if(unit){ // Try to set the other watched literal to true
                // Get the other watched literal
                int secondWatchedLit = (lits[f[clauseNum].watched1] == -propagatedLit) ? lits[f[clauseNum].watched2] 
                    : lits[f[clauseNum].watched1];
                int var = abs(secondWatchedLit);
                bool newVal = secondWatchedLit > 0 ? true : false;
                // If value was previously assigned and not equal to new truth value
                if(a[var].level >= 0 && a[var].truthVal != newVal){
                    return make_tuple(-1, clauseNum, var);
                } else if(a[var].level < 0){
                    setAssignment(a, var, newVal, level, step, clauseNum, numAssigned);
                    q.push(secondWatchedLit);
                }
                ++it;
            } else { // Replace watched literal
                it = watchLists[-propagatedLit].erase(it);
                watchLists[lits[replacementIdx]].insert(clauseNum);
                // Choose which watchedLit in Clause to replace
                (lits[f[clauseNum].watched1] == -propagatedLit) ? f[clauseNum].watched1 = replacementIdx 
                    : f[clauseNum].watched2 = replacementIdx;
            }
        }
    }
    return make_tuple(0,0,0);
}

inline void setAssignment(vector<VarAssignment>& a, int var, bool truthVal, int level, int& step, 
                   unsigned int antecedent, unsigned int& numAssigned){
    a[var].setAssignment(truthVal, level, step, antecedent);
    step++;
    numAssigned++;
}

inline void unsetAssignment(vector<VarAssignment>& a, int var, unsigned int& numAssigned){
    a[var].unsetAssignment();
    --numAssigned;
}

pair<int, Clause> analyzeConflict(vector<Clause>& f, vector<VarAssignment>& a, unsigned int clauseNum){
    // Get max level in conflicting clause
    const vector<int>& lits = f[clauseNum].getLits();
    auto maxIt = max_element(lits.begin(), lits.end(), [a](const int& litA, const int& litB){
            return a[abs(litA)].level < a[abs(litB)].level;});
    int clauseLvl = a[abs(*maxIt)].level;
    if(clauseLvl <= 0){
        return make_pair(-1, Clause(vector<int>()));
    }

    vector<int> newLits = lits;
    while(numLitsAtLvl(newLits, clauseLvl, a) > 1){
        // Get last assigned variable at specified level
        unsigned int maxStep = 0;
        int lastAssignedVar = 0;
        for(unsigned int i = 0; i < newLits.size(); ++i){
            if((a[abs(newLits[i])].level == clauseLvl) && (a[abs(newLits[i])].step >= maxStep)){
                lastAssignedVar = abs(newLits[i]);
                maxStep = a[abs(newLits[i])].step;
            }
        }
        unsigned int antecedent = a[lastAssignedVar].antecedent;
        resolve(newLits, f[antecedent].getLits(), lastAssignedVar);
    }

    // Find second largest level in clause to backtrack to. That way, the sole lit in the highest level 
    // will be made unit
    int largest = 0;
    int secondLargest = 0;
    for(unsigned int i = 0; i < newLits.size(); ++i){
        int levelI = a[abs(newLits[i])].level;
        if(levelI > largest){
            secondLargest = largest;
            largest = levelI;
        } else if(levelI > secondLargest){
            secondLargest = levelI;
        }
    }
    int newLevel = secondLargest;
    Clause newClause(newLits);
    return make_pair(newLevel, newClause);
}

unsigned int numLitsAtLvl(const vector<int>& lits, int level, const vector<VarAssignment>& a){
    unsigned int count = 0;
    for(int lit : lits){
        if(a[abs(lit)].level == level){
            count++;
        }
    }
    return count;
}

void resolve(vector<int>& lits, const vector<int>& lits2, int conflictVar){
    set<int> s(lits.begin(), lits.end());
    set<int> s2(lits2.begin(), lits2.end());
    s.erase(conflictVar);
    s.erase(-conflictVar);
    s2.erase(conflictVar);
    s2.erase(-conflictVar);
    lits.clear();
    set_union(s.begin(), s.end(), s2.begin(), s2.end(), back_inserter(lits));
}

inline void addToWatchLists(map<int, unordered_set<unsigned int>>& watchLists, Clause& c, const unsigned int clauseNum){
    const vector<int>& lits = c.getLits();
    watchLists[lits[c.watched1]].insert(clauseNum);
    watchLists[lits[c.watched2]].insert(clauseNum);
}

unsigned int backtrack(vector<VarAssignment>& a, Vsids& vsids, const int newLevel, unsigned int& numAssigned){
    unsigned int maxStep = 0;
    for(unsigned int i = 1; i < a.size(); ++i){
        if(a[i].level > newLevel){
            unsetAssignment(a, i, numAssigned);
            vsids.addToContention(i);
        } else if(a[i].level == newLevel && a[i].step > maxStep){
            maxStep = a[i].step;
        }
    }
    return maxStep;
}

}
