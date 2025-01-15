#include "JellyfishInstance.hpp"

std::vector<JellyfishInstance> jellyfishInstances;

void addJellyfishInstance(glm::vec3 position, float amplitude, float speed) {
	jellyfishInstances.push_back({ position, amplitude, speed });
}