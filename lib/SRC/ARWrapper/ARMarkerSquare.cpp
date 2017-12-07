/*
 *  ARMarkerSquare.cpp
 *  ARToolKit5
 *
 *  This file is part of ARToolKit.
 *
 *  ARToolKit is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  ARToolKit is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with ARToolKit.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  As a special exception, the copyright holders of this library give you
 *  permission to link this library with independent modules to produce an
 *  executable, regardless of the license terms of these independent modules, and to
 *  copy and distribute the resulting executable under terms of your choice,
 *  provided that you also meet, for each linked independent module, the terms and
 *  conditions of the license of that module. An independent module is a module
 *  which is neither derived from nor based on this library. If you modify this
 *  library, you may extend this exception to your version of the library, but you
 *  are not obligated to do so. If you do not wish to do so, delete this exception
 *  statement from your version.
 *
 *  Copyright 2015 Daqri, LLC.
 *  Copyright 2010-2015 ARToolworks, Inc.
 *
 *  Author(s): Philip Lamb.
 *
 */

#include <ARWrapper/ARMarkerSquare.h>
#include <ARWrapper/ARController.h>
#ifndef MAX
#  define MAX(x,y) (x > y ? x : y)
#endif

ARMarkerSquare::ARMarkerSquare() : ARMarker(SINGLE),
    m_loaded(false),
    m_arPattHandle(NULL),
    m_cf(0.0f),
    m_cfMin(AR_CONFIDENCE_CUTOFF_DEFAULT),
    patt_id(-1),
    patt_type(-1),
    useContPoseEstimation(true)
{
}


ARMarkerSquare::~ARMarkerSquare()
{
    if (m_loaded) unload();
}

bool ARMarkerSquare::unload()
{
    if (m_loaded) {
        freePatterns();
        if (patt_type == AR_PATTERN_TYPE_TEMPLATE && patt_id != -1) {
            if (m_arPattHandle) {
                arPattFree(m_arPattHandle, patt_id);
                m_arPattHandle = NULL;
            }
        }
        patt_id = patt_type = -1;
        m_cf = 0.0f;
        m_width = 0.0f;
        m_loaded = false;
    }
    
    return (true);
}

bool ARMarkerSquare::initWithPatternFile(const char* path, ARdouble width, ARPattHandle *arPattHandle)
{
	// Ensure the pattern string is valid
	if (!path || !arPattHandle) return false;
    
    if (m_loaded) unload();

	ARController::logv(AR_LOG_LEVEL_INFO, "Loading single AR marker from file '%s', width %f.", path, width);
	
    m_arPattHandle = arPattHandle;
	patt_id = arPattLoad(m_arPattHandle, path);
	if (patt_id < 0) {
		ARController::logv(AR_LOG_LEVEL_ERROR, "Error: unable to load single AR marker from file '%s'.", path);
        arPattHandle = NULL;
		return false;
	}
	
    patt_type = AR_PATTERN_TYPE_TEMPLATE;
    m_width = width;
    
	visible = visiblePrev = false;
    
    // An ARPattern to hold an image of the pattern for display to the user.
	allocatePatterns(1);
	patterns[0]->loadTemplate(patt_id, m_arPattHandle, (float)m_width);

    m_loaded = true;
	return true;
}

bool ARMarkerSquare::initWithPatternFromBuffer(const char* buffer, ARdouble width, ARPattHandle *arPattHandle)
{
	// Ensure the pattern string is valid
	if (!buffer || !arPattHandle) return false;

    if (m_loaded) unload();

	ARController::logv(AR_LOG_LEVEL_INFO, "Loading single AR marker from buffer, width %f.", width);
	
    m_arPattHandle = arPattHandle;
	patt_id = arPattLoadFromBuffer(m_arPattHandle, buffer);
	if (patt_id < 0) {
		ARController::logv(AR_LOG_LEVEL_ERROR, "Error: unable to load single AR marker from buffer.");
		return false;
	}
	
    patt_type = AR_PATTERN_TYPE_TEMPLATE;
	m_width = width;

	visible = visiblePrev = false;
	
    // An ARPattern to hold an image of the pattern for display to the user.
	allocatePatterns(1);
	patterns[0]->loadTemplate(patt_id, arPattHandle, (float)m_width);
	
    m_loaded = true;
	return true;
}

