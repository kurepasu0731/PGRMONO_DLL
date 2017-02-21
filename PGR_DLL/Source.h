#pragma once


#include "PGROpenCV.h"
#include <opencv2/opencv.hpp>


#define DLLExport __declspec (dllexport)

#define PGRWIDTH 1920
#define PGRHEIGHT 1200


//VideoWriter
cv::VideoWriter videoWriter;

extern "C" {
   DLLExport void* getPGR(int device);
   DLLExport void initPGR(void* camera, int device);
   DLLExport void releasePGR(void* camera, int device);
   DLLExport void getPGRTexture(void* camera, int device, unsigned char* data, bool isRecord, bool isShowWin);
   DLLExport void showPixelData(unsigned char* data);
	//**ドット検出関連**//
   DLLExport void setDotsParameters(void* camera, double AthreshVal, int DotThreshValMin, int DotThreshValMax, int DotThreshValBright, float resizeScale);
   DLLExport int getDotsCount(void* camera);
   DLLExport void getDotsData(void* camera, int *data);
   //マスク生成
	DLLExport void createCameraMask_pgr(void* camera, unsigned char* cam_data);
}