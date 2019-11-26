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

    int sat = 0;
    vector<int> res = CDCL(f, numVars, sat);
    if(sat == 1){
        cout << "sat" << " ";
        for(int val : res){
            cout << val << " ";
        }
        cout << endl;
    } else {
        cout << "unsat" << endl;
    }
    return sat;
}
