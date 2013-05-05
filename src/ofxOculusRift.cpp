//
//  ofxOculusRift.cpp
//  OculusRiftRendering
//
//  Created by Andreas Müller on 30/04/2013.
//
//

#include "ofxOculusRift.h"

// ---------------------------------------------------------------------------------------------------------------------------------------------------
//
ofxOculusRift::ofxOculusRift()
{

}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
//
ofxOculusRift::~ofxOculusRift()
{
	shutdown();
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
//
bool ofxOculusRift::init( int _width, int _height, int _fboNumSamples )
{
	initFBO( _width, _height );
	
	hmdWarpShader.load("Shaders/HmdWarp");
	
	ofDisableArbTex();

		ofFbo::Settings tmpSettings = ofFbo::Settings();
		tmpSettings.width			= _width/2;
		tmpSettings.height			= _height;
		tmpSettings.internalformat	= GL_RGB;
		tmpSettings.textureTarget	= GL_TEXTURE_2D;
		tmpSettings.numSamples		= _fboNumSamples;
		
		eyeFboLeft.allocate( tmpSettings );
		eyeFboRight.allocate( tmpSettings );
	
	ofEnableArbTex();
	
	setNearClip( 0.001f );
	setFarClip( 2048.0f );
	setFov( 90.0f );

	setInterOcularDistance( -0.6f );
	setShaderScaleFactor( 1.0f );
	setDoWarping( true );

	
	return initSensor();
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
//
void ofxOculusRift::beginRenderSceneLeftEye()
{
	beginRender( getInterOcularDistance() * -0.5f, &eyeFboLeft );
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
//
void ofxOculusRift::endRenderSceneLeftEye()
{
	endRender( &eyeFboLeft );
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
//
void ofxOculusRift::beginRenderSceneRightEye()
{
	beginRender( getInterOcularDistance() * 0.5f, &eyeFboRight );	
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
//
void ofxOculusRift::endRenderSceneRightEye()
{
	endRender( &eyeFboRight );
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
//
void ofxOculusRift::beginRender( float _interOcularShift, ofFbo* _fbo  )
{
	ofPushView();

		_fbo->begin();
		ofClear(0,0,0); // Todo: get the proper clear color
	
		setupScreenPerspective( _interOcularShift, ofGetWidth(), ofGetHeight(), ofGetOrientation(), false, getFov(), getNearClip(), getFarClip()  );
	
		ofSetMatrixMode(OF_MATRIX_MODELVIEW);
		ofLoadIdentityMatrix();
	
		ofPushMatrix();
	
			// flip for FBO
			ofScale(1,-1,1);
	
			ofMultMatrix( getHeadsetViewOrientationMat() );
			ofTranslate( getPosition() );
}


// ---------------------------------------------------------------------------------------------------------------------------------------------------
//
void ofxOculusRift::endRender( ofFbo* _fbo )
{
		ofPopMatrix();
		_fbo->end();
	ofPopView();

}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
//
void ofxOculusRift::draw( ofVec2f pos, ofVec2f size )
{
	// Todo: rewrite this
	
	
	ofPushView();
	
		// draw into the new fbo we have made
		ofSetColor( 255 );
		glBindFramebuffer(GL_FRAMEBUFFER, framebufferID);
			ofClear(0,0,0);
			eyeFboLeft.draw( 0.0f, 0.0f );
			eyeFboRight.draw( eyeFboLeft.getWidth(), 0.0f );
	
			// is this being drawn correctly?
	
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		
		ofSetMatrixMode(OF_MATRIX_PROJECTION);
		ofLoadIdentityMatrix();
		ofSetMatrixMode(OF_MATRIX_MODELVIEW);
		ofLoadIdentityMatrix();
		
		ofSetColor( 255 );
			
		if( doWarping )
		{
			glEnable( GL_TEXTURE_2D );
			glBindTexture(GL_TEXTURE_2D, colorTextureID);
			
			renderDistortedEyeNew( true,  0.0f, 0.0f, 0.5f, 1.0f);
			renderDistortedEyeNew( false, 0.5f, 0.0f, 0.5f, 1.0f);
			
			glDisable( GL_TEXTURE_2D );
		}
		
	ofPopView();

	/*
	glBegin(GL_TRIANGLE_STRIP);
		glTexCoord2f(pos.x,			pos.y		);   glVertex2f(-1.0f, -1.0f);
		glTexCoord2f(pos.x+size.x,	pos.y	 	);   glVertex2f(0.0f, -1.0f);
		glTexCoord2f(pos.x,			pos.y+size.y);   glVertex2f(-1.0f, 1.0f);
		glTexCoord2f(pos.x+size.x,	pos.y+size.y);   glVertex2f(0.0f, 1.0f);
	glEnd();
	 */
	
	if( !doWarping )
	{
		ofSetColor(255);
		eyeFboLeft.draw( 0.0f, 0.0f );
		eyeFboRight.draw( eyeFboLeft.getWidth(), 0.0f );
	}
	
	needSensorReadingThisFrame = true;
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
//
void ofxOculusRift::readSensorIfNeededThisFrame()
{
	if( needSensorReadingThisFrame )
	{
		Vector3f tmpAcc = FusionResult.GetAcceleration();
		acc.set( ofVec3f(tmpAcc.x, tmpAcc.y, tmpAcc.z) );
		
		Quatf quaternion = FusionResult.GetOrientation();
		setOrientation( ofQuaternion(quaternion.x, quaternion.y, quaternion.z, quaternion.w) ); // sets the orientation quat in ofNode
		
		needSensorReadingThisFrame = false;
	}
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
//
void ofxOculusRift::setNeedSensorReadingThisFrame( bool _needSensorReading )
{
	needSensorReadingThisFrame = _needSensorReading;
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
//
ofQuaternion ofxOculusRift::getHeadsetOrientationQuat()
{
	readSensorIfNeededThisFrame();
	
	return ofNode::getOrientationQuat();
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
//
ofQuaternion ofxOculusRift::getHeadsetViewOrientationQuat()
{
	return getHeadsetOrientationQuat().inverse();
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
//
ofMatrix4x4 ofxOculusRift::getHeadsetOrientationMat()
{
	ofMatrix4x4 tmpMat;
	getHeadsetOrientationQuat().get( tmpMat );
	return tmpMat;
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
//
ofMatrix4x4 ofxOculusRift::getHeadsetViewOrientationMat()
{
	ofMatrix4x4 tmpMat;
	getHeadsetViewOrientationQuat().get( tmpMat );
	return tmpMat;
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
//
ofVec3f	ofxOculusRift::getAcceleration()
{
	return acc;
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
//
void ofxOculusRift::setInterOcularDistance( float _iod )
{
	interOcularDistance = _iod;
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
//
float ofxOculusRift::getInterOcularDistance()
{
	return interOcularDistance;
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
//
void ofxOculusRift::setShaderScaleFactor( float _scale )
{
	shaderScaleFactor = _scale;
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
//
float ofxOculusRift::getShaderScaleFactor()
{
	getShaderScaleFactor();
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
//
void ofxOculusRift::setDoWarping( bool _doWarping )
{
	doWarping = _doWarping;
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
//
bool ofxOculusRift::getDoWarping()
{
	return doWarping;
}

//--------------------------------------------------------------
void ofxOculusRift::renderDistortedEyeNew( bool _isLeftEye, float _x, float _y, float _w, float _h )
{
	float as = _w/_h;
	
	float DistortionXCenterOffset = -0.25f;
	if ( _isLeftEye ) { DistortionXCenterOffset = 0.25f; }
	
	hmdWarpShader.begin();
	
	hmdWarpShader.setUniform2f("LensCenter", _x + (_w + DistortionXCenterOffset * 0.5f)*0.5f, _y + _h*0.5f );
	hmdWarpShader.setUniform2f("ScreenCenter", _x + _w*0.5f, _y + _h*0.5f );
	hmdWarpShader.setUniform2f("Scale", (_w/2.0f) * shaderScaleFactor, (_h/2.0f) * shaderScaleFactor * as );
	hmdWarpShader.setUniform2f("ScaleIn", (2.0f/_w), (2.0f/_h) / as );
	hmdWarpShader.setUniform4f("HmdWarpParam", K0, K1, K2, K3 );
	
	if( _isLeftEye )
	{
		glBegin(GL_TRIANGLE_STRIP);
			glTexCoord2f(0.0f, 0.0f);   glVertex2f(-1.0f, -1.0f);
			glTexCoord2f(0.5f, 0.0f);   glVertex2f(0.0f, -1.0f);
			glTexCoord2f(0.0f, 1.0f);   glVertex2f(-1.0f, 1.0f);
			glTexCoord2f(0.5f, 1.0f);   glVertex2f(0.0f, 1.0f);
		glEnd();
	}
	else
	{
		glBegin(GL_TRIANGLE_STRIP);
			glTexCoord2f(0.5f, 0.0f);   glVertex2f(0.0f, -1.0f);
			glTexCoord2f(1.0f, 0.0f);   glVertex2f(1.0f, -1.0f);
			glTexCoord2f(0.5f, 1.0f);   glVertex2f(0.0f, 1.0f);
			glTexCoord2f(1.0f, 1.0f);   glVertex2f(1.0f, 1.0f);
		glEnd();
	}
	
	hmdWarpShader.end();
}


// ---------------------------------------------------------------------------------------------------------------------------------------------------
//
bool ofxOculusRift::initSensor()
{
	System::Init();
	
	pManager = *DeviceManager::Create();
	
	pHMD = *pManager->EnumerateDevices<HMDDevice>().CreateDevice();
	
	if (pHMD)
	{
		InfoLoaded = pHMD->GetDeviceInfo(&Info);
		pSensor = *pHMD->GetSensor();
	}
	else
	{
		pSensor = *pManager->EnumerateDevices<SensorDevice>().CreateDevice();
	}
	
	if (pSensor)
	{
		FusionResult.AttachToSensor(pSensor);
	}
	
	if (pHMD)	{ ofLogVerbose() << " [x] HMD Found" << endl; }
	else		{ ofLogVerbose() << " [ ] HMD Not Found" << endl; }
	
	if (pSensor) {  ofLogVerbose() << " [x] Sensor Found" << endl; }
	else {			ofLogVerbose() << " [ ] Sensor Not Found" << endl; }
	
	ofLogVerbose() << "--------------------------" << endl;
	
	if (InfoLoaded)
	{
		ofLogVerbose() << " DisplayDeviceName: " << Info.DisplayDeviceName << endl;
		ofLogVerbose() << " ProductName: " << Info.ProductName << endl;
		ofLogVerbose() << " Manufacturer: " << Info.Manufacturer << endl;
		ofLogVerbose() << " Version: " << Info.Version << endl;
		ofLogVerbose() << " HResolution: " << Info.HResolution<< endl;
		ofLogVerbose() << " VResolution: " << Info.VResolution<< endl;
		ofLogVerbose() << " HScreenSize: " << Info.HScreenSize<< endl;
		ofLogVerbose() << " VScreenSize: " << Info.VScreenSize<< endl;
		ofLogVerbose() << " VScreenCenter: " << Info.VScreenCenter<< endl;
		ofLogVerbose() << " EyeToScreenDistance: " << Info.EyeToScreenDistance << endl;
		ofLogVerbose() << " LensSeparationDistance: " << Info.LensSeparationDistance << endl;
        ofLogVerbose() << " InterpupillaryDistance: " << Info.InterpupillaryDistance << endl;
        ofLogVerbose() << " DistortionK[0]: " << Info.DistortionK[0] << endl;
        ofLogVerbose() << " DistortionK[1]: " << Info.DistortionK[1] << endl;
		ofLogVerbose() << " DistortionK[2]: " << Info.DistortionK[2] << endl;
		ofLogVerbose() << "--------------------------" << endl;
	}

	bool success = false;
	
	if ( (pSensor) )
	{
		success = true;
	}

	return success;
	
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
//
void ofxOculusRift::clearSensor()
{
	pSensor.Clear();
	pManager.Clear();
	
	System::Destroy();
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
//
void ofxOculusRift::shutdown()
{
	clearSensor();
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
//
void ofxOculusRift::setupScreenPerspective(float _interOcularDistance, float width, float height, ofOrientation orientation, bool vFlip, float fov, float nearDist, float farDist)
{
	if(width == 0) width = ofGetWidth();
	if(height == 0) height = ofGetHeight();
	
	float viewW = ofGetViewportWidth();
	float viewH = ofGetViewportHeight();
	
	float eyeX = viewW / 2;
	float eyeY = viewH / 2;
	float halfFov = PI * fov / 360;
	float theTan = tanf(halfFov);
	float dist = eyeY / theTan;
	float aspect = (float) viewW / viewH;
	
	if(nearDist == 0) nearDist = dist / 10.0f;
	if(farDist == 0) farDist = dist * 10.0f;
	
	ofSetMatrixMode(OF_MATRIX_PROJECTION);
	ofLoadIdentityMatrix();
	
	ofTranslate( ofPoint(_interOcularDistance,0,0) );
	
	ofMatrix4x4 persp;
	persp.makePerspectiveMatrix(fov, aspect, nearDist, farDist);
	ofMultMatrix( persp );
	//gluPerspective(fov, aspect, nearDist, farDist);
	
	
	ofSetMatrixMode(OF_MATRIX_MODELVIEW);
	ofLoadIdentityMatrix();
	
	ofMatrix4x4 lookAt;
	lookAt.makeLookAtViewMatrix( ofVec3f(eyeX, eyeY, dist),  ofVec3f(eyeX, eyeY, 0),  ofVec3f(0, 1, 0) );
	ofLoadMatrix( lookAt );
	
	//note - theo checked this on iPhone and Desktop for both vFlip = false and true
	if(ofDoesHWOrientation()){
		if(vFlip){
			ofScale(1, -1, 1);
			ofTranslate(0, -height, 0);
		}
	}else{
		if( orientation == OF_ORIENTATION_UNKNOWN ) orientation = ofGetOrientation();
		switch(orientation) {
			case OF_ORIENTATION_180:
				ofRotate(-180, 0, 0, 1);
				if(vFlip){
					ofScale(1, -1, 1);
					ofTranslate(-width, 0, 0);
				}else{
					ofTranslate(-width, -height, 0);
				}
				
				break;
				
			case OF_ORIENTATION_90_RIGHT:
				ofRotate(-90, 0, 0, 1);
				if(vFlip){
					ofScale(-1, 1, 1);
				}else{
					ofScale(-1, -1, 1);
					ofTranslate(0, -height, 0);
				}
				break;
				
			case OF_ORIENTATION_90_LEFT:
				ofRotate(90, 0, 0, 1);
				if(vFlip){
					ofScale(-1, 1, 1);
					ofTranslate(-width, -height, 0);
				}else{
					ofScale(-1, -1, 1);
					ofTranslate(-width, 0, 0);
				}
				break;
				
			case OF_ORIENTATION_DEFAULT:
			default:
				if(vFlip){
					ofScale(1, -1, 1);
					ofTranslate(0, -height, 0);
				}
				break;
		}
	}
	
}


//--------------------------------------------------------------
void ofxOculusRift::initFBO(int screenWidth, int screenHeight)
{
	glGenFramebuffers(1, &framebufferID);
	glGenTextures(1, &colorTextureID);
	glGenRenderbuffers(1, &depthRenderBufferID);
	
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferID);
	
	// initialize color texture
	glBindTexture(GL_TEXTURE_2D, colorTextureID);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, screenWidth, screenHeight, 0,GL_RGBA, GL_INT, NULL );
	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D, colorTextureID, 0);
	
	// initialize depth renderbuffer
	glBindRenderbuffer(GL_RENDERBUFFER, depthRenderBufferID);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, screenWidth, screenHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER, depthRenderBufferID);
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}



