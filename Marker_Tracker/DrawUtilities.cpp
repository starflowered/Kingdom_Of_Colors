#include "DrawUtilities.h"

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>


unsigned int framebuffer;
unsigned int textureColorbuffer;
unsigned int rbo;
unsigned int quadVAO, quadVBO;

float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
    // positions   // texCoords
    -1.0f,  1.0f,  0.0f, 1.0f,
    -1.0f, -1.0f,  0.0f, 0.0f,
     1.0f, -1.0f,  1.0f, 0.0f,

    -1.0f,  1.0f,  0.0f, 1.0f,
     1.0f, -1.0f,  1.0f, 0.0f,
     1.0f,  1.0f,  1.0f, 1.0f
};
void checkCompileErrors(GLuint shader, std::string type)
{
    GLint success;
    GLchar infoLog[1024];
    if (type != "PROGRAM")
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
    else
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
}



unsigned int createShader(const char* vertexPath, const char* fragmentPath)
{
    

    // 1. retrieve the vertex/fragment source code from filePath
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;
    // ensure ifstream objects can throw exceptions:
    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        // open files
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);
        std::stringstream vShaderStream, fShaderStream;
        // read file's buffer contents into streams
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();
        // close file handlers
        vShaderFile.close();
        fShaderFile.close();
        // convert stream into string
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
    }
    catch (std::ifstream::failure& e)
    {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
    }
    //glewInit();
    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();
    // 2. compile shaders
    unsigned int vertex, fragment;
    // vertex shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    checkCompileErrors(vertex, "VERTEX");
    // fragment Shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT");
    // shader Program
    unsigned ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM");
    // delete the shaders as they're linked into our program now and no longer necessary
   // glDeleteShader(vertex);
   // glDeleteShader(fragment);
    return ID;

}
unsigned int drawShader;


void ogl_draw_background_image(const Mat& img, const int win_width, const int win_height)
{
 
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // determine current scale factor image:window (the latter could be resized)
    const int img_width = img.cols;
    const int img_height = img.rows;
    //float img_scale = (float)winWidth / (float)imgWidth;
    const float img_scale_width = static_cast<float>(win_width) / static_cast<float>(img_width);
    const float img_scale_height = static_cast<float>(win_height) / static_cast<float>(img_height);

    // In the ortho view all objects stay the same size at every distance
    glOrtho(0.0, img_width * img_scale_width, 0.0, img_height * img_scale_height, -1, 1);

    // Render the camera picture as background texture
    // Making a raster of the image -> -1 otherwise overflow
    glRasterPos2i(0, img_height * img_scale_height - 1); // select pixel from which to start drawing (here top left)
    // scale according to current window size
    glPixelZoom(img_scale_width, -img_scale_height);

    // Load and render the camera image -> unsigned byte because of img_bgr.data as unsigned char array
    // 3 channels -> pixel wise rendering
    glDrawPixels(img_width, img_height, GL_BGR_EXT, GL_UNSIGNED_BYTE, img.data);

    // Activate depth -> that snowman can be scaled with depth
    glEnable(GL_DEPTH_TEST);
}

void init_gl(int argc, char* argv[])
{
    // Added in Exercise 8 - End *****************************************************************

    // For our connection between OpenCV/OpenGL
    // Pixel storage/packing stuff -> how to handle the pixel on the graphics card
    // For glReadPixels​ -> Pixel representation in the frame buffer
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    // For glTexImage2D​ -> Define the texture image representation
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    // Turn the texture coordinates from OpenCV to the texture coordinates OpenGL
    glPixelZoom(1.0, -1.0);

    // Added in Exercise 8 - End *****************************************************************

    // Enable and set colors
    glEnable(GL_COLOR_MATERIAL);
    glClearColor(0, 0, 0, 1.0);

    // Enable and set depth parameters
    glEnable(GL_DEPTH_TEST);
    glClearDepth(1.0);

    // Light parameters
    constexpr GLfloat light_amb[] = {0.2f, 0.2f, 0.2f, 1.0};
    constexpr GLfloat light_pos[] = {1.0, 1.0, 1.0, 0.0};
    constexpr GLfloat light_dif[] = {0.7f, 0.7f, 0.7f, 1.0};

    // Enable lighting
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_amb);
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_dif);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);


}

