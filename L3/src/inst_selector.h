#pragma once

#include "L3.h"
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <map>
#include <vector> 
#include <typeinfo>
#include <cmath>


namespace L3{
    void instructionSelection(Program* p, int Verbose);
    void generateContexts(Function *f);
    void generateTrees(Context *cont);
    void condenseContexts(Context *cont);
    void FindMergeCandidates(Context *cont);
    void printTree(Node* root);
    void mergeTree(Node* merge_this, Node* root);
    void appendtoTree(Node* treeNode,  Node* toAppend);
    bool removeConsecutiveMoves(Node* leaf);

    void condenseTree(Node* root);
    void cleanContext(Context *cont);
    void getLeafNodes(Node* root, std::vector<Node *> &leaf_vec, std::vector<std::string> &leaf_vec_str);
    void maximalMunch(Node* root, std::vector<std::string> &insts);
    void handleCallTrees(Tree* tree);
    std::string handleCalls(Instruction* Inst, bool isRetVar);
    std::vector<std::string> genMovOP(Node* dest, Node* source, Operator op);
    std::vector<std::string> genArithOP(Node* dest, Node* source1, Node* source2, Operator op);
    std::vector<std::string> genLeaOP(Node* dest, Node* source1, Node* source2, Node* source3);
    std::vector<std::string> genLoadArithOP(Node* dest, Node* source1, Node* source2, Operator op);
    bool checkLeaChildren(Node* child1, Node* child2);
    void matchArithOP(Node* root, std::vector<std::string> &insts);
    bool matchMovOP(Node* root, std::vector<std::string> &insts);
    bool matchLeaOP(Node* root, std::vector<std::string> &insts);
    bool matchLoadArithOP(Node* root, std::vector<std::string> &insts);
}
