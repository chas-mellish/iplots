//
//  CocoaApp.m
//  Acinonyx
//
//  Created by Simon Urbanek
//  Copyright 2008 Simon Urbanek. All rights reserved.
//

#import "CocoaApp.h"
#import "CocoaWindow.h"

#import "ATimeSeriesPlot.h"
#import "AScatterPlot.h"
#import "AParallelCoordPlot.h"
#import "ABarChart.h"
#import "AHistogram.h"
#import "AMarkerValuesPlot.h"

#import "REngine.h"
#import "ARVector.h"

#import "ALinearProjection.h"

extern "C" {
	CocoaWindow *ACocoa_CreateWindow(AVisual *visual, APoint position);
	void ACocoa_Init();
}

CocoaWindow *ACocoa_CreateWindow(AVisual *visual, APoint position)
{
	ARect aFrame = visual->frame();
	NSRect rect = NSMakeRect(position.x, position.y, aFrame.width, aFrame.height);
	
	CocoaWindow *window = [[CocoaWindow alloc] initWithContentRect:rect visual:visual];
	
	[window setTitle:[NSString stringWithUTF8String:visual->caption()]];
    [window makeKeyAndOrderFront:nil];
	// [window setDelegate:self];
	// no idea why, but the shadow is not there until you cycle the shadow setting...
	//[window setHasShadow:NO];
	//[window setHasShadow:YES];
	//[window setReleasedWhenClosed:YES];
	
	return window;
}

static NSAutoreleasePool *staticPool;

void ACocoa_Init() {
	NSApplicationLoad();
	staticPool = [[NSAutoreleasePool alloc] init];
}

@implementation CocoaApp

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
	ALog("applicationDidFinishLaunching:");
	REngine *eng = REngine::mainEngine();
	RObject *o = eng->parseAndEval("as.double({n<-1e2;  x<-rep(seq(1:(n/5)),5)})");

	AMarker *mark = new AMarker(o->length());
	mark->setColorMap(new ADefaultColorMap());
	mark->enableUndo(16);

	ADataVector *vx = new ARDoubleVector(mark, o);
	vx->setName("x");
	o->release();
	o = eng->parseAndEval("y<-rnorm(n)*3+1");
	ADataVector *vy = new ARDoubleVector(mark, o);
	vy->setName("y");
	o->release();
	
	o = eng->parseAndEval("factor(LETTERS[as.integer(y - min(y))+1L])");
	ARFactorVector *fv = new ARFactorVector(mark, o->value());
	fv->setName("letters");
	o->release();
	
	ARect aFrame = AMkRect(0, 0, 400, 300);
	
	AVisual *visual = new ATimeSeriesPlot(NULL, aFrame, 0, vx, vy, fv);
	ACocoa_CreateWindow(visual, AMkPoint(50, 100));
	visual->release();

	visual = new AScatterPlot(NULL, aFrame, 0, vx, vy);
	ACocoa_CreateWindow(visual, AMkPoint(500, 400));
	// FIXME: we should assign the result or something ...
	visual->release();
	
	ADataVector *pcv[] = { vx, vy, fv, vx, vx, vx, vy, vy };
	
	visual = new AParallelCoordPlot(NULL, aFrame, 0, 8, pcv);
	ACocoa_CreateWindow(visual, AMkPoint(500, 100));
	visual->release();
	
	visual = new ABarChart(NULL, aFrame, 0, fv);
	ACocoa_CreateWindow(visual, AMkPoint(950, 100));
	visual->release();
	
	visual = new AMarkerValuesPlot(NULL, aFrame, 0, mark);
	ACocoa_CreateWindow(visual, AMkPoint(450, 100));
	visual->release();

	//	visual = new AHistogram(NULL, aFrame, 0, vx);
//	ACocoa_CreateWindow(visual, AMkPoint(50, 600));
//	visual->release();
//	
	fv->release();
	vx->release();
	vy->release();
	mark->release();
}

@end
