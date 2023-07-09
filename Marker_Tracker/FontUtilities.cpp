#include "FontUtilities.h"
#include <opencv2/imgproc.hpp>


/*
	This class is responsible for the text Output on the screen.
*/



GLuint FontUtilities::shader;
FT_Library FontUtilities::ft;
FT_Face FontUtilities::face;
std::map<GLchar, Character> FontUtilities::Characters;
GLuint FontUtilities::buffer;


void APIENTRY GLDebugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* msg, const void* data) {
	printf("%d: %s\n", id, msg);
}

/**
 * \brief initializes this class by creating textures of the character symbols and preparing the shader
 * that writes the text on the screen.
 * \param width camera width
 * \param height camera height
 */

void FontUtilities::init(int width, int height)
{
	//for some odd reasons, I have to init them here again
	glewInit();
	glfwInit();
	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDebugMessageCallback(GLDebugMessageCallback, NULL);
	glViewport(0, 0, width, height);

	FT_Init_FreeType(&ft);

	FT_New_Face(ft, "Fonts/FreeSans.ttf", 0, &face);

	FT_Set_Pixel_Sizes(face, 0, 48);

	shader = CompileShaders("shaders//vs.glsl", "shaders//fs.glsl");


	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction

	for (unsigned char c = 0; c < 128; c++)
	{
		// load character glyph 
		if (FT_Load_Char(face, c, FT_LOAD_RENDER))
		{
			std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
			continue;
		}
		// generate texture
		unsigned int texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RED,
			face->glyph->bitmap.width,
			face->glyph->bitmap.rows,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			face->glyph->bitmap.buffer
		);
		// set texture options
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// now store character for later use
		Character character = {
			texture,
			glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
			glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
			face->glyph->advance.x
		};
		Characters.insert(std::pair<char, Character>(c, character));
	}

	FT_Done_Face(face);
	FT_Done_FreeType(ft);

	glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(width), 0.0f, static_cast<float>(height));
	glUseProgram(shader);
	glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));


	GLuint vao;
	glCreateVertexArrays(1, &vao);
	glBindVertexArray(vao);


	glCreateBuffers(1, &buffer);

	glNamedBufferStorage(buffer, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_STORAGE_BIT);
	glVertexArrayVertexBuffer(vao, 0, buffer, 0, sizeof(GLfloat) * 4);
	glVertexArrayAttribFormat(vao, 0, 4, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(vao, 0, 0);
	glEnableVertexArrayAttrib(vao, 0);

	glUniform3f(6, 0.88f, 0.59f, 0.07f);
}

/**
 * \brief Renders a given text on top of whatever currently is in the window frame buffer
 * \param xOffset x-offset of where the text should start in the image
 * \param yOffset y-offset of where the text should start in the image
 * \param scale How big the text should be. 1.0 = 48pixel height per symbol
 */

void  FontUtilities::render_text(std::string text, GLfloat xOffset, GLfloat yOffset, GLfloat scale, color color )
{

	GLfloat x = xOffset;
	GLfloat y = yOffset;

	

	glUseProgram(shader);
	glUniform4f(2, color.r, color.g, color.b, color.a);
	

	//Write character for character on the screen
	std::string::const_iterator c;
	for (c = text.begin(); c != text.end(); c++) {
		Character ch = Characters[*c];
		GLfloat xpos = x + ch.Bearing.x * scale;
		GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

		GLfloat w = ch.Size.x * scale;
		GLfloat h = ch.Size.y * scale;
		// Update VBO for each character
		GLfloat vertices[6 * 4] = {
			 xpos,     ypos + h,   0.0f, 0.0f ,
			 xpos,     ypos,       0.0f, 1.0f ,
			 xpos + w, ypos,       1.0f, 1.0f ,

			 xpos,     ypos + h,   0.0f, 0.0f ,
			 xpos + w, ypos,       1.0f, 1.0f ,
			 xpos + w, ypos + h,   1.0f, 0.0f
		};

		glNamedBufferSubData(buffer, 0, sizeof(GLfloat) * 6 * 4, vertices);
		glBindTexture(GL_TEXTURE_2D, ch.TextureID);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		x += (ch.Advance >> 6) * scale;

	}
}


/**
 * \brief creates a gl-program with the given vertex and fragment shader
 * \param vs_path path of the vertex-shader file
 * \param fs_path path of the fragment-shader file
 */

GLuint  FontUtilities::CompileShaders(const char* vs_path, const char* fs_path) {

	GLuint shader_programme = glCreateProgram();

	GLuint vs, tcs, tes, gs, fs;


	FILE* vs_file;
	long vs_file_len;
	char* vertex_shader;

	vs_file = fopen(vs_path, "rb");

	fseek(vs_file, 0, SEEK_END);
	vs_file_len = ftell(vs_file);
	rewind(vs_file);

	vertex_shader = (char*)malloc(vs_file_len + 1);


	fread(vertex_shader, vs_file_len, 1, vs_file);
	vertex_shader[vs_file_len] = '\0';
	fclose(vs_file);

	vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vertex_shader, NULL);
	glCompileShader(vs);

	GLint isCompiled = 0;
	glGetShaderiv(vs, GL_COMPILE_STATUS, &isCompiled);

	if (isCompiled == GL_FALSE) {
		GLint maxLength = 0;
		glGetShaderiv(vs, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		char* error = (char*)malloc(maxLength);
		glGetShaderInfoLog(vs, maxLength, &maxLength, error);
		printf("Vertex shader error: ");
		printf(error);
		free(error);
	}

	glAttachShader(shader_programme, vs);
	free(vertex_shader);


	FILE* fs_file;
	long fs_file_len;
	char* fragment_shader;

	fs_file = fopen(fs_path, "rb");

	fseek(fs_file, 0, SEEK_END);
	fs_file_len = ftell(fs_file);
	rewind(fs_file);

	fragment_shader = (char*)malloc(fs_file_len + 1);

	fread(fragment_shader, fs_file_len, 1, fs_file);
	fragment_shader[fs_file_len] = '\0';
	fclose(fs_file);

	fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &fragment_shader, NULL);
	glCompileShader(fs);

	glGetShaderiv(fs, GL_COMPILE_STATUS, &isCompiled);

	if (isCompiled == GL_FALSE) {
		GLint maxLength = 0;
		glGetShaderiv(fs, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		char* error = (char*)malloc(maxLength);
		glGetShaderInfoLog(fs, maxLength, &maxLength, error);
		printf("Fragment shader error: ");
		printf(error);
		free(error);
	}

	glAttachShader(shader_programme, fs);
	free(fragment_shader);


	glLinkProgram(shader_programme);


	glDeleteShader(vs);



	glDeleteShader(fs);


	return shader_programme;
}