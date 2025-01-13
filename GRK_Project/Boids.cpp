#include "Boids.hpp"

std::vector<Boid> boids;

void initializeBoids(int numBoids) {
    for (int i = 0; i < numBoids; ++i) {
        Boid boid;
        boid.position = glm::vec3(0.0f, rand() % 10 + 5, 0.0f);
        boid.velocity = glm::vec3(rand() % 10 - 5, rand() % 10 - 5, rand() % 10 - 5);
        boid.acceleration = glm::vec3(0.0f);
        boids.push_back(boid);
    }
}

glm::vec3 separation(Boid& boid, float separationDistance) {
    glm::vec3 steer(0.0f);
    int count = 0;
    for (Boid& other : boids) {
        float distance = glm::distance(boid.position, other.position);
        if (distance > 0 && distance < separationDistance) {
            glm::vec3 diff = boid.position - other.position;
            diff = glm::normalize(diff);
            diff /= distance;
            steer += diff;
            count++;
        }
    }
    if (count > 0) {
        steer /= (float)count;
    }
    return steer;
}

glm::vec3 alignment(Boid& boid, float neighborDistance) {
    glm::vec3 sum(0.0f);
    int count = 0;
    for (Boid& other : boids) {
        float distance = glm::distance(boid.position, other.position);
        if (distance > 0 && distance < neighborDistance) {
            sum += other.velocity;
            count++;
        }
    }
    if (count > 0) {
        sum /= (float)count;
        sum = glm::normalize(sum);
    }
    return sum;
}

glm::vec3 cohesion(Boid& boid, float neighborDistance) {
    glm::vec3 sum(0.0f);
    int count = 0;
    for (Boid& other : boids) {
        float distance = glm::distance(boid.position, other.position);
        if (distance > 0 && distance < neighborDistance) {
            sum += other.position;
            count++;
        }
    }
    if (count > 0) {
        sum /= (float)count;
        sum = glm::normalize(sum - boid.position);
    }
    return sum;
}

glm::mat4 calculateOrientation(const glm::vec3& velocity) {
    glm::vec3 forward = glm::normalize(velocity);
    glm::vec3 up(0.0f, 1.0f, 0.0f); // Arbitrary up vector
    glm::vec3 right = glm::normalize(glm::cross(up, forward));
    glm::vec3 adjustedUp = glm::cross(forward, right);

    glm::mat4 orientation(1.0f);
    orientation[0] = glm::vec4(right, 0.0f);
    orientation[1] = glm::vec4(adjustedUp, 0.0f);
    orientation[2] = glm::vec4(forward, 0.0f);
    return orientation;
}

glm::vec3 checkBorders(Boid& boid, float boundary) {
    glm::vec3 steer(0.0f);

    if (boid.position.x < -boundary) {
        steer.x = 1.0f;
    }
    else if (boid.position.x > boundary) {
        steer.x = -1.0f;
    }

    if (boid.position.y < 4.0f) {
        steer.y = 3.0f;
    }
    else if (boid.position.y > 12.0f) {
        steer.y = -1.0f;
    }

    if (boid.position.z < -boundary) {
        steer.z = 1.0f;
    }
    else if (boid.position.z > boundary) {
        steer.z = -1.0f;
    }

    return steer;
}

void updateBoids(float deltaTime) {
    float separationDistance = 1.0f;
    float neighborDistance = 2.0f;
    float boundary = 5.0f; // Define the boundary limit

    for (Boid& boid : boids) {
        glm::vec3 sep = separation(boid, separationDistance);
        glm::vec3 ali = alignment(boid, neighborDistance);
        glm::vec3 coh = cohesion(boid, neighborDistance);
        glm::vec3 borderSteer = checkBorders(boid, boundary);

        boid.acceleration = sep + ali + coh + borderSteer;
        boid.velocity += boid.acceleration * deltaTime;
        boid.position += boid.velocity * deltaTime;

        // Limit speed
        float maxSpeed = 3.0f;
        if (glm::length(boid.velocity) > maxSpeed) {
            boid.velocity = glm::normalize(boid.velocity) * maxSpeed;
        }
    }
}