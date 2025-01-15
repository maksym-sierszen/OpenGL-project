#ifndef PARTICLESYSTEM_HPP
#define PARTICLESYSTEM_HPP

#include <vector>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <cstdlib>

struct Particle {
	glm::vec3 position;
	glm::vec3 velocity;
	float lifetime;  // Remaining lifetime in seconds
	float size;
	glm::vec4 color; // RGBA
};

extern std::vector<Particle> particle;

void emitParticle();
void updateParticles(float deltaTime);
void renderParticles(GLuint program, GLuint texture);

#endif
