
#include <memory>
#include <exception>
#include <future>

#include <X11/extensions/Xcomposite.h>
#include <X11/Xutil.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xrender.h>
#include <X11/extensions/Xdamage.h>

#include "main.hpp"

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define FORMAT XRenderFindVisualFormat (d, DefaultVisual (d, DefaultScreen(d)))
#define DEPTH DefaultDepth (d, DefaultScreen(d))
#define WIDTH DisplayWidth( d, DefaultScreen(d) )
#define HEIGHT DisplayHeight( d, DefaultScreen(d) )
#define DIMS WIDTH, HEIGHT, DEPTH

Tree* recFindTree(Tree* tr, Window w) {
	if(tr->isLeaf() && tr->win == w) {
		return tr;
	} else if (!tr->isEmpty()) {
		for(int i = 0; i < tr->children.size(); i++) {
			Tree* ch = &tr->children.at(i);
			Tree* ret = recFindTree(ch, w);
			if(ret) {
				return ret;
			}
		}
	}
	return nullptr;
}

Tree* findTree(Window w) {
	Tree* ret = recFindTree(&tree, w);
	return ret;
}

void recurseMap(Tree tr, int x, int y, int w, int h) {
	if(tr.isLeaf()) {
		XMoveResizeWindow(d, tr.win, x, y, w, h);
	} else {
		for(int i = 0; i < tr.children.size(); i++) {
			Tree ch = tr.children.at(i);
			if(tr.split == false) {
				recurseMap(ch, i * (w / tr.children.size()), y, w / tr.children.size(), h);
			}
			else if(tr.split == true) {
				recurseMap(ch, x, i * (h / tr.children.size()), w, h / tr.children.size());
			}
		}
	}
}

void mapAll() {
	recurseMap(tree, 0, 0, WIDTH, HEIGHT);
}

Tree* currTree;

void makeTree(Window win) {
	currTree->children.push_back(Tree(win));
}

void treefy(Window win) {
	makeTree(win);
	mapAll();
}

void damageAll() {
	Window root, parent, *children;
	uint children_count;
	XQueryTree(d, DefaultRootWindow(d), &root, &parent, &children, &children_count);

	for (uint i = 1; i < children_count; i++) {
		XDamageCreate( d, children[i], XDamageReportNonEmpty );
	}
}

void damage(Window win) {
	XDamageCreate( d, win, XDamageReportNonEmpty );
}

void paint() {

	Window root, parent, *children;
	uint children_count;
	XQueryTree(d, DefaultRootWindow(d), &root, &parent, &children, &children_count);

	XRenderColor color;
	color.alpha = 50000;
	color.red = 0;
	color.green = 20000;
	color.blue = 40000;

	XRenderFillRectangle(d, PictOpSrc, mBackbuffer, &color,
		0, 0, WIDTH, HEIGHT);

	XRenderColor color2;
	color2.alpha = 50000;
	color2.red = 0;
	color2.green = 25000;
	color2.blue = 50000;

	XRenderFillRectangle(d, PictOpSrc, mBackbuffer, &color2, WIDTH / 2 - 50, HEIGHT / 2 - 50, 50, 50);

	for (uint i = 1; i < children_count; i++) {
		
		XWindowAttributes theAttr;
		XGetWindowAttributes(d, children[i], &theAttr);

		XRenderPictFormat* mFormat = XRenderFindVisualFormat( d, theAttr.visual );

		Pixmap pictMap = XCompositeNameWindowPixmap( d, children[i] );

		Picture picture = XRenderCreatePicture(d, pictMap, mFormat, 0, 0);

		XRenderColor borderC;
		borderC.alpha = 0;
		borderC.red = 0;
		borderC.green = 5000;
		borderC.blue = 5000;

		XRenderComposite (d, PictOpOver,
			picture, None, mBackbuffer,
			0, 0, 0, 0,
			theAttr.x, theAttr.y, theAttr.width, theAttr.height);

		if(children[i] == focus) {
			XRenderFillRectangle(d, PictOpOver, mBackbuffer, &borderC, 
				theAttr.x, theAttr.y-5, theAttr.width, 5);
		}
	}

	XRenderComposite (d, PictOpSrc,
		mBackbuffer, None, mFrontbuffer,
		0, 0, 0, 0, 
		0, 0, WIDTH, HEIGHT);

}

