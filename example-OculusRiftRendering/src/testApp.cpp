#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup()
{
	ofSetLogLevel( OF_LOG_VERBOSE );	
	
	ofSetFrameRate(999);
	ofSetVerticalSync( false );
	
	//ofSetFrameRate( 60 );
	//ofSetVerticalSync( true );
		
	fontWorld.loadFont( "Fonts/DIN.otf", 18, true, false, true );
	//fontWorld.loadFont( "Fonts/DIN.otf", 18 );

	oculusRift.init( 1280, 800, 4 );
	oculusRift.setPosition( 0,-30,0 );
	
	lastUpdateTime = ofGetElapsedTimef();
}


//--------------------------------------------------------------
void testApp::update()
{
	float currTime = ofGetElapsedTimef();
	float frameDeltaTime = currTime - lastUpdateTime;
	lastUpdateTime = currTime;
	
	if(		 ofGetKeyPressed('i') ) { oculusRift.setInterOcularDistance( oculusRift.getInterOcularDistance() + 0.001f ); }
	else if( ofGetKeyPressed('o') ) { oculusRift.setInterOcularDistance( oculusRift.getInterOcularDistance() - 0.001f ); }
	else if( ofGetKeyPressed('k') ) { oculusRift.setInterOcularDistance( oculusRift.getInterOcularDistance() + 1.0f ); }
	else if( ofGetKeyPressed('l') ) { oculusRift.setInterOcularDistance( oculusRift.getInterOcularDistance() - 1.0f ); }
	
	if(	ofGetKeyPressed(OF_KEY_UP) )    { oculusRift.dolly(  30.0f * frameDeltaTime ); }
	if( ofGetKeyPressed(OF_KEY_DOWN) )  { oculusRift.dolly( -30.0f * frameDeltaTime ); }
	if( ofGetKeyPressed(OF_KEY_LEFT) )  { oculusRift.truck(  30.0f * frameDeltaTime ); }
	if( ofGetKeyPressed(OF_KEY_RIGHT) ) { oculusRift.truck( -30.0f * frameDeltaTime ); }
	
}


//--------------------------------------------------------------
void testApp::draw()
{
	oculusRift.beginRenderSceneLeftEye();
		drawSceneGeometry();
	oculusRift.endRenderSceneLeftEye();
	
	oculusRift.beginRenderSceneRightEye();
		drawSceneGeometry();
	oculusRift.endRenderSceneRightEye();
	
	ofSetColor( 255 );
	oculusRift.draw( ofVec2f(0,0), ofVec2f( ofGetWidth(), ofGetHeight() ) );
}

//--------------------------------------------------------------
void testApp::drawSceneGeometry()
{
	ofSetColor(120);
	
	ofPushMatrix();
		ofRotate(90, 0, 0, -1);
		ofDrawGridPlane(500.0f, 40.0f, false );
	ofPopMatrix();
	
	ofSetColor( 255, 0, 0 );
	
	ofPushMatrix();
		ofTranslate( ofPoint(10,0,-80) );
		for( int i = 0; i < 20; i ++ )
		{
			ofBox( ofPoint(0,25,i * -100), 50);
		}
	ofPopMatrix();
	
	string tmpStr = "Do Warping: " + ofToString( oculusRift.getDoWarping() ) + "\n";
	tmpStr += "Inter Ocular Distance: "  + ofToString( oculusRift.getInterOcularDistance() ) + "\n";
	tmpStr += "Shader Scale Factor: "  + ofToString( oculusRift.getShaderScaleFactor() ) + "\n";
	
	ofSetColor( 255 );
	
	ofPushMatrix();
		ofTranslate( ofPoint(-60,280,-200) );
		ofRotateZ( 180 );
		ofRotateY( 180 );
		fontWorld.drawString( tmpStr, 0.0f, 0.0f );
	ofPopMatrix();
	
	ofSetColor(255);
}

//--------------------------------------------------------------
void testApp::keyPressed(int key)
{
	if( key == 'f' )
	{
		ofToggleFullscreen();
	}
	if( key == 'w' )
	{
		oculusRift.setDoWarping( !oculusRift.getDoWarping() );
	}
}

//--------------------------------------------------------------
void testApp::keyReleased(int key)
{

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y)
{

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button)
{

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