#include <vector>
#include <random>

std::vector<Particle> particles;
const int maxParticles = 1000;

void emitParticle() {
    Particle p;
    p.position = glm::vec3(0.0f, 0.0f, 0.0f);  // Start at the emitter's location
    p.velocity = glm::vec3(
        static_cast<float>(rand()) / RAND_MAX - 0.5f,
        static_cast<float>(rand()) / RAND_MAX * 2.0f,
        static_cast<float>(rand()) / RAND_MAX - 0.5f
    );  // Randomized initial velocity
    p.lifetime = 3.0f;  // 3 seconds lifetime
    p.size = 0.1f;
    p.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);  // White bubbles
    particles.push_back(p);
}

void updateParticles(float deltaTime) {
    for (auto& particle : particles) {
        particle.position += particle.velocity * deltaTime;
        particle.lifetime -= deltaTime;
        particle.color.a = particle.lifetime / 3.0f;  // Fade over lifetime
    }
    particles.erase(std::remove_if(particles.begin(), particles.end(),
        [](Particle& p) { return p.lifetime <= 0; }),
        particles.end());
}

void renderParticles(GLuint program, GLuint texture) {
    glUseProgram(program);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    for (const auto& particle : particles) {
        glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), particle.position);
        modelMatrix = glm::scale(modelMatrix, glm::vec3(particle.size));

        glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, &modelMatrix[0][0]);
        glUniform4fv(glGetUniformLocation(program, "color"), 1, &particle.color[0]);

        Core::DrawContext(models::sphere);  // Use a small sphere model for bubbles
    }
}