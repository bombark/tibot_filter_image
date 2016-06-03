#include <iostream>
#include <string>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <tiobj.hpp>
#include <unistd.h>
#include <stdio.h>
#include <functional>

using namespace std;
using namespace cv;

template<typename T>
void setParam(vector<TiVar*>& params, TiObj& data, string name, T _default){
	if ( data.has( name ) ){
		params.push_back(  &data.at( name )  );
	} else {
		TiVar* var = new TiVar();
		*var = _default;
		params.push_back(  var  );
	}
}



void gaussian_init(vector<TiVar*>& params, TiObj& data){
	setParam<int>(params, data, "ksize", 3);
	setParam<double>(params, data, "sigmaX", 1.0);
	setParam<double>(params, data, "sigmaY", 1.0);
}

Mat gaussian_exec(Mat& src, vector<TiVar*>& params){
	Mat dst;
	int size = params[0]->atInt();
	double gx = params[1]->atDbl();
	double gy = params[2]->atDbl();
	GaussianBlur(src, dst, Size(size,size), gx, gy);
	return dst;
}



void median_init(vector<TiVar*>& params, TiObj& data){
	setParam<int>(params, data, "ksize", 3);
}

Mat median_exec(Mat& src, vector<TiVar*>& params){
	Mat dst;
	int ksize = params[0]->atInt();
	medianBlur(src, dst, ksize);
	return dst;
}



void laplacian_init(vector<TiVar*>& params, TiObj& data){
	setParam<int>(params, data, "ksize", 1);
	setParam<double>(params, data, "scale", 1.0);
	setParam<double>(params, data, "delta", 0.0);
}

Mat laplacian_exec(Mat& src, vector<TiVar*>& params){
	Mat dst;
	int ksize    = params[0]->atInt();
	double scale = params[1]->atDbl();
	double delta = params[2]->atDbl();
	Laplacian( src, dst, CV_16S, ksize, scale, delta, BORDER_DEFAULT );
	return dst;
}



void sobel_init(vector<TiVar*>& params, TiObj& data){
	setParam<double>(params, data, "scale", 1.0);
	setParam<double>(params, data, "delta", 0.0);
}

Mat sobel_exec(Mat& src, vector<TiVar*>& params){
	Mat dst, grad_x, grad_y;
	double scale = params[0]->atDbl();
	double delta = params[1]->atDbl();

	Sobel( src, grad_x, CV_16S, 1, 0, 3, scale, delta, BORDER_DEFAULT );
	Sobel( src, grad_y, CV_16S, 0, 1, 3, scale, delta, BORDER_DEFAULT );
	addWeighted( grad_x, 0.5, grad_y, 0.5, 0, dst );

	return dst;
}




void canny_init(vector<TiVar*>& params, TiObj& data){
	setParam<int>(params, data, "threshold1", 30);
	setParam<int>(params, data, "threshold2", 50);
	setParam<int>(params, data, "apertureSize", 3);
}

Mat canny_exec(Mat& src, vector<TiVar*>& params){
	Mat dst, grad_x, grad_y;
	int threshold1   = params[0]->atInt();
	int threshold2   = params[1]->atInt();
	int apertureSize = params[2]->atInt();
	Canny( src, dst, threshold1, threshold2, apertureSize );
	return dst;
}



void resize_init(vector<TiVar*>& params, TiObj& data){
	if ( data.has("sizeX") && data.has("sizeY") ){
		params.push_back( &data.at("sizeX") );
		params.push_back( &data.at("sizeY") );
	} else if ( data.has("fx") && data.has("fy") ){
		/*TiVar* sizeX, sizeY;
		sizeX = new TiVar();
		*sizeX = data.atDbl("fx") * ?????

		pa[0] = (double) 

		pa[1] = new TiVar();*/
	}
}

Mat resize_exec(Mat& src, vector<TiVar*>& params){


	Mat dst;
	int sizeX = params[0]->atInt();
	int sizeY = params[1]->atInt();
	resize( src, dst, Size(sizeY,sizeX), 0.0, 0.0, INTER_LINEAR );
	return dst;
}





Mat null_exec(Mat& src, vector<TiVar*>& params){
	cerr << "[Error]: function not implemented\n";
	Mat a;
	return a;
}


class Caller{
	vector<TiVar*> params;
	std::function<Mat(Mat&,vector<TiVar*>&)> function;

	public:
		Caller(){
			this->function = null_exec;
		}
	
		void config(
			std::function<void(vector<TiVar*>&,TiObj&)> init,
			std::function<Mat(Mat&,vector<TiVar*>&)> exec,
			TiObj& data
		){
			init(this->params, data);
			this->function = exec;
		}
		inline Mat exec(Mat& src){
			return this->function(src, this->params);
		}
};





int main(){


	TiObj params( getenv("params") );

	vector<Caller*> callerpkg;
	for (int i=0; i<params.size(); i++){
		TiObj& method = params.box[i];
		if ( method.is("Method") ){
			Caller* caller = new Caller();
			string method_name = method.atStr("method");
			if ( method_name == "Gaussian" ){
				caller->config( gaussian_init, gaussian_exec, method );
			} else if ( method_name == "Sobel" ){
				caller->config( sobel_init, sobel_exec, method );
			} else if ( method_name == "Median" ){
				caller->config( median_init, median_exec, method );
			} else if ( method_name == "Laplacian" ){
				caller->config( laplacian_init, laplacian_exec, method );
			} else if ( method_name == "Canny" ){
				caller->config( canny_init, canny_exec, method );
			} else if ( method_name == "Resize" ){
cout << method;
				caller->config( resize_init, resize_exec, method );
			}
			callerpkg.push_back( caller );
		}
	}
	if ( callerpkg.size() == 0 )
		exit(1);



	Mat img, res;
	char buf[64];
	while ( scanf("%s", buf) ){
		img = imread("image.jpg");
		res = callerpkg[0]->exec( img );
		for ( int i=1; i<callerpkg.size(); i++){
			res = callerpkg[i]->exec( res );
		}
		imwrite("output.jpg", res);

		//usleep(sleep_time);
		printf("#end");
		fflush(stdout);
	}




	return 0;
}
