#ifdef _WIN32
#include <Windows.h>
#endif

#include "ThreadPool.h"
#include <assert.h>
#include <stdbool.h>
#include <GL/gl.h>
#include <math.h>
#include <xmmintrin.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define GLFW_DLL
#include <GLFW/glfw3.h>

GLFWwindow * window;
Device * GPU;

//#define MAX_PARTICLES (2097152*4)
#define MAX_PARTICLES 256

typedef struct ParticleSystem {
	// Each __m128 contains 2 2D coordinates
	union {
		__m128 velocities_m128[MAX_PARTICLES/2];
		float velocities[MAX_PARTICLES*2];
	};
	union {
		__m128 positions_m128[MAX_PARTICLES/2];
		float positions[MAX_PARTICLES*2];
	};
} ParticleSystem;

#define NUMBER_OF_PARTICLE_SYSTEMS 4

ParticleSystem particleSystems[NUMBER_OF_PARTICLE_SYSTEMS];
double deltaTime = 0;

int draw_particle_system(size_t pSystemIndex)
{
	//printf("draw_particle_system\n");
	glColor3f(0,1,1);
	glBegin(GL_POINTS);
	for(unsigned int i = 0; i < MAX_PARTICLES; i++) {
		glVertex2f(particleSystems[pSystemIndex].positions[i*2], particleSystems[pSystemIndex].positions[i*2+1]);
	}
	glEnd();

	return 0;
}

int update_particle_system(size_t pSystemIndex)
{
	//printf("update_particle_system\n");

	/* Simulate some simple physics */

	__m128 deltaTime_m128 = _mm_set1_ps((float)deltaTime);

	#define GRAVITY 60

	const float acceleration_[] = {0,GRAVITY,0,GRAVITY};
	__m128 acceleration = _mm_load_ps(acceleration_);
	acceleration = _mm_mul_ps(acceleration, deltaTime_m128);

	__m128 min = _mm_set1_ps(0);

	const float max_[] = {
		640,480,640,480
	};
	__m128 max = _mm_load_ps(max_);

	for(unsigned int j = 0; j < MAX_PARTICLES/2; j++) {
		__m128 velocity = particleSystems[pSystemIndex].velocities_m128[j];
		velocity = _mm_add_ps(velocity, acceleration);				
		particleSystems[pSystemIndex].velocities_m128[j] = velocity;

		velocity = _mm_mul_ps(velocity, deltaTime_m128);

		__m128 position = particleSystems[pSystemIndex].positions_m128[j];
		position = _mm_add_ps(position, velocity);

		position = _mm_min_ps(position, max);
		position = _mm_max_ps(position, min);

		particleSystems[pSystemIndex].positions_m128[j] = position;
	}

	create_job(GPU, draw_particle_system, pSystemIndex);
	return 0;
}

double timeOfLastFrame = 0;

int render_frame(size_t a);

int swap_buffers_and_poll(size_t a)
{
	//printf("swap_buffers_and_poll\n");
	(void)a;

	glfwSwapBuffers(window);
	glfwPollEvents();

	if(glfwWindowShouldClose(window)) {
		glfwTerminate();
	} else {
		render_frame(0);
	}
	
	return 0;
}

int render_frame(size_t a)
{
	//printf("render_frame\n");
	(void)a;
	double now = glfwGetTime();
	deltaTime = now - timeOfLastFrame;
	timeOfLastFrame = now;

	glClear(GL_COLOR_BUFFER_BIT);
	set_final_job(GPU, swap_buffers_and_poll, 0);

	for(size_t i = 0; i < NUMBER_OF_PARTICLE_SYSTEMS; i++)
		create_job(NULL, update_particle_system, i);

	return 0;
}

int create_window(size_t a)
{
	(void)a;
	//printf("create_window\n");

	assert(glfwInit());

	glfwWindowHint(GLFW_SAMPLES, 1);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 1);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

	glfwWindowHint(GLFW_DEPTH_BITS, 0);
	glfwWindowHint(GLFW_STENCIL_BITS, 0);

	window = glfwCreateWindow(640, 480, "Particle Systems Demo", NULL, NULL);
	assert(window);

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glClearColor(0,0,0,1);

	glDisable(GL_CULL_FACE);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0,640,480,0, 0, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glPointSize(3);

	timeOfLastFrame = glfwGetTime();
	
	assert(!glGetError());
	return 0;
}

int main()
{
	srand(time(NULL));

	/* Create particle systems */

	for(unsigned int j = 0; j < NUMBER_OF_PARTICLE_SYSTEMS; j++)
	{
		for(unsigned int i = 0; i < MAX_PARTICLES; i++) {
			float x = ((int)i-(MAX_PARTICLES/2)) * 0.07f;
			float y = -30.0f - 0.17f*(rand() % 1000);

			particleSystems[j].velocities[i*2] = x;
			particleSystems[j].velocities[i*2+1] = y;

			particleSystems[j].positions[i*2] = (640/NUMBER_OF_PARTICLE_SYSTEMS) * j + ((640/NUMBER_OF_PARTICLE_SYSTEMS) / 2);
			particleSystems[j].positions[i*2+1] = 480;
		}
	}

	thread_pool_init(2);

	GPU = create_device("GPU (OpenGL 1.1)");

	create_job(GPU, create_window, 0);
	create_job(GPU, render_frame, 0);

	start_worker_threads();
	return 0;
}
