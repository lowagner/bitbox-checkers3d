#ifndef BB3D_H
#define BB3D_H
#include "bitbox.h"
#include "math.h" // sqrt
#include "stdint.h" // uint8_t

typedef struct _vertex {
    union {
        struct {
            float world[3]; // coordinates in the world reference frame
        };
        struct {
            float x, y, z;
        };
    };
    union {
        struct {
            int image[2]; // coordinates for the image
            float image_z; // remaining z-coordinate
        };
        struct {
            int ix, iy; // coordinates for the image
            float iz; // remaining z-coordinate
        };
    };
    uint8_t player_index;
    uint8_t power_level;
    uint16_t color;
} vertex;

typedef struct _Camera {
    float viewer[3]; // position of the camera
    float viewee[3]; // object which the camera is looking at
    float right[3]; // vector which points right in the camera reference frame
    float down[3]; // vector which points down in the camera reference frame
    float forward[3]; // vector which points forward in the camera reference frame
    float magnification; // multiplier to project points onto screen
    float view_matrix[12]; // camera matrix
} Camera;

inline float sqr(float x)
{
    return x*x;
}

// normalize vector_in and store it in vector_out:
inline void normalize(float *vector_out, float *vector_in)
{
    float mag = sqrt(sqr(vector_in[0]) + sqr(vector_in[1]) + sqr(vector_in[2]));
    #ifdef DEBUG
    if (mag == 0.0f)
    {
        message("divide by zero in normalization!!!\n");
        return;
    }
    #endif
    vector_out[0] = vector_in[0]/mag;
    vector_out[1] = vector_in[1]/mag;
    vector_out[2] = vector_in[2]/mag;
    //vector_out[3] = 1.0f; 
}
// TODO:consider dropping vector_in, vector_out unless there is a use-case;
//      i.e. just use put in one vector to normalize.

// dot product
inline float dot(float *vector1, float *vector2)
{
    return vector1[0]*vector2[0] + vector1[1]*vector2[1] + vector1[2]*vector2[2];
}

// safe cross product:  vector_out can be one of vector1 or vector2, 
// at the expense of extra memory used:
void cross(float *vector_out, float *vector1, float *vector2);

// unsafe cross:  vector_out should be unique, different than vector1 or vector2:
void cross0(float *vector_out, float *vector1, float *vector2);

// safe matrix multiply:  vector_out = matrix * vector,
// vector_out could be the same as vector:
void matrix_multiply_vector(float *vector_out, float *matrix, float *vector);

// unsafe matrix multiply:  vector_out should be unique, different than vector:
void matrix_multiply_vector0(float *vector_out, float *matrix, float *vector);

// safe matrix matrix multiply; matrix_out can be one of matrix1 or matrix2:
void matrix_multiply_matrix(float *matrix_out, float *matrix1, float *matrix2);

// unsafe matrix matrix multiply; matrix_out SHOULD NOT be one of matrix1 or matrix2:
void matrix_multiply_matrix0(float *matrix_out, float *matrix1, float *matrix2);

// get a 3x4 matrix which describes a translation to the vector origin:
void get_translation_matrix(float *matrix, float *origin);

// setup the camera view:
void get_view(Camera *camera); 

// convenience function to swap the memory of two vertices:
inline void swap(vertex *v1, vertex *v2)
{
    vertex v3 = *v1; 
    *v1 = *v2;
    *v2 = v3;
}

#endif
