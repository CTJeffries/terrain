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
#include <math.h>
#include <vector>
// RGB Class
class RGB {
    double r, g, b;
    
    int toInt (double value) {
        return (value < 0.0) ? 0 : (value > 1.0) ? 255 :
        (int) (value * 255.0);
    }
    
public:
    RGB(){
        this->r = 1.0;
        this->g = 1.0;
        this->b = 1.0;
    }
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


// Triple Class
class Triple {
  double x, y, z;

  public:
    Triple(){
        this->x = 0.0;
        this->y = 0.0;
        this->z = 0.0;
    }
    Triple (double x, double y, double z) {
      this->x = x;
      this->y = y;
      this->z = z;
    }

    Triple add (Triple t) {
      return Triple (x + t.x, y + t.y, z + t.z);
    }

    Triple subtract (Triple t) {
      return Triple (x - t.x, y - t.y, z - t.z);
    }

    Triple cross (Triple t) {
      return Triple (y * t.z - z * t.y, z * t.x - x * t.z,
        x * t.y - y * t.x);
    }

    double dot (Triple t) {
      return x * t.x + y * t.y + z * t.z;
    }

    double length2 () {
      return dot (*this);
    }

    Triple normalize () {
      return scale (1.0 / sqrt (length2 ()));
    }

    Triple scale (double scale) {
      return Triple (x * scale, y * scale, z * scale);
    }
};

// Triangle Class
class Triangle {
    int i[3];
    int j[3];
  Triple n;
    RGB color;
  /*Color color;*/

  public:
    Triangle (){
        for (int t = 0; t < 3; t++) {
            i[t] = 0;
            j[t] = 0;
        }
    }
    Triangle (int i0, int j0, int i1, int j1, int i2, int j2) :  n(0.0, 0.0, 0.0), color(1.0, 1.0, 1.0){
      i[0] = i0;
      i[1] = i1;
      i[2] = i2;
      j[0] = j0;
      j[1] = j1;
      j[2] = j2;
    }
};


class FractalTerrain {
    std::vector<std::vector<double> > terrain;
    double roughness, min, max;
    int divisions;

    RGB blue;
    RGB green;
    RGB white;

public:
    FractalTerrain (int lod, double roughness);
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
