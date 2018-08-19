//
//  parse.h
//  inference
//
//  Created by Young Kim on 11/7/17.
//  Copyright © 2017 Young Kim. All rights reserved.
//

#ifndef parse_h
#define parse_h

#include <vector>
#include <set>
#include <string>
#include <sstream> // stringstream


using namespace std;

// === Helper function definition ===

void replaceAll(string &s, const string &from, const string &to) {
    while(s.find(from) != string::npos) {
        size_t pos = s.find(from);
        s.replace(pos, from.length(), to);
    }
};

string trim(const string &s, const string &remove) {
    size_t strBegin = s.find_first_not_of(remove);
    if(strBegin == string::npos) {
        return "";
    }
    
    size_t strEnd = s.find_last_not_of(remove);
    size_t strRange = strEnd - strBegin + 1;
    
    return s.substr(strBegin, strRange);
};

vector<string> split(const string &s, char delim) {
    stringstream ss(s);
    string item;
    
    vector<string> tokens;
    while (getline(ss, item, delim)) {
        // remove whitespaces
        replaceAll(item, " ", "");
        // item = trim(item, " ");
        tokens.push_back(item);
    }
    return tokens;
};

bool isLowerCase(const char c) {
    string lower = "abcdefghijklmnopqrstuvwxyz";
    return (lower.find(c) != string::npos);
};

bool isUpperCase(const char c) {
    return !isLowerCase(c);
};

bool isInVector(const string &s, vector<string> &v) {
    for(int i = 0; i < v.size(); ++i) {
        //cout << "@isInVector" << endl;
        //cout << "compare " << v[i] << " == " << s << endl;
        if(v[i] == s) {
            //cout << "\t is true!" << endl;
            return true;
        }
    }
    return false;
};

int indexAt(const string &s, vector<string> &v) {
    for(int i = 0; i < v.size(); ++i) {
        if(v[i] == s) {
            return i;
        }
    }
    return -1;
};

// === Predicate parse function definition ===

// return true if arg is constant
bool isConstant(const string &s) {
    return !isLowerCase(s[0]);
};

// return true if arg is variable
bool isVariable(const string &s) {
    return isLowerCase(s[0]);
};

// return true iff '~' is present
bool parseNot(const string &s) {
    return (s.find('~') != string::npos);
};

// return str of full predicate name (w neg if present)
string parseFullName(const string &s) {
    return s.substr(0, s.find("("));
};

// return str of predicate name (w/o neg)
string parseName(const string &s) {
    size_t pos1 = s.find("(");
    
    if(parseNot(s)) {
        return s.substr(1, (pos1 - 1));
    } else {
        return s.substr(0, pos1);
    }
};

// return vector str of args in predicate str
vector<string> parseArg(const string &s) {
    size_t pos1 = s.find("(");
    size_t pos2 = s.find(")");
    
    string temp = s.substr((pos1 + 1), (pos2 - pos1 - 1));
    return split(temp, ',');
};

// return negated pred str
string negatePred(const string &s) {
    if(parseNot(s)) {
        return s.substr(s.find("~") + 1);
    } else {
        return "~" + s;
    }
};

// return true iff ~A(Constant), A(Constant)
bool isContradiction(const string &s1, const string &s2) {
    return (negatePred(s1) == s2);
};

// return resolution of str s1 and str s2
string resolution(string s1, string s2) {
    vector<string> pv1 = split(s1, '|');
    vector<string> pv2 = split(s2, '|');
    
    set<int> iv1; // index for pv1
    set<int> iv2; // index for pv2
    
    string res = s1 + " " + s2;
    // cout << res << endl;
    for(int i = 0; i < pv1.size(); ++i) {
        for(int j = 0; j < pv2.size(); ++j) {
            if(isContradiction(pv1[i], pv2[j])) {
                replaceAll(s1, pv1[i], "");
                replaceAll(s2, pv2[j], "");
                
                iv1.insert(i);
                iv2.insert(j);
                
                // cout << "i = " << i << " j = " << j << endl;
            }
        }
    }
    res = trim(res, "| ");
    // cout << res << endl;
    
    string resolved = "";
    for(int i = 0; i < pv1.size(); ++i) {
        if(iv1.find(i) == iv1.end()) {
            resolved += pv1[i] + " | ";
        }
    }
    for(int j = 0; j < pv2.size(); ++j) {
        if(iv2.find(j) == iv2.end()) {
            resolved += pv2[j] + " | ";
        }
    }
    resolved = trim(resolved, "| ");
    
    
    return resolved;
    // return res;
};