bool ARMarkerSquare::initWithBarcode(int barcodeID, ARdouble width)
{
	if (barcodeID < 0) return false;
    
    if (m_loaded) unload();

	ARController::logv(AR_LOG_LEVEL_INFO, "Adding single AR marker with barcode %d, width %f.", barcodeID, width);
	
	patt_id = barcodeID;
	
    patt_type = AR_PATTERN_TYPE_MATRIX;
	m_width = width;
    
	visible = visiblePrev = false;
		
    // An ARPattern to hold an image of the pattern for display to the user.
	allocatePatterns(1);
	patterns[0]->loadMatrix(patt_id, AR_MATRIX_CODE_3x3, (float)m_width); // FIXME: need to determine actual matrix code type.

    m_loaded = true;
	return true;
}

//MOD
bool ARMarkerSquare::initWithBarcode_groups(int barcodeID, ARdouble width, int type)
{
	if (barcodeID < 0) return false;
    
    if (m_loaded) unload();

	ARController::logv(AR_LOG_LEVEL_INFO, "Adding single AR marker with barcode %d, width %f, type %d.", barcodeID, width, type);
	
	patt_id = barcodeID;
	
    patt_type = AR_PATTERN_TYPE_MATRIX;
	m_width = width;

    m_type = type;
    
	visible = visiblePrev = false;
		
    // An ARPattern to hold an image of the pattern for display to the user.
	allocatePatterns(1);
	patterns[0]->loadMatrix(patt_id, AR_MATRIX_CODE_3x3, (float)m_width); // FIXME: need to determine actual matrix code type.

    m_loaded = true;
	return true;
}

ARdouble ARMarkerSquare::getConfidence()
{
    return (m_cf);
}

ARdouble ARMarkerSquare::getConfidenceCutoff()
{
    return (m_cfMin);
}

void ARMarkerSquare::setConfidenceCutoff(ARdouble value)
{
    if (value >= AR_CONFIDENCE_CUTOFF_DEFAULT && value <= 1.0f) {
        m_cfMin = value;
    }
}

