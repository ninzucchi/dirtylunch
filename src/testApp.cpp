#include "testApp.h"
// #define DEBUG

//--------------------------------------------------------------
void testApp::setup(){
    
    ofSetFrameRate(60);
    ofSetVerticalSync(true);
	ofSetLogLevel(OF_LOG_VERBOSE);
    
    gestureSamples = 15;
    detectingGestures = false;
    
    lastSwipeTime = 0;
    debounceTime = 800;
    scrollEvents = 10;
    sleepMillis = 15;
    
	leap.open();
    
	l1.setPosition(200, 300, 50);
	l2.setPosition(-200, -200, 50);
    
	cam.setOrientation(ofPoint(-20, 0, 0));
    
    cursorPos.set(0,0);
    
    beginGestureDetection();
}

//--------------------------------------------------------------
void testApp::update(){
	fingersFound.clear();
    vector <Hand> hands = leap.getLeapHands();
    
    if( leap.isFrameNew()) {
        if( hands.size()) {
            int nFingers = hands[0].fingers().count();
            
            // setMappingX(float minX, float maxX, float outputMinX, float outputMaxX)
            leap.setMappingX(-230, 230, -ofGetWidth()/2, ofGetWidth()/2); //0, glutGet(GLUT_SCREEN_WIDTH));
            leap.setMappingY(90, 490, -ofGetWidth()/2, ofGetWidth()/2); //-glutGet(GLUT_SCREEN_HEIGHT), 0);
            leap.setMappingZ(-150, 150, -200, 200);

            for(int i = 0; i < hands.size(); i++){
                for(int j = 0; j < hands[i].fingers().count(); j++){

                    const Finger & finger = hands[i].fingers()[j];
                    
                    // Convert Leap point to ofPoint
                    ofPoint pt = leap.getMappedofPoint( finger.tipPosition() );
                    ofPoint pt_raw = leap.getofPoint( finger.tipPosition() );
                    
                    // lets get the correct trail (ofPolyline) out of our map - using the finger id as the key
                    ofPolyline & polyline = fingerTrails[finger.id()];

                    // clear line if we've traveled too far
                    if( polyline.size() && (pt-polyline[polyline.size()-1] ).length() > 50 ){
                        polyline.clear();
                    }

                    // add our point to our trail
                    polyline.addVertex(pt);

                    // store fingers seen this frame for drawing
                    fingersFound.push_back(finger.id());
                    
                    // store first finger for gesture detection
                    if (j == 0 && i == 0){
                        if (pt.z < 0){
                            if (!detectingGestures) beginGestureDetection();
                            gesturePositions.push_front(pt);
                            if (gesturePositions.size() > gestureSamples){
                                gesturePositions.pop_back();
                            }
                        }
                        else {
                            if (detectingGestures) endGestureDetection();
                        }
                    }
                }
            }
            if (detectingGestures){
                switch( performSwipeAnalysis()){
                        // up : 126
                        // down : 125
                        // next (j) : 38
                        // prev (k) : 40
                        // next feed (n) : 45
                        // prev feed (p) : 35
                        // expand (v) : 9
                        
                    case SWIPE_UP:
                        if (nFingers == 1){
                            for (int i = 0; i < scrollEvents; i++){
                                ofSleepMillis(sleepMillis);
                                simulateScroll(UP);
                            }
                        }
                        else if (nFingers == 2){
                            simulateKey(40);
                        }
                        else if (nFingers >= 3){
                            simulateKey(35);
                            simulateKey(38);
                        }
                        break;
                        
                    case SWIPE_DOWN:
                        if (nFingers == 1){
                            for (int i = 0; i < scrollEvents; i++){
                                ofSleepMillis(sleepMillis);
                                simulateScroll(DOWN);
                            }
                        }
                        else if (nFingers == 2){
                            simulateKey(38);
                        }
                        else if (nFingers >= 3){
                            simulateKey(45);
                            simulateKey(38);
                        }
                        break;
                        
                    case SWIPE_RIGHT:
                        simulateKey(9);
                        break;
                        
                    case SWIPE_LEFT:
                        simulateKey(9);
                        break;
                }
            }
        }
        else {
            if(detectingGestures) endGestureDetection();
        }
    }
    leap.markFrameAsOld();
}

//--------------------------------------------------------------
void testApp::reset(){
    cout << "reset" << endl;
    gesturePositions.clear();
    gestureVelocity = 0;
    gestureVector.set(0,0);
    gestureSlope = 0;
}

//--------------------------------------------------------------
int testApp::performSwipeAnalysis(){
    int SWIPE_THRESHOLD = 10;
    
    ofVec2f vector = getGestureVector();
    
    if (vector.length() > SWIPE_THRESHOLD){
        int thisSwipeTime = ofGetElapsedTimeMillis();
        
        if ( thisSwipeTime - lastSwipeTime > debounceTime){
            lastSwipeTime = thisSwipeTime;
            vector = vector.getRotated(-45);
            
            cout << "vec.x : " << vector.x << " | vec.y : " << vector.y << " | len : " << vector.length() << endl;
            
            if (vector.x > 0 && vector.y > 0){
                return SWIPE_UP;
            }
            else if (vector.x > 0 && vector.y <= 0){
                return SWIPE_RIGHT;
            }
            else if (vector.x <= 0 && vector.y > 0){
                return SWIPE_LEFT;
            }
            else if (vector.x <= 0 && vector.y <= 0){
                return SWIPE_DOWN;
            }
        }
    }
    return SWIPE_NULL;
}

//--------------------------------------------------------------
ofVec2f testApp::getGestureVector(){
    int n = gesturePositions.size();
    cout << n << endl;
    ofVec3f totalVector = ofVec3f();

    for (int i = 0; i < n; i++ ){
        if (i < n - 1){
            ofVec3f p1 = ofVec3f(gesturePositions.at(i));
            ofVec3f p2 = ofVec3f(gesturePositions.at(i + 1));
            ofVec3f pointToPointVector = p1 - p2;
            totalVector += pointToPointVector;                  // totalVector is sum of differences betwen all points (not completely accurate)
        }
    }
    totalVector /= n;
    gestureVelocity = totalVector.length();
    gestureVector = ofVec2f(totalVector.x, totalVector.y);

    return gestureVector;
}

//--------------------------------------------------------------
void testApp::beginGestureDetection(){
    detectingGestures = true;
    reset();
}

void testApp::endGestureDetection(){
    detectingGestures = false;
    reset();
}

//--------------------------------------------------------------
void testApp::draw(){
    drawBackground();
    
#ifdef DEBUG
    enable3D();
    drawPlane();
    drawFingerStrips();
    disable3D();
#endif
    
    drawGestureData();
}

//--------------------------------------------------------------
void testApp::drawBackground(){
    ofBackgroundGradient(ofColor(30, 30, 30), ofColor(10, 10, 10), OF_GRADIENT_BAR);
}

//--------------------------------------------------------------
void testApp::drawGestureData(){
    int chartHeight = 100;
    int velocityChartWidth = 30;
    int vectorChartWidth = 100;
    int spacer = 20;

    ofColor activeColor = ofColor(215, 45, 10);
    ofColor inactiveColor = ofColor(40, 40, 30);
    
	ofPushMatrix();
    
    //-------- velocity graphic ---------//

    ofTranslate(spacer, ofGetHeight() - chartHeight - spacer);  // Draw background
    ofSetColor(inactiveColor);
    ofRect(0, 0, velocityChartWidth, chartHeight);
    
    ofSetColor(activeColor);                                    // Draw velocity
    float scaledVelocityHeight = (gestureVelocity / 20) * chartHeight;
    ofRect(0, chartHeight, velocityChartWidth, -scaledVelocityHeight);
    
    ofSetColor(150);                                            // Draw Limiter
    float velocityLimitHeight = 60;
    ofRect(-spacer/4, chartHeight - velocityLimitHeight, velocityChartWidth + spacer/2, 1);

    //-------- vector graphic ---------//

    ofTranslate(velocityChartWidth + spacer, 0);                // Draw background
    ofSetColor(inactiveColor);
    ofRect(0, 0, vectorChartWidth, chartHeight);
    
    ofSetColor(150);                                            // Draw Axes
    ofRect(-spacer/4, chartHeight/2, vectorChartWidth + spacer/2, 1);
    ofRect(vectorChartWidth/2, -spacer/4, 1, chartHeight + spacer/2);

    ofSetColor(activeColor);
    ofTranslate(vectorChartWidth/2, chartHeight/2);                // Draw background
    ofVec2f scaledVectorPos = ofVec2f(gestureVector.x, -gestureVector.y) * 2;
    ofCircle(scaledVectorPos, 5);
    ofLine(scaledVectorPos, ofVec2f(0,0));
    
    //-------- active graphic ---------//

    ofSetColor(activeColor);
    ofTranslate(velocityChartWidth + vectorChartWidth + spacer, 0);                // Draw background
    if(detectingGestures) ofCircle(0,0,15);
    
    ofPopMatrix();
}

//--------------------------------------------------------------
void testApp:: simulateKey(int key){
    cout << "Pressing " << key << endl;
    
    CGEventRef e = CGEventCreateKeyboardEvent (NULL, (CGKeyCode)key, true);
    CGEventPost(kCGSessionEventTap, e);
    CFRelease(e);
}

//--------------------------------------------------------------
void testApp::simulateClick(direction dir){
    CGEventType eventType;
    
    switch (dir){
        case DOWN:
            eventType = kCGEventLeftMouseDown;
            break;
        case UP:
            eventType = kCGEventLeftMouseUp;
            break;
    }
    
    CGEventRef event = CGEventCreate(NULL);
    CGPoint cursor = CGEventGetLocation(event);
    
    CGEventRef mouseEvent = CGEventCreateMouseEvent ( CGEventSourceCreate(NULL), eventType, cursor, kCGMouseButtonLeft );
    CGEventSetType(mouseEvent, eventType);
    CGEventPost( kCGSessionEventTap, mouseEvent );
    
    CFRelease(event);
    CFRelease(mouseEvent);
}

//--------------------------------------------------------------
void testApp::simulateScroll(direction dir){
    int scrollIncrement;
    
    switch (dir){
        case DOWN:
            scrollIncrement = -100;
            break;
        case UP:
            scrollIncrement = 100;
            break;
    }
    
    CGEventRef event = CGEventCreate(NULL);
    CGPoint cursor = CGEventGetLocation(event);
    
    CGEventRef mouseEventScrollDown = CGEventCreateScrollWheelEvent(CGEventSourceCreate(NULL), kCGScrollEventUnitPixel, 1, scrollIncrement);
    CGEventSetType( mouseEventScrollDown, kCGEventScrollWheel );
    CGEventPost( kCGSessionEventTap, mouseEventScrollDown );
    
    CFRelease(event);
    CFRelease(mouseEventScrollDown);
}

//--------------------------------------------------------------
//---- 3D DEBUG DRAW -------------------------------------------
//--------------------------------------------------------------

void testApp::enable3D(){
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    ofEnableLighting();
    l1.enable();
    l2.enable();
    m1.begin();
    cam.begin();
}

//--------------------------------------------------------------
void testApp::disable3D(){
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_NORMALIZE);
    ofDisableLighting();
    l1.disable();
    l2.disable();
    m1.end();
    cam.end();
}

//--------------------------------------------------------------
void testApp::drawPlane(){
    ofPushMatrix();
    ofRotate(90, 0, 0, 1);
    ofSetColor(50);
    ofDrawGridPlane(800, 15, false);
    ofPopMatrix();
}

//--------------------------------------------------------------
void testApp::drawFingerStrips(){
	for(int i = 0; i < fingersFound.size(); i++){
		ofxStrip strip;
		int id = fingersFound[i];
		
		ofPolyline & polyline = fingerTrails[id];
        strip.generate(polyline.getVertices(), 5, ofPoint(0, 0.5, 0.5) );
		
		ofSetColor(255 - id * 15, 0, id * 25);
		strip.getMesh().draw();
	}
    m1.end();
}

/* --------------------------------------------------------------
void testApp::positionCursor(){
    if (fingersFound.size()){
        (glutGet(GLUT_WINDOW_X) + cursorPos.x, glutGet(GLUT_WINDOW_Y) - cursorPos.y);
    }
}
*/