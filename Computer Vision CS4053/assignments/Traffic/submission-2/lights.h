#pragma once

#include "utilities.h"
using namespace std;
using namespace cv;

enum LightState {
    RED,
    AMBER,
    RED_AMBER,
    GREEN
};

struct Light {
    Rect inner;
    Rect outer;
    LightState state;

    Rect light;

    double inner_dice=0;
    double outer_dice=0;
    bool inner_found = false;
    bool outer_found = false;
    bool correct_state = false;

    void set_found(bool found, bool inner) 
    {
        if(inner) inner_found = found;
        else outer_found = found;
    }

    bool get_found(bool inner) 
    {
        if(inner) return inner_found;
        else return outer_found;
    }

    void set_dice(double dice, bool inner) 
    {
        if(inner) inner_dice = dice;
        else outer_dice = dice;
    }

    double get_dice(bool inner) 
    {
        if(inner) return inner_dice;
        else return outer_dice;
    }
};

char * state_str(LightState state) { 
    switch(state)
    {
        case RED: return "RED";
        case RED_AMBER: return "RED-AMBER";
        case GREEN: return "GREEN";
        case AMBER: return "AMBER";
        default: return NULL;
    }
}

Scalar state_color(LightState state) {
    Scalar state_colors[] = {
        Scalar(0,0,255),
        Scalar(0,180,255),
        Scalar(0,100,255),
        Scalar(0,255,0)
    };

    return state_colors[state];
}