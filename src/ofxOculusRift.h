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
    
    void beginBackground();
    void endBackground();
	
	void beginLeftEye();
	void endLeftEye();
	
	void beginRightEye();
	void endRightEye();
	
	void draw();
	
	void setUsePredictedOrientation(bool usePredicted);
	
	void reloadShader();
	
	ofRectangle getOculusViewport();
	
  private:
	bool bSetup;
	bool bUsePredictedOrientation;
	
	Ptr<DeviceManager>	pManager;
	Ptr<HMDDevice>		pHMD;
	Ptr<SensorDevice>	pSensor;
	SensorFusion*       pFusionResult;
	HMDInfo				hmdInfo;

	OVR::Util::Render::StereoConfig stereo;
	float renderScale;

    ofFbo backgroundTarget;
    
	ofVboMesh leftEyeMesh;
	ofVboMesh rightEyeMesh;
	ofFbo renderTarget;
	ofShader distortionShader;

	void setupEyeParams(OVR::Util::Render::StereoEye eye);
	void setupShaderUniforms(OVR::Util::Render::StereoEye eye);
	
};
