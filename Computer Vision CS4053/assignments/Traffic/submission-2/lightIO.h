#pragma once

#include "utilities.h"
#include "lights.h"

// CSV Reading library for reading ground truth from https://github.com/ben-strasser/fast-cpp-csv-parser
#include "csv.h"

#include <iostream> 
#include <iomanip>
#include <sstream>
#include <vector>
#include <set> 
#include <numeric>
using namespace std;
using namespace cv;

// Read Images In
vector<Mat> get_traffic_images(int n)
{
    String res_path("resources/");
    String input_path(res_path + "input/CamVidLights");

    vector<Mat> images(n);

    ostringstream path;
    for(int i = 1; i <= n; i++)
    {
        path.str("");
        path << input_path << setw(2) << setfill('0') << i << ".png";
        images[i-1] = imread( path.str(), IMREAD_COLOR );
    }
    return images;
}

// Get Ground Truth Data 
vector<vector<Light>> get_ground_truth(char * truth_file, int n)
{
    // Load Ground Truth
    vector<vector<Light>> ground_truth(n); 

    io::CSVReader<10> ground_truth_file(truth_file);
    ground_truth_file.read_header(io::ignore_extra_column, "state","c_tlc","c_tlr","c_brc","c_brr","f_tlc","f_tlr","f_brc","f_brr","num");
    int state,c_tlc,c_tlr,c_brc,c_brr,f_tlc,f_tlr,f_brc,f_brr,num;
    while(ground_truth_file.read_row(state,c_tlc,c_tlr,c_brc,c_brr,f_tlc,f_tlr,f_brc,f_brr,num)){
        Light light;
        light.state = (LightState) state;
        light.inner = Rect(Point(c_tlc,c_tlr), Point(c_brc, c_brr));
        light.outer = Rect(Point(f_tlc,f_tlr), Point(f_brc, f_brr));

        ground_truth[num-1].push_back(light);
    }

    return ground_truth;
}

// Output Result Images
void write_traffic_images(vector<Mat> &images)
{
    String res_path("");
    String input_path(res_path + "output/CamVidLights");

    ostringstream path;
    for(int i = 0; i < images.size(); i++)
    {
        path.str("");
        path << input_path << setw(2) << setfill('0') << i+1 << ".png";
        imwrite(path.str(), images[i]);
    }
}