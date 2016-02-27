#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxAssimpModelLoader.h"
#include "ofxDelaunay.h"

class ofApp : public ofBaseApp{

public:
	void setup();
	void update();
	void draw();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);

	void loadShaders();

protected:
	struct Particle
	{
		ofVec4f pos;
		ofFloatColor color;
		ofVec4f normal;
	};

private:
	enum {
		WIDTH = 1280,
		HEIGHT = 720,
        FBO_SIZE = 1024
	};
	
	bool bDebugVisible = true;
	std::shared_ptr<ofxGuiGroup> mGui;

	ofParameterGroup mUniforms;
	ofParameter<float> uElapsedTime;

	ofParameterGroup gSettings;
	ofParameter<float> gPercentage;
	
	std::shared_ptr<ofFbo> mFbo;
	std::shared_ptr<ofShader> mShader;
	std::shared_ptr<ofEasyCam> mCamera;

	std::shared_ptr<ofxAssimpModelLoader> mModel;

	std::shared_ptr<ofVbo> mVbo;
	
	std::vector<Particle> mParticles;
	ofBufferObject particleBuffer;
	std::vector<int> mIndices;
	ofBufferObject indicesBuffer;

	std::shared_ptr<ofxDelaunay> triangulation;
};
