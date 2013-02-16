#pragma once

#include "ofMain.h"
#include "ofxLeapMotion.h"
#include "ofxStrip.h"
#include "ofxMouseControl.h"
#include "../../../libs/glut/lib/osx/GLUT.framework/Versions/A/Headers/glut.h"

class testApp : public ofBaseApp{
    
public:
    void setup();
    void update();
    void reset();
    
    void draw();
    void drawPlane();
    void drawBackground();
    void drawFingerStrips();
    void drawGestureData();
    
    void enable3D();
    void disable3D();
    
    void beginGestureDetection();
    void endGestureDetection();
    
    enum swipe { SWIPE_NULL, SWIPE_UP, SWIPE_DOWN, SWIPE_LEFT, SWIPE_RIGHT };
    enum direction { UP, DOWN };

    int performSwipeAnalysis();
    ofVec2f getGestureVector();
    
    void simulateKey(int key);
    
    void simulateClick(direction dir);
    void simulateScroll(direction dir);

    void positionCursor();
    
    CGPoint mousePosition;

	ofxLeapMotion leap;
    
	vector <ofxLeapMotionSimpleHand> simpleHands;
    vector <ofPoint> callibrationPoints;
    
	vector <int> fingersFound;
	ofEasyCam cam;
	ofLight l1;
	ofLight l2;
	ofMaterial m1;
    
    ofPoint cursorPos;
    
    bool detectingGestures;
    int gestureSamples;
    deque<ofPoint> gesturePositions;
    
    float gestureVelocity;
    ofVec2f gestureVector;
    float gestureSlope;
    
    int lastSwipeTime;
    int debounceTime;
    int scrollEvents;
    int sleepMillis;
	
	map <int, ofPolyline> fingerTrails;
};
