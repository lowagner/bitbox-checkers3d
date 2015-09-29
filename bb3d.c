#include "bb3d.h"
#include "string.h" // memcpy


void cross(float *vector_out, float *vector1, float *vector2)
{
    // in case vector_out is one of vector1 or vector2, 
    // we create the result independently then put it into vector_out.
    float x, y, z;
    x = vector1[1]*vector2[2] - vector1[2]*vector2[1];
    y = vector1[2]*vector2[0] - vector1[0]*vector2[2];
    z = vector1[0]*vector2[1] - vector1[1]*vector2[0];
    vector_out[0] = x;
    vector_out[1] = y;
    vector_out[2] = z;
    //vector_out[3] = 1.0f; 
}

void cross0(float *vector_out, float *vector1, float *vector2)
{
    // if we know that vector_out is not vector1 or vector2,
    // we can do things a bit more efficiently:
    vector_out[0] = vector1[1]*vector2[2] - vector1[2]*vector2[1];
    vector_out[1] = vector1[2]*vector2[0] - vector1[0]*vector2[2];
    vector_out[2] = vector1[0]*vector2[1] - vector1[1]*vector2[0];
    //vector_out[3] = 1.0f; 
}

void matrix_multiply_vector(float *vector_out, float *matrix, float *vector)
{
    // in case vector_out is vector, 
    // we create the result independently then put it into vector_out.
    float x, y, z; //, w;
    x = matrix[0]*vector[0] + matrix[1]*vector[1] + matrix[ 2]*vector[2] + matrix[3]; // *vector[3] which is 1
    y = matrix[4]*vector[0] + matrix[5]*vector[1] + matrix[ 6]*vector[2] + matrix[7];
    z = matrix[8]*vector[0] + matrix[9]*vector[1] + matrix[10]*vector[2] + matrix[11];
    //w = matrix[12]*vector[0] + matrix[13]*vector[1] + matrix[14]*vector[2] + matrix[15]*vector[3];
    vector_out[0] = x;
    vector_out[1] = y;
    vector_out[2] = z;
    // vector_out[3] = 1.0f; 
}

void matrix_multiply_vector0(float *vector_out, float *matrix, float *vector)
{
    // unsafe, if vector_out == vector.
    vector_out[0] = matrix[0]*vector[0] + matrix[1]*vector[1] + matrix[ 2]*vector[2] + matrix[3];
    vector_out[1] = matrix[4]*vector[0] + matrix[5]*vector[1] + matrix[ 6]*vector[2] + matrix[7];
    vector_out[2] = matrix[8]*vector[0] + matrix[9]*vector[1] + matrix[10]*vector[2] + matrix[11];
    //vector_out[3] = matrix[12]*vector[0] + matrix[13]*vector[1] + matrix[14]*vector[2] + matrix[15]*vector[3];
    //vector_out[3] = 1.0f; 
}

void matrix_multiply_matrix(float *matrix_out, float *matrix1, float *matrix2)
{
    float matrix[12]; // since matrix_out might be one of matrix1 or matrix2, put result here.
    // no fancy algorithms, just brute force matrix multiplication, assuming
    // the bottom row of each matrix (e.g. matrix[12:16]) is [0,0,0,1]
    matrix[0]  = matrix1[0]*matrix2[0] + matrix1[1]*matrix2[4] + matrix1[2]*matrix2[8];
    matrix[1]  = matrix1[0]*matrix2[1] + matrix1[1]*matrix2[5] + matrix1[2]*matrix2[9];
    matrix[2]  = matrix1[0]*matrix2[2] + matrix1[1]*matrix2[6] + matrix1[2]*matrix2[10];
    matrix[3]  = matrix1[0]*matrix2[3] + matrix1[1]*matrix2[7] + matrix1[2]*matrix2[11] + matrix1[3];
    matrix[4]  = matrix1[4]*matrix2[0] + matrix1[5]*matrix2[4] + matrix1[6]*matrix2[8];
    matrix[5]  = matrix1[4]*matrix2[1] + matrix1[5]*matrix2[5] + matrix1[6]*matrix2[9];
    matrix[6]  = matrix1[4]*matrix2[2] + matrix1[5]*matrix2[6] + matrix1[6]*matrix2[10];
    matrix[7]  = matrix1[4]*matrix2[3] + matrix1[5]*matrix2[7] + matrix1[6]*matrix2[11] + matrix1[7];
    matrix[8]  = matrix1[8]*matrix2[0] + matrix1[9]*matrix2[4] + matrix1[10]*matrix2[8];
    matrix[9]  = matrix1[8]*matrix2[1] + matrix1[9]*matrix2[5] + matrix1[10]*matrix2[9];
    matrix[10] = matrix1[8]*matrix2[2] + matrix1[9]*matrix2[6] + matrix1[10]*matrix2[10];
    matrix[11] = matrix1[8]*matrix2[3] + matrix1[9]*matrix2[7] + matrix1[10]*matrix2[11] + matrix1[11];

    memcpy(matrix_out, matrix, sizeof(float)*12);
}

