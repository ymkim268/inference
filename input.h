//
//  input.h
//  inference
//
//  Created by Young Kim on 11/6/17.
//  Copyright © 2017 Young Kim. All rights reserved.
//

#ifndef input_h
#define input_h

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <stdlib.h> // atoi

#include "parse.h"

using namespace std;

/* Input Strcut Definitions */
struct Input {
    int nq; // num of queries
    int ns; // num of given statements in knowledge base
    
    vector<string> q; // array of str queries
    vector<string> s; // array of str statements
    
    void printInput();
    
};

void Input::printInput() {
    cout << "Printing Input ..." << endl;
    
    cout << "\tNumber of Queries = " << nq << endl;
    cout << "\tNumber of Statements = " << ns << endl;
    
    cout << "\tList of queries: " << endl;;
    for(int i = 0; i < q.size(); ++i) {
        cout << "\t" << q[i] << endl;
    }
    
    cout << "\tList of statements: " << endl;;
    for(int i = 0; i < s.size(); ++i) {
        cout << "\t" << s[i] << endl;
    }
};

/* Helper Definitions */
void readFile(std::string fileName, Input &input) {
    // open input file to read
    ifstream infile;
    infile.open(fileName.c_str(), fstream::in);
    
    // parse input file
    if(infile.is_open()) {
        string str_nq, str_ns;
        
        getline(infile, str_nq);
        input.nq = atoi(str_nq.c_str());
        
        // build q vector
        for(int i = 0; i < input.nq; ++i) {
            string line;
            getline(infile, line);
            
            // unify all OR, NOT to '|', '~'
            replaceAll(line, "OR", "|");
            replaceAll(line, "NOT ", "~");
            
            input.q.push_back(line);
        }
        
        getline(infile, str_ns);
        input.ns = atoi(str_ns.c_str());
        
        // build s vector
        for(int i = 0; i < input.ns; ++i) {
            string line;
            getline(infile, line);
            
            // unify all OR, NOT to '|', '~'
            replaceAll(line, "OR", "|");
            replaceAll(line, "NOT ", "~");
            
            input.s.push_back(line);
        }
    } else {
        cout << "ERROR: Failed to open file!" << endl;
    }
    infile.close();
};

void writeFile(std::string fileName, std::string &output) {
    
    // open output file to write
    ofstream outfile(fileName.c_str());
    if(outfile.is_open()) {
        outfile << output;
    } else {
        cout << "ERROR: Failed to write file!" << endl;
    }
    outfile.close();
    
};


#endif /* input_h */
