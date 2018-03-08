#include <GL/glew.h>

#include <GL/freeglut.h>
#include <GL/glext.h>
#include <GL/glx.h>

#include <assert.h>

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define VERTEX_LOC 0

struct Ubo {
	float color[4] = {0, 1, 0, 1};
};

static bool init();
static void cleanup();

static void display();
static void keydown(unsigned char key, int x, int y);
static unsigned int load_shader(const char *fname, int type);
static unsigned int create_program(const char *vfname, const char *pfname);
static bool link_program(unsigned int prog);

static const char *vs_path = "data/spirv/vertex.spv";
static const char *fs_path = "data/spirv/pixel.spv";

static unsigned int sprog;
static unsigned int vbo;
static unsigned int vao;

static float vertices[] = {
	-0.5, -0.5,
	0.5, -0.5,
	0.5, 0.5,

	-0.5, -0.5,
	0.5, 0.5,
	-0.5, 0.5
};

static int num_vertices = 6;
static Ubo test_ubo;
static unsigned int ubo;

int main(int argc, char **argv)
{
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutInitContextVersion(4, 5);

	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(800, 600);
	glutCreateWindow("gltest");


	glClear(GL_COLOR_BUFFER_BIT);

	glutDisplayFunc(display);
	glutKeyboardFunc(keydown);

	if(!init())
		return 1;

	atexit(cleanup);
	glutMainLoop();
}

static bool init()
{
	glewInit();

	glClearColor(0.0, 0.0, 1.0, 1.0);

	glGenBuffers(1, &ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, ubo);
	glBufferData(GL_UNIFORM_BUFFER, sizeof test_ubo, &test_ubo, GL_STREAM_DRAW);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof vertices, vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	if(!(sprog = create_program(vs_path, fs_path))) {
		fprintf(stderr, "Failed to create shader program.\n");
		return false;
	}

	glUseProgram(sprog);

	assert(glGetError() == GL_NO_ERROR);
	return true;
}

static void display()
{
	glEnableVertexAttribArray(VERTEX_LOC);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glVertexAttribPointer(VERTEX_LOC, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_UNIFORM_BUFFER, ubo);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof test_ubo, &test_ubo);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);

	glDrawArrays(GL_TRIANGLES, 0, num_vertices);

	assert(glGetError() == GL_NO_ERROR);

	glutSwapBuffers();
}

static void keydown(unsigned char key, int /*x*/, int /*y*/)
{
	switch(key) {
	case 27:
		exit(0);
	}
}

static unsigned int create_program(const char *vfname, const char *pfname)
{
	unsigned int vs, ps;
	unsigned int prog;

	if(!(vs = load_shader(vfname, GL_VERTEX_SHADER)))
		return false;
	if(!(ps = load_shader(pfname, GL_FRAGMENT_SHADER))) {
		glDeleteShader(vs);
		return false;
	}

	prog = glCreateProgram();
	glAttachShader(prog, vs);
	glAttachShader(prog, ps);

	if(!link_program(prog)) {
		glDeleteShader(vs);
		glDeleteShader(ps);
		glDeleteProgram(prog);
		return false;
	}

	glDetachShader(prog, vs);
	glDetachShader(prog, ps);

	return prog;
}

static bool link_program(unsigned int prog)
{
		int status, loglen;
	char *buf;

	glLinkProgram(prog);

	glGetProgramiv(prog, GL_LINK_STATUS, &status);
	if(status) {
		printf("successfully linked shader program\n");
	} else {
		printf("failed to link shader program\n");
	}

	glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &loglen);
	if(loglen > 0 && (buf = (char*)malloc(loglen + 1))) {
		glGetProgramInfoLog(prog, loglen, 0, buf);
		buf[loglen] = 0;
		printf("%s\n", buf);
		free(buf);
	}

	return status ? true : false;
}

unsigned int load_shader(const char *fname, int type)
{
	unsigned int sdr;
	unsigned int fsz;
	char *buf;
	FILE *fp;
	int status, loglen;

	if(!(fp = fopen(fname, "rb"))) {
		fprintf(stderr, "failed to open shader: %s\n", fname);
		return 0;
	}
	fseek(fp, 0, SEEK_END);
	fsz = ftell(fp);
	rewind(fp);

	if(!(buf = (char*)malloc(fsz + 1))) {
		fprintf(stderr, "failed to allocate %d bytes\n", fsz + 1);
		fclose(fp);
		return 0;
	}
	if(fread(buf, 1, fsz, fp) < fsz) {
		fprintf(stderr, "failed to read shader: %s\n", fname);
		free(buf);
		fclose(fp);
		return 0;
	}
	buf[fsz] = 0;
	fclose(fp);

	sdr = glCreateShader(type);

	glShaderBinary(1, &sdr, GL_SHADER_BINARY_FORMAT_SPIR_V_ARB, buf, fsz);
	glSpecializeShaderARB(sdr, "main", 0, 0, 0);

		free(buf);

	glGetShaderiv(sdr, GL_COMPILE_STATUS, &status);
	if(status) {
		printf("successfully compiled shader: %s\n", fname);
	} else {
		printf("failed to compile shader: %s\n", fname);
	}

	glGetShaderiv(sdr, GL_INFO_LOG_LENGTH, &loglen);
	if(loglen > 0 && (buf = (char*)malloc(loglen + 1))) {
		glGetShaderInfoLog(sdr, loglen, 0, buf);
		buf[loglen] = 0;
		printf("%s\n", buf);
		free(buf);
	}

	if(!status) {
		glDeleteShader(sdr);
		return 0;
	}

	return sdr;
}

static void cleanup()
{
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ubo);
}