bool ARMarkerSquare::updateWithDetectedMarkers(ARMarkerInfo* markerInfo, int markerNum, AR3DHandle *ar3DHandle) {

    //ARController::logv("ARMarkerSquare::update()");
    
	if (patt_id < 0) return false;	// Can't update if no pattern loaded

    visiblePrev = visible;

	if (markerInfo) {

        int k = -1;
        if (patt_type == AR_PATTERN_TYPE_TEMPLATE) { 
            // Iterate over all detected markers.
            for (int j = 0; j < markerNum; j++ ) {
                if (patt_id == markerInfo[j].idPatt) {
                    // The pattern of detected trapezoid matches marker[k].
                    if (k == -1) {
                        if (markerInfo[j].cfPatt > m_cfMin) k = j; // Count as a match if match confidence exceeds cfMin.
                    } else if (markerInfo[j].cfPatt > markerInfo[k].cfPatt) k = j; // Or if it exceeds match confidence of a different already matched trapezoid (i.e. assume only one instance of each marker).
                }
            }
            if (k != -1) {
                markerInfo[k].id = markerInfo[k].idPatt;
                markerInfo[k].cf = markerInfo[k].cfPatt;
                markerInfo[k].dir = markerInfo[k].dirPatt;
            }
        } else {
            for (int j = 0; j < markerNum; j++) {
                if (patt_id == markerInfo[j].idMatrix) {
                    if (k == -1) {
                        if (markerInfo[j].cfMatrix >= m_cfMin) k = j; // Count as a match if match confidence exceeds cfMin.
                    } else if (markerInfo[j].cfMatrix > markerInfo[k].cfMatrix) k = j; // Or if it exceeds match confidence of a different already matched trapezoid (i.e. assume only one instance of each marker).
                }
            }
            if (k != -1) {
                markerInfo[k].id = markerInfo[k].idMatrix;
                markerInfo[k].cf = markerInfo[k].cfMatrix;
                markerInfo[k].dir = markerInfo[k].dirMatrix;
            }
        }
        
		// Consider marker visible if a match was found.
        if (k != -1) {
            visible = true;
            m_cf = markerInfo[k].cf;
			//MOD start
			//adds support for persistent marker visualization based on positional memory
			lastMarkerInfo = markerInfo[k];
			wasVisible = true;
			visCounter = MAX_VALID_FRAMES;
			//MOD end
            // If the model is visible, update its transformation matrix
			if (visiblePrev && useContPoseEstimation) {
				// If the marker was visible last time, use "cont" version of arGetTransMatSquare
				arGetTransMatSquareCont(ar3DHandle, &(markerInfo[k]), trans, m_width, trans);
			} else {
				// If the marker wasn't visible last time, use normal version of arGetTransMatSquare
				arGetTransMatSquare(ar3DHandle, &(markerInfo[k]), m_width, trans);
			}
        } else {
			//MOD start
			//for presistent marker visualization: checks if it's been too long since last time that marker was visible
			if (--visCounter == 0) {	
				wasVisible = false;		
			}
			//MOD end
			visible = false;
            m_cf = 0.0f;
        }

	} else {
		//MOD start
		//for presistent marker visualization: checks if it's been too long since last time that marker was visible
		//if (--visCounter == 0) {		
			wasVisible = false;			
		//}
		//MOD end
		visible = false;
        m_cf = 0.0f;
    }

	return (ARMarker::update()); // Parent class will finish update.
}
//MOD
bool ARMarkerSquare::updateWithMemoryMarkers(ARHandle* arHandle, int* markerNum, std::vector<ARMarkerInfo>& MIVector) {
	
	if (patt_id < 0) return false;
	if (arHandle) {
		if (!visible && wasVisible && (arHandle->marker2_num >= 1) && lastMarkerInfo.id != 0) {
			//ARMarkerInfo* markerInfoTemp = arGetMarker(arHandle);
			//int markerNumTemp = arGetMarkerNum(arHandle);
			////first time marker disappears log the median x and y of the markers that are still visible
			//
			//float median_x;
			//float median_y;
			//if (visCounter == MAX_VALID_FRAMES) {
			//	int valid_markers = 0;
			//	 median_x = 0;
			//	 median_y = 0;
			//	for (int i = 0; i < markerNumTemp; i++) {
			//		if (markerInfoTemp[i].id != -1) {
			//			median_x += markerInfoTemp[i].pos[0];
			//			median_y += markerInfoTemp[i].pos[1];
			//			++valid_markers;
			//		}
			//	}
			//	median_x = median_x / valid_markers;
			//	median_y = median_y / valid_markers;
			//}
			//if (visCounter == 1) {
			//	float current_median_x = 0;
			//	float current_median_y = 0;
			//	int valid_markers = 0;
			//	for (int i = 0; i < markerNumTemp; i++) {
			//		if (markerInfoTemp[i].id != -1) {
			//			current_median_x += markerInfoTemp[i].pos[0];
			//			current_median_y += markerInfoTemp[i].pos[1];
			//			++valid_markers;
			//		}
			//	}
			//	current_median_x = current_median_x / valid_markers;
			//	current_median_y = current_median_x / valid_markers;
			//	if (std::abs(median_x - current_median_x) < 200 || std::abs(median_y - current_median_y) < 200) {
			//		visCounter = MAX_VALID_FRAMES - 1;
			//		MIVector.push_back(lastMarkerInfo);
			//	}
			//}

			//SIMPLIFIED if there are atleast 2 markers still visible then show the rest
			visCounter = MAX_VALID_FRAMES - 1;
			MIVector.push_back(lastMarkerInfo);
		}
	}
	return true;
}

