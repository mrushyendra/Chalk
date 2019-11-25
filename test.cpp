#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "solver.h"

void initialCheckTest(vector<Clause>& f, const unsigned int numVars);
void initWatchListsTest(vector<Clause>& f);

int main(int argc, char** argv){
    if(argc < 2){
        cerr << "Usage: ./solver filename" << endl;
        return -1;
    }

    //Open file containing boolean formula in simplified version of DIMACS format (http://www.satcompetition.org/2009/format-benchmarks2009.html)
    ifstream inFile;
    inFile.open(argv[1]);
    if(!inFile){
        cerr << "Unable to open specified file: " << argv[1] << endl;
        return -1;
    }
    
    //skip comments
    string s;
    while(getline(inFile, s, '\n')){
        if(s.size() <= 0){
            cerr << "Incorrect file format" << endl;
            return -2;
        } else if(s[0] == 'c'){
            continue;
        } else {
            break;
        }
    }

    unsigned int numVars;
    unsigned int numClauses;
    istringstream sstream(s);
    string t;
    sstream >> t >> t >> numVars >> numClauses;

    //read in clauses
    vector<Clause> f;
    for(unsigned int i = 0; i < numClauses; ++i){
        vector<int> lits;
        int lit;
        while(true){
            inFile >> lit;
            if(lit == 0){
                f.emplace_back(lits);
                break;
            } else {
                lits.push_back(lit);
            }
        }
    }
    inFile.close();

    initWatchListsTest(f);
    initialCheckTest(f, numVars);
    return 0;
}

void initWatchListsTest(vector<Clause>& f){
    map<int, unordered_set<unsigned int>> watchLists = initWatchLists(f);
    cout << "Watchlist Test 1: " << (watchLists[-1] == unordered_set<unsigned int>{0, 6, 10, 14, 19, 22}) << endl;
}


void initialCheckTest(vector<Clause>& f, const unsigned int numVars){
    vector<VarAssignment> assignment(numVars + 1);
    unsigned int numAssigned = 0; //number of variables that solver has assigned
    unsigned int level = 0;
    Vsids vsids(f); //decision heuristic

    //map from variable to set of clauses in which it is watched. Only for clauses with >= 2 literals
    map<int, unordered_set<unsigned int>> watchLists = initWatchLists(f);

    int res = initialCheck(f, assignment, watchLists, level, numAssigned);
    cout << "res: " << res << endl;

    for(int i = 0; i < assignment.size(); ++i){
        cout << "Is set: " << (assignment[i].level >= 0 ? "True" : "False") << " Var " << i << " value: " << assignment[i].truthVal << endl;
    }
}
