#include "Source.h"
#include "DebugLogWrapper.h"

DLLExport void* getPGR(int device)
{

	//-1だったら再生モード
	if(device != -1)
	{
		//普通にPGRから取ってくるとき
		return static_cast<void*>(new TPGROpenCV(0));
	}
	else
	{
		//録画再生用	
		return static_cast<void*>(new cv::VideoCapture("capture.avi"));
	}
}

DLLExport void initPGR(void* pgr, int device)
{
	if(device != -1)
	{
		auto pgrOpenCV = static_cast<TPGROpenCV*>(pgr);

		//モノクロ1chで取ってくる
		pgrOpenCV->init(FlyCapture2::PIXEL_FORMAT_MONO8, FlyCapture2::HQ_LINEAR);
		pgrOpenCV->start();

		//録画用
		videoWriter = cv::VideoWriter("capture.avi", CV_FOURCC('X','V','I','D'), pgrOpenCV->getFramerate(), cv::Size(PGRWIDTH, PGRHEIGHT), true); //isColorで3ch or 1ch
	}
}

DLLExport void releasePGR(void* pgr, int device)
{
    // ウィンドウを閉じる
	if(device != -1) 
	{
		videoWriter.release();
		auto pgrOpenCV = static_cast<TPGROpenCV*>(pgr);
		pgrOpenCV->stop();
		pgrOpenCV->release();
	}
	else
	{
		auto vc = static_cast<cv::VideoCapture*>(pgr);
		delete vc;
	}
	cv::destroyAllWindows();
}

DLLExport void getPGRTexture(void* pgr, int device, unsigned char* data, bool isRecord, bool isShowWin)
{
	if(device != -1)
	{
		auto pgrOpenCV = static_cast<TPGROpenCV*>(pgr);

		if(isRecord && !videoWriter.isOpened())
		{
			//録画用
			//videoWriter = cv::VideoWriter("capture.avi", CV_FOURCC('X','V','I','D'), pgrOpenCV->getFramerate(), cv::Size(PGRWIDTH, PGRHEIGHT), false);

		}


			// カメラ画の取得
			cv::Mat img;
			//pgrOpenCV->queryFrame();
			img = pgrOpenCV->getVideo();

			if(!img.empty())
			{
				//録画フラグがたっていたら録画もする
				if(isRecord) videoWriter << img;

				//std::cout << "img size:" << img.rows << " * " << img.cols << std::endl;
    
				if(isShowWin)
				{
					// リサイズ
					cv::Mat resized_img(img.rows * 0.5, img.cols * 0.5, img.type());
					cv::resize(img, resized_img, resized_img.size(), cv::INTER_CUBIC);
    
					// 別ウィンドウの画を更新
					cv::imshow("camera", resized_img);
				}
				//↓↓処理するのはこっちに戻ってやるのだからx反転色変換いらないのでは？？
				// RGB --> ARGB 変換
			 //   cv::Mat argb_img, flip_img;
			 //   //cv::cvtColor(resized_img, argb_img, CV_RGB2BGRA);
			 //   cv::cvtColor(img, argb_img, CV_RGB2BGRA);
			 //   std::vector<cv::Mat> bgra;
			 //   cv::split(argb_img, bgra);
			 //   std::swap(bgra[0], bgra[3]);
			 //   std::swap(bgra[1], bgra[2]);
				////x軸反転
				//cv::flip(argb_img, flip_img, 0);
			 //   std::memcpy(data, flip_img.data, flip_img.total() * flip_img.elemSize());

				std::memcpy(data, img.data, img.total() * img.elemSize());
			}
	}
	else
	{
		auto vc = static_cast<cv::VideoCapture*>(pgr);

		// カメラ画の取得
		cv::Mat img;
		*vc >> img;

		if(!img.empty())
		{

			if(isShowWin)
			{
				// リサイズ
				cv::Mat resized_img(img.rows * 0.5, img.cols * 0.5, img.type());
				cv::resize(img, resized_img, resized_img.size(), cv::INTER_CUBIC);
    
				// 別ウィンドウの画を更新
				cv::imshow("camera", resized_img);
			}

			std::memcpy(data, img.data, img.total() * img.elemSize());
		}
	}
}

DLLExport void showPixelData(unsigned char* data)
{
	//カメラ画像をMatに復元
	cv::Mat pgr_img(PGRHEIGHT, PGRWIDTH, CV_8UC3, data);

	// 別ウィンドウの画を更新
	cv::imshow("PGR", pgr_img);

}


//**ドット検出関連**//
DLLExport void setDotsParameters(void* pgr, double AthreshVal, int DotThreshValMin, int DotThreshValMax, int DotThreshValBright, float resizeScale)
{
auto pgrOpenCV = static_cast<TPGROpenCV*>(pgr);

pgrOpenCV->setDotsParameters(AthreshVal, DotThreshValMin, DotThreshValMax, DotThreshValBright, resizeScale);

}

DLLExport int getDotsCount(void* pgr)
{
	auto pgrOpenCV = static_cast<TPGROpenCV*>(pgr);
	return pgrOpenCV->getDotsCount();
}

DLLExport void getDotsData(void* pgr, int *data)
{
	auto pgrOpenCV = static_cast<TPGROpenCV*>(pgr);
	std::vector<int> datavec;
	pgrOpenCV->getDotsData(datavec); //dataのサイズはdotsCount*2
	//data = &datavec[0];
	memcpy(data, &datavec[0], datavec.size()*sizeof(int));
}

//マスク生成
DLLExport void createCameraMask_pgr(void* pgr, unsigned char* cam_data)
{
	auto pgrOpenCV = static_cast<TPGROpenCV*>(pgr);

	//カメラ画像をMatに復元
	cv::Mat cam_img(PGRHEIGHT, PGRWIDTH, CV_8UC4, cam_data);
	//プロジェクタ画像はUnity側で生成されたので、反転とかする
	//BGR <-- ARGB 変換
	cv::Mat bgr_img, flip_cam_img;
	std::vector<cv::Mat> bgra;
	cv::split(cam_img, bgra);
	std::swap(bgra[0], bgra[3]);
	std::swap(bgra[1], bgra[2]);
	cv::cvtColor(cam_img, bgr_img, CV_BGRA2BGR);
	//x軸反転
	cv::flip(bgr_img, flip_cam_img, 0);

	//膨張処理かけとく
	cv::Mat resizeimg, dilatedimg, resultimg;
	cv::Mat element(9,9,CV_8U, cv::Scalar(1));
	cv::bitwise_not(flip_cam_img, flip_cam_img);
	cv::resize(flip_cam_img, resizeimg, cv::Size(), 0.5, 0.5);
	cv::dilate(resizeimg, dilatedimg, element, cv::Point(-1,-1), 1);
	cv::resize(dilatedimg, resultimg, cv::Size(), 2.0, 2.0);
	cv::bitwise_not(resultimg, resultimg);
	pgrOpenCV->mask = resultimg.clone();


	//pgrOpenCV->mask = flip_cam_img.clone();

	cv::imshow("mask", pgrOpenCV->mask);

}