int main () {
	currTree = &tree;

	d = XOpenDisplay(nullptr);

	XSetErrorHandler([](Display* display, XErrorEvent* e) -> int {/* std::cout << (int)e->error_code << std::endl; /**/ return 0;});

	int version = XCompositeVersion();
	int major = version / 1000;
	int minor = (version - (major * 1000)) / 100;
	int revision = version - (major * 1000) - (minor * 100);

	std::cout << "Composite Version: " << major << "." << minor << "." << revision << std::endl;

	XCompositeQueryVersion(d, &major, &minor);

    XGrabKey(d, XKeysymToKeycode(d, XStringToKeysym("Return")), Mod1Mask|Mod2Mask,
            DefaultRootWindow(d), True, GrabModeAsync, GrabModeAsync);

	XGrabKey(d, XKeysymToKeycode(d, XStringToKeysym("H")), Mod1Mask|Mod2Mask,
        	DefaultRootWindow(d), True, GrabModeAsync, GrabModeAsync);

	XGrabKey(d, XKeysymToKeycode(d, XStringToKeysym("V")), Mod1Mask|Mod2Mask,
        	DefaultRootWindow(d), True, GrabModeAsync, GrabModeAsync);

	XGrabButton(d, 1, Mod1Mask|Mod2Mask, DefaultRootWindow(d), True,
					ButtonPressMask|ButtonReleaseMask|PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);
	XGrabButton(d, 3, Mod1Mask|Mod2Mask, DefaultRootWindow(d), True,
					ButtonPressMask|ButtonReleaseMask|PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);

	int damage_event, damage_error;
	XDamageQueryExtension( d, &damage_event, &damage_error );

	XGrabServer( d );

	XCompositeRedirectSubwindows( d, DefaultRootWindow(d), CompositeRedirectManual );

	XSelectInput( d, DefaultRootWindow(d),
			SubstructureNotifyMask | ExposureMask |
			StructureNotifyMask | PropertyChangeMask |
			SubstructureRedirectMask
	);

	XUngrabServer( d );

	XRenderPictFormat* mFormat;

	mFormat = XRenderFindVisualFormat( d, DefaultVisual( d, DefaultScreen(d) ) );

	XRenderPictureAttributes pa;
	pa.subwindow_mode = IncludeInferiors;
	mFrontbuffer = XRenderCreatePicture( d, DefaultRootWindow(d), mFormat, CPSubwindowMode, &pa );

	Pixmap pixmap = XCreatePixmap( d, DefaultRootWindow(d), DIMS);
	mBackbuffer = XRenderCreatePicture( d, pixmap, mFormat, 0, 0 );
	XFreePixmap( d, pixmap );

	XSync( d, false );

	XButtonEvent start;
	XWindowAttributes attr;

	Window w = XCreateSimpleWindow (d, DefaultRootWindow(d), 0, 0, 1, 1, 0, None,
			     None);

    Xutf8SetWMProperties (d, w, "xcompmgr", "xcompmgr", NULL, 0, NULL, NULL,
			  NULL);

	Atom a = XInternAtom (d, "_NET_WM_CM_S0", False);

    XSetSelectionOwner (d, a, w, 0);

	damageAll();
	paint();

	while(true)
	{
		XEvent e;
		XNextEvent(d, &e);

		if(e.type == KeyPress) {
			XKeyPressedEvent ev = e.xkey;
			if(ev.keycode == XKeysymToKeycode(d, XStringToKeysym("Return")))
				std::async(std::launch::async, [] () {system("bash -c \"kitty &\"");});
			else if(ev.keycode == XKeysymToKeycode(d, XStringToKeysym("H")) && focus != 0) {
				findTree(focus)->split = true;
			} else if(ev.keycode == XKeysymToKeycode(d, XStringToKeysym("V")) && focus != 0) {
				findTree(focus)->split = false;
			}
		}
		else if(e.type == ButtonPress && e.xbutton.subwindow != None)
		{
			XGetWindowAttributes(d, e.xbutton.subwindow, &attr);
			start = e.xbutton;
			XRaiseWindow(d, e.xbutton.subwindow);
			XSetInputFocus(d, e.xbutton.subwindow, 0, 0);
			focus = e.xbutton.subwindow;
			
			Tree* focused = findTree(focus);
			currTree = focused;
			focused->children.push_back(Tree(focused->win));
			focused->win = 0;
		}
		else if(e.type == MotionNotify && start.subwindow != None)
		{
			int xdiff = e.xbutton.x_root - start.x_root;
			int ydiff = e.xbutton.y_root - start.y_root;
			XMoveResizeWindow(d, start.subwindow,
					attr.x + (start.button==1 ? xdiff : 0),
					attr.y + (start.button==1 ? ydiff : 0),
					MAX(1, attr.width + (start.button==3 ? xdiff : 0)),
					MAX(1, attr.height + (start.button==3 ? ydiff : 0)));
		}
		else if(e.type == CreateNotify) {
			XCreateWindowEvent ev = e.xcreatewindow;
			damage(ev.window);
		}
		else if(e.type == MapRequest) {
			XMapRequestEvent ev = e.xmaprequest;
			XMapWindow(d, ev.window);
			treefy(ev.window);
		}
		else if(e.type == ButtonRelease)
			start.subwindow = None;
		else if ( e.type == damage_event + XDamageNotify ) {
			paint();
			XDamageNotifyEvent *ev = reinterpret_cast<XDamageNotifyEvent*>( &e );
        	XDamageSubtract( d, ev->damage, None, None );
    	}
	}
}
