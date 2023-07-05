#include "FontUtilities.h"
#include <opencv2/imgproc.hpp>


GLuint shader; 
FT_Library ft;
FT_Face face;
std::map<GLchar, Character> Characters;
GLuint buffer;

void BindCVMat2GLTexture(const cv::Mat& image, GLuint& imageTexture)
{
	if (image.empty()) {
		std::cout << "image empty" << std::endl;
	}
	else {
		//glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glGenTextures(1, &imageTexture);
		glBindTexture(GL_TEXTURE_2D, imageTexture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// Set texture clamping method
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

		cv::cvtColor(image, image, cv::COLOR_RGB2BGR);

		glTexImage2D(GL_TEXTURE_2D,         // Type of texture
			0,                   // Pyramid level (for mip-mapping) - 0 is the top level
			GL_RGB,              // Internal colour format to convert to
			image.cols,          // Image width  i.e. 640 for Kinect in standard mode
			image.rows,          // Image height i.e. 480 for Kinect in standard mode
			0,                   // Border width in pixels (can either be 1 or 0)
			GL_RGB,              // Input image format (i.e. GL_RGB, GL_RGBA, GL_BGR etc.)
			GL_UNSIGNED_BYTE,    // Image data type
			image.ptr());        // The actual image data itself
	}
}


void APIENTRY GLDebugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* msg, const void* data) {
	printf("%d: %s\n", id, msg);
}
void FontUtilities::init(GLFWwindow* window, int width, int height)
{
	glfwInit();
	glfwMakeContextCurrent(window);

	glewInit();
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(GLDebugMessageCallback, NULL);
	glViewport(0, 0, width, height);

	shader = FontUtilities::CompileShaders(true, false, false, false, true);

	glUseProgram(shader);
	int uniform_WindowSize = glGetUniformLocation(shader, "WindowSize");
	glUniform2f(uniform_WindowSize, width,height);

	FT_Init_FreeType(&ft);


	FT_New_Face(ft, "Fonts/FreeSans.ttf", 0, &face);

	FT_Set_Pixel_Sizes(face, 0, 48);

	

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	for (GLubyte c = 0; c < 128; c++) {
		FT_Load_Char(face, c, FT_LOAD_RENDER);

		GLuint texture;
		glCreateTextures(GL_TEXTURE_2D, 1, &texture);
		glTextureStorage2D(texture, 1, GL_R8, face->glyph->bitmap.width, face->glyph->bitmap.rows);
		glTextureSubImage2D(texture, 0, 0, 0, face->glyph->bitmap.width, face->glyph->bitmap.rows, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);

		glBindTexture(GL_TEXTURE_2D, texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);

		Character character = {
		texture,
		glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
		glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
		face->glyph->advance.x
		};
		Characters.insert(std::pair<GLchar, Character>(c, character));
	}

	FT_Done_Face(face);
	FT_Done_FreeType(ft);

	glm::mat4 projection = glm::ortho(0.0f, (float)width, 0.0f, (float)height);
	glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(projection));

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

int  FontUtilities::render_text(GLFWwindow* window, int width, int height,const Mat& image){

	std::string text("Super long text longer than initial");
	
	
		//glClear(GL_COLOR_BUFFER_BIT);
		GLfloat x = 1.0f;
		GLfloat y = 300.0f;
		GLfloat scale = 1.0f;

		glActiveTexture(GL_TEXTURE0);
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
		
		glActiveTexture(GL_TEXTURE1);
		GLuint texture;
		BindCVMat2GLTexture(image, texture);
		glBindTexture(GL_TEXTURE_2D, texture);	

	return 0;
}



