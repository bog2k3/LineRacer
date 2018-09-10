#ifndef __TRANSFORM_H__
#define __TRANSFORM_H__

struct Transform {
    float scale = 1;    // real-to-virtual pixel ratio
    float transX = 0;   // virtual pixel offset
    float transY = 0;   // virtual pixel offset
};

#endif //__TRANSFORM_H__