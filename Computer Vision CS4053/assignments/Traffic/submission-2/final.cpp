#include <iostream> 
#include <vector>
#include <numeric>

// Code from course text sample code 

#include "utilities.h"

using namespace cv;
using namespace std;

// My own code or suffieciently changed course text sample code 
#include "metrics.h"
#include "lights.h"
#include "lightIO.h"

// Constants
const int N_IMAGES = 14;

int K = 11;
int K_THRESH = 2;
int K_ITER = 5;

bool QUIET = false;

// Perform K-Means segmentation on the image
Mat kmeans_clustering(Mat& image, int k, int iterations);

// Determine whether segmented image regions are lights.
void match_lights(vector<vector<Point>> &contours, vector<Vec4i> &hierarchy, Mat &input, 
    vector<Light>& lights, Mat &contours_image);

int main(int argc, const char * argv[]) 
{
    if(argc > 1 && !strcmp(argv[1], "quiet")) QUIET = true;

    // Read in images and ground truth
    vector<Mat> input = get_traffic_images(N_IMAGES);
    vector<vector<Light>> ground_truth = get_ground_truth( "resources/ground-truth.csv", N_IMAGES);

    // Prepare Output
    vector<Mat> output;
    vector<vector<Light>> results (N_IMAGES);
    Mat row1, row2, display_image;
    char c;

	for(int N =0; N <input.size(); N++) 
    {
        // Lights found in image
        vector<Light> lights;

        // Segment image using K-Means Clustering
        Mat processed_input = kmeans_clustering(input[N],K, K_ITER);

        // Threshold based on brightness of lowest K_THRESH clusters in image.
        Mat grey_image, binary_image;
        cvtColor(processed_input, grey_image, CV_BGR2GRAY);
        threshold(grey_image, binary_image, K_THRESH* 255/K, 255, THRESH_BINARY);

        // Retrieve connected components
        vector<vector<Point>> contours;
        vector<Vec4i> hierarchy;
        findContours(binary_image,contours,hierarchy,CV_RETR_TREE,CV_CHAIN_APPROX_NONE);

        // Use various features to identify which regions are lights.          
        Mat refined_contours_image;
        match_lights(contours, hierarchy, input[N], lights, refined_contours_image);
        
        // Evaluate metrics for image
        compare_with_ground_truth(lights, ground_truth[N], true);
        compare_with_ground_truth(lights, ground_truth[N], false);

        // Prepare visual output for results and ground truth
        Mat match_image = input[N].clone();
        for(int i = 0; i<lights.size(); i++)
        {
            rectangle(match_image, lights[i].inner, Scalar(255,0,0));
            rectangle(match_image, lights[i].outer, Scalar(255,0,0));
        
            Point tl = lights[i].outer.tl();
            writeText(match_image, state_str(lights[i].state),tl.y,tl.x, state_color(lights[i].state), 0.75, 2);
        }
        for(int i = 0; i<ground_truth[N].size(); i++)
        {
            Scalar color = ground_truth[N][i].get_found(true) ? Scalar(0,255,0) : Scalar(0,0,255);
            rectangle(match_image, ground_truth[N][i].inner, color);
            color = ground_truth[N][i].get_found(false) ? Scalar(0,255,0) : Scalar(0,0,255);
            rectangle(match_image, ground_truth[N][i].outer, color);
        }

        // Show Visual output
        if(!QUIET)
        {
            cvtColor(binary_image, binary_image, CV_GRAY2BGR);
            row1 = JoinImagesHorizontally(processed_input, "K-Means", binary_image, "Segmented Image", 4);
            row2 = JoinImagesHorizontally(refined_contours_image, "Recognised Regions", match_image, "Final Results", 4);
            display_image = JoinImagesVertically(row1, "", row2, "", 4);

            resize(display_image, display_image, Size(),0.7, 0.7, CV_INTER_AREA);
            imshow("Output", display_image);
            c = cvWaitKey();
            cvDestroyAllWindows();
            if(c == 'q') return 0;
        }

        // Store the matched lights and visual output.
        output.push_back(refined_contours_image);
        results[N] = lights;
	}
    

    // Output Recognition Metrics
    cout << "Inner" <<endl;
    get_recognition_results(results, ground_truth, true);

    cout << "Outer" <<endl;
    get_recognition_results(results, ground_truth, false);

    write_traffic_images(output);
    return 0;
}


