#pragma once 
#include "lights.h"
#include "utilities.h"

static inline double recall(double tp, double fn) 
{
    return tp /( tp + fn);
}

static inline double precision(double tp, double fp) 
{
    return tp/(tp + fp);
}

static inline double accuracy( double tp, double tn, double total) 
{
    return (tp + tn) / total;
}

// Evaluate recognition metrics for the specified thing
void compare_with_ground_truth(vector<Light> &results, vector<Light> &ground_truth, bool inner = false)
{
    for(int i = 0; i < ground_truth.size(); i++)
    {
        Rect truth = inner ? ground_truth[i].inner : ground_truth[i].outer; 

        for(int j = 0; j < results.size(); j++)
        {
            Rect res = inner ? results[j].inner : results[j].outer;
            
            // Difference in area, proportional to the area of the ground truth - percentage difference in area
            int area_err = fabs(res.area() - truth.area()) / (double) truth.area();

            // Area of overlap, proportional to the area of the ground truth - percentage of overlap
            double overlap_prop = ((double) (res & truth).area() / (double) truth.area());
                
            double dice = 2.0* double((res & truth).area()) / double(res.area() + truth.area());

            if( area_err < .2 && overlap_prop > .8)
            {
                if(!ground_truth[i].get_found(inner))
                {
                    ground_truth[i].set_found(true, inner); results[j].set_found(true, inner);
                    ground_truth[i].set_dice(dice,inner);
                    
                    if(ground_truth[i].state == results[j].state) 
                    {
                        ground_truth[i].correct_state = true; results[j].correct_state = true;
                    }
                }
                else 
                {
                    cout << "Warning: Multiple overlapping matches were found" << endl;
                    
                    // Get worst case
                    if(ground_truth[i].state != results[j].state) 
                        ground_truth[i].correct_state = false;      
                    if(ground_truth[i].get_dice(inner) > dice) ground_truth[i].set_dice(dice,inner);
                }
            }
        }

    }
}

// Calculate recognition metrics
void get_recognition_results(vector<vector<Light>> &results, vector<vector<Light>> &ground_truth, bool inner = true)
{
    // Init values
    int tp = 0, fp = 0, fn = 0, state_tp = 0, total_matches = 0, total_truths = 0;
    double dice_sum = 0;

    // Go through all the images
    for(int i = 0; i < ground_truth.size(); i++)
    {
        total_matches += results[i].size();
        total_truths += ground_truth[i].size();

        for(auto&& truth : ground_truth[i])
        {
            dice_sum += truth.get_dice(inner);
     
            if(truth.get_found(inner)) tp++;
            else fn++;
     
            if(truth.correct_state) state_tp++;
        }

        for(auto&& res : results[i])
        {
            if(!res.get_found(inner)) fp++;
        }
    }
    
    cout << "FP=" << fp << " TP=" << tp << " FN=" << fn << endl;
    cout << "Recall: " << recall(tp, fn) 
    << " Precision: " << precision(tp, fp) 
    << " Accuracy: " << accuracy(tp, 0, tp+fp+fn) 
    << " Avg Dice: " << dice_sum/total_truths
    << " States " << 100* (double)state_tp/total_truths  << "% Correct" << endl;
}