//
//  ofxOculusRift.cpp
//  OculusRiftRendering
//
//  Created by Andreas Müller on 30/04/2013.
//  Updated by James George September 27th 2013
//  Updated by Jason Walters October 22 2013
//

#include "ofxOculusRift.h"

#define GLSL(version, shader)  "#version " #version "\n" #shader
static const char* OculusWarpVert = GLSL(120,
uniform vec2 dimensions;
varying vec2 oTexCoord;
 
void main()
{
	oTexCoord = gl_MultiTexCoord0.xy / dimensions;
	gl_FrontColor = gl_Color;
	gl_Position = ftransform();
});

static const char* OculusWarpFrag = GLSL(120,
uniform vec2 LensCenter;
uniform vec2 ScreenCenter;
uniform vec2 Scale;
uniform vec2 ScaleIn;
uniform vec4 HmdWarpParam;
uniform vec4 ChromAbParam;
uniform sampler2DRect Texture0;
uniform vec2 dimensions;
varying vec2 oTexCoord;

void main()
{
	vec2  theta = (oTexCoord - LensCenter) * ScaleIn; // Scales to [-1, 1]
	float rSq = theta.x * theta.x + theta.y * theta.y;
    vec2  theta1 = theta * (HmdWarpParam.x +
                            HmdWarpParam.y * rSq +
							HmdWarpParam.z * rSq * rSq +
                            HmdWarpParam.w * rSq * rSq * rSq);
    
    // Detect whether blue texture coordinates are out of range
    // since these will scale out the furthest.
    vec2 thetaBlue = theta1 * (ChromAbParam.z + ChromAbParam.w * rSq);
    vec2 tcBlue = LensCenter + Scale * thetaBlue;
    if (!all(equal(clamp(tcBlue, ScreenCenter - vec2(0.25, 0.5), ScreenCenter + vec2(0.25, 0.5)), tcBlue))) {
        gl_FragColor = vec4(0);
    }
    else {
        // Now do blue texture lookup.
        float blue = texture2DRect(Texture0, tcBlue * dimensions).b;
        
        // Do green lookup (no scaling).
        vec2 tcGreen = LensCenter + Scale * theta1;
        vec4 center = texture2DRect(Texture0, tcGreen * dimensions);
        
        // Do red scale and lookup.
        vec2 thetaRed = theta1 * (ChromAbParam.x + ChromAbParam.y * rSq);
        vec2 tcRed = LensCenter + Scale * thetaRed;
        float red = texture2DRect(Texture0, tcRed * dimensions).r;
        
        gl_FragColor = vec4(red, center.g, blue, center.a) * gl_Color;
    }
});

ofMatrix4x4 toOf(const Matrix4f& m){
	return ofMatrix4x4(m.M[0][0],m.M[1][0],m.M[2][0],m.M[3][0],
					   m.M[0][1],m.M[1][1],m.M[2][1],m.M[3][1],
					   m.M[0][2],m.M[1][2],m.M[2][2],m.M[3][2],
					   m.M[0][3],m.M[1][3],m.M[2][3],m.M[3][3]);
}

Matrix4f toOVR(const ofMatrix4x4& m){
	const float* cm = m.getPtr();
	return Matrix4f(cm[ 0],cm[ 1],cm[ 2],cm[ 3],
					cm[ 4],cm[ 5],cm[ 6],cm[ 7],
					cm[ 8],cm[ 9],cm[10],cm[11],
					cm[12],cm[13],cm[14],cm[15]);
}

ofRectangle toOf(const OVR::Util::Render::Viewport vp){
	return ofRectangle(vp.x,vp.y,vp.w,vp.h);
}

ofxOculusRift::ofxOculusRift(){
	baseCamera = NULL;
	bSetup = false;
	lockView = false;
	bUsePredictedOrientation = true;
	bUseOverlay = false;
	bUseBackground = false;
	overlayZDistance = -200;
	oculusScreenSpaceScale = 2;
}

ofxOculusRift::~ofxOculusRift(){
	if(bSetup){
		pSensor.Clear();
        pHMD.Clear();
		pManager.Clear();
        
        delete pFusionResult;
                
		System::Destroy();
		bSetup = false;
	}
}


bool ofxOculusRift::setup(){
	
	if(bSetup){
		ofLogError("ofxOculusRift::setup") << "Already set up";
		return false;
	}
	
	System::Init();
    
    pFusionResult = new SensorFusion();
	pManager = *DeviceManager::Create();
	pHMD = *pManager->EnumerateDevices<HMDDevice>().CreateDevice();
    
	if (pHMD == NULL){
		ofLogError("ofxOculusRift::setup") << "HMD not found";
		return false;
	}
	
	if(!pHMD->GetDeviceInfo(&hmdInfo)){
		ofLogError("ofxOculusRift::setup") << "HMD Info not loaded";
		return false;
	}
	
	pSensor = *pHMD->GetSensor();
	if (pSensor == NULL){
		ofLogError("ofxOculusRift::setup") << "No sensor returned";
		return false;
	}
	
	if(!pFusionResult->AttachToSensor(pSensor)){
		ofLogError("ofxOculusRift::setup") << "Sensor Fusion failed";
		return false;
	}
	
	stereo.SetFullViewport(OVR::Util::Render::Viewport(0,0, hmdInfo.HResolution, hmdInfo.VResolution));
	stereo.SetStereoMode(OVR::Util::Render::Stereo_LeftRight_Multipass);
	stereo.SetHMDInfo(hmdInfo);
    if (hmdInfo.HScreenSize > 0.0f)
    {
        if (hmdInfo.HScreenSize > 0.140f) // 7"
            stereo.SetDistortionFitPointVP(-1.0f, 0.0f);
        else
            stereo.SetDistortionFitPointVP(0.0f, 1.0f);
    }

	renderScale = stereo.GetDistortionScale();
	
	//account for render scale?
	float w = hmdInfo.HResolution;
	float h = hmdInfo.VResolution;
	
	renderTarget.allocate(w, h, GL_RGB, 4);
    backgroundTarget.allocate(w/2, h);
    overlayTarget.allocate(256, 256, GL_RGBA);
	
	backgroundTarget.begin();
    ofClear(0.0, 0.0, 0.0);
	backgroundTarget.end();
	
	overlayTarget.begin();
	ofClear(0.0, 0.0, 0.0);
	overlayTarget.end();
	
	//left eye
	leftEyeMesh.addVertex(ofVec3f(0,0,0));
	leftEyeMesh.addTexCoord(ofVec2f(0,h));
	
	leftEyeMesh.addVertex(ofVec3f(0,h,0));
	leftEyeMesh.addTexCoord(ofVec2f(0,0));
	
	leftEyeMesh.addVertex(ofVec3f(w/2,0,0));
	leftEyeMesh.addTexCoord(ofVec2f(w/2,h));

	leftEyeMesh.addVertex(ofVec3f(w/2,h,0));
	leftEyeMesh.addTexCoord(ofVec2f(w/2,0));
	
	leftEyeMesh.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);
	
	//Right eye
	rightEyeMesh.addVertex(ofVec3f(w/2,0,0));
	rightEyeMesh.addTexCoord(ofVec2f(w/2,h));

	rightEyeMesh.addVertex(ofVec3f(w/2,h,0));
	rightEyeMesh.addTexCoord(ofVec2f(w/2,0));

	rightEyeMesh.addVertex(ofVec3f(w,0,0));
	rightEyeMesh.addTexCoord(ofVec2f(w,h));
	
	rightEyeMesh.addVertex(ofVec3f(w,h,0));
	rightEyeMesh.addTexCoord(ofVec2f(w,0));
	
	rightEyeMesh.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);
	
	reloadShader();
	
	bSetup = true;
	return true;
}

bool ofxOculusRift::isSetup(){
	return bSetup;
}

void ofxOculusRift::setupEyeParams(OVR::Util::Render::StereoEye eye){
	
	OVR::Util::Render::StereoEyeParams eyeRenderParams = stereo.GetEyeRenderParams( eye );
	OVR::Util::Render::Viewport VP = eyeRenderParams.VP;

	if(bUseBackground){
		glPushAttrib(GL_ALL_ATTRIB_BITS);
		glDisable(GL_LIGHTING);
		glDisable(GL_DEPTH_TEST);
		backgroundTarget.getTextureReference().draw(toOf(VP));
		glPopAttrib();
	}
	
	ofSetMatrixMode(OF_MATRIX_PROJECTION);
	ofLoadIdentityMatrix();
	
	ofMatrix4x4 projectionMatrix = toOf(eyeRenderParams.Projection);

	ofLoadMatrix( projectionMatrix );
	
	ofSetMatrixMode(OF_MATRIX_MODELVIEW);
	ofLoadIdentityMatrix();
	
	
	if(bUsePredictedOrientation){
		//headRotation = toOf(Matrix4f(pFusionResult->GetPredictedOrientation()));
		orientationMatrix = toOf(Matrix4f(pFusionResult->GetPredictedOrientation()));
	}
	else{
		//headRotation = toOf(Matrix4f(pFusionResult->GetOrientation()));
		orientationMatrix = toOf(Matrix4f(pFusionResult->GetOrientation()));
	}
	
	ofMatrix4x4 headRotation = orientationMatrix;
	if(baseCamera != NULL){
		headRotation = headRotation * baseCamera->getGlobalTransformMatrix();
		baseCamera->begin();
		baseCamera->end();

	}
	
	// lock the camera when enabled...
	if (!lockView) {
		ofLoadMatrix( ofMatrix4x4::getInverseOf( headRotation ));
	}
	
	ofViewport(toOf(VP));
	ofMatrix4x4 viewAdjust = toOf(eyeRenderParams.ViewAdjust);
	ofMultMatrix(viewAdjust);
	
}

ofRectangle ofxOculusRift::getOculusViewport(){
	OVR::Util::Render::StereoEyeParams eyeRenderParams = stereo.GetEyeRenderParams( OVR::Util::Render::StereoEye_Left );
	return toOf(eyeRenderParams.VP);
}

void ofxOculusRift::reloadShader(){
	//this allows you to hack on the shader if you'd like
	if(ofFile("Shaders/HmdWarp.vert").exists() && ofFile("Shaders/HmdWarp.frag").exists()){
		distortionShader.load("Shaders/HmdWarp");
	}
	//otherwise we load the hardcoded one
	else{
		distortionShader.setupShaderFromSource(GL_VERTEX_SHADER, OculusWarpVert);
		distortionShader.setupShaderFromSource(GL_FRAGMENT_SHADER, OculusWarpFrag);
		distortionShader.linkProgram();
	}
}


void ofxOculusRift::beginBackground(){
	bUseBackground = true;
    backgroundTarget.begin();
    ofClear(0.0, 0.0, 0.0);
    ofPushView();
    ofPushMatrix();
    ofViewport(getOculusViewport());
    
}

void ofxOculusRift::endBackground(){
    ofPopMatrix();
    ofPopView();
    backgroundTarget.end();
}


void ofxOculusRift::beginOverlay(float overlayZ, float width, float height){
	bUseOverlay = true;
	overlayZDistance = overlayZ;
	
	if(overlayTarget.getWidth() != width || overlayTarget.getHeight() != height){
		overlayTarget.allocate(width, height, GL_RGBA, 4);
		overlayTarget.begin();
		ofClear(0,0,0,0.0);
		overlayTarget.end();
	}
	
	overlayMesh.clear();
	ofRectangle overlayrect = ofRectangle(-width/2,-height/2,width,height);
	overlayMesh.addVertex( ofVec3f(overlayrect.getMinX(), overlayrect.getMinY(), overlayZ) );
	overlayMesh.addVertex( ofVec3f(overlayrect.getMaxX(), overlayrect.getMinY(), overlayZ) );
	overlayMesh.addVertex( ofVec3f(overlayrect.getMinX(), overlayrect.getMaxY(), overlayZ) );
	overlayMesh.addVertex( ofVec3f(overlayrect.getMaxX(), overlayrect.getMaxY(), overlayZ) );

	overlayMesh.addTexCoord( ofVec2f(0, height ) );
	overlayMesh.addTexCoord( ofVec2f(width, height) );
	overlayMesh.addTexCoord( ofVec2f(0,0) );
	overlayMesh.addTexCoord( ofVec2f(width, 0) );
	
	overlayMesh.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);
	
	overlayTarget.begin();
    ofClear(0.0, 0.0, 0.0, 0.0);
	
    ofPushView();
    ofPushMatrix();
}

void ofxOculusRift::endOverlay(){
    ofPopMatrix();
    ofPopView();
    overlayTarget.end();
}

void ofxOculusRift::beginLeftEye(){
	
	if(!bSetup) return;
	
	renderTarget.begin();
	ofClear(0,0,0);
	ofPushView();
	ofPushMatrix();
	setupEyeParams(OVR::Util::Render::StereoEye_Left);	
}

void ofxOculusRift::endLeftEye(){
	if(!bSetup) return;
	
	if(bUseOverlay){
		renderOverlay();
	}
	
	ofPopMatrix();
	ofPopView();
}

void ofxOculusRift::beginRightEye(){
	if(!bSetup) return;
	
	ofPushView();
	ofPushMatrix();
	
	setupEyeParams(OVR::Util::Render::StereoEye_Right);
}

void ofxOculusRift::endRightEye(){
	if(!bSetup) return;

	if(bUseOverlay){
		renderOverlay();
	}

	ofPopMatrix();
	ofPopView();
	renderTarget.end();	
}

void ofxOculusRift::renderOverlay(){

//	cout << "renering overlay!" << endl;
	
	ofPushStyle();
	ofPushMatrix();
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	
	if(baseCamera != NULL){
		ofTranslate(baseCamera->getPosition());
		ofMatrix4x4 baseRotation;
		baseRotation.makeRotationMatrix(baseCamera->getOrientationQuat());
		if(lockView){
			ofMultMatrix(baseRotation);
		}
		else {
			ofMultMatrix(orientationMatrix*baseRotation);
		}
	}
	
	ofEnableAlphaBlending();
	overlayTarget.getTextureReference().bind();
	overlayMesh.draw();
	overlayTarget.getTextureReference().unbind();
	
	glPopAttrib();
	ofPopMatrix();
	ofPopStyle();

}

//TODO head orientation not considered
ofVec3f ofxOculusRift::worldToScreen(ofVec3f worldPosition, bool considerHeadOrientation){

	if(baseCamera == NULL){
		return ofVec3f(0,0,0);
	}
	ofRectangle viewport = getOculusViewport();
	return baseCamera->worldToScreen(worldPosition, viewport);
}

//TODO head orientation not considered
ofVec3f ofxOculusRift::screenToWorld(ofVec3f screenPt, bool considerHeadOrientation) {

	if(baseCamera == NULL){
		return ofVec3f(0,0,0);
	}
    
    ofVec3f oculus2DPt = screenToOculus2D(screenPt, considerHeadOrientation);
    ofRectangle viewport = getOculusViewport();
    return baseCamera->screenToWorld(oculus2DPt, viewport);
}

//TODO head orientation not considered
ofVec3f ofxOculusRift::screenToOculus2D(ofVec3f screenPt, bool considerHeadOrientation){

	ofRectangle viewport = getOculusViewport();
//  viewport.x -= viewport.width  / 2;
//	viewport.y -= viewport.height / 2;
	viewport.scaleFromCenter(oculusScreenSpaceScale);
    return ofVec3f(ofMap(screenPt.x, 0, ofGetWidth(),  viewport.getMinX(), viewport.getMaxX()),
                   ofMap(screenPt.y, 0, ofGetHeight(), viewport.getMinY(), viewport.getMaxY()),
                   screenPt.z);    
}

//TODO: head position!
ofVec3f ofxOculusRift::mousePosition3D(float z, bool considerHeadOrientation){
//	ofVec3f cursor3D = screenToWorld(cursor2D);
	return screenToWorld(ofVec3f(ofGetMouseX(), ofGetMouseY(), z) );
}

float ofxOculusRift::distanceFromMouse(ofVec3f worldPoint){
	//map the current 2D position into oculus space
	return distanceFromScreenPoint(worldPoint, ofVec3f(ofGetMouseX(), ofGetMouseY()) );
}

float ofxOculusRift::distanceFromScreenPoint(ofVec3f worldPoint, ofVec2f screenPoint){
	ofVec3f cursorRiftSpace = screenToOculus2D(screenPoint);
	ofVec3f targetRiftSpace = worldToScreen(worldPoint);
	
	float dist = ofDist(cursorRiftSpace.x, cursorRiftSpace.y,
						targetRiftSpace.x, targetRiftSpace.y);
	return dist;
}


void ofxOculusRift::multBillboardMatrix(){
	if(baseCamera == NULL){
		return;
	}
	ofVec3f mouse3d = mousePosition3D();
	ofNode n;
	n.setPosition(  mouse3d );
	n.lookAt(baseCamera->getPosition());
	ofVec3f axis; float angle;
	n.getOrientationQuat().getRotate(angle, axis);
	// Translate the object to its position.
	ofTranslate( mouse3d );
	// Perform the rotation.
	ofRotate(angle, axis.x, axis.y, axis.z);
}

void ofxOculusRift::draw(){
	
	if(!bSetup) return;
	
	distortionShader.begin();
	distortionShader.setUniformTexture("Texture0", renderTarget.getTextureReference(), 0);
	distortionShader.setUniform2f("dimensions", renderTarget.getWidth(), renderTarget.getHeight());
	const OVR::Util::Render::DistortionConfig& distortionConfig = stereo.GetDistortionConfig();
    distortionShader.setUniform4f("HmdWarpParam",
								  distortionConfig.K[0],
								  distortionConfig.K[1],
								  distortionConfig.K[2],
								  distortionConfig.K[3]);
    distortionShader.setUniform4f("ChromAbParam",
                                  distortionConfig.ChromaticAberration[0],
                                  distortionConfig.ChromaticAberration[1],
                                  distortionConfig.ChromaticAberration[2],
                                  distortionConfig.ChromaticAberration[3]);
	
	setupShaderUniforms(OVR::Util::Render::StereoEye_Left);
	leftEyeMesh.draw();
	
	setupShaderUniforms(OVR::Util::Render::StereoEye_Right);
	rightEyeMesh.draw();

	distortionShader.end();
	
	bUseOverlay = false;
	bUseBackground = false;
}

void ofxOculusRift::setupShaderUniforms(OVR::Util::Render::StereoEye eye){

	float w = .5;
	float h = 1.0;
	float y = 0;
	float x;
	float xCenter;
	const OVR::Util::Render::DistortionConfig& distortionConfig = stereo.GetDistortionConfig();
	if(eye == OVR::Util::Render::StereoEye_Left){
		x = 0;
		xCenter = distortionConfig.XCenterOffset;
	}
	else if(eye == OVR::Util::Render::StereoEye_Right){
		x = .5;
		xCenter = -distortionConfig.XCenterOffset;
	}
    	
    float as = float(renderTarget.getWidth())/float(renderTarget.getHeight())*.5;
    // We are using 1/4 of DistortionCenter offset value here, since it is
    // relative to [-1,1] range that gets mapped to [0, 0.5].
	ofVec2f lensCenter(x + (w + xCenter * 0.5f)*0.5f,
					   y + h*0.5f);
	
    distortionShader.setUniform2f("LensCenter", lensCenter.x, lensCenter.y);
	
	ofVec2f screenCenter(x + w*0.5f, y + h*0.5f);
    distortionShader.setUniform2f("ScreenCenter", screenCenter.x,screenCenter.y);
	
    // MA: This is more correct but we would need higher-res texture vertically; we should adopt this
    // once we have asymmetric input texture scale.
    float scaleFactor = 1.0f / distortionConfig.Scale;
	//	cout << "scale factor " << scaleFactor << endl;
	
	ofVec2f scale( (w/2) * scaleFactor, (h/2) * scaleFactor * as);
	ofVec2f scaleIn( (2/w), (2/h) / as);
	
    distortionShader.setUniform2f("Scale", scale.x,scale.y);
    distortionShader.setUniform2f("ScaleIn",scaleIn.x,scaleIn.y);

//	cout << "UNIFORMS " << endl;
//	cout << "	scale " << scale << endl;
//	cout << "	scale in " << scaleIn << endl;
//	cout << "	screen center " << screenCenter << endl;
//	cout << "	lens center " << lensCenter << endl;
//	cout << "	scale factor " << scaleFactor << endl;
}

void ofxOculusRift::setUsePredictedOrientation(bool usePredicted){
	bUsePredictedOrientation = usePredicted;
}


