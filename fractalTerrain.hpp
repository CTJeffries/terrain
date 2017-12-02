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

// Triple Class
class Triple {
  double x, y, z;

  public:
    Triple (double x, double y, double z) {
      this.x = x;
      this.y = y;
      this.z = z;
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
      return dot (this);
    }

    Triple normalize () {
      return scale (1.0 / Math.sqrt (length2 ()));
    }

    Triple scale (double scale) {
      return Triple (x * scale, y * scale, z * scale);
    }
}

// Triangle Class
class Triangle {
  int[] i = int[3];
  int[] j = int[3];
  Triple n;
  RGB[] rgb = RGB[3];
  Color color;

  public:
    Triangle (int i0, int j0, int i1, int j1, int i2, int j2) {
      i[0] = i0;
      i[1] = i1;
      i[2] = i2;
      j[0] = j0;
      j[1] = j1;
      j[2] = j2;
    }
}

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
}

class FractalTerrain {
    double[][] terrain;
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
