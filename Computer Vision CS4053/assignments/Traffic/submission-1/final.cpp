#include <iostream> 
#include <iomanip>
#include <sstream>
#include <vector>

// Code from course text sample code 

#include "Utilities.h"

void FindLocalMaxima( Mat& input_image, Mat& local_maxima, double threshold_value );
void FindLocalMinima( Mat& input_image, Mat& local_minima, double threshold_value );

// CSV Reading library for reading ground truth from https://github.com/ben-strasser/fast-cpp-csv-parser
#include "csv.h"

using namespace cv;
using namespace std;

// My own code or suffieciently changed course text sample code 
#include "metrics.h"

enum LightState {
    RED,
    AMBER,
    RED_AMBER,
    GREEN
};

struct Light {
    Rect inner;
    Rect outer;
    double correlation;
    LightState state;
};

// Read Images In
vector<Mat> get_traffic_images();

// Do any preprocessing on Image
void preprocess(Mat& input, Mat& output);

// Determine whether segmented image regions are lights, and where they are
void match_lights(vector<vector<Point>> &contours, vector<Vec4i> &hierarchy,  Mat &input, Mat &output, vector<Light>& lights, Mat &contours_image);

// Modified version of DrawMatchingTemplateRectangles to return bounding rectangles
void get_matching_template_rectangles( vector<Rect> &output, Mat& matched_template_map, Mat& template_image, Scalar passed_colour);

// Output Result Images
void write_traffic_images(vector<Mat> &images);

// Evaluate Recognition Metrics
void GetRecognitionResults(vector<vector<Light>> &results, bool centre);

// Constants
const int N_IMAGES = 14;

double CORRELATION_THRESH = .78;

double MAX_ANGLE = 20;
double MIN_WIDTH = 1;

double MIN_A_RATIO = 1.0;
double MAX_A_RATIO = 4.0;

double MIN_HOLE_A_RATIO = 0.5;
double MAX_HOLE_A_RATIO = 1.8;

int THRESHOLD_SIZE = 67;

int main(int argc, const char * argv[]) 
{
    char c;
    // Read in images
    vector<Mat> input = get_traffic_images();

    vector<Mat> output;
    vector<vector<Light>> results (N_IMAGES);

    // Interactive output 
    Mat row1, row2, displayImage;

	for(int N =0; N <input.size(); N++) 
    {
        // Lights found in image
        vector<Light> lights;

        // Noise Reduction and Preprocessing
        Mat processed_input;
        preprocess(input[N], processed_input);
        
        // Segment image into regions via thresholding of binary image
        Mat grey_image, binary_image, closed_image;
        cvtColor(processed_input, grey_image, CV_BGR2GRAY);
        
        adaptiveThreshold(grey_image, binary_image, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, THRESHOLD_SIZE, 1);

        // Close Regions
        dilate(binary_image, closed_image, Mat());
        erode( closed_image,closed_image, Mat());

        // Process regions in segmented image.
        vector<vector<Point>> contours;
        vector<Vec4i> hierarchy;

        findContours(closed_image,contours,hierarchy,CV_RETR_TREE,CV_CHAIN_APPROX_NONE);


        // Use various features to identify regions, and template match to finalise          
        Mat refined_contours_image, match_image;
        match_lights(contours, hierarchy, input[N], match_image, lights, refined_contours_image);
        

        // Prepare visual output
        for(int i = 0; i<lights.size(); i++)
        {
            rectangle(match_image, lights[i].inner, Scalar(255,0,0));
            rectangle(match_image, lights[i].outer, Scalar(0,0,255));
            char * state;
            if(lights[i].state == RED) state = "RED";
            if(lights[i].state == RED_AMBER) state = "RED AMBER";
            if(lights[i].state == GREEN) state = "GREEN";
            if(lights[i].state == AMBER) state = "AMBER";
            
            char str[50];
            sprintf(str, "%f, STATE: %s", lights[i].correlation, state);
            writeText(match_image, str, lights[i].outer.tl().y,lights[i].outer.tl().x);
        }

        cvtColor(binary_image, binary_image, CV_GRAY2BGR);
        cvtColor(closed_image, closed_image, CV_GRAY2BGR);
        row1 = JoinImagesHorizontally(processed_input, "Input Image", binary_image, "Thresholded Image", 4);
        row2 = JoinImagesHorizontally(closed_image, "Recognised Regions", refined_contours_image, "Template Match - Result", 4);
        displayImage = JoinImagesVertically(row1, "", row2, "", 4);

        resize(displayImage, displayImage, Size(),0.7, 0.7, CV_INTER_AREA);
        imshow("Output", displayImage);
        c = cvWaitKey();
        cvDestroyAllWindows();
        if(c == 'q') return 0;
        
        // Store the matched lights and visual output.
        output.push_back(match_image);
        results[N] = lights;
	}
    
    // Write visual output
    write_traffic_images(output);

    // Output Recognition Metrics
    cout << "Centres" <<endl;
    GetRecognitionResults(results, true);


    cout << "Outer" <<endl;
    GetRecognitionResults(results, false);

    return 0;
}

