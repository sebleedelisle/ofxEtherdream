#include "ofxEtherdream.h"

//--------------------------------------------------------------
void ofxEtherdream::setup(bool bStartThread, int idEtherdream) {
    
    idEtherdreamConnection = idEtherdream;
    
    startEtherdreamLib();
	
	if(thread.isRunning()) {
		stopThread();
		bStartThread = true; 
	}
    setPPS(30000);
    setWaitBeforeSend(false);
    
    init();
    
    idEtherdreamConnection = getEtherdreamId();
    
    if(bStartThread) start();
}


//--------------------------------------------------------------
bool ofxEtherdream::stateIsFound() {
    return state == ETHERDREAM_FOUND;
}

//--------------------------------------------------------------
bool ofxEtherdream::checkConnection(bool bForceReconnect) {
    if((device==NULL) || (device->state == ST_SHUTDOWN || device->state == ST_BROKEN || device->state == ST_DISCONNECTED)) {
        
        if(bForceReconnect) {
            kill();
            setup(true, idEtherdreamConnection);
        }
        
        return false;
    }
    return true;
}

//--------------------------------------------------------------
void ofxEtherdream::init() {
//    int device_num = etherdream_dac_count();
//	if (!device_num || idEtherdreamConnection>device_num) {
//		ofLogWarning() << "ofxEtherdream::init - No DACs found";
//		return 0;
//	}
    
//	for (int i=0; i<device_num; i++) {
//		ofLogNotice() << "ofxEtherdream::init - " << i << " Ether Dream " << etherdream_get_id(etherdream_get(i));
//    }
    
    ofLogNotice("ofxEtherdream::init - initializing etherdream "+ofToString(idEtherdreamConnection));
    
    device = etherdream_get(idEtherdreamConnection);
    
    ofLogNotice() << "ofxEtherdream::init - Connecting...";
    if (device == NULL || etherdream_connect(device) < 0) {
        ofLogWarning() << "ofxEtherdream::init - No DACs found";
        return 1;
    }

    ofLogNotice() << "ofxEtherdream::init - done";
    
    state = ETHERDREAM_FOUND;
}

//--------------------------------------------------------------
void ofxEtherdream::threadedFunction() {
    while (isThreadRunning() != 0) {
        
        switch (state) {
            case ETHERDREAM_NOTFOUND:
                if(bAutoConnect) init();
                break;
                
            case ETHERDREAM_FOUND:
                if(lock()) {
                    send();
                    unlock();
                }
                break;
        }
    }
}

//--------------------------------------------------------------
void ofxEtherdream::start() {
    startThread(true);  // TODO: blocking or nonblocking?
}

//--------------------------------------------------------------
void ofxEtherdream::stop() {
    stopThread();
}

//--------------------------------------------------------------
void ofxEtherdream::send() {
    if(!stateIsFound() || points.empty()) return;
    
    if(bWaitBeforeSend) etherdream_wait_for_ready(device);
    else if(!etherdream_is_ready(device)) return;
    
    // DODGY HACK: casting ofxIlda::Point* to etherdream_point*
    int res = etherdream_write(device, (etherdream_point*)points.data(), points.size(), pps, 1);
    if (res != 0) {
        ofLogVerbose() << "ofxEtherdream::write " << res;
    }
    points.clear();
}


//--------------------------------------------------------------
void ofxEtherdream::clear() {
    if(lock()) {
        points.clear();
        unlock();
    }
}

//--------------------------------------------------------------
void ofxEtherdream::addPoints(const vector<ofxIlda::Point>& _points) {
    if(lock()) {
        if(!_points.empty()) {
            points.insert(points.end(), _points.begin(), _points.end());
        }
        unlock();
    }
}

//
////--------------------------------------------------------------
//void ofxEtherdream::addPoints(const ofxIlda::Frame &ildaFrame) {
//    addPoints(ildaFrame.getPoints());
//}
//

//--------------------------------------------------------------
void ofxEtherdream::setPoints(const vector<ofxIlda::Point>& _points) {
    if(lock()) {
        points = _points;
        unlock();
    }
}

//
////--------------------------------------------------------------
//void ofxEtherdream::setPoints(const ofxIlda::Frame &ildaFrame) {
//    setPoints(ildaFrame.getPoints());
//}

//--------------------------------------------------------------
void ofxEtherdream::setWaitBeforeSend(bool b) {
    if(lock()) {
        bWaitBeforeSend = b;
        unlock();
    }
}

//--------------------------------------------------------------
bool ofxEtherdream::getWaitBeforeSend() const {
    return bWaitBeforeSend;
}


//--------------------------------------------------------------
void ofxEtherdream::setPPS(int i) {
    if(lock()) {
        pps = i;
        unlock();
    }
}

//--------------------------------------------------------------
int ofxEtherdream::getPPS() const {
    return pps;
}

unsigned long ofxEtherdream::getEtherdreamId(){
    return etherdream_get_id(device);
}