void matrix_multiply_matrix0(float *matrix_out, float *matrix1, float *matrix2)
{
    // matrix_out is NOT one of matrix1 or matrix2.
    // no fancy algorithms, just brute force matrix multiplication, assuming
    // the bottom row of each matrix (e.g. matrix[12:16]) is [0,0,0,1]
    matrix_out[0]  = matrix1[0]*matrix2[0] + matrix1[1]*matrix2[4] + matrix1[2]*matrix2[8];
    matrix_out[1]  = matrix1[0]*matrix2[1] + matrix1[1]*matrix2[5] + matrix1[2]*matrix2[9];
    matrix_out[2]  = matrix1[0]*matrix2[2] + matrix1[1]*matrix2[6] + matrix1[2]*matrix2[10];
    matrix_out[3]  = matrix1[0]*matrix2[3] + matrix1[1]*matrix2[7] + matrix1[2]*matrix2[11] + matrix1[3];
    matrix_out[4]  = matrix1[4]*matrix2[0] + matrix1[5]*matrix2[4] + matrix1[6]*matrix2[8];
    matrix_out[5]  = matrix1[4]*matrix2[1] + matrix1[5]*matrix2[5] + matrix1[6]*matrix2[9];
    matrix_out[6]  = matrix1[4]*matrix2[2] + matrix1[5]*matrix2[6] + matrix1[6]*matrix2[10];
    matrix_out[7]  = matrix1[4]*matrix2[3] + matrix1[5]*matrix2[7] + matrix1[6]*matrix2[11] + matrix1[7];
    matrix_out[8]  = matrix1[8]*matrix2[0] + matrix1[9]*matrix2[4] + matrix1[10]*matrix2[8];
    matrix_out[9]  = matrix1[8]*matrix2[1] + matrix1[9]*matrix2[5] + matrix1[10]*matrix2[9];
    matrix_out[10] = matrix1[8]*matrix2[2] + matrix1[9]*matrix2[6] + matrix1[10]*matrix2[10];
    matrix_out[11] = matrix1[8]*matrix2[3] + matrix1[9]*matrix2[7] + matrix1[10]*matrix2[11] + matrix1[11];
}

void get_translation_matrix(float *matrix, float *origin)
{
    memset(matrix, 0, 12*sizeof(float)); // could use 11 here, since we set the 12th element.
    matrix[0] = 1.0f; // matrix[0][0]
    matrix[5] = 1.0f; // matrix[1][1]
    matrix[10] = 1.0f; // matrix[2][2]
    // in the following, a translation requires minus signs, 
    // because we want vectors to think of the origin vector as their new origin.
    matrix[3] = -origin[0];
    matrix[7] = -origin[1];
    matrix[11] = -origin[2];
    // thus any vector which is equal to origin will become a zero vector,
    // which is exactly what we want.
}

void get_view(Camera *camera)
{
    // get the 3x4 matrix for looking at position camera->viewee from position camera->viewer,
    // with vector camera->down as world down direction.  

    // first we need to get rotation matrix, for camera (or viewer) orientation:
    float rotation_matrix[12];
    // get forward direction:
    camera->forward[0] = camera->viewee[0] - camera->viewer[0];
    camera->forward[1] = camera->viewee[1] - camera->viewer[1];
    camera->forward[2] = camera->viewee[2] - camera->viewer[2];
    normalize(camera->forward, camera->forward);
    // get right vector, what points over to your right.
    cross0(camera->right, camera->down, camera->forward); // right = down cross forward
    normalize(camera->right, camera->right);
    // recompute down as: down = right x forward, in case forward and down weren't perpendicular:
    cross0(camera->down, camera->forward, camera->right); 
    // forward and right are normalized/perpendicular now so down is normalized, too.

    // copy right, down, and forward to create the rotation matrix:
    rotation_matrix[0] = camera->right[0]; rotation_matrix[1] = camera->right[1]; rotation_matrix[2] = camera->right[2]; rotation_matrix[3] = 0.0;
    rotation_matrix[4] = camera->down[0]; rotation_matrix[5] = camera->down[1]; rotation_matrix[6] = camera->down[2]; rotation_matrix[7] = 0.0;
    rotation_matrix[8] = camera->forward[0]; rotation_matrix[9] = camera->forward[1]; rotation_matrix[10] = camera->forward[2]; rotation_matrix[11] = 0.0;

    // then we setup the matrix as a translation to the viewer position:
    get_translation_matrix(camera->view_matrix, camera->viewer);
    // the full view matrix is first a translation to the viewer, then a rotation,
    // and recall that matrix operations are performed right to left:
    matrix_multiply_matrix(camera->view_matrix, rotation_matrix, camera->view_matrix);
}