// perform unify of string s1 and s2 where s1 and s2 are single literal predicate
int unify(const string &s1, const string &s2, map<string, string> &psub, map<string, string> &csub) {
    
    vector<string> input1 = parseArg(s1); // parent
    vector<string> input2 = parseArg(s2); // child
    
    // pred: same predicate name, same num of args!
    if(input1.size() != input2.size()) {
        return -1;
    }
    
    for(int i = 0; i < input1.size(); ++i) {
        // case 1: if s1.x is var and s2.C is const, assign s1.x = s2.C
        if(isVariable(input1[i]) && isConstant(input2[i])) {
            // sub[input1[i]] = input2[i];
            psub[input1[i]] = input2[i];
        // case 2: if s1.C is const and s2.x is var, assign s2.x = s1.C
        } else if(isConstant(input1[i]) && isVariable(input2[i])) {
            // sub[input2[i]] = input1[i];
            csub[input2[i]] = input1[i];
        // case 3: if s1.x is var and s2.y is var, assign s2.y = s1.x
        } else if(isVariable(input1[i]) && isVariable(input2[i])) {
            // sub[input2[i]] = input1[i];
            csub[input2[i]] = input1[i];
        // case 4: if s1.C is const and s2.C is const, check s1.C == s2.C
        } else if(isConstant(input1[i]) && isConstant(input2[i])) {
            if(input1[i] != input2[i]) {
                return -1;
            }
        } else {
            // invalid child!
            return -1;
        }
    }
    
    return 0;
};

string subsitute(const string &s, map<string, string> &sub) {
    string unified = parseFullName(s);
    unified += "(";
    
    vector<string> input = parseArg(s);
    for(int i = 0; i < input.size(); ++i) {
        
        // if key in subst map, replace
        if(sub.find(input[i]) != sub.end()) {
            unified += sub[input[i]];
        } else {
            unified += input[i];
        }
        
        if(i != (input.size() - 1)) {
            unified += ",";
        }
    }
    unified += ")";
    
    return unified;
};

string joinPred(vector<string> &v) {
    string clause = "";
    for(int i = 0; i < v.size(); ++i) {
        clause += v[i];
        if(i != (v.size() - 1)) {
            clause += " | ";
        }
    }
    return clause;
};

string substClause(const string &s, map<string, string> &sub) {
    vector<string> clause;
    vector<string> v = split(s, '|');
    for(int i = 0; i < v.size(); ++i) {
        string subst = subsitute(v[i], sub);
        
        if(!isInVector(subst, clause)) {
            clause.push_back(subst);
        }
        
    }
    return joinPred(clause);
};



// return predicate str with arg a replaced by constant c
string replaceArg(const string &p, const string &a, const string &c) {
    vector<string> v = parseArg(p);
    
    if(!isInVector(a, v)) {
        // invalid arg a
        return "";
    } else {
        v[indexAt(a,v)] = c; // replace arg 'a' with const 'c'
        
        string newArgStr = "";
        for(int i = 0; i < v.size(); ++i) {
            if(i == (v.size() - 1)) {
                newArgStr += v[i];
            } else {
                newArgStr += v[i] + ",";
            }
        }
        
        return parseFullName(p) + "(" + newArgStr + ")";
    }
};

// p is parent single predicate, c is child clause which may have more than 1 predicate
// find predicate in child clause to resolve
vector<string> resolvePredicate(const string &p, const string &c) {
    
    string parent = parseFullName(p);
    
    // cout << "@resolvePredicate" << endl;
    
    vector<string> resolve;
    vector<string> childPredicate = split(c, '|');
    for(int i = 0; i < childPredicate.size(); ++i) {
        string child = parseFullName(childPredicate[i]);
        
        // cout << "\t trying to resolve " << p << " w/ " << childPredicate[i] << endl;
        
        if(isContradiction(parent, child)) {
            resolve.push_back(childPredicate[i]);
        }
    }
    
    return resolve;
};

// curr will resolve with child clause; standarize the variables before unification
string standarizeVariable(const string & curr, const string & child) {
    

    
    vector<string> currPred = split(curr, '|');
    vector<string> childPred = split(child, '|');
    
   
    
    
    set<string> currVarSet; // contain all var in curr clause
    set<string> childVarSet; // contain all var in child clause
    
    for(int i = 0; i < currPred.size(); ++i) {
        vector<string> currArg = parseArg(currPred[i]);
        for(int j = 0; j < currArg.size(); ++j) {
            if(isVariable(currArg[j])) {
                currVarSet.insert(currArg[j]);
            }
        }
    }
    
    for(int i = 0; i < childPred.size(); ++i) {
        vector<string> childArg = parseArg(childPred[i]);
        for(int j = 0; j < childArg.size(); ++j) {
            if(isVariable(childArg[j])) {
                childVarSet.insert(childArg[j]);
            }
        }
    }
    
    vector<string> standarizeVar;
    for(set<string>::iterator it = childVarSet.begin(); it != childVarSet.end(); ++it) {
        // var in child clause is in var of curr clause
        // need to standarize the var!
        if(currVarSet.find(*it) != currVarSet.end()) {
            standarizeVar.push_back((*it));
        }
    }
    
    map<string, string> standarizeVarMap;
    for(int i = 0; i < standarizeVar.size(); ++i) {
        
        int counter = 1;
        string newVar = standarizeVar[i] + to_string(counter);
        while(currVarSet.find(newVar) != currVarSet.end()) {
            counter++;
            newVar = standarizeVar[i] + to_string(counter);
        }
        standarizeVarMap[standarizeVar[i]] = newVar;
    }
    
    if(standarizeVarMap.size() != 0) {
        return substClause(child, standarizeVarMap);
    } else {
        return child; // no change, bc no need to standarize!
    }
    
};


