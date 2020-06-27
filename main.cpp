
#include <iostream>
#include <memory>
#include <exception>

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
	color.alpha = 1;
	color.red = 1;
	color.green = 1;
	color.blue = 1;

	XRenderFillRectangle(d, PictOpSrc, mBackbuffer, &color, 0, 0, WIDTH, HEIGHT);

	for (uint i = 1; i < children_count; i++) {
		
		XWindowAttributes theAttr;
		XGetWindowAttributes(d, children[i], &theAttr);

		XRenderPictFormat* mFormat = XRenderFindVisualFormat( d, theAttr.visual );

		Pixmap pictMap = XCompositeNameWindowPixmap( d, children[i] );

		Picture picture = XRenderCreatePicture(d, pictMap, mFormat, 0, 0);
		
		
		XRenderComposite (d, PictOpSrc,
			picture, None, mBackbuffer,
			0, 0, 0, 0, theAttr.x, theAttr.y, theAttr.width, theAttr.height);			
	}

	XRenderComposite (d, PictOpSrc,
		mBackbuffer, None, mFrontbuffer,
		0, 0, 0, 0, 0, 0, WIDTH, HEIGHT);

}

int main () {
	d = XOpenDisplay(nullptr);

	XSetErrorHandler([](Display* display, XErrorEvent* e) -> int { std::cout << (int)e->error_code << std::endl; return 0; });

	int version = XCompositeVersion();
	int major = version / 1000;
	int minor = (version - (major * 1000)) / 100;
	int revision = version - (major * 1000) - (minor * 100);

	std::cout << "Composite Version: " << major << "." << minor << "." << revision << std::endl;

	XCompositeQueryVersion(d, &major, &minor);

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
			StructureNotifyMask | PropertyChangeMask );

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

	damageAll();
	paint();

	while(true)
	{
		XEvent e;
		XNextEvent(d, &e);

		if(e.type == ButtonPress && e.xbutton.subwindow != None)
		{
			XGetWindowAttributes(d, e.xbutton.subwindow, &attr);
			start = e.xbutton;
			XRaiseWindow(d, e.xbutton.subwindow);
			XSetInputFocus(d, e.xbutton.subwindow, 0, 0);
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
		else if(e.type == ButtonRelease)
			start.subwindow = None;
		else if ( e.type == damage_event + XDamageNotify ) {
			paint();
			XDamageNotifyEvent *ev = reinterpret_cast<XDamageNotifyEvent*>( &e );
        	XDamageSubtract( d, ev->damage, None, None );
    	}
	}
}
