//
//  main.cpp
//  inference
//
//  Created by Young Kim on 11/6/17.
//  Copyright © 2017 Young Kim. All rights reserved.
//

#include <iostream>
#include <map>
#include <set>
#include <list>

#include "input.h"
// #include "predicate.h"
// #include "node.h"

using namespace std;

string vectorStr(vector<string> *s) {
    string temp = "[";
    for(int i = 0; i < s->size(); ++i) {
        if(i == (s->size() - 1)) {
            temp += (*s)[i];
        } else {
            temp += (*s)[i] + ", ";
        }
    }
    temp += "]";
    return temp;
};

void printMap(map<string, vector<string> > &m) {
    for(map<string, vector<string> >::iterator it = m.begin(); it != m.end(); ++it) {
        cout << it->first << " => " << vectorStr(&(it->second)) << endl;
    }
}

void buildKB(Input &input, map<string, vector<string> > &kb) {
    // building KB map
    for(int i = 0 ; i < input.s.size(); ++i) {
        string st = input.s[i];
        vector<string> pred = split(st, '|');
        
        // convert ~A | B to A => B
        // (ex. ~A | ~B | C to A ^ B => C)
        for(int i = 0; i < (pred.size() - 1); ++i) {
            if(pred[i].find('~') != string::npos) {
                pred[i].replace(0, 1, ""); // remove negation
            } else {
                pred[i].insert(0, "~"); // add negation
            }
        }
        
        string con = pred.back(); // conclusion as key
        if(pred.size() != 1) {
            pred.pop_back();
            
            string pre = ""; // construct premise
            for(int i = 0; i < pred.size(); ++i) {
                if(i != (pred.size() - 1)) {
                    pre += pred[i] + "^";
                } else {
                    pre += pred[i];
                }
            }
            
            kb[con].push_back(pre);
        } else {
            kb[con]; // conclusion only
        }
    }
    
};

void extractKB(map<string, vector<string> > &imap, map<string, vector<string> > &cmap, map<string, vector<string> > &kb) {
    for(map<string, vector<string> >::iterator it = kb.begin(); it != kb.end(); ++it) {
        if((it->second).size() != 0) {
            imap[parseFullName(it->first)].push_back(it->first);
        } else {
            cmap[parseFullName(it->first)].push_back(it->first);
        }
    }
};


bool isKey(string &s, map<string, vector<string> > &m) {
    string key = parseFullName(s);
    return (m.find(key) != m.end());
};

void buildIndexMap(vector<string> &kb, map<string, vector<string> > &indexMap) {
    for(int i = 0; i < kb.size(); ++i) {
        string sent = kb[i];
        vector<string> pred = split(sent, '|');
        
        for(int j = 0; j < pred.size(); ++j) {
            indexMap[parseFullName(pred[j])].push_back(sent);
        }
    }
};

void buildPredMap(vector<string> &kb, map<string, vector<string> > &predMap) {
    for(int i = 0; i < kb.size(); ++i) {
        string sent = kb[i];
        vector<string> pred = split(sent, '|');
        
        for(int j = 0; j < pred.size(); ++j) {
            string pname = parseFullName(pred[j]);
            
            if(predMap.find(parseFullName(pred[j])) != predMap.end()) {
                for(int k = 0; k != predMap[pname].size(); ++k) {
                    if(pred[j] != predMap[pname][k]) {
                        predMap[pname].push_back(pred[j]);
                    }
                }
            } else {
                predMap[pname].push_back(pred[j]);
            }
        }
    }
};

// find res pred p (single literal) in sentence s
string findResPred(const string &p, const string &s) {
    vector<string> v = split(s, '|');
    for(int i = 0; i < v.size(); ++i) {
        if(parseFullName(negatePred(p)) == parseFullName(v[i])) {
            return v[i];
        }
    }
    return ""; // should not reach here!
};


vector<string> getRes(const string &p, map<string, vector<string> > &pred) {
    return pred[parseFullName(negatePred(p))];
};

vector<string> getResChildren(const string &s, map<string, vector<string> > &pred) {
    vector<string> resChildren;
    vector<string> pv = split(s, '|'); // predicate vector of curr node
    for(int i = 0; i < pv.size(); ++i) {
        vector<string> cv = getRes(pv[i], pred); // res vector of child
        for(int j = 0; j < cv.size(); ++j) {
            resChildren.push_back(cv[j]);
        }
    }
    return resChildren;
};

// return vec of child to perform resolution for a single literal p
vector<string> getChild(string &p, map<string, vector<string> > &index) {
    return index[parseFullName(negatePred(p))];
};

// return vec of child of parent node
vector<string> getChildren(string &s, map<string, vector<string> > &index) {
    vector<string> children;
    vector<string> pv = split(s, '|'); // predicate vector of curr node
    for(int i = 0; i < pv.size(); ++i) {
        vector<string> cv = getChild(pv[i], index); // predicate vector of child
        for(int j = 0; j < cv.size(); ++j) {
            
            if(isInVector(cv[j], children)) {
                //
            } else {
                children.push_back(cv[j]);
            }
        }
    }
    return children;
};

