/* 
 * Based on Sample code provided as part of "A Practical Introduction to Computer Vision with OpenCV"
 * by Kenneth Dawson-Howe � Wiley & Sons Inc. 2014. 
 * Modified by Caelen Feller
 */

#pragma once
#include "opencv2/core.hpp"
// #include "opencv2/core/cuda.hpp"
#pragma GCC diagnostic ignored "-Wwrite-strings"
#include "opencv2/video.hpp"
#include "opencv2/objdetect.hpp"
#include "opencv2/highgui.hpp"
#include <stdio.h>
#include <iostream>
#include <string>
#define PI 3.14159265358979323846

using namespace std;
using namespace cv;

void FindLocalMaxima(Mat& input_image, Mat& local_maxima, double threshold_value);

void writeText( Mat image, char* text, int row, int column, Scalar colour=-1.0, double scale=0.4, int thickness=1 );
Mat JoinImagesHorizontally( Mat& image1, char* name1, Mat& image2, char* name2, int spacing=0, Scalar colour=-1.0 );
Mat JoinImagesVertically( Mat& image1, char* name1, Mat& image2, char* name2, int spacing=0, Scalar colour=-1.0 );
void addGaussianNoise(Mat &image, double average=0.0, double standard_deviation=10.0);

VideoWriter* OpenVideoFile( char* filename, VideoCapture& video_to_emulate, int horizontal_multiple=1, int vertical_multiple=1, int spacing=0 );
VideoWriter* OpenVideoFile( char* filename, int codec, Size image_size, double fps, int horizontal_multiple=1, int vertical_multiple=1, int spacing=0 );
void WriteVideoFrame( VideoWriter* output_video, Mat& video_frame );
void CloseVideoFile( VideoWriter* video );

void invertImage(Mat &image, Mat &result_image);
int CameraCalibration( string passed_settings_filename );

void invertImage(Mat &image, Mat &result_image);
Mat StretchImage( Mat& image );
Mat convert_32bit_image_for_display(Mat& passed_image, double zero_maps_to=0.0, double passed_scale_factor=-1.0 );
void show_32bit_image( char* window_name, Mat& passed_image, double zero_maps_to=0.0, double passed_scale_factor=-1.0 );
Mat ComputeDefaultImage( Mat& passed_image );
void DrawHistogram( MatND histograms[], int number_of_histograms, Mat& display_image );