Mat kmeans_clustering(Mat& image, int k, int iterations)
{
	CV_Assert(image.type() == CV_8UC3);
	// Populate an n*3 array of float for each of the n pixels in the image
	Mat samples(image.rows*image.cols, image.channels(), CV_32F);
	float* sample = samples.ptr<float>(0);
	for (int row = 0; row<image.rows; row++)
		for (int col = 0; col<image.cols; col++)
			for (int channel = 0; channel < image.channels(); channel++)
				samples.at<float>(row*image.cols + col, channel) =
				(uchar)image.at<Vec3b>(row, col)[channel];
	// Apply k-means clustering to cluster all the samples so that each sample
	// is given a label and each label corresponds to a cluster with a particular
	// centre.
	Mat labels;
	Mat centers;
	kmeans(samples, k, labels, TermCriteria(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 1, 0.0001),
		iterations, KMEANS_PP_CENTERS, centers);
    
    // Sort the centers
    vector<pair<int, float>> grey_centers(k, pair<int, float>(0,0.0));
    for(int i = 0; i < centers.rows; i++)
    {
        grey_centers[i].first = i;
        for (int j = 0; j < centers.cols; j++)
            grey_centers[i].second += centers.at<float>(i,j);
    }       
    sort( grey_centers.begin(),grey_centers.end(), [&](auto i,auto j){return i.second < j.second ;} );
    
    vector<int> ordered_indices(k);
    for(int i = 0; i < k; i++)
        for(int j = 0; j < k; j++)
            if(i == grey_centers[j].first) ordered_indices[i] = j;
        
	// Output quantised greyscale image, segmented and ordered by the centers.
	Mat result_image = Mat(image.size(), image.type());
	for (int row = 0; row<image.rows; row++)
		for (int col = 0; col<image.cols; col++)
			for (int channel = 0; channel < image.channels(); channel++)
                result_image.at<Vec3b>(row, col)[channel] = (uchar)(ordered_indices[*(labels.ptr<int>(row*image.cols + col))] * 255/(k-1));
    return result_image;
}

// Utility functions

// Compute median of single channel image using its histogram
double median(Mat channel)
{
    double middle = (channel.rows*channel.cols) / 2;
    int bin = 0;
    double med = -1.0;

    int histSize = 256;
    float range[] = { 0, 256 };
    const float* histRange = { range };
    bool uniform = true;
    bool accumulate = false;
    Mat hist;
    calcHist( &channel, 1, 0, Mat(), hist, 1, &histSize, &histRange, uniform, accumulate );

    for ( int i = 0; i < histSize && med < 0.0; ++i )
    {
        bin += cvRound( hist.at< float >( i ) );
        if ( bin > middle && med < 0.0 )
            med = i;
    }

    return med;
}

// Scalar conversion from BGR to RGB
Scalar ScalarBGR2HSV(Scalar &BGR) {
    Mat hsv;
    Mat bgr(1,1, CV_8UC3, BGR);
    cvtColor(bgr, hsv, CV_BGR2HSV);
    return Scalar(hsv.data[0], hsv.data[1], hsv.data[2]);
}

// Makes comparision of RGB easier
bool operator >(const Scalar &a, const Scalar &b)
{
    return (a.val[0] > b.val[0]) || (a.val[1] > b.val[1]) || (a.val[2] > b.val[2]);
}

// Constants for Region Processing

double MIN_CIRCULARITY = 1.0;

double MIN_A_RATIO = 0.6;
double MAX_A_RATIO = 1.8;

double MIN_P_A_RATIO = 1.0;
double MAX_P_A_RATIO = 4.0;

double MIN_HOLE_H_RATIO = 1.0;
double MAX_HOLE_H_RATIO = 10.0;
double MIN_HOLE_W_RATIO = 0.8;
double MAX_HOLE_W_RATIO = 5.0;

Scalar GREEN_MIN = Scalar(70, 25, 90);
Scalar GREEN_MAX = Scalar(95, 255, 255);

Scalar RED_MIN = Scalar(0, 25, 50);
Scalar RED_MAX = Scalar(15, 255, 255);

Scalar AMBER_MIN = Scalar(15, 25, 50);
Scalar AMBER_MAX = Scalar(35, 255, 255);

Scalar BLACK_MIN = Scalar(0, 0, 0);
Scalar BLACK_MAX = Scalar(255, 160, 30);
        