bool findInVector(const string &s, vector<string> &v) {
    for(int i = 0; i < v.size(); ++i) {
        if(v[i] == s) {
            return true;
        }
    }
    return false;
};


// resolution map where key = parent node's pred, value = child node's pred
void buildResMap(string &s, map<string, vector<string> > &pred, map<string, vector<string> > &res) {
    vector<string> pv = split(s, '|'); // predicate vector of curr node
    for(int i = 0; i < pv.size(); ++i) {
        res[pv[i]] = getRes(pv[i], pred); // predicate vector of child
    }
};

// resolution map where key = parent node's pred, value = child node's pred
void buildChildMap(string &s, map<string, vector<string> > &index, map<string, vector<string> > &child) {
    vector<string> pv = split(s, '|'); // predicate vector of curr node
    
    for(int i = 0; i < pv.size(); ++i) {
        
        vector<string> cv = getChild(pv[i], index); // predicate vector of child
        
        for(int j = 0; j < cv.size(); ++j) {
            if(!(isInVector(cv[j], child[pv[i]]))) {
                child[pv[i]].push_back(cv[j]);
            }
        }
    }
};


void getSubst(string &s, map<string, vector<string> > &indexMap, vector<string> &subParent, vector<string> &subChild) {
    
    map<string, vector<string> > childMap;
    (void) buildChildMap(s, indexMap, childMap);
    
    vector<string> parentPredicate = split(s, '|'); // vector of pred of parent sentence s
    
    for(int i = 0; i < parentPredicate.size(); ++i) {
        
        vector<string> cv = childMap[parentPredicate[i]];
        for(int j = 0; j < cv.size(); ++j) {
            //cout << "resolve: " << parentPredicate[i] << " w/ " << cv[j] << endl;
            
            
            // standarized var here? btw curr clause s and child clause cv[i]
            string strdChild = standarizeVariable(s, cv[j]);
            // cout << "s = " << s << endl;
            // cout << "\t w/std = " << strdChild << endl;
            if(strdChild != cv[j]) {
                cv[j] = strdChild;
            }
            
            // vec of predicates to resolve between
            vector<string> resolve = resolvePredicate(parentPredicate[i], cv[j]);
            
            if(resolve.size() == 0) {
                // cout << "child is invalid!" << endl;
            } else {
                for(int k = 0; k < resolve.size(); ++k) {
                    map<string, string> psubMap;
                    map<string, string> csubMap;
                    if(unify(parentPredicate[i], resolve[k], psubMap, csubMap) == 0) {
                        
                        // cout << "resolved!" << endl;
                        // cout << "\t child = " << substClause(cv[j], subMap) << endl;
                        // cout << "\t parent = " << substClause(s, subMap) << endl;
                        
                        subParent.push_back(substClause(s, psubMap));
                        subChild.push_back(substClause(cv[j], csubMap));
                    }
                }
            }
        }
    }
};



string listToString(list<string> & l) {
    string ret = "[";
    for(list<string>::iterator it = l.begin(); it != l.end(); ++it) {
        ret += *it + ", ";
    }
    ret += "]";
    return ret;
};

bool isInList(const string & s, list<string> &l) {
    for(list<string>::iterator it = l.begin(); it != l.end(); ++it) {
        if((*it) == s) {
            return true;
        }
    }
    return false;
};




bool isAllArgsConstant(const string &predicate) {
    vector<string> arg = parseArg(predicate);
    for(int i = 0; i < arg.size(); ++i) {
        if(isVariable(arg[i])) {
            return false;
        }
    }
    return true;
};


void buildConclusionMap(map<string, vector<string> > &conclMap, map<string, vector<string> > &indexMap) {
    for(map<string, vector<string> >::iterator it = indexMap.begin(); it != indexMap.end(); ++it) {
        vector<string> v = it->second;
        for(int i = 0; i < v.size(); ++i) {
            if(isAllArgsConstant(v[i])) {
                conclMap[it->first].push_back(v[i]);
            }
        }
    }
};

bool occursCheck(const string &resolvent, map<string, vector<string> > &conclMap) {
    vector<string> rPred = split(resolvent, '|');
    
    for(int i = 0; i < rPred.size(); ++i) {
        
        vector<string> concl = conclMap[parseName(rPred[i])];
        //cout << vectorStr(&concl) << endl;
        
        if(concl.size() != 0) {
            if(isAllArgsConstant(rPred[i])) {
                // all arg is constant; check if entailed by kb!
                
                //cout << rPred[i] << endl;
                if(!(isInVector(rPred[i], concl) || isInVector(negatePred(rPred[i]), concl))) {
                    // cout << "not in concl!" << endl;
                    return false;
                }
            } else {
                // cout << "@occursCheck else clause!" << endl;
                vector<string> parg = parseArg(rPred[i]);
                for(int j = 0; j < parg.size(); ++j) {
                    
                    
                    if(isConstant(parg[j])) {
                        // cout << "\t" << parg[j] << endl;
                        
                        for(int k = 0; k < concl.size(); ++k) {
                            vector<string> carg = parseArg(concl[k]);
                            
                            // cout << "comapre " << parg[j] << ", " << carg[j] << endl;
                            if(parg[j] == carg[j]) {
                                return true;
                            }
                        }
                    }
                }
            }
        }
        
    }
    
    return true;
};


