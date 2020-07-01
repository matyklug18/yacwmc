#include <vector>
#include <string>
#include <iostream>

void print(std::string toPrint) {
    std::cout << "DEBUG:_" << toPrint << std::endl;
}

void print(int toPrint) {
    std::cout << "DEBUG:_" << toPrint << std::endl;
}


class Tree {
    public:
        std::vector<Tree> children;

        Window win;

        bool split = false;

        bool isLeaf() {
            return win != 0;
        }

        bool isEmpty() {
            return !isLeaf() && children.size() == 0;
        }

        void remChild(Window w) {
            for(int i = 0; i < children.size(); i++) {
                if(children.at(i).win == w) {
                    children.erase(children.begin() + i);
                }
            }
        }

        void remChild(Tree* trp) {
            for(int i = 0; i < children.size(); i++) {
                if(&children.at(i) == trp) {
                    children.erase(children.begin() + i);
                }
            }
        }

        Tree() {}

        Tree(Window w) {
            win = w;
        }

        Tree(Window w, bool s) {
            win = w;
            split = s;
        }
};

Display* d;
Picture mFrontbuffer;
Picture mBackbuffer;
Window focus;
Tree tree;