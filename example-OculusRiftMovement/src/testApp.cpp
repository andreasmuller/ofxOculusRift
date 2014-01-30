#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup()
{
	ofBackground(0);
	ofSetLogLevel( OF_LOG_VERBOSE );
//	ofSetVerticalSync( true );
//    ofSetWindowPosition(1920, 0);
//    ofToggleFullscreen();
	showOverlay = false;
	predictive = true;
	
	ofHideCursor();
	
	//oculusRift.baseCamera = &cam;
	oculusRift.setup( &cam );
	
	for(int i = 0; i < 20; i++){
		DemoSphere d;
		d.color = ofColor(ofRandom(255),
						  ofRandom(255),
						  ofRandom(255));
		
		d.pos = ofVec3f(ofRandom(-500, 500),0,ofRandom(-500,500));
		
		d.floatPos.x = d.pos.x;
		d.floatPos.z = d.pos.z;
		
		d.radius = ofRandom(2, 50);
        
        d.bHighlighted = false;
        
		demos.push_back(d);
	}
	
	//enable mouse;
	cam.begin();
	cam.end();
	cam.setGlobalPosition( 0, 100, 0 );
	
	lastMouse = ofVec2f( ofGetMouseX(), ofGetMouseY() );
}


//--------------------------------------------------------------
void testApp::update()
{
	ofVec2f mouse = ofVec2f(ofGetMouseX(), ofGetMouseY());
	ofVec2f mouseVel = mouse - lastMouse;
	lastMouse = mouse;
	
	//ofVec3f forward =  cam.getZAxis();
	//ofVec3f sideways = cam.getXAxis();
	
	ofVec3f forward =  ofVec3f(0,0,1) * oculusRift.getOrientationQuat() * cam.getOrientationQuat();
	ofVec3f sideways = ofVec3f(1,0,0) * oculusRift.getOrientationQuat() * cam.getOrientationQuat();
	
	float movementSpeed = 2.0f;
	if( ofGetKeyPressed('w') ) cam.setPosition( cam.getPosition() + (-forward * movementSpeed) );
	if( ofGetKeyPressed('s') ) cam.setPosition( cam.getPosition() + ( forward * movementSpeed) );
	
	if( ofGetKeyPressed('a') ) cam.setPosition( cam.getPosition() + (-sideways * movementSpeed) );
	if( ofGetKeyPressed('d') ) cam.setPosition( cam.getPosition() + ( sideways * movementSpeed) );
	
	// Rotation
	float rotationDrag = 0.8f;
	float rotationInertia = 0.2f;
	float mouseSensitivity = 0.3f;
	
	
	rotationSpeed *= rotationDrag;
	if( rotationSpeed.length() < 0.00000001f ) { rotationSpeed = ofVec3f(0,0,0); }
	
	if( ofGetMousePressed( OF_MOUSE_BUTTON_RIGHT) )
	{
		//rotationSpeed.x = ofLerp( rotationSpeed.x, -mouseVel.y * mouseSensitivity, rotationInertia );
		rotationSpeed.y = ofLerp( rotationSpeed.y,  mouseVel.x * mouseSensitivity, rotationInertia );
	}
	
	//ofQuaternion tmpRotX( rotationSpeed.x, ofVec3f(1,0,0));
	ofQuaternion tmpRotY( rotationSpeed.y, ofVec3f(0,1,0));
	
	//cam.setOrientation( tmpRotX * cam.getOrientationQuat() * tmpRotY );
	cam.setOrientation( cam.getOrientationQuat() * tmpRotY );
	
	
	
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

	if(oculusRift.isSetup())
	{
		if(showOverlay)
		{
			oculusRift.beginOverlay(-230, 320,240);
			ofRectangle overlayRect = oculusRift.getOverlayRectangle();
			
			ofPushStyle();
			ofEnableAlphaBlending();
			ofFill();
			ofSetColor(255, 40, 10, 200);
			
			ofRect(overlayRect);
			
			ofSetColor(255,255);
			ofFill();
			ofDrawBitmapString("ofxOculusRift by\nAndreas Muller\nJames George\nJason Walters\nElie Zananiri\nFPS:"+ofToString(ofGetFrameRate())+"\nPredictive Tracking " + (oculusRift.getUsePredictiveOrientation() ? "YES" : "NO"), 40, 40);
			
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
    
	
	
	//billboard and draw the mouse
	if(oculusRift.isSetup()){
		
		ofPushMatrix();
		ofSetColor(255, 0, 0);
		oculusRift.multBillboardMatrix();
		ofCircle(0,0,.5);
		ofPopMatrix();
	
	}
	
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
	
	if(key == 'o'){
		showOverlay = !showOverlay;
	}
	if(key == 'r'){
		oculusRift.reset();
		
	}
	if(key == 'h'){
		ofHideCursor();
	}
	if(key == 'H'){
		ofShowCursor();
	}
	
	if(key == 'p'){
		predictive = !predictive;
		oculusRift.setUsePredictedOrientation(predictive);
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