int getFactorMap(const string &s1, const string &s2, map<string, string> & m) {
    
    vector<string> arg1 = parseArg(s1);
    vector<string> arg2 = parseArg(s2);
    
    if(arg1.size() == arg2.size()) {
        for(int i = 0;  i < arg1.size(); ++i) {
            
            if(isConstant(arg1[i]) && isConstant(arg2[i])) {
                if(arg1[i] == arg2[i]) {
                    m[arg1[i]] = arg2[i];
                } else {
                    return -1;
                }
            } else if(isConstant(arg1[i]) && isVariable(arg2[i])) {
                if(m.find(arg2[i]) == m.end()) {
                    m[arg2[i]] = arg1[i];
                } else {
                    if(m[arg2[i]] != arg1[i]) {
                        return -1;
                    }
                }
            } else if(isVariable(arg1[i]) && isConstant(arg2[i])) {
                if(m.find(arg1[i]) == m.end()) {
                    m[arg1[i]] = arg2[i];
                } else {
                    if(m[m[arg1[i]]] != arg2[i]) {
                        return -1;
                    }
                }
            } else if(isVariable(arg1[i]) && isVariable(arg2[i])) {
                
                if(m.find(arg1[i]) == m.end()) {
                    m[arg1[i]] = arg2[i];
                } else {
                    m[arg2[i]] = m[arg1[i]];
                }
                
            }
        }
    }
    
    for(map<string, string>::iterator it = m.begin(); it != m.end(); ++it) {
        if(m.find(it->second) != m.end()) {
            if(isVariable(it->second)) {
                m[it->first] = m[it->second];
            }
        }
    }
    
    return 0;
};

// factoring resolved clause to remove like predicates through unification
int factoring(const string &s, string & ret) {
    // cout << "@factoring!" << endl;
    vector<string> resolvedPredicate = split(s, '|');
    
    // map<string, vector<string>> factor;
    
    vector<string> toFactor;
    
    for(int i = 0; i < resolvedPredicate.size(); ++i) {
        for(int j = i + 1; j < resolvedPredicate.size(); ++j) {
            string p1 = resolvedPredicate[i];
            string p2 = resolvedPredicate[j];
            
            // cout << "comparing " << p1 << " == " << p2 << endl;
            if(parseFullName(p1) == parseFullName(p2)) {
                // need to factor by unifying into single literal
                //cout << "need to factor!" << endl;
                //cout << p1 << endl;
                //cout << p2 << endl;
                
                // factor[parseFullName(p1)].push_back(p1);
                // factor[parseFullName(p1)].push_back(p2);
                
                toFactor.push_back(p1);
                toFactor.push_back(p2);
                break;
            }
        }
    }
    
    if(toFactor.size() == 0) {
        // no matching predicates to factor
        return 0;
    }
    
    
    
    string temp = "";
    
    if(toFactor.size() == 2) {
        map<string, string> factorSubMap;
        if(getFactorMap(toFactor[0], toFactor[1], factorSubMap) == 0) {
            temp += substClause(s, factorSubMap);
        } else {
            // cout << "factoring failed! return false!" << endl;
            return -1;
        }
    } else {
        return -1;
    }
    
    /* string retFactored = "";
    for(map<string, vector<string>>::iterator it = factor.begin(); it != factor.end(); ++it) {

        vector<string> toFactor = it->second;
        
        string temp = "";
        
        if(toFactor.size() == 2) {
            map<string, string> factorSubMap;
            if(getFactorMap(toFactor[0], toFactor[1], factorSubMap) == 0) {
                temp += substClause(s, factorSubMap);
            } else {
                // cout << "factoring failed! return false!" << endl;
                return -1;
            }
        }
        
        
        
        retFactored += temp;
    }
  
    s = retFactored;
     */
    
    
    ret = temp;
    
    return 1;
};







#endif /* parse_h */
