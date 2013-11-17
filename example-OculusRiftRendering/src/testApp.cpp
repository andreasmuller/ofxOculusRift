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
	
	ofHideCursor();
	
	oculusRift.baseCamera = &cam;
	oculusRift.setup();
	
	for(int i = 0; i < 20; i++){
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
		demos[i].floatPos.y = ofSignedNoise(ofGetElapsedTimef()/10.0,
									  demos[i].pos.x/100.0,
									  demos[i].pos.z/100.0,
									  demos[i].radius*100.0) * demos[i].radius*20.;
		
	}
    
    if(oculusRift.isSetup()){
		
        for(int i = 0; i < demos.size(); i++){
			float dist = oculusRift.distanceFromMouse( demos[i].floatPos );
            demos[i].bHighlighted = (dist < 50);
        }
    }
}


//--------------------------------------------------------------
void testApp::draw()
{

	
	if(oculusRift.isSetup()){
		
		if(showOverlay){
			
//			glDisable(GL_DEPTH_TEST);
			
			oculusRift.beginOverlay(-200);
			ofRectangle overlayRect = oculusRift.getOverlayRectangle();
			
			ofPushStyle();
			ofEnableAlphaBlending();
			ofFill();
			ofSetColor(255, 40, 10, 200);
			
			ofRect(overlayRect);
			
			ofSetColor(255,255);
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
		ofSetColor(demos[i].bHighlighted ? ofColor::white.getLerped(ofColor::red, sin(ofGetElapsedTimef()*10.0)*.5+.5) : demos[i].color);
		ofSphere(demos[i].radius);
		ofPopMatrix();
	}
    
	
	ofSetColor(255, 0, 0);
	//billboard and draw the mouse
	ofPushMatrix();
	ofNode n;
	n.setPosition( oculusRift.mousePosition3D() );
	n.lookAt(cam.getPosition());
	ofVec3f axis; float angle;
    n.getOrientationQuat().getRotate(angle, axis);
    // Translate the object to its position.
    ofTranslate(oculusRift.mousePosition3D() );
    // Perform the rotation.
    ofRotate(angle, axis.x, axis.y, axis.z);
    ofCircle(0,0,.5);
	ofPopMatrix();
	
	ofPopStyle();
    
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
	
	if(key == 'h'){
		ofHideCursor();
	}
	if(key == 'H'){
		ofShowCursor();
	}
}

//--------------------------------------------------------------
void testApp::keyReleased(int key)
{

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y)
{
 //   cursor2D.set(x, y, cursor2D.z);
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button)
{
//    cursor2D.set(x, y, cursor2D.z);
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
