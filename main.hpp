#include <vector>
#include <string>
#include <iostream>

class Tree {
    public:
        std::vector<Tree> children;

        Window win;

        bool split;

        bool isLeaf() {
            return win != 0;
        }

        bool isEmpty() {
            return !isLeaf() && children.size() == 0;
        }

        Tree() {

        }

        Tree(Window w) {
            win = w;
        }
};

Display* d;
Picture mFrontbuffer;
Picture mBackbuffer;
Window focus;
Tree tree;

void print(std::string toPrint) {
    std::cout << "DEBUG:_" << toPrint << std::endl;
}

void print(int toPrint) {
    std::cout << "DEBUG:_" << toPrint << std::endl;
}