void preprocess(Mat& input, Mat& output)
{
    output = input.clone();
    // This ended up not being used, but is a useful stub 
    // if you need to do content aware exposure correction, 
    // Preliminary segmentation, scaling, etc.
}

void write_traffic_images(vector<Mat> &images)
{
    String resPath("");
    String inputPath(resPath + "output/CamVidLights");

    ostringstream path;
    for(int i = 0; i < images.size(); i++)
    {
        path.str("");
        path << inputPath << setw(2) << setfill('0') << i+1 << ".png";
        imwrite(path.str(), images[i]);
    }
}

vector<Mat> get_traffic_images()
{
    String resPath("resources/");
    String inputPath(resPath + "input/CamVidLights");

    vector<Mat> images(N_IMAGES);

    ostringstream path;
    for(int i = 1; i <= N_IMAGES; i++)
    {
        path.str("");
        path << inputPath << setw(2) << setfill('0') << i << ".png";
        images[i-1] = imread( path.str(), IMREAD_COLOR );
    }
    return images;
}


void match_lights(vector<vector<Point>> &contours, vector<Vec4i> &hierarchy,  Mat &input, Mat &output, vector<Light>& lights, Mat &contours_image)
{    
    vector<Mat> templates = {
        imread("resources/templates/red.png"), 
        imread("resources/templates/amber.png"),
        imread("resources/templates/red-amber.png"),
        imread("resources/templates/green.png")
    };

    // Visual output for potential regions, without template matching
    contours_image = Mat::zeros(input.size(), CV_8UC3);
    contours_image = Scalar(255,255,255);
    output = input.clone();

    for (int contour=0; (contour < contours.size()); contour++)
    {
        // Refine regions to search
        // Does the region have a hole?
        if(hierarchy[contour][2] == -1) continue;
        
        // Find the largest hole in the region
        int max_child = hierarchy[contour][2];
        for(int child = hierarchy[contour][2]; hierarchy[child][0] > -1; child = hierarchy[child][0])
            if (contours[max_child].size() < contours[child].size()) max_child = child;
        
        // Is the region and hole large enough
        if (contours[contour].size() > 10 && contours[max_child].size() > 2)
        {
            // Get bounding boxes of region and hole
            RotatedRect min_bounding_rectangle = minAreaRect(contours[contour]);
            RotatedRect child_min_bounding_rectangle = minAreaRect(contours[max_child]);

            // Ensure region is not too rotated - traffic light will not be more than 20 degrees tilted
            if(min_bounding_rectangle.angle > MAX_ANGLE ) continue;

            // Ensure region is portrait
            if(min_bounding_rectangle.boundingRect().width > min_bounding_rectangle.boundingRect().height) continue;

            //Ensure rotated region is portrait
            if (min_bounding_rectangle.size.width > min_bounding_rectangle.size.height)
            {
                swap(min_bounding_rectangle.size.width, min_bounding_rectangle.size.height);
                min_bounding_rectangle.angle += 90.f;
            }

            // Get aspect ratio of Region and child
            double aspect_ratio = min_bounding_rectangle.size.height / min_bounding_rectangle.size.width;
            double child_aspect_ratio = child_min_bounding_rectangle.size.height / child_min_bounding_rectangle.size.width;
            
            if(
                aspect_ratio > MIN_A_RATIO && aspect_ratio < MAX_A_RATIO     // Is it correctly proportioned?
                && min_bounding_rectangle.size.width > MIN_WIDTH   // Is it too small/too large? 
                && child_aspect_ratio > MIN_HOLE_A_RATIO && child_aspect_ratio < MAX_HOLE_A_RATIO // Is the hole circular enough?
            ) {   
                Scalar colour( rand()&0xFF,rand()&0xFF,rand()&0xFF );
                drawContours( contours_image, contours, contour, colour, CV_FILLED, 8, hierarchy );                    

                // Template match region against all traffic light templates
                for(int temp = 0; temp < templates.size(); temp++) 
                {
                    // Result Light
                    Light light;
                    
                    Rect searchArea = min_bounding_rectangle.boundingRect();
                    
                    // Check if bdd box is in the image
                    if( (searchArea & cv::Rect(0, 0, input.cols, input.rows)) != searchArea) continue;
                    
                    Mat searchImage = input(searchArea).clone();

                    // Warp Search Area to better fit the template
                    Mat rot = getRotationMatrix2D(Point2f(searchImage.cols/2,searchImage.rows/2), min_bounding_rectangle.angle, 1);

                    Rect2f bbox = RotatedRect(Point2f(searchImage.cols/2, searchImage.rows/2), searchImage.size(), min_bounding_rectangle.angle).boundingRect2f();
                    rot.at<double>(0,2) += bbox.width/2.0 - searchImage.cols/2.0;
                    rot.at<double>(1,2) += bbox.height/2.0 - searchImage.rows/2.0;
                    
                    warpAffine(searchImage, searchImage, rot, bbox.size());

                    // Scale the search area  and template to be the correct size
                    double scale =(double)templates[temp].rows/ min_bounding_rectangle.size.height;
                    Mat scaled_template = templates[temp].clone();

                    resize(scaled_template, scaled_template, Size(), 1.0/scale, 1.0/scale, CV_INTER_AREA);
                    resize(searchImage, searchImage, Size(), scale, scale, CV_INTER_AREA);

                    // Ensure the search area is not too small - overly narrow slivers
                    if(searchImage.cols <= templates[temp].cols || searchImage.rows <= templates[temp].rows) continue;
                    
                    // Template match the search area
                    Mat correlation_image; 
                    double min_correlation, max_correlation;
                    
                    matchTemplate( searchImage, templates[temp], correlation_image, CV_TM_CCORR_NORMED );
                    minMaxLoc( correlation_image, &min_correlation, &max_correlation );

                    // Check if the match is good enough! 
                    if(max_correlation < CORRELATION_THRESH) continue;
                    
                    // Check if the region has been matched by another template, pick best

                    light.outer = searchArea;
                    light.state = (LightState) temp;
                    light.correlation = max_correlation;

                    bool found = false;
                    for(int j = 0; j < lights.size(); j++)
                    {
                        bool intersects = ((searchArea & lights[j].outer).area() > 0);
                        if(intersects)
                        {
                            if(lights[j].correlation < max_correlation) lights[j] = light;
                            found = true;
                        }
                    }
                    if(!found) lights.push_back(light); 

                    
                    // Find other lights of same size in image                     
                    Mat matched_template_map;

                    // Template match scaled template
                    matchTemplate( input, scaled_template, correlation_image, CV_TM_CCORR_NORMED );
                    minMaxLoc( correlation_image, &min_correlation, &max_correlation );

                    FindLocalMaxima( correlation_image, matched_template_map, max_correlation*0.9999 );
                    
                    vector<Rect> rectangles;
                    get_matching_template_rectangles( rectangles, matched_template_map, scaled_template, Scalar(0,0,255));
                    
                    
                    // Ensure Results do not intersect with current results
                    for(int rect = 0; rect < rectangles.size(); rect++)
                    {
                        bool found = false;
                        for(int j = 0; j < lights.size(); j++)
                        {
                            bool intersects = ((rectangles[rect] & lights[j].outer).area() > 0);
                            if(intersects)
                            {
                                if(rectangles[rect].area() >= lights[j].inner.area())
                                {
                                    lights[j].inner = rectangles[rect];
                                    lights[j].correlation = max_correlation;
                                }
                                found  = true;
                            }
                        }
                        if(!found) 
                        {
                            light = Light();
                            light.outer = rectangles[rect];
                            light.inner = rectangles[rect];
                            light.correlation = max_correlation;    
                        }
                    }
                    
                }
            }
        }
    }
}

