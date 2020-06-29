<<<<<<< HEAD
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
=======

class Tree {
	Tree* a;
	Tree* b;

	Window* win;

    bool isLeaf() {
        return win;
    }
>>>>>>> 5c6ae9108f63bdb7c6c9b798c3b403ab57ae6de6
};

Display* d;
Picture mFrontbuffer;
Picture mBackbuffer;
Window focus;
<<<<<<< HEAD
Tree tree;

void print(std::string toPrint) {
    std::cout << "DEBUG:_" << toPrint << std::endl;
}

void print(int toPrint) {
    std::cout << "DEBUG:_" << toPrint << std::endl;
}
=======
Tree tree;
>>>>>>> 5c6ae9108f63bdb7c6c9b798c3b403ab57ae6de6
