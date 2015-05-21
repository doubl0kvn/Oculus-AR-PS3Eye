#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup()
{
    ofSetFrameRate(60);
    
    eyeHeight = ofGetWindowWidth() / 2;
    eyeWidth = ofGetWindowHeight();
    
    camWidth = 800;
    camHeight = 640;
    
    camWidth = eyeWidth;
    camHeight = eyeHeight;
    switchInt = 0;
    
    tex.resize(2);
    fbo.resize(2);
    
    ofVideoGrabber vid;
    vid.initGrabber(camWidth, camHeight);
    vector< ofVideoDevice > devices = vid.listDevices();
    for(int i = 0; i < devices.size(); i++) {
        cout << "Device ID: " << devices[i].id << " Device Name: " << devices[i].deviceName << " Hardware Name: " << devices[i].hardwareName << " Is Available: " << devices[i].bAvailable << endl;
    }
    
    
    
    //Choosing cameras on assumption that left camera plugged in first, then the right.  Also assumes that the computer only has 1 camera by default
    vidGrabber[0].setDeviceID(1);
    vidGrabber[1].setDeviceID(2);
    for(int i = 0; i < 2; i++)
    {
        vidGrabber[i].setDesiredFrameRate(15);
        vidGrabber[i].initGrabber(camWidth, camHeight);
        
        tex.at(i).setTextureWrap(GL_REPEAT, GL_REPEAT);
        tex.at(i).allocate(camHeight,camWidth,GL_RGB,GL_RGBA);
        tbIm[i].allocate(camHeight,camWidth,OF_IMAGE_COLOR_ALPHA);
        fbo.at(i).allocate(640,800);
        sleep(5);
    }
    
    
    //There are the parameters for the polynomial warp function to correct for the Oculus Rift and Webcam Lenses
    K0 = 1.0;
    K1 = 5.74;
    K2 = 0.27;
    K3 = 0.0;
    _x = 0.0f;
    _y = 0.0f;
    _w = 1.0f;
    _h = 1.0f;
    as = 640.0f/800.0f;
    DistortionXCenterOffset = 90;
    
    ofEnableNormalizedTexCoords();
    
    hmdWarpShader.load("shaders/HmdWarpExp");
}

//--------------------------------------------------------------
void ofApp::update(){
    
    eyeHeight = ofGetWindowWidth() / 2;
    eyeWidth = ofGetWindowHeight();
    camWidth = eyeWidth;
    camHeight = eyeHeight;
    
    int i = 0;
    vidGrabber[i].update();
    
    if (vidGrabber[i].isFrameNew())
    {
        for(int j = 0; j < camHeight; j++)
        {
            for(int k = 0; k < camWidth; k++)
            {
                tbIm[i].setColor(j,k,vidGrabber[i].getPixelsRef().getColor(k, j));
            }
        }
    }
    tbIm[i].reloadTexture();
    tbIm[i].mirror(true,true);
    
    tex.at(i) = tbIm[i].getTextureReference();
    
    i++;
    vidGrabber[i].update();
    
    if (vidGrabber[i].isFrameNew())
    {
        for(int j = 0; j < camHeight; j++)
        {
            for(int k = 0; k < camWidth; k++)
            {
                tbIm[i].setColor(j,k,vidGrabber[i].getPixelsRef().getColor(k, j));
            }
        }
    }
    tbIm[i].reloadTexture();
    tex.at(i) = tbIm[i].getTextureReference();
}

//--------------------------------------------------------------
void ofApp::draw()
{
    eyeHeight = ofGetWindowWidth() / 2;
    eyeWidth = ofGetWindowHeight();
    camWidth = eyeWidth;
    camHeight = eyeHeight;
    
    fbo.at(0).getTextureReference().bind();
    
    hmdWarpShader.begin();
    hmdWarpShader.setUniformTexture("tex", fbo.at(0).getTextureReference(), 0);
    hmdWarpShader.setUniform2f("LensCenter", DistortionXCenterOffset, 0 );
    hmdWarpShader.setUniform2f("ScreenCenter", _x + _w*1.0f, _y + _h*1.0f );
    hmdWarpShader.setUniform2f("Scale", (_w/1.0f) * 1.0f, (_h/1.0f) * 1.0f * as );
    hmdWarpShader.setUniform2f("ScaleIn", (1.0f/_w), (1.0f/_h) / as );
    hmdWarpShader.setUniform4f("HmdWarpParam", K0, K1, K2, K3 );
    
    glBegin(GL_QUADS);
    glTexCoord2f(1,0); glVertex3f(0,0,0);
    glTexCoord2f(0,0); glVertex3f(camHeight,0,0);
    glTexCoord2f(0,1); glVertex3f(camHeight,camWidth,0);
    glTexCoord2f(1,1); glVertex3f(0,camWidth,0);
    glEnd();
    
    hmdWarpShader.end();
    
    fbo.at(0).getTextureReference().unbind();
    
    // -----------
    
    fbo.at(1).getTextureReference().bind();
    
    hmdWarpShader.begin();
    hmdWarpShader.setUniformTexture("tex", fbo.at(1).getTextureReference(), 0);
    hmdWarpShader.setUniform2f("LensCenter", DistortionXCenterOffset , 1);
    hmdWarpShader.setUniform2f("ScreenCenter", _x + _w*1.0f, _y + _h*1.0f );
    hmdWarpShader.setUniform2f("Scale", (_w/1.0f) * 1.0f, (_h/1.0f) * 1.0f * as );
    hmdWarpShader.setUniform2f("ScaleIn", (1.0f/_w), (1.0f/_h) / as );
    hmdWarpShader.setUniform4f("HmdWarpParam", K0, K1, K2, K3 );
    
    ofPushMatrix();
    ofTranslate(camHeight,0);
    glBegin(GL_QUADS);
    glBegin(GL_QUADS);
    glTexCoord2f(1,0); glVertex3f(0,0,0);
    glTexCoord2f(0,0); glVertex3f(camHeight,0,0);
    glTexCoord2f(0,1); glVertex3f(camHeight,camWidth,0);
    glTexCoord2f(1,1); glVertex3f(0,camWidth,0);
    glEnd();
    
    glEnd();
    ofPopMatrix();
    hmdWarpShader.end();
    fbo.at(1).getTextureReference().unbind();
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key)
{
    if(key == 'f')
    {
        ofLogNotice("Starting FUll SCREEN");
        ofToggleFullscreen();
        eyeHeight = ofGetWindowWidth() / 2;
        eyeWidth = ofGetWindowHeight();
        camWidth = eyeWidth;
        camHeight = eyeHeight;
        
        for(int i = 0; i < 2; i++)
        {
            vidGrabber[i].initGrabber(camWidth, camHeight);
            
            tex.at(i).setTextureWrap(GL_REPEAT, GL_REPEAT);
            tex.at(i).allocate(camHeight,camWidth,GL_RGB,GL_RGBA);
            tbIm[i].allocate(camHeight,camWidth,OF_IMAGE_COLOR_ALPHA);
            fbo.at(i).allocate(640,800);
            sleep(5);
        }
    }
}