// Calculate recognition metrics
void GetRecognitionResults(vector<vector<Light>> &results, bool centre = true)
{
    // Load Ground Truth
    vector<vector<Light>> ground_truth(N_IMAGES); 

    io::CSVReader<10> ground_truth_file("resources/ground-truth.csv");
    ground_truth_file.read_header(io::ignore_extra_column, "state","c_tlc","c_tlr","c_brc","c_brr","f_tlc","f_tlr","f_brc","f_brr","num");
    int state,c_tlc,c_tlr,c_brc,c_brr,f_tlc,f_tlr,f_brc,f_brr,num;
    while(ground_truth_file.read_row(state,c_tlc,c_tlr,c_brc,c_brr,f_tlc,f_tlr,f_brc,f_brr,num)){
        Light light;
        light.state = (LightState) state;
        light.inner = Rect(Point(c_tlc,c_tlr), Point(c_brc, c_brr));
        light.outer = Rect(Point(f_tlc,f_tlr), Point(f_brc, f_brr));

        ground_truth[num-1].push_back(light);
    }

    // Init values
    int total_truths = 30; // THIS IS LISTED AS 28 IN QUESTION, I'VE PROVIDED METRICS FOR BOTH VALUES IN THE REPORT

    int tp = 0, fp = 0, fn = 0, state_tp = 0, total_matches = 0;

    // Go through all the images
    for(int i=0; i<ground_truth.size(); i++)
    {
        // Keep track of if match is found in ground truth
        vector<bool> result_found(results[i].size(), false);
        total_matches += results[i].size();

        for(int k = 0; k < ground_truth[i].size(); k++)
        {
            Rect truth = centre ? ground_truth[i][k].inner : ground_truth[i][k].outer;
            LightState truth_state = ground_truth[i][k].state;
            
            // Keep track of if ground truth light is found 
            bool found = false, correct_state = false;

            for(int j = 0; j < results[i].size(); j++)
            {   
                Rect res = centre ? results[i][j].inner : results[i][j].outer;
                LightState res_state = results[i][j].state;
                
                // Difference in area, proportional to the area of the ground truth - percentage difference in area
                int area_err = fabs(res.area() - truth.area()) / (double) truth.area();

                // Area of overlap, proportional to the area of the ground truth - percentage of overlap
                double overlap_prop = ((double) (res & truth).area() / (double) truth.area());

                if( area_err < .2 && overlap_prop > .8)
                {
                    if(!found)
                    {
                        found = true; result_found[j] = true;
                        if(res_state == truth_state) correct_state = true;
                        tp++;
                    }
                    else 
                    {
                        cout << "Warning: Multiple overlapping matches were found" << endl;
                        fp++;  result_found[j] = true;
                        if(res_state != truth_state) correct_state = false;
                    }
                }
            }
            if(!found) fn++;
            if(correct_state) state_tp++;
        }

        for(int j = 0; j<results[i].size();j++)
            if(!result_found[j]) fp++;
    }
    
    cout << "FP=" << fp << " TP=" << tp << " FN=" << fn << endl;
    cout << "Recall: " << recall(tp, fn) << " Precision: " << precision(tp, fp) << " Accuracy: " << accuracy(tp, 0, total_matches) 
    << " States " << 100* (double)state_tp/30.0  << "% Correct" << endl;
}