void ogl_display_pnp(GLFWwindow* window, const Mat& img_bgr, map<int, hexagon>& hexagon_map,
    map<int, marker>& marker_map)
{
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glUseProgram(0);
    glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    int win_width, win_height;
    glfwGetFramebufferSize(window, &win_width, &win_height);
    ogl_draw_background_image(img_bgr, win_width, win_height);

    // calling this prevented anything drawn to be visible when using an image! 
    //ogl_set_viewport_and_frustum_pnp(window, win_width, win_height);

    for (auto& hexagon : hexagon_map | views::values)
    {
        draw_hexagon(hexagon, marker_map);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST);
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(drawShader);
    int uniform_WindowSize = glGetUniformLocation(drawShader, "WindowSize");
    glUniform2f(uniform_WindowSize, win_width, win_height);
    glBindVertexArray(quadVAO);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);	// use the color attachment texture as the texture of the quad plane
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
/*    glUseProgram(drawShader);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    // make sure we clear the framebuffer's content
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    int win_width, win_height;
    glfwGetFramebufferSize(window, &win_width, &win_height);
    ogl_draw_background_image(img_bgr, win_width, win_height);


    // calling this prevented anything drawn to be visible when using an image! 
    // ogl_set_viewport_and_frustum_pnp(window, win_width, win_height);

    for (auto& hexagon : hexagon_map | views::values)
    {
        //draw_hexagon(hexagon, marker_map);
    }

    
    glUseProgram(texShader);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    //glDisable(GL_DEPTH_TEST); // disable depth test so screen-space quad isn't discarded due to depth test.
    // clear all relevant buffers
    //glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // set clear color to white (not really necessary actually, since we won't be able to see behind the quad anyways)
    glClear(GL_COLOR_BUFFER_BIT);

    
    //FontUtilities::render_text(window, win_width, win_height,img_bgr);*/

    //glBindVertexArray(quadVAO);
    //glBindTexture(GL_TEXTURE_2D, textureColorbuffer);	// use the color attachment texture as the texture of the quad plane
    //glDrawArrays(GL_TRIANGLES, 0, 6);

}

void ogl_set_viewport_and_frustum_pnp(GLFWwindow* window, int width, int height)
{
    if (width == 0)
        glfwGetFramebufferSize(window, &width, &height);

    glViewport(0, 0, width, height);

    // Setup for the camera matrix
    glMatrixMode(GL_PROJECTION);

    /*
    * According to the book by Ram Kumar "Demystifying AR", 2018:
    * youtube: https://www.youtube.com/watch?v=ZGu8wafekpM
    * book: https://github.com/ramkalath/Augmented_Reality_Tutorials/blob/master/demystifying_AR.pdf
    * code: https://github.com/ramkalath/Augmented_Reality_Tutorials
    *
    * see also: ksimek.github.io/2013/06/03/calibrated_cameras_in_opengl/
    *
    * see also: Christoph Krautz's analysis of different world coordinate systems:
    * post: https://medium.com/comerge/what-are-the-coordinates-225f1ec0dd78
    */

    constexpr float near = 0.1f;
    constexpr float far = 100.0f;
    const int max_d = max(width, height);
    const auto fx = static_cast<float>(max_d);
    const auto fy = static_cast<float>(max_d);
    const float cx = static_cast<float>(width) / 2.0f;
    const float cy = static_cast<float>(height) / 2.0f;

    constexpr float a = -(far + near) / (far - near);
    constexpr float b = -(2 * far * near) / (far - near);
    const float perspective_view[16] = {
        // transposed
        fx / cx, 0.0f, 0.0f, 0.0f,
        0.0f, fy / cy, 0.0f, 0.0f,
        0.0f, 0.0f, a, -1.0f,
        0.0f, 0.0f, b, 0.0f
    };
    glLoadMatrixf(perspective_view);
}

int setup_framebuffer()
{
    int width = camera_width;
    int height = camera_height;
    glewInit();
    glfwInit();
   // drawShader= createShader("shaders//vs_drawing.glsl", "shaders//fs_drawing.glsl");
   // texShader = createShader("shaders//vs.glsl", "shaders//fs.glsl");

    drawShader=FontUtilities::CompileShaders(true, false, false, false, true);
    checkCompileErrors(drawShader, "PROGRAM");
    glUseProgram(drawShader);

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    // generate texture

    glGenTextures(1, &textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width,height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    // attach it to currently bound framebuffer object
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);


    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width,height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

  

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
        return -1;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    cout << "done setup " << endl;
    return 0;
}

