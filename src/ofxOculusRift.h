//
//  ofxOculusRift.h
//  OculusRiftRendering
//
//  Created by Andreas Müller on 30/04/2013.
//
//

#pragma once 

#include "ofMain.h"

#include "OVR.h"
using namespace OVR;
#include <iostream>

//#define STD_GRAV 9.81 // What SHOULD work with Rift, but off by 1000
#define STD_GRAV 0.00981  // This gives nice 1.00G on Z with Rift face down !!!

#define DTR = 0.0174532925f


static float K0 = 1.0f;
static float K1 = 0.22f;
static float K2 = 0.24f;
static float K3 = 0.0f;


class ofxOculusRift : public ofCamera
{
	public:
	
		ofxOculusRift();
		~ofxOculusRift();
	
		bool				init( int _width, int _height, int _fboNumSamples = 0 );
	
		void				beginRenderSceneLeftEye();
		void				endRenderSceneLeftEye();
	
		void				beginRenderSceneRightEye();
		void				endRenderSceneRightEye();

		void				draw( ofVec2f pos, ofVec2f size );
	
		//ofQuaternion		getOrientationQuat() const; //

		ofQuaternion		getHeadsetOrientationQuat(); // this is confusing, but I'm trying to read the sensor
														 // (if needed this frame) when getOrientationQuat is called,
														 // but getOrientationQuat is marked const. For now adding 'Headset'
														 // prefix to the get functions. Todo: re-write
		
		ofQuaternion		getHeadsetViewOrientationQuat();	// the orientation to make a view from the headset
	
		ofMatrix4x4			getHeadsetOrientationMat();
		ofMatrix4x4			getHeadsetViewOrientationMat();
	
		ofVec3f				getAcceleration();
		
		void				setNeedSensorReadingThisFrame( bool _needSensorReading );
	
		void				setInterOcularDistance( float _iod );
		float				getInterOcularDistance();
		
		void				setShaderScaleFactor( float _scale );
		float				getShaderScaleFactor();
	
		void				setDoWarping( bool _doWarping );
		bool				getDoWarping();
	
		void				shutdown();
		
	private:
	
		bool				initSensor();
		void				clearSensor();
	
		void				beginRender( float interOcularShift, ofFbo* _fbo  );
		void				endRender( ofFbo* _fbo );
	
		void				readSensorIfNeededThisFrame();
	
		void				setupScreenPerspective(float _interOcularDistance,
													float width, float height,
													ofOrientation orientation,
													bool vFlip,
													float fov,
													float nearDist, float farDist );
	
		void				renderDistortedEyeNew( bool _isLeftEye, float x, float y, float w, float h );
	
		void				initFBO(int screenWidth, int screenHeight);

		bool				doWarping;	
		ofShader			hmdWarpShader;
		float				shaderScaleFactor;

	
		float				interOcularDistance;
	
		ofFbo				eyeFboLeft;  // Todo: draw straight into a full sized FBO
		ofFbo				eyeFboRight;
	
		bool				needSensorReadingThisFrame;
	
		ofVec3f				acc;
	
		// Todo: re-write to use an ofFbo, either re-write shader to do texture rect coordinates or init a gl_texture_2d that is npot
		GLuint				colorTextureID;
		GLuint				framebufferID;
		GLuint				depthRenderBufferID;
	
		Ptr<DeviceManager>	pManager;
		Ptr<HMDDevice>		pHMD;
		Ptr<SensorDevice>	pSensor;
		SensorFusion		FusionResult;
		HMDInfo				Info;
		bool				InfoLoaded;
	
};
