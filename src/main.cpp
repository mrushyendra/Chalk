#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "solver.h"

using namespace std;

int main(int argc, char** argv){
    if(argc < 2){
        cerr << "Usage: ./solver filename" << endl;
        return -1;
    }

    // Open file containing boolean formula in simplified version of DIMACS format
    // (http://www.satcompetition.org/2009/format-benchmarks2009.html)
    ifstream inFile;
    inFile.open(argv[1]);
    if(!inFile){
        cerr << "Unable to open specified file: " << argv[1] << endl;
        return -1;
    }

    // Skip comments
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

    // Read in clauses and preprocess
    vector<solver::Clause> f;
    for(unsigned int i = 0; i < numClauses; ++i){
        unordered_set<int> seen;
        int lit;
        bool isSat = false; // Unknown
        while(true){
            inFile >> lit;
            if(lit == 0){
                break;
            } else {
                if(seen.find(-lit) != seen.end()){ // Both variable and its negation present in same clause - automatically SAT
                    isSat = true;
                } else {
                    seen.insert(lit);
                }
            }
        }
        if(!isSat){
            vector<int> lits(seen.begin(), seen.end());
            f.emplace_back(lits);
        }
    }
    inFile.close();

    pair<int, vector<int>> res = solver::CDCL(f, numVars);
    if(res.first == 1){
        cout << "sat";
        for(int val : res.second){
            cout << " " << val;
        }
    } else {
        cout << "unsat" << endl;
    }
    return res.first;
}
