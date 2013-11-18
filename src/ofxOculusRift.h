//
//  ofxOculusRift.cpp
//  OculusRiftRendering
//
//  Created by Andreas Müller on 30/04/2013.
//  Updated by James George September 27th 2013
//  Updated by Jason Walters October 22nd 2013
//

#pragma once 

#include "ofMain.h"

#include "OVR.h"
using namespace OVR;
#include <iostream>

//#define STD_GRAV 9.81 // What SHOULD work with Rift, but off by 1000
//#define STD_GRAV 0.00981  // This gives nice 1.00G on Z with Rift face down !!!

//#define DTR = 0.0174532925f



class ofxOculusRift
{
  public:
		
	ofxOculusRift();
	~ofxOculusRift();
	
	//set a pointer to the camera you want as the base perspective
	//the oculus rendering will create a stereo pair from this camera
	//and mix in the head transformation
	ofCamera* baseCamera;
	
	bool setup();
	bool isSetup();
	bool lockView;
    
	//draw background, before rendering eyes
    void beginBackground();
    void endBackground();

	//draw overlay, before rendering eyes
	void beginOverlay(float overlayZDistance = -150, float width = 256, float height = 256);
    void endOverlay();

	void beginLeftEye();
	void endLeftEye();
	
	void beginRightEye();
	void endRightEye();
	
	void draw();
	
	void setUsePredictedOrientation(bool usePredicted);
	
	void reloadShader();

	//default 1 has more constrained mouse movement,
	//while turning it up increases the reach of the mouse
	float oculusScreenSpaceScale;
	
	//projects a 3D point into 2D, optionally accounting for the head orientation
	ofVec3f worldToScreen(ofVec3f worldPosition, bool considerHeadOrientation = false);
	ofVec3f screenToWorld(ofVec3f screenPt, bool considerHeadOrientation = false);
    ofVec3f screenToOculus2D(ofVec3f screenPt, bool considerHeadOrientation = false);
	//returns a 3d position of the mouse projected in front of the camera, at point z
	ofVec3f mousePosition3D(float z = 0, bool considerHeadOrientation = false);
	
	//sets up the view so that things drawn in 2D are billboarded to the caemra,
	//centered at the mouse
	//Good way to draw custom cursors. don't forget to push/pop matrix around the call
	void multBillboardMatrix();
	
	float distanceFromMouse(ofVec3f worldPoint);
	float distanceFromScreenPoint(ofVec3f worldPoint, ofVec2f screenPoint);
	
	ofRectangle getOverlayRectangle() {
		return ofRectangle(0,0,
						   overlayTarget.getWidth(),
						   overlayTarget.getHeight());
	}
	ofFbo& getOverlayTarget(){
		return overlayTarget;
	}
	ofFbo& getBackgroundTarget(){
		return backgroundTarget;
	}
	
	ofRectangle getOculusViewport();


  private:
	bool bSetup;
	bool bUsePredictedOrientation;
	bool bUseBackground;
	bool bUseOverlay;
	float overlayZDistance;
	Ptr<DeviceManager>	pManager;
	Ptr<HMDDevice>		pHMD;
	Ptr<SensorDevice>	pSensor;
	SensorFusion*       pFusionResult;
	HMDInfo				hmdInfo;

	OVR::Util::Render::StereoConfig stereo;
	float renderScale;
	ofMesh overlayMesh;
	ofMatrix4x4 orientationMatrix;
	
	ofVboMesh leftEyeMesh;
	ofVboMesh rightEyeMesh;
	ofFbo renderTarget;
    ofFbo backgroundTarget;
	ofFbo overlayTarget;
	ofShader distortionShader;

	void setupEyeParams(OVR::Util::Render::StereoEye eye);
	void setupShaderUniforms(OVR::Util::Render::StereoEye eye);
	
	void renderOverlay();
};