void match_lights(vector<vector<Point>> &contours, vector<Vec4i> &hierarchy,  Mat &input, vector<Light>& lights, Mat &contours_image)
{    
    // Visual output for potential regions.
    contours_image = Mat::zeros(input.size(), CV_8UC3);
    contours_image = Scalar(255,255,255);

    // Go through the contours
    for (int contour=0; contour < contours.size(); contour++)
    {
        double perimeter = arcLength(contours[contour], true);
        double area = contourArea(contours[contour]);
        double circularity = area/perimeter;

        // Exclude tiny regions    
        if(perimeter == 0 || area == 0) continue;
        if(circularity < MIN_CIRCULARITY) continue;

        //Compute bounding rects
        RotatedRect min_bounding_rectangle = minAreaRect(contours[contour]);
        Rect bound_rect = boundingRect(contours[contour]);
        
        // Check it's correctly proportioned   
        double aspect_ratio = double(bound_rect.height) / double(bound_rect.width);
        if(aspect_ratio > MAX_A_RATIO || aspect_ratio < MIN_A_RATIO) continue;

        // Get Just the pixels in the contour
        Mat region, contour_mask = Mat::zeros(input.size(), CV_8UC1);
        drawContours(contour_mask, contours, contour, (255), CV_FILLED);
        input.copyTo(region, contour_mask);

        // Crop to the region
        region = region(bound_rect);

        // Get median color of region
        vector<Mat> bgr_planes;
        split( region, bgr_planes );
        Scalar color = Scalar(median(bgr_planes[0]), median(bgr_planes[1]), median(bgr_planes[2]));
        Scalar color_HSV = ScalarBGR2HSV(color);

        // Test that it is in the correct color ranges for a light
        if( (GREEN_MIN > color_HSV || color_HSV > GREEN_MAX)
            && (RED_MIN > color_HSV || color_HSV > RED_MAX)
            && (AMBER_MIN > color_HSV || color_HSV > AMBER_MAX)
        ) continue;         
        
        // Find if the circle has a black rectangle about it 

        // Test for parent existence        
        if(hierarchy[contour][3] == -1) continue;

        int parent = hierarchy[contour][3];

        // Compute parent bounding rects
        RotatedRect parent_min_bounding_rectangle = minAreaRect(contours[parent]);
        Rect parent_bound_rect = boundingRect(contours[parent]);

        // Ensure it's portrait
        double parent_aspect_ratio = double(parent_bound_rect.height) / double(parent_bound_rect.width);
        if(parent_aspect_ratio < MIN_P_A_RATIO || parent_aspect_ratio > MAX_P_A_RATIO) continue; 

        // Compute ratios of hole size to parent size
        double hole_ratio_height  = parent_bound_rect.height / min_bounding_rectangle.size.height;
        double hole_ratio_width  = parent_bound_rect.width / min_bounding_rectangle.size.width;
        
        // Ensure they are within tolerance
        if(hole_ratio_height < MIN_HOLE_H_RATIO || hole_ratio_height > MAX_HOLE_H_RATIO) continue;
        if(hole_ratio_width < MIN_HOLE_W_RATIO || hole_ratio_width > MAX_HOLE_W_RATIO) continue;
        
        // Test if the parent is dark enough in median
        Mat parent_contour_mask = Mat::zeros(input.size(), CV_8UC1), parent_region;
        drawContours(parent_contour_mask, contours, parent, (255), CV_FILLED);
        input.copyTo(parent_region, parent_contour_mask);
        parent_region = parent_region(parent_bound_rect);
        vector<Mat> parent_bgr_planes;
        split( parent_region, parent_bgr_planes );
        Scalar parent_color = Scalar(median(parent_bgr_planes[0]), median(parent_bgr_planes[1]), median(parent_bgr_planes[2]));
        Scalar parent_color_HSV = ScalarBGR2HSV(parent_color);

        if(BLACK_MIN > parent_color_HSV || parent_color_HSV > BLACK_MAX ) continue;
        
        // Draw the Results
        drawContours( contours_image, contours, contour, color, CV_FILLED, 8, hierarchy );                    
        drawContours( contours_image, contours, parent, parent_color, CV_FILLED, 8, hierarchy );                    

        // Prepare resultant bounding boxes 
        Light light;
        light.outer = parent_bound_rect;

        // Find state of light
        double light_height = double(parent_bound_rect.height) / 3.0;
        double y_state = fabs(bound_rect.y - parent_bound_rect.y)/light_height;     
        
        switch(int(round(y_state)))
        {
            case 1:  
            light.state = AMBER; break;
            case 2:
            light.state = GREEN; break;
            default: 
            light.state = RED;
        }

        // Ensure the found light does not intesect with a previous one, 
        // if it does, ensure that it updates the state if the light is roughly the same size
        bool found = false;
        for(int j = 0; j < lights.size(); j++)
        {
            bool intersects = ((parent_bound_rect & lights[j].outer).area() > 0);

            if(intersects)
            {
                if(lights[j].state == RED && light.state == AMBER) lights[j].state = RED_AMBER;
                if(lights[j].state == AMBER && light.state == RED) lights[j].state = RED_AMBER;
                found = true;
            }
        }

        //  Find inner bounding box if necessary
        if(!found) 
        {
            if( parent_aspect_ratio > 2 ) light.inner = parent_bound_rect;
            else {
                light.inner = Rect(bound_rect.x-(double(bound_rect.width)/2), 
                bound_rect.y-(light_height*y_state), 
                bound_rect.width*2, 
                light_height*3);
            }

            lights.push_back(light); 
        }
    }
}