bool ARMarkerSquare::updateWithDetectedMarkersStereo(ARMarkerInfo* markerInfoL, int markerNumL, ARMarkerInfo* markerInfoR, int markerNumR, AR3DStereoHandle *handle, ARdouble transL2R[3][4]) {
    
    //ARController::logv("ARMarkerSquare::update()");
    
	if (patt_id < 0) return false;	// Can't update if no pattern loaded
    
    visiblePrev = visible;
    visible = false;
    m_cf = 0.0f;
    
	if (markerInfoL && markerInfoR) {
        
        int kL = -1, kR = -1;
        if (patt_type == AR_PATTERN_TYPE_TEMPLATE) {
            // Iterate over all detected markers.
            for (int j = 0; j < markerNumL; j++ ) {
                if (patt_id == markerInfoL[j].idPatt) {
                    // The pattern of detected trapezoid matches marker[kL].
                    if (kL == -1) {
                        if (markerInfoL[j].cfPatt > m_cfMin) kL = j; // Count as a match if match confidence exceeds cfMin.
                    } else if (markerInfoL[j].cfPatt > markerInfoL[kL].cfPatt) kL = j; // Or if it exceeds match confidence of a different already matched trapezoid (i.e. assume only one instance of each marker).
                }
            }
            if (kL != -1) {
                markerInfoL[kL].id = markerInfoL[kL].idPatt;
                markerInfoL[kL].cf = markerInfoL[kL].cfPatt;
                markerInfoL[kL].dir = markerInfoL[kL].dirPatt;
            }
            for (int j = 0; j < markerNumR; j++ ) {
                if (patt_id == markerInfoR[j].idPatt) {
                    // The pattern of detected trapezoid matches marker[kR].
                    if (kR == -1) {
                        if (markerInfoR[j].cfPatt > m_cfMin) kR = j; // Count as a match if match confidence exceeds cfMin.
                    } else if (markerInfoR[j].cfPatt > markerInfoR[kR].cfPatt) kR = j; // Or if it exceeds match confidence of a different already matched trapezoid (i.e. assume only one instance of each marker).
                }
            }
            if (kR != -1) {
                markerInfoR[kR].id = markerInfoR[kR].idPatt;
                markerInfoR[kR].cf = markerInfoR[kR].cfPatt;
                markerInfoR[kR].dir = markerInfoR[kR].dirPatt;
            }
        } else {
            for (int j = 0; j < markerNumL; j++) {
                if (patt_id == markerInfoL[j].idMatrix) {
                    if (kL == -1) {
                        if (markerInfoL[j].cfMatrix >= m_cfMin) kL = j; // Count as a match if match confidence exceeds cfMin.
                    } else if (markerInfoL[j].cfMatrix > markerInfoL[kL].cfMatrix) kL = j; // Or if it exceeds match confidence of a different already matched trapezoid (i.e. assume only one instance of each marker).
                }
            }
            if (kL != -1) {
                markerInfoL[kL].id = markerInfoL[kL].idMatrix;
                markerInfoL[kL].cf = markerInfoL[kL].cfMatrix;
                markerInfoL[kL].dir = markerInfoL[kL].dirMatrix;
            }
            for (int j = 0; j < markerNumR; j++) {
                if (patt_id == markerInfoR[j].idMatrix) {
                    if (kR == -1) {
                        if (markerInfoR[j].cfMatrix >= m_cfMin) kR = j; // Count as a match if match confidence exceeds cfMin.
                    } else if (markerInfoR[j].cfMatrix > markerInfoR[kR].cfMatrix) kR = j; // Or if it exceeds match confidence of a different already matched trapezoid (i.e. assume only one instance of each marker).
                }
            }
            if (kR != -1) {
                markerInfoR[kR].id = markerInfoR[kR].idMatrix;
                markerInfoR[kR].cf = markerInfoR[kR].cfMatrix;
                markerInfoR[kR].dir = markerInfoR[kR].dirMatrix;
            }
        }
        
        if (kL != -1 || kR != -1) {
            
            ARdouble err;
            
            if (kL != -1 && kR != -1) {
                err = arGetStereoMatchingErrorSquare(handle, &markerInfoL[kL], &markerInfoR[kR]);
                //ARLOG("stereo err = %f\n", err);
                if (err > 16.0) {
                    //ARLOG("Stereo matching error: %d %d.\n", markerInfoL[kL].area, markerInfoR[kR].area);
                    if (markerInfoL[kL].area > markerInfoR[kR].area ) kR = -1;
                    else                                              kL = -1;
                }
            }
            
            err = arGetTransMatSquareStereo(handle, (kL == -1 ? NULL : &markerInfoL[kL]), (kR == -1 ?  NULL : &markerInfoR[kR]), m_width, trans);
            if (err < 10.0) {
                visible = true;
                m_cf = MAX(markerInfoL[kL].cf, markerInfoR[kR].cf);
            }
            
            //if (kL == -1)      ARLOG("[%2d] right:      err = %f\n", patt_id, err);
            //else if (kR == -1) ARLOG("[%2d] left:       err = %f\n", patt_id, err);
            //else               ARLOG("[%2d] left+right: err = %f\n", patt_id, err);
        }
    }
    
	return (ARMarker::update(transL2R)); // Parent class will finish update.
}

