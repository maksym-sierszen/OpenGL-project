#ifndef BOIDS_HPP
#define BOIDS_HPP

#include <vector>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <cstdlib>

struct Boid {
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 acceleration;
};

extern std::vector<Boid> boids;

void initializeBoids(int numBoids);
glm::vec3 separation(Boid& boid, float separationDistance);
glm::vec3 alignment(Boid& boid, float neighborDistance);
glm::vec3 cohesion(Boid& boid, float neighborDistance);
glm::mat4 calculateOrientation(const glm::vec3& velocity);
glm::vec3 checkBorders(Boid& boid, float boundary);
void updateBoids(float deltaTime);

#endif // BOIDS_HPP