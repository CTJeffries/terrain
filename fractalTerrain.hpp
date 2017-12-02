//
//  FractalTerrain.hpp
//  
//
//  Created by John Bacher on 12/2/17.
//
//

#ifndef FractalTerrain_hpp
#define FractalTerrain_hpp

#include <iostream>

// RGB Class
class RGB {
    double r, g, b;
    
    int toInt (double value) {
        return (value < 0.0) ? 0 : (value > 1.0) ? 255 :
        (int) (value * 255.0);
    }
    
public:
    RGB (double r, double g, double b) {
        this->r = r;
        this->g = g;
        this->b = b;
    }
    
    RGB add(RGB rgb) {
        return RGB(r + rgb.r, g + rgb.g, b + rgb.b);
    }
    
    RGB subtract(RGB rgb) {
        return RGB(r - rgb.r, g - rgb.g, b - rgb.b);
    }
    
    RGB scale(double scale) {
        return RGB(r * scale, g * scale, b * scale);
    }
    
    int toRGB() {
        return (0xff << 24) | (toInt(r) << 16) |
        (toInt(g) << 8) | toInt(b);
    }
};

class FractalTerrain {
    double **terrain;
    double roughness, min, max;
    int divisions;
    
    RGB blue;
    RGB green;
    RGB white;
    
public:
    FractalTerrain (int lod, double roughness);
    ~FractalTerrain(){
        delete [] terrain;
    }
    void diamond (int x, int y, int side, double scale);
    void square (int x, int y, int side, double scale);
    double rnd ();
    double getAltitude (double i, double j);
    RGB getColor (double i, double j);
    
    void print(){
        for (int i = 0; i < divisions; i++) {
            for (int j = 0; j < divisions; j++) {
                std::cout << terrain[i][j] << "\t";
            }
            std::cout << "\n";
        }
    }
};

#endif /* FractalTerrain_hpp */
