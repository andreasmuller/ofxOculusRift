ofxOculusRift
================

Oculus Rift addon for openFrameworks lets you attach the rift rendering and headtracking pipeline to any existing ofCamera.

Originally by Andreas Muller
Refactored by James George

	void testApp::setup(){
		oculusRift.baseCamera = &cam; //attach to your camera
		//opens the device, an Oculus must be plugged in 
		//as it uses the params returned from the head set to configure 
		//the resolution settings
		oculusRift.setup();
		    
		//must run in full screen mode
		ofToggleFullScreen();
	}
	  
	void testApp::draw(){
	  
		//move your camera wherever you'd like, this becomes the base
		//position of the view
		   
		//now render using oculus flow
		oculusRift.beginLeftEye();
		drawScene();
		oculusRift.endLeftEye();
		
		oculusRift.beginRightEye();
		drawScene();
		oculusRift.endRightEye();
		
		//pushes the render texture to the viewer
		oculusRift.draw();
	}


	void testApp::drawScene(){
		//draw anything here in 3d, will be rendered once for each eye
	}
