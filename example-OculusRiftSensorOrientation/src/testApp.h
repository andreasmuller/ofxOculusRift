#pragma once

#include "ofMain.h"
#include "ofxOculusRift.h"

class testApp : public ofBaseApp
{
	public:
		void setup();
		void update();
		void draw();
			
		ofxOculusRift		oculusRift;
	
		ofEasyCam			cam;
};
