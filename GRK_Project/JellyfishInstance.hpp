#ifndef JELLYFISHINSTANCE_HPP
#define JELLYFISHINSTANCE_HPP

#include <vector>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <cstdlib>

struct JellyfishInstance {
	glm::vec3 startPosition;
	float amplitude;
	float speed;
};

extern std::vector<JellyfishInstance> jellyfishInstances;

void addJellyfishInstance(glm::vec3 position, float amplitude, float speed);

#endif