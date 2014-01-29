#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup()
{
	ofSetFrameRate(999);
	ofSetVerticalSync( false );
		
	oculusRift.setup();
}

//--------------------------------------------------------------
void testApp::update()
{


}

//--------------------------------------------------------------
void testApp::draw()
{
	
	ofEnableDepthTest();
	
		cam.begin();
		
			ofPushMatrix();

				ofMultMatrix( oculusRift.getOrientationQuat() );

				ofSetColor( 255, 0, 0 );
				ofBox( 200 );
		
				ofTranslate( 0.0f, 0.0f, 100.0f );
				ofSetColor( 0, 0, 255 );
				ofBox( 100 );

			ofPopMatrix();
		
		cam.end();
	
	ofDisableDepthTest();
}