void ogl_setup_coord_sys_pnp(Mat ocv_pmat)
{
    // flip y-axis and z-axis
    // - to rotate CV-coordinate system (right-handed)
    // - to align with GL-coordinate system (right-handed)
    //   (see book "Demystified AR")
    //
    // also: transpose matrix to account for colum-major order of oGL arrays
    float ogl_p_mat[16];
    ogl_p_mat[0] = static_cast<float>(ocv_pmat.at<double>(0, 0));
    ogl_p_mat[1] = static_cast<float>(-ocv_pmat.at<double>(1, 0));
    ogl_p_mat[2] = static_cast<float>(-ocv_pmat.at<double>(2, 0));
    ogl_p_mat[3] = 0.0f;
    ogl_p_mat[4] = static_cast<float>(ocv_pmat.at<double>(0, 1));
    ogl_p_mat[5] = static_cast<float>(-ocv_pmat.at<double>(1, 1));
    ogl_p_mat[6] = static_cast<float>(-ocv_pmat.at<double>(2, 1));
    ogl_p_mat[7] = 0.0f;
    ogl_p_mat[8] = static_cast<float>(ocv_pmat.at<double>(0, 2));
    ogl_p_mat[9] = static_cast<float>(-ocv_pmat.at<double>(1, 2));
    ogl_p_mat[10] = static_cast<float>(-ocv_pmat.at<double>(2, 2));
    ogl_p_mat[11] = 0.0f;
    ogl_p_mat[12] = static_cast<float>(ocv_pmat.at<double>(0, 3));
    ogl_p_mat[13] = static_cast<float>(-ocv_pmat.at<double>(1, 3));
    ogl_p_mat[14] = static_cast<float>(-ocv_pmat.at<double>(2, 3));
    ogl_p_mat[15] = 1.0f;

    // Load the transposed matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(ogl_p_mat);
}

void draw_hexagon(hexagon& hexagon, map<int, marker>& marker_map)
{
    cout << "draw_hexagon" << endl; 
    if(hexagon.type == hexagon_type::full)
    {
        draw_hexagon_full(hexagon, marker_map);
    }
    else
    {
        draw_hexagon_by_color(hexagon, marker_map);
    }
}

// draw 6 triangles, share center point and one corner each
void draw_hexagon_full(hexagon& hexagon, map<int, marker>& marker_map)
{
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    // adapt y coordinate to opengl's coordinate system
    glTranslatef(hexagon.center_position.x, -hexagon.center_position.y + camera_height, 0);
    const float rotation = get_hexagon_rotation(hexagon, marker_map);
    // rotate around z-axis
    glRotatef(30.f - rotation, 0.0f, 0.0f, 1.0f);
    glColor3f(0.5f, 0, 0);
    draw_circle(hexagon.radius * 1.55f, 6);
    glPopMatrix();
}

void draw_hexagon_by_color(hexagon& hexagon, map<int, marker>& marker_map)
{
    const float rad = hexagon.radius * 1.55f;

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    
    // adapt y coordinate to opengl's coordinate system
    glTranslatef(hexagon.center_position.x, -hexagon.center_position.y + camera_height, 0);
    const float hexagon_rotation = get_hexagon_rotation(hexagon, marker_map);

    glRotatef(30.f - hexagon_rotation, 0.0f, 0.0f, 1.0f);
    
    constexpr double angle = (M_PI / 3);

    // const auto [r, g, b, a] = colors[0];
    
    for (int i = 0; i < 6; ++i)
    {
        // get each marker's colour and apply, start with 0
        // const auto marker = marker_map[hexagon.markers[i]];
        const auto [r, g, b, a] = marker_map[hexagon.markers[i]].color;
    
        glNormal3f(0.0f, 0.0f, 1.0f);
        glBegin(GL_TRIANGLES);
        {
            glColor3f(r, g, b);
            const auto total_angle = static_cast<float>(-i * angle);
            glVertex2f(0,0);
            glVertex2f(rad * cosf(total_angle), rad * sinf(total_angle));
            glVertex2f(rad * cosf(total_angle - angle), rad * sinf(total_angle - angle));
        }
        glEnd();
    }
    
    glPopMatrix();
}

float get_hexagon_rotation(const hexagon& hexagon, map<int, marker>& marker_map)
{
    // find slope of line between zero_marker and center and use atan(m) to get angle between line and x axis (screen axis):
    Point2f v = marker_map[hexagon.markers[0]].center_position - hexagon.center_position;
    float angle = atan(v.y / v.x) * 180.f / M_PI;
    if(v.y < 0 || v.x < 0)
    {
        if(v.x > 0)
            return angle;

        return angle + 180;
    }
    return angle;
}