GLuint  FontUtilities::CompileShaders(bool vs_b, bool tcs_b, bool tes_b, bool gs_b, bool fs_b) {

	GLuint shader_programme = glCreateProgram();

	GLuint vs, tcs, tes, gs, fs;

	if (vs_b) {
		FILE* vs_file;
		long vs_file_len;
		char* vertex_shader;

		vs_file = fopen("shaders//vs.glsl", "rb");

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
	}
	if (tcs_b) {
		FILE* tcs_file;
		long tcs_file_len;
		char* tessellation_control_shader;

		tcs_file = fopen("shaders//tcs.glsl", "rb");

		fseek(tcs_file, 0, SEEK_END);
		tcs_file_len = ftell(tcs_file);
		rewind(tcs_file);

		tessellation_control_shader = (char*)malloc(tcs_file_len + 1);

		fread(tessellation_control_shader, tcs_file_len, 1, tcs_file);
		tessellation_control_shader[tcs_file_len] = '\0';
		fclose(tcs_file);


		tcs = glCreateShader(GL_TESS_CONTROL_SHADER);
		glShaderSource(tcs, 1, &tessellation_control_shader, NULL);
		glCompileShader(tcs);

		GLint isCompiled = 0;
		glGetShaderiv(tcs, GL_COMPILE_STATUS, &isCompiled);

		if (isCompiled == GL_FALSE) {
			GLint maxLength = 0;
			glGetShaderiv(tcs, GL_INFO_LOG_LENGTH, &maxLength);

			// The maxLength includes the NULL character
			char* error = (char*)malloc(maxLength);
			glGetShaderInfoLog(tcs, maxLength, &maxLength, error);
			printf("Tessellation control shader error: ");
			printf(error);
			free(error);
		}

		glAttachShader(shader_programme, tcs);
		free(tessellation_control_shader);
	}
	if (tes_b) {
		FILE* tes_file;
		long tes_file_len;
		char* tessellation_evaluation_shader;

		tes_file = fopen("shaders//tes.glsl", "rb");

		fseek(tes_file, 0, SEEK_END);
		tes_file_len = ftell(tes_file);
		rewind(tes_file);

		tessellation_evaluation_shader = (char*)malloc(tes_file_len + 1);

		fread(tessellation_evaluation_shader, tes_file_len, 1, tes_file);
		tessellation_evaluation_shader[tes_file_len] = '\0';
		fclose(tes_file);

		tes = glCreateShader(GL_TESS_EVALUATION_SHADER);
		glShaderSource(tes, 1, &tessellation_evaluation_shader, NULL);
		glCompileShader(tes);

		GLint isCompiled = 0;
		glGetShaderiv(tes, GL_COMPILE_STATUS, &isCompiled);

		if (isCompiled == GL_FALSE) {
			GLint maxLength = 0;
			glGetShaderiv(tes, GL_INFO_LOG_LENGTH, &maxLength);

			// The maxLength includes the NULL character
			char* error = (char*)malloc(maxLength);
			glGetShaderInfoLog(tes, maxLength, &maxLength, error);
			printf("Tessellation evaluation shader error: ");
			printf(error);
			free(error);
		}

		glAttachShader(shader_programme, tes);
		free(tessellation_evaluation_shader);
	}
	if (gs_b) {
		FILE* gs_file;
		long gs_file_len;
		char* geometry_shader;

		gs_file = fopen("shaders//gs.glsl", "rb");

		fseek(gs_file, 0, SEEK_END);
		gs_file_len = ftell(gs_file);
		rewind(gs_file);

		geometry_shader = (char*)malloc(gs_file_len + 1);

		fread(geometry_shader, gs_file_len, 1, gs_file);
		geometry_shader[gs_file_len] = '\0';
		fclose(gs_file);

		gs = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(gs, 1, &geometry_shader, NULL);
		glCompileShader(gs);

		GLint isCompiled = 0;
		glGetShaderiv(gs, GL_COMPILE_STATUS, &isCompiled);

		if (isCompiled == GL_FALSE) {
			GLint maxLength = 0;
			glGetShaderiv(gs, GL_INFO_LOG_LENGTH, &maxLength);

			// The maxLength includes the NULL character
			char* error = (char*)malloc(maxLength);
			glGetShaderInfoLog(gs, maxLength, &maxLength, error);
			printf("Geometry shader error: ");
			printf(error);
			free(error);
		}

		glAttachShader(shader_programme, gs);
		free(geometry_shader);
	}
	if (fs_b) {
		FILE* fs_file;
		long fs_file_len;
		char* fragment_shader;

		fs_file = fopen("shaders//fs.glsl", "rb");

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

		GLint isCompiled = 0;
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
	}

	glLinkProgram(shader_programme);

	if (vs_b) {
		glDeleteShader(vs);
	}
	if (tcs_b) {
		glDeleteShader(tcs);
	}
	if (tes_b) {
		glDeleteShader(tes);
	}
	if (gs_b) {
		glDeleteShader(gs);
	}
	if (fs_b) {
		glDeleteShader(fs);
	}

	return shader_programme;
}