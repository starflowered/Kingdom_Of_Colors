#pragma once

#include <GL/gl.h>
#include <cmath>

/* PI */
#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif


inline void draw_sphere(const double radius, const int lats, const int longs)
{
    for (int i = 0; i <= lats; i++)
    {
        const double lat0 = M_PI * (-0.5 + static_cast<double>(i - 1) / lats);
        const double z0 = radius * sin(lat0);
        const double zr0 = radius * cos(lat0);

        const double lat1 = M_PI * (-0.5 + static_cast<double>(i) / lats);
        const double z1 = radius * sin(lat1);
        const double zr1 = radius * cos(lat1);

        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= longs; j++)
        {
            const double lng = 2 * M_PI * static_cast<double>(j - 1) / longs;
            const double x = cos(lng);
            const double y = sin(lng);

            glNormal3f(x * zr0, y * zr0, z0);
            glVertex3f(x * zr0, y * zr0, z0);
            glNormal3f(x * zr1, y * zr1, z1);
            glVertex3f(x * zr1, y * zr1, z1);
        }
        glEnd();
    }
}
