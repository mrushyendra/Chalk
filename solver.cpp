#include "solver.h"

Clause::Clause(vector<int>& lits){
}

Clause::~Clause(){
}

VarAssignment::VarAssignment(){
}

VarAssignment::~VarAssignment(){
}

Decider::Decider(vector<Clause>& f){
}

Decider::~Decider(){
}

Vsids::Vsids(vector<Clause>& f): Decider(f) {
}

Vsids::~Vsids(){
}

void Vsids::stepCounter(){
}

void Vsids::updateDecider(Clause& newClause){
}

int CDCL(vector<Clause>& f){
    return 0;
}