string toOutput(vector<string> &out) {
    string output = "";
    for(int i = 0; i < out.size(); ++i) {
        output += out[i] + "\n";
    }
    return output;
};


int main(int argc, const char * argv[]) {
    Input input;
    readFile("input.txt", input);
    // input.printInput();
    
    //cout << "building index map ..." << endl;
    
    //map<string, vector<string> > predMap; // key: A value: [A(x), A(y)]
    map<string, vector<string> > indexMap; // key: A value: [~B(x,y) | ~C(x,y) | A(x)]
    map<string, vector<string> > conclMap;
    
    //(void) buildPredMap(input.s, predMap);
    (void) buildIndexMap(input.s, indexMap);
    (void) buildConclusionMap(conclMap, indexMap);
    
    //printMap(conclMap);
    //cout << endl;
    //printMap(indexMap);
    //cout << endl;
    
    
    /*cout << "testing testing" << endl;
    string x = "~Parent(Liz,y) | ~Parent(y,yy) | ~Ancestor(yy,Billy)";
    string y;
    
    if(factoring(x, y) != -1) {
        cout << y << endl;
    } else {
        cout << x << " = factoring failed!" << endl;
    }
    
    
    cout << "testing occursCheck!" << endl;
    string z = "~D(x,Alice)";
    if(occursCheck(z, conclMap) == false) {
        cout << z << " does not occur, dont add!!" << endl;
    } else {
        cout << z << " can occur, cont!" << endl;
    }*/


    
    vector<string> output;
    for(int id = 0; id < input.q.size(); ++id) {
        
    

    // cout << ">> query = " << input.q[id] << endl;

    string root = negatePred(input.q[id]);
    // cout << ">> root = " << root << endl;
    
    
    list<string> newClauses;
    newClauses.push_front(root);
    
    list<string> checkClauses;
    int trueFlag = -1;
    int stopFlag = -1;
    while(stopFlag != 0) {
        
        if(newClauses.size() == 0) {
            stopFlag = 0;
            break;
        }
        
        string currClause = newClauses.front();
        newClauses.pop_front();
        checkClauses.push_back(currClause);
        
        // cout << "popped! = " << currClause << endl;
        // cout << "size of newClause = " << newClauses.size() << endl;
        
        vector<string> substParent; // subsitution for parent s
        vector<string> substChild; // subsitution for child of parent
        (void) getSubst(currClause, indexMap, substParent, substChild); // perform unification here
        
        if(substChild.size() == 0) {
            // currClause is not entailed by KB; no child to resolve with!
            // cout << "this node of " << currClause << " is leaf! return false!" << endl;
        } else {
            
            vector<string> resolvents;
            
            // resolve currClause with each of unified child clause
            for(int i = 0; i < substChild.size(); ++i) {
                //cout << "@Resolution of " << endl;
                //cout << "\t" << substParent[i] << " and " << substChild[i] << endl;
                
                
                // factoring of resolvent here!!!
                
                string resolvent = resolution(substParent[i], substChild[i]);
                // cout << "resolvent before factor = " << resolvent << endl;
                
                string factoredResolvent;
                int factoredFlag = factoring(resolvent, factoredResolvent);
                if(factoredFlag == 1) {
                    resolvent = factoredResolvent;
                }
                
                // cout << "resolvent after factor = " << resolvent << endl;
                
                
                if(resolvent == "") {
                    // resolved = "" -> contradiction; return true!
                    //cout << "contradiction = true! return true!" << endl;
                    
                    
                    trueFlag = 0;
                    stopFlag = 0;
                    break;
                } else if (resolvent == root) {
                    // resolved = root clause -> it's a loop!
                    //cout << "loop detected = return false!" << endl;
                    checkClauses.push_back(resolvent);
                } else if (isInList(resolvent, checkClauses)) {
                    //cout << "already checked! = do not add to loop!" << endl;
                }else {
                    if(factoredFlag != -1) {
                        if(occursCheck(resolvent, conclMap)) {
                            newClauses.push_back(resolvent);
                            // cout << "push resolvent = " << resolvent << endl;
                        } else {
                            //cout << "not occurs! = dont not add!" << endl;
                            //cout << resolvent << endl;
                            checkClauses.push_back(resolvent);
                        }
                    } else {
                        //cout << "factoring unifcation error false!" << endl;
                        checkClauses.push_back(resolvent);
                    }
                }
            }
            
            
        }
        
        // cout << "curr clause = " << listToString(newClauses) << endl;
        // cout << "clause history = " << listToString(checkClauses) << endl;
    }
        

    
    if(trueFlag == 0) {
        //cout << "true! entailed by KB!" << endl;
        output.push_back("TRUE");
    } else {
        //cout << "false! not entailed by KB!" << endl;
        output.push_back("FALSE");
    }
    
    }
    
    //cout << "to output!" << endl;
    cout << vectorStr(&output) << endl;
    
    string out = toOutput(output);
    
    writeFile("output.txt", out);
    
    
    

    return 0;
}
