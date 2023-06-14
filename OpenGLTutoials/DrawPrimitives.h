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

inline void draw_cone_top(const GLfloat radius, const GLdouble height, const GLint slices)
{
    const double angle = 2 * M_PI / slices;
    
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0, 0, static_cast<GLfloat>(height));

    for (int i = 0; i <= slices; i++)
    {
        const auto total_angle = static_cast<float>(i * angle);
        glVertex3f(radius * cosf(total_angle), radius * sinf(total_angle), 0.f);
    }
    
    glEnd();
}

inline void draw_circle(const GLfloat radius, const GLint slices)
{
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(0, 0);

    const double angle = 2 * M_PI / slices;

    for (int i = 0; i <= slices; i++)
    {
        const auto total_angle = static_cast<float>(i * angle);
        glVertex2f(radius * cosf(total_angle), radius * sinf(total_angle));
    }

    glEnd();
}


inline void draw_cone(const GLfloat radius, const GLdouble height, const GLint slices)
{
    // draw cone top
    draw_cone_top(radius, height, slices);

    // draw cone bottom
    draw_circle(radius, slices);
}

