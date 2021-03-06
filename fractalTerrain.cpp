//
//  FractalTerrain.cpp
//
//
//  Created by John Bacher on 12/2/17.
//
//

#include "fractalTerrain.hpp"

// Generates fractal terrain.
FractalTerrain::FractalTerrain (int lod, double roughness) {
        this->roughness = roughness;
        this->divisions = 1 << lod;
        terrain = std::vector<std::vector <double> >(divisions + 1, std::vector<double>(divisions + 1));

        blue = RGB(0.0, 0.3, 1.0);
        green = RGB(0.0, 0.8, 0.1);
        white = RGB(0.68, 0.68, 0.7);

        terrain[0][0] = rnd ();
        terrain[0][divisions] = rnd ();
        terrain[divisions][divisions] = rnd ();
        terrain[divisions][0] = rnd ();
        double rough = roughness;
        // Diamond-Square algorithm.
        for (int i = 0; i < lod; ++ i) {
            int q = 1 << i, r = 1 << (lod - i), s = r >> 1;
            for (int j = 0; j < divisions; j += r)
                for (int k = 0; k < divisions; k += r)
                    diamond (j, k, r, rough);
            if (s > 0)
                for (int j = 0; j <= divisions; j += s)
                    for (int k = (j + s) % r; k <= divisions; k += r)
                        square (j - s, k - s, r, rough);
            rough *= roughness;
        }

        min = max = terrain[0][0];
        for (int i = 0; i <= divisions; ++ i)
            for (int j = 0; j <= divisions; ++ j)
                if (terrain[i][j] < min) min = terrain[i][j];
                else if (terrain[i][j] > max) max = terrain[i][j];
    }

 void FractalTerrain::diamond (int x, int y, int side, double scale) {
        if (side > 1) {
            int half = side / 2;
            double avg = (terrain[x][y] + terrain[x + side][y] +
                          terrain[x + side][y + side] + terrain[x][y + side]) * 0.25;
            terrain[x + half][y + half] = avg + rnd () * scale;
        }
    }

 void FractalTerrain::square (int x, int y, int side, double scale) {
        int half = side / 2;
        double avg = 0.0, sum = 0.0;
        if (x >= 0)
        { avg += terrain[x][y + half]; sum += 1.0; }
        if (y >= 0)
        { avg += terrain[x + half][y]; sum += 1.0; }
        if (x + side <= divisions)
        { avg += terrain[x + side][y + half]; sum += 1.0; }
        if (y + side <= divisions)
        { avg += terrain[x + half][y + side]; sum += 1.0; }
        terrain[x + half][y + half] = avg / sum + rnd () * scale;
    }

 double FractalTerrain::rnd () {
    return rand() % 40000 / 10000.0 - 2;
    }

 double FractalTerrain::getAltitude (double i, double j) {
        double alt = terrain[(int) (i * divisions)][(int) (j * divisions)];
        return (alt - min) / (max - min);
    }



 RGB FractalTerrain::getColor (double i, double j) {
        double a = getAltitude (i, j);
        if (a < .5)
            return blue.add (green.subtract (blue).scale ((a - 0.0) / 0.5));
        else
            return green.add (white.subtract (green).scale ((a - 0.5) / 0.5));
    }