void get_matching_template_rectangles( vector<Rect>& output, Mat& matched_template_map, Mat& template_image, Scalar passed_colour=-1.0 )
{
	int image_channels = matched_template_map.channels();
	int values_on_each_row = matched_template_map.cols;
	for (int row=0; row < matched_template_map.rows; row++) {
		uchar* curr_point = matched_template_map.ptr<uchar>(row);
		for (int column=0; column < values_on_each_row; column++)
		{
			if (*curr_point > 0)
			{
				// Scalar colour( rand()&0xFF, rand()&0xFF, rand()&0xFF );
				Point ul( column, row );
                Point br( column + template_image.cols , row + template_image.rows );
                output.push_back(Rect(ul,br));
				// rectangle( display_image, ul, br, (passed_colour.val[0] == -1.0) ? colour : passed_colour, 1, 8, 0 );
			}
			curr_point += image_channels;
		}
	}
}

void FindLocalMaxima( Mat& input_image, Mat& local_maxima, double threshold_value )
{
	Mat dilated_input_image,thresholded_input_image,thresholded_input_8bit;
	dilate(input_image,dilated_input_image,Mat());
	compare(input_image,dilated_input_image,local_maxima,CMP_EQ);
	threshold( input_image, thresholded_input_image, threshold_value, 255, THRESH_BINARY );
	thresholded_input_image.convertTo( thresholded_input_8bit, CV_8U );
	bitwise_and( local_maxima, thresholded_input_8bit, local_maxima );
}

void FindLocalMinima( Mat& input_image, Mat& local_minima, double threshold_value )
{
	Mat eroded_input_image,thresholded_input_image,thresholded_input_8bit;
	erode(input_image,eroded_input_image,Mat());
	compare(input_image,eroded_input_image,local_minima,CMP_EQ);
	threshold( input_image, thresholded_input_image, threshold_value, 255, THRESH_BINARY_INV );
	thresholded_input_image.convertTo( thresholded_input_8bit, CV_8U );
	bitwise_and( local_minima, thresholded_input_8bit, local_minima );
}