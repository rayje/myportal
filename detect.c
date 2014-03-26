#include "detect.h"

int in_sector(double x, double z, float sector[4]) {
    float tol = 0.4;
    if (x <= sector[0]+tol && x >= sector[1]-tol &&
        z <= sector[2]+tol && z >= sector[3]-tol) {
        return 1; 
    }

    return 0;
}