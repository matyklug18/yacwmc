
class Tree {
	Tree* a;
	Tree* b;

	Window* win;

    bool isLeaf() {
        return win;
    }
};

Display* d;
Picture mFrontbuffer;
Picture mBackbuffer;
Window focus;
Tree tree;