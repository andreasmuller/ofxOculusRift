#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup()
{
	ofBackground(0);
	ofSetLogLevel( OF_LOG_VERBOSE );
	ofSetVerticalSync( true );
//    ofSetWindowPosition(1920, 0);
//    ofToggleFullscreen();
	showOverlay = false;
	
	oculusRift.baseCamera = &cam;
	oculusRift.setup();
	
	for(int i = 0; i < 1; i++){
		DemoSphere d;
		d.color = ofColor(ofRandom(255),
						  ofRandom(255),
						  ofRandom(255));
		
		d.pos = ofVec3f(ofRandom(-500, 500),
						0,
						ofRandom(-500,500));
		
		d.floatPos.x = d.pos.x;
		d.floatPos.z = d.pos.z;
		
		d.radius = ofRandom(2, 50);
        
        d.bHighlighted = false;
        
		demos.push_back(d);
	}
	
	//enable mouse;
	cam.begin();
	cam.end();
}


//--------------------------------------------------------------
void testApp::update()
{
	for(int i = 0; i < demos.size(); i++){
//		demos[i].floatPos.y = ofSignedNoise(ofGetElapsedTimef()/10.0,
//									  demos[i].pos.x/100.0,
//									  demos[i].pos.z/100.0,
//									  demos[i].radius*100.0) * demos[i].radius*20.;
		
	}
    
    if(oculusRift.isSetup()){
        cursor3D = oculusRift.screenToWorld(cursor2D);
		
        // Check for collisions with cursor.
        cursorRift = oculusRift.screenToOculus2D(cursor2D);
        for(int i = 0; i < demos.size(); i++){
            demoRift = oculusRift.screenToOculus2D(demos[i].floatPos);
            cout << "distance = " << ofDist(cursorRift.x, cursorRift.y, demoRift.x, demoRift.y) << endl;
        }
    }
}


//--------------------------------------------------------------
void testApp::draw()
{

	
	if(oculusRift.isSetup()){
		
		if(true || showOverlay){
			
//			glDisable(GL_DEPTH_TEST);
			
			oculusRift.beginOverlay(-150);
			ofRectangle overlayRect = oculusRift.getOverlayRectangle();
			
			ofPushStyle();
			ofEnableAlphaBlending();
			ofFill();
			ofSetColor(255, 40, 10, 200);
			ofRect(overlayRect.x,
				   overlayRect.y,
				   overlayRect.width*.75,
				   overlayRect.height*.75);
			
			ofSetColor(255,255);
			ofRect(overlayRect.getCenter().x-10,
				   overlayRect.getCenter().y-10,
				   20,20);
//
			
			ofFill();
			ofDrawBitmapString("FPS: " + ofToString(ofGetFrameRate()), overlayRect.getCenter()  );
			//ofDrawBitmapString("RIFT DEMO by James George", 256, 256);
			
			ofPopStyle();
			oculusRift.endOverlay();
		}
        
		glEnable(GL_DEPTH_TEST);
		oculusRift.beginLeftEye();
		drawScene();
		oculusRift.endLeftEye();
		
		oculusRift.beginRightEye();
		drawScene();
		oculusRift.endRightEye();
		
		oculusRift.draw();
		
		glDisable(GL_DEPTH_TEST);
	}
	else{
		cam.begin();
		drawScene();
		cam.end();
	}
	
    ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2);
    ofSetColor(255, 0, 0);
    ofCircle(cursorRift.x, cursorRift.y, 20);
    ofSetColor(0, 255, 0);
    ofCircle(demoRift.x, demoRift.y, 20);
    ofSetColor(0, 0, 255);
    ofLine(cursorRift.x, cursorRift.y, demoRift.x, demoRift.y);
}

//--------------------------------------------------------------
void testApp::drawScene()
{
	
	ofPushMatrix();
	ofRotate(90, 0, 0, -1);
	ofDrawGridPlane(500.0f, 10.0f, false );
	ofPopMatrix();
	
	ofPushStyle();
	ofNoFill();
	for(int i = 0; i < demos.size(); i++){
		ofPushMatrix();
//		ofRotate(ofGetElapsedTimef()*(50-demos[i].radius), 0, 1, 0);
		ofTranslate(demos[i].floatPos);
//		ofRotate(ofGetElapsedTimef()*4*(50-demos[i].radius), 0, 1, 0);
		ofSetColor(demos[i].bHighlighted? ofColor::white : demos[i].color);
		ofSphere(demos[i].radius);
		ofPopMatrix();
	}
		
	ofPopStyle();
    
    ofSetColor(255, 0, 0);
    ofCircle(cursor3D, 2);
}

//--------------------------------------------------------------
void testApp::keyPressed(int key)
{
	if( key == 'f' )
	{
		//gotta toggle full screen for it to be right
		ofToggleFullscreen();
	}
	
	if(key == 's'){
		oculusRift.reloadShader();
	}
	
	if(key == 'l'){
		oculusRift.lockView = !oculusRift.lockView;
	}
	
	if(key == 'r'){
		showOverlay = !showOverlay;
	}
}

//--------------------------------------------------------------
void testApp::keyReleased(int key)
{

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y)
{
    cursor2D.set(x, y, cursor2D.z);
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button)
{
    cursor2D.set(x, y, cursor2D.z);
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button)
{

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button)
{

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h)
{

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg)
{

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo)
{

}
