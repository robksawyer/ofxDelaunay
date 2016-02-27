#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetWindowShape(WIDTH, HEIGHT);
	ofSetWindowPosition((ofGetScreenWidth() - ofGetWidth()) / 2, (ofGetScreenHeight() - ofGetHeight()) / 2);
	ofSetFrameRate(60);
	ofDisableArbTex();
	//ofSetVerticalSync(true);

	auto loadModel = [&]()
	{
		mModel = shared_ptr<ofxAssimpModelLoader>(new ofxAssimpModelLoader);
		mModel->loadModel("20160224.ply");

		auto& mesh = mModel->getMesh(0);
		cout << "Vertices: " << mesh.getNumVertices() << endl; // 207582
		cout << "Indices: " << mesh.getNumIndices() << endl; // 1125891
		cout << "Colors: " << mesh.getNumColors() << endl; // 207582
		cout << "Normals: " << mesh.getNumNormals() << endl; // 207582
		cout << "TexCoords: " << mesh.getNumTexCoords() << endl; // 0
	};
    
	std::thread t(loadModel);

    {
        ofFbo::Settings s;
        s.width = s.height = FBO_SIZE;
		s.numSamples = ofFbo::maxSamples();
        s.useDepth = true;
        s.colorFormats.emplace_back(GL_RGBA);
        
        mFbo = shared_ptr<ofFbo>(new ofFbo);
        mFbo->allocate(s);
    }

	{
		mCamera = shared_ptr<ofEasyCam>(new ofEasyCam);
		mCamera->setupPerspective(true, 45, 1, 1500);
		mCamera->setAspectRatio(1.0f);
		mCamera->setDistance(750);
	}
    
    {
        mUniforms.setName("Uniforms");
        mUniforms.add(uElapsedTime.set("uElapsedTime", ofGetElapsedTimef()));

		gSettings.setName("Settings");
		gSettings.add(gPercentage.set("Percentage", 2.0f, 0.1f, 2.0f));
        
        mGui = shared_ptr<ofxGuiGroup>(new ofxGuiGroup);
        mGui->setup("GUI");
        mGui->add(mUniforms);
		mGui->add(gSettings);
    }

	t.join();
	{
		auto& mesh = mModel->getMesh(0);
		for (size_t i = 0; i < mesh.getNumVertices(); i++)
		{
			Particle p;
			p.pos = mesh.getVertex(i);
			p.pos.w = 1.0f;
			p.color = mesh.getColor(i);
			p.normal = mesh.getNormal(i);
			mParticles.emplace_back(p);
		}
		particleBuffer.allocate();
		particleBuffer.setData(sizeof(Particle) * mParticles.size(), &mParticles.front(), GL_DYNAMIC_DRAW);

		for (size_t i = 0; i < mesh.getNumIndices(); i++)
		{
			mIndices.emplace_back(mesh.getIndex(i));
		}
		indicesBuffer.allocate();
		indicesBuffer.setData(sizeof(int) * mIndices.size(), &mIndices.front(), GL_DYNAMIC_DRAW);
	}
	
	{
		mVbo = shared_ptr<ofVbo>(new ofVbo);
		mVbo->setVertexBuffer(particleBuffer, 4, sizeof(Particle));
		mVbo->setColorBuffer(particleBuffer, sizeof(Particle), sizeof(ofVec4f) * 1);
		mVbo->setNormalBuffer(particleBuffer, sizeof(Particle), sizeof(ofVec4f) * 2);
		mVbo->setIndexBuffer(indicesBuffer);

		particleBuffer.bindBase(GL_SHADER_STORAGE_BUFFER, 0);
	}

	{
		triangulation = shared_ptr<ofxDelaunay>(new ofxDelaunay);
	}

	loadShaders();
}

//--------------------------------------------------------------
void ofApp::update(){
	ofSetWindowTitle("oF Application: " + ofToString(ofGetFrameRate(), 1));
	uElapsedTime = ofGetElapsedTimef();
	
	float inc = 100.0f / gPercentage;
	auto& mesh = mModel->getMesh(0);
	mParticles.clear();
	for (float k = 0.0f; k < mesh.getNumVertices(); k += inc)
	{
		size_t i = k;
		Particle p;
		p.pos = mesh.getVertex(i);
		p.pos.w = 1.0f;
		p.color = mesh.getColor(i);
		//float hue = k / mesh.getNumVertices();
		//p.color = ofFloatColor::fromHsb(hue, 0.7, 0.9);
		//p.color = ofFloatColor(1);
		p.normal = mesh.getNormal(i);
		mParticles.emplace_back(p);
	}
	particleBuffer.updateData(mParticles);

	if (mParticles.size() < 5000)
	{
		triangulation->reset();
		for (auto& p : mParticles)
			triangulation->addPoint(p.pos);
#if 0
		triangulation->triangulate();

		mIndices.clear();
		for (auto& i : triangulation->triangleMesh.getIndices())
			mIndices.emplace_back(i);
#else
		mIndices.clear();
		mIndices = triangulation->getTriangulatedIndices();
#endif
		indicesBuffer.updateData(mIndices);
	}

	cout << mVbo->getNumVertices() << " : " << mParticles.size() << " : " << mIndices.size() << endl;

	ofEnableDepthTest();
	ofPushStyle();
	ofSetColor(255);

	mFbo->begin();
	ofClear(0);
    if (mShader)
    {
        mShader->begin();
        mShader->setUniforms(mUniforms);
    }
	
    if (mShader)
    {
        mShader->end();
    }

	mCamera->begin();

	ofPushMatrix();
	ofRotateZ(90);
	ofRotateX(180);

	ofEnableBlendMode(OF_BLENDMODE_ALPHA);
	ofFill();
	mVbo->drawElements(GL_TRIANGLES, mIndices.size());

	ofEnableBlendMode(OF_BLENDMODE_ADD);
	ofNoFill();
	mVbo->drawElements(GL_TRIANGLES, mIndices.size());

	ofPopMatrix();

	mCamera->end();
	
	mFbo->end();
	ofPopStyle();
}

//--------------------------------------------------------------
void ofApp::draw(){
	ofClear(0);

	mFbo->draw(0, (ofGetHeight() - ofGetWidth()) / 2, ofGetWidth(), ofGetWidth());

	if (bDebugVisible)
	{
		ofDisableDepthTest();
		ofEnableBlendMode(OF_BLENDMODE_ALPHA);
		mGui->draw();
	}
		
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	auto toggleFullscreen = [&]()
	{
		ofToggleFullscreen();
		if (!(ofGetWindowMode() == OF_FULLSCREEN))
		{
			ofSetWindowShape(WIDTH, HEIGHT);
			auto pos = ofVec2f(ofGetScreenWidth() - WIDTH, ofGetScreenHeight() - HEIGHT) * 0.5f;
			ofSetWindowPosition(pos.x, pos.y);
		}
	};
	
	switch (key)
	{
	case OF_KEY_F1:
		bDebugVisible = !bDebugVisible;
		break;
	case OF_KEY_F5:
		loadShaders();
		break;
	case OF_KEY_F11:
		toggleFullscreen();
		break;
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}

void ofApp::loadShaders()
{
	printf("%s load shaders\n", ofGetTimestampString().c_str());

	mShader.reset();
	mShader = shared_ptr<ofShader>(new ofShader);
	mShader->setupShaderFromFile(GL_VERTEX_SHADER, "shaders/basic.vert");
	mShader->setupShaderFromFile(GL_FRAGMENT_SHADER, "shaders/basic.frag");
	mShader->linkProgram();
}