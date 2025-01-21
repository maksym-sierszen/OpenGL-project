
#include <iostream>
#include <cmath>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>
#include <Windows.h>

#include "glew.h"
#include "glm.hpp"
#include "ext.hpp"
#include "Shader_Loader.h"
#include "Render_Utils.h"
#include "Texture.h"
#include "Box.cpp"
#include "SOIL/SOIL.h"
#include "Models.hpp"
#include "Shadows.hpp"
#include "../Boids.hpp"
#include "../JellyfishInstance.hpp"

//window variables
const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
int WIDTH = 1000, HEIGHT = 1000;

//variables -------------------------------------------------------------------------------------------------------------------------------------------------------- variables

//depth
GLuint depthMapSunFBO;
GLuint depthMapSun;

//shaders
GLuint programPBR;
GLuint programTex;
GLuint programSun;
GLuint programSkybox;
GLuint programDepth;
Core::Shader_Loader shaderLoader;

//sun
float sunx = 9.0f, suny = 4.0f, sunz = 2.0f;
glm::vec3 sunDir = glm::vec3(sunx, suny, sunz);
glm::vec3 sunPos = glm::vec3(5.0f, 20.0f, 8.0f);
glm::vec3 sunColor = glm::vec3(0.9f, 0.9f, 0.7f);
float sunForce = 5;

//player
glm::vec3 playerPos = glm::vec3(0.0f, 1.25f, -10.0f);
glm::vec3 playerDir = glm::vec3(-0.0f, 0.00f, 1.0f);

//camera
glm::vec3 cameraStartPos = playerPos - 0.5 * playerDir + glm::vec3(0, 2, 0) * 0.2f;
glm::vec3 cameraPos = glm::vec3(cameraStartPos.x, cameraStartPos.y, cameraStartPos.z);
glm::vec3 cameraDir = playerDir;
glm::vec3 playerVelocity(0.0f, 0.0f, 0.0f);

std::vector<glm::mat4> seaweedMatrices;

//mouse
double lastX, lastY;
bool firstMouse = true;
float yaw = 90.0f; // Yaw jest początkowo ustawiony na 90 stopni, aby skierować kamerę wzdłuż osi z
float pitch = 0.0f;
float sensitivity = 0.1f;

//aspect and exposition
float aspectRatio = 1.f;
float exposition = 1.f;

//main lamp light
glm::vec3 pointlightPos = glm::vec3(0.0f, 2.0f, 0.0f);
glm::vec3 pointlightColorON = glm::vec3(0.9, 0.6, 0.6) * 4;
glm::vec3 pointlightColor = pointlightColorON * 0;

// player animation when collision with jellyfish
bool checkCollision(glm::vec3 playerPosition, glm::vec3 jellyfishPosition, float jellyfishRadius = 0.5f) {
	float distance = glm::distance(playerPosition, jellyfishPosition);
	return distance <= jellyfishRadius;
}

bool isPlayerBlinking = false;
float blinkTimer = 0.0f;
float blinkDuration = 1.0f; // Czas trwania mrugania w sekundach


//projections for shadows
glm::mat4 sunVP = glm::ortho(-25.f, 25.f, -25.f, 25.f, 1.0f, 80.0f) * glm::lookAt(sunPos, sunPos - sunDir, glm::vec3(0, 1, 0));


//delta time ------------------------------------------------------------------------------------------------------------------------------------------------------- delta time
float lastTime = -1.f;
float deltaTime = 0.f;
void updateDeltaTime(float time) {
	if (lastTime < 0) {
		lastTime = time;
		return;
	}

	deltaTime = time - lastTime;
	if (deltaTime > 0.1) deltaTime = 0.1;
	lastTime = time;
}

glm::mat4 rotateAroundPivot(float degrees, glm::vec3 axis, glm::vec3 pivot)
{
	glm::mat4 to_pivot = glm::translate(glm::mat4(), -pivot);
	glm::mat4 from_pivot = glm::translate(glm::mat4(), pivot);
	glm::mat4 rotate = from_pivot * glm::rotate(glm::mat4(), glm::radians(degrees), axis) * to_pivot;
	return rotate;
}


//camera and perspective matrix ------------------------------------------------------------------------------------------------------------------ camera and perspective matrix
/*glm::mat4 createCameraMatrix()
{
	glm::vec3 cameraSide = glm::normalize(glm::cross(cameraDir, glm::vec3(0.f, 1.f, 0.f)));
	glm::vec3 cameraUp = glm::normalize(glm::cross(cameraSide, cameraDir));
	glm::mat4 cameraRotrationMatrix = glm::mat4({
		cameraSide.x,cameraSide.y,cameraSide.z,0,
		cameraUp.x,cameraUp.y,cameraUp.z ,0,
		-cameraDir.x,-cameraDir.y,-cameraDir.z,0,
		0.,0.,0.,1.,
		});
	cameraRotrationMatrix = glm::transpose(cameraRotrationMatrix);
	glm::mat4 cameraMatrix = cameraRotrationMatrix * glm::translate(-cameraPos);

	return cameraMatrix;
}*/

glm::mat4 createCameraMatrix()
{
	glm::vec3 cameraSide = glm::normalize(glm::cross(cameraDir, glm::vec3(0.f, 1.f, 0.f)));
	glm::vec3 cameraUp = glm::normalize(glm::cross(cameraSide, cameraDir));
	//glm::vec3 cameraPosition = playerPos - cameraDir * 0.5f + glm::vec3(0, 2, 0) * 0.2f; // Adjust the distance and height as needed
	//cameraPos = playerPos - 0.5 * playerDir + glm::vec3(0, 2, 0) * 0.2f;
	glm::mat4 cameraRotationMatrix = glm::mat4({
		cameraSide.x,cameraSide.y,cameraSide.z,0,
		cameraUp.x,cameraUp.y,cameraUp.z ,0,
		-cameraDir.x,-cameraDir.y,-cameraDir.z,0,
		0.,0.,0.,1.,
		});
	cameraRotationMatrix = glm::transpose(cameraRotationMatrix);
	glm::mat4 cameraMatrix = cameraRotationMatrix * glm::translate(-cameraPos);

	return cameraMatrix;
}


glm::mat4 createPerspectiveMatrix()
{

	glm::mat4 perspectiveMatrix;
	float n = 0.05f;
	float f = 100.0f;
	float a1 = min(aspectRatio, 1.0f);
	float a2 = min(1.0f / aspectRatio, 1.0f);
	perspectiveMatrix = glm::mat4({
		1, 0.0f, 0.0f, 0.0f,
		0.0f, aspectRatio, 0.0f, 0.0f,
		0.0f, 0.0f, (f + n) / (n - f), 2 * f * n / (n - f),
		0.0f, 0.0f, -1.0f, 0.0f,
		});


	perspectiveMatrix = glm::transpose(perspectiveMatrix);

	return perspectiveMatrix;
}



//drawPBR ----------------------------------------------------------------------------------------------------------------------------------------------------------- drawPBR
void drawObjectPBR(Core::RenderContext& context, glm::mat4 modelMatrix, glm::vec3 color, GLuint textureId, float roughness, float metallic, float brightness)
{
	GLuint program;
	if (textureId == NULL)
	{
		program = programPBR;
	}
	else
	{
		program = programTex;
	}

	glUseProgram(program);

	glActiveTexture(GL_TEXTURE0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, depthMapSun);
	glUniformMatrix4fv(glGetUniformLocation(program, "sunVP"), 1, GL_FALSE, (float*)&sunVP);


	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 transformation = viewProjectionMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	glUniform1f(glGetUniformLocation(program, "exposition"), exposition);

	glUniform1f(glGetUniformLocation(program, "roughness"), roughness);
	glUniform1f(glGetUniformLocation(program, "metallic"), metallic);
	glUniform1f(glGetUniformLocation(program, "brightness"), brightness);

	glUniform3f(glGetUniformLocation(program, "cameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);
	glUniform3f(glGetUniformLocation(program, "cameraStartPos"), cameraStartPos.x, cameraStartPos.y, cameraStartPos.z);

	glUniform3f(glGetUniformLocation(program, "sunDir"), sunDir.x, sunDir.y, sunDir.z);
	glUniform3f(glGetUniformLocation(program, "sunColor"), sunColor.x * sunForce / 100, sunColor.y * sunForce / 100, sunColor.z * sunForce / 100);

	glUniform3f(glGetUniformLocation(program, "lightPos"), pointlightPos.x, pointlightPos.y, pointlightPos.z);
	glUniform3f(glGetUniformLocation(program, "lightColor"), pointlightColor.x, pointlightColor.y, pointlightColor.z);



	if (textureId == NULL)
	{
		glUniform3f(glGetUniformLocation(program, "color"), color.x, color.y, color.z);
	}
	else
	{
		Core::SetActiveTexture(textureId, "colorTexture", programTex, 0);
	}

	Core::DrawContext(context);
}

//animations ----------------------------------------------------------------------------------------------------------------------------------------------------- animations
void animatePlayer()
{
		glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), playerPos);
		modelMatrix = glm::scale(modelMatrix, glm::vec3(5.0f));

		if (glm::length(playerVelocity) > 0.001f) {
			playerDir = glm::normalize(playerVelocity);
		}

		glm::vec3 targetDir = glm::normalize(playerDir);
		playerDir = glm::mix(playerDir, targetDir, 0.1f);

		glm::vec3 right = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), playerDir));
		glm::vec3 up = glm::cross(playerDir, right);

		glm::mat4 rotationMatrix(1.0f);
		rotationMatrix[0] = glm::vec4(right, 0.0f);
		rotationMatrix[1] = glm::vec4(up, 0.0f);
		rotationMatrix[2] = glm::vec4(playerDir, 0.0f);

		modelMatrix *= rotationMatrix;

		glm::vec3 playerColor = isPlayerBlinking ? glm::vec3(1.0f, 0.0f, 0.0f) : glm::vec3(0.5f, 0.5f, 1.0f);

		float brightness = isPlayerBlinking ? 100.0f + 20.0f * sin(10.0f * blinkTimer) : 30.0f;
		drawObjectPBR(models::nemo, modelMatrix, playerColor, textures::nemo, 0.0f, 0.0f, brightness);

		if (isPlayerBlinking) {
			blinkTimer += deltaTime;
			if (blinkTimer >= blinkDuration) {
				isPlayerBlinking = false;
				blinkTimer = 0.0f;
			}
		}
}

void animateInteractive()
{
	// COS INTERAKTYWNEGO TUTAJ DO ANIMACJI 
	// NP ZE JAK USER PODEJDZIE DO CZEGOS TO TO ZACZYNA SIE RUSZAC WTEDY NP RYBA ODPLYWA ALBO SKRZYNIA SKARBOW SIE OTWIERA ALBO COS INNEGO
	// MOZE SIE JAKIS LISTENER PRZYDAC WTEDY KTORY WYKRYJE USERA
}

void animateShark(glm::mat4 startingPos)
{
	// Example animation: make the trout swim in a circle
	float time = glfwGetTime();
	float radius = 8.0f;
	float speed = 0.3f;

	float x = radius * cos(speed * time);
	float z = radius * sin(speed * time);
	glm::vec3 position = glm::vec3(x, 1.0f, z);

	// Calculate the angle to rotate the trout to face the direction of movement
	float angle = atan2(z, x);

	glm::mat4 modelMatrix = glm::translate(startingPos, position);
	modelMatrix = glm::rotate(modelMatrix, -angle, glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate the trout in the opposite direction

	// Apply the animation to the trout model
	//applyAnimation(models::troutScene, models::trout, time);
	drawObjectPBR(models::shark, modelMatrix, glm::vec3(), textures::shark, 0.0f, 0.0f, 30.0f);

}

void animateJellyfishInstances() {
	static float time = 0.0f;
	time += deltaTime;

	for (const auto& jellyfish : jellyfishInstances) {
		float yOffset = jellyfish.amplitude * sin(time * jellyfish.speed);
		glm::vec3 newPosition = jellyfish.startPosition + glm::vec3(0.0f, yOffset, 0.0f);
		glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), newPosition);
		modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5f));


		if (checkCollision(playerPos, newPosition)) {
			isPlayerBlinking = true;
			blinkTimer = 0.0f;
			glm::vec3 jellyfishColor = glm::vec3(1.0f, 0.5f, 0.5f); // Zmiana koloru na bardziej widoczny
			modelMatrix = glm::scale(modelMatrix, glm::vec3(1.2f)); // Powiększenie
			drawObjectPBR(models::jellyfish, modelMatrix, jellyfishColor, textures::jellyfish, 0.0f, 0.0f, 50.0f);
		}
		else {
			drawObjectPBR(models::jellyfish, modelMatrix, glm::vec3(), textures::jellyfish, 0.0f, 0.0f, 30.0f);
		}
	}
}

//render scene objects ----------------------------------------------------------------------------------------------------------------------------------- render scene objects
void renderSun()
{
	glUseProgram(programSun);
	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 transformation = viewProjectionMatrix * glm::translate(sunPos + glm::vec3(0.0f, 0.0f, 0.0f)) * glm::scale(glm::vec3(1.0f));
	glUniformMatrix4fv(glGetUniformLocation(programSun, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glUniform3f(glGetUniformLocation(programSun, "color"), sunColor.x * 7.5f, sunColor.y * 7.5f, sunColor.z * 7.5f);
	glUniform1f(glGetUniformLocation(programSun, "exposition"), exposition);

	sunDir = glm::vec3(sunx, suny, sunz);

	Core::DrawContext(models::sphere);
}

void renderSkybox(Core::RenderContext& context, glm::mat4 modelMatrix, GLuint textureID)
{
	glDepthFunc(GL_LEQUAL);
	glUseProgram(programSkybox);
	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * glm::mat4(glm::mat3(createCameraMatrix()));
	glm::mat4 transformation = viewProjectionMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(programSkybox, "transformation"), 1, GL_FALSE, (float*)&transformation);
	Core::SetActiveTexture(textureID, "skybox", programSkybox, 0);
	Core::DrawContext(context);
	glDepthFunc(GL_LESS);
	glUseProgram(0);
}

void renderShadows(GLuint program, GLuint FBO, glm::mat4 VP) {

	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glClear(GL_DEPTH_BUFFER_BIT);

	glUseProgram(program);

	//render structures
	drawObjectDepth(program, models::ground, VP, glm::mat4());


	//render animals

	//render environment



	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, WIDTH, HEIGHT);
}

void renderBoids() {
	updateBoids(deltaTime);
	for (Boid& boid : boids) {
		glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), boid.position);
		glm::mat4 orientation = calculateOrientation(boid.velocity);
		modelMatrix *= orientation;
		// Render the boid using the model matrix
		//drawObjectPBR(models::trout, modelMatrix, glm::vec3(1.0f), textures::trout, 0.5f, 0.5f, 1.0f);
		drawObjectPBR(models::trout, modelMatrix, glm::vec3(), textures::trout, 0.0f, 0.0f, 30.0f);
	}
}

bool isCrabMoving = false;
float crabSwingAngle = 0.0f; // Kąt bujania
float crabSwingSpeed = 10.0f; // Szybkość bujania

void renderCrabs() {
	glm::vec3 largestCrabPos = glm::vec3(-1.8f, -3.5f, -4.7f);

	// Sprawdź odległość gracza od największego kraba
	float detectionDistance = 6.5f; // Odległość wykrywania
	float distance = glm::distance(playerPos, largestCrabPos);
	if (distance < detectionDistance) {
		isCrabMoving = true;
	}
	else {
		isCrabMoving = false;
	}

	// Ruch huśtawkowy kraba
	if (isCrabMoving) {
		crabSwingAngle = sin(glfwGetTime() * crabSwingSpeed) * 3.0f; // Oscylacja kąta (±10 stopni)
	}
	else {
		crabSwingAngle = 0.0f; // Zatrzymanie ruchu
	}

	glm::mat4 crabMatrix1 = glm::mat4();
	crabMatrix1 = glm::translate(crabMatrix1, glm::vec3(-1.8f, -3.5f, -4.7f));
	crabMatrix1 = glm::rotate(crabMatrix1, glm::radians(crabSwingAngle), glm::vec3(1.0f, 0.0f, 0.0f)); // Huśtanie na osi Z
	crabMatrix1 = glm::rotate(crabMatrix1, glm::radians(160.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // Obrót w osi Y (ustawienie w scenie)
	crabMatrix1 = glm::scale(crabMatrix1, glm::vec3(0.2f, 0.2f, 0.2f));
	drawObjectPBR(models::crab, crabMatrix1, glm::vec3(), textures::crab, 0.0f, 0.0f, 30.0f);

	int totalCrabs = 5;
	for (int i = 0; i < totalCrabs; ++i) {
		glm::mat4 crabMatrix = glm::mat4();

		int row = (i < 3) ? 0 : 1;
		float xSpacing = 1.5f;
		float xPosition = (row == 0) ? -3.0f + (i * xSpacing) : -1.5f + ((i - 3) * xSpacing);
		float zPosition = -4.0f - (row * 1.3f);

		crabMatrix = glm::translate(crabMatrix, glm::vec3(xPosition, -1.0f, zPosition));
		crabMatrix = glm::rotate(crabMatrix, glm::radians(-190.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		crabMatrix = glm::scale(crabMatrix, glm::vec3(0.05f, 0.05f, 0.05f));

		drawObjectPBR(models::crab, crabMatrix, glm::vec3(), textures::crab, 0.0f, 0.0f, 30.0f);
	}
}

void renderSeashells() {

	glm::vec3 positions[3] = {
	glm::vec3(3.0f, -0.35f, -3.0f),
	glm::vec3(1.0f, -0.33f, -4.0f),
	glm::vec3(-3.0f, -0.32f, -2.0f)
	};

	for (int i = 0; i < 3; ++i) {
		glm::mat4 shellMatrix = glm::mat4();

	
		float rotationInDegrees = 30.0f * i; 
		float rotationInRadians = glm::radians(rotationInDegrees);

		shellMatrix = glm::translate(shellMatrix, positions[i]);
		shellMatrix = glm::rotate(shellMatrix, rotationInRadians, glm::vec3(0.0f, 1.0f, 0.0f));   
		shellMatrix = glm::scale(shellMatrix, glm::vec3(0.01f, 0.01f, 0.01f));

		drawObjectPBR(models::seashell, shellMatrix, glm::vec3(), textures::seashell, 0.3f, 0.5f, 15.0f);
	}
}


void renderSeaweed()
{
	srand(static_cast<unsigned int>(time(0))); // Inicjalizacja generatora liczb losowych

	int totalSeaweed = 100; // Liczba wodorostów
	float radius = 12.0f; // Promień okręgu
	glm::vec3 center = glm::vec3(0.0f, 0.0f, 0.0f); // Środek okręgu

	for (int i = 0; i < totalSeaweed; ++i) {
		glm::mat4 seaweedMatrix = glm::mat4();

		// Wyliczenie pozycji wodorostów w kształcie okręgu lub wypełnionym środku
		float angle = glm::radians(360.0f / totalSeaweed * i); // Kąt dla każdego wodorostu
		float randomOffset = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 4.0f - 2.0f; // Losowy offset w zakresie -2.0f do 2.0f
		float currentRadius = (i % 2 == 0) ? radius : radius * (0.5f + 0.5f * ((i % 3) / 2.0f)); // Wypełnienie środka
		currentRadius += randomOffset; // Dodanie losowego offsetu do promienia
		float xPosition = center.x + currentRadius * cos(angle);
		float zPosition = center.z + currentRadius * sin(angle);
		float yPosition = center.y; // Stała pozycja w osi Y (na dnie)

		seaweedMatrix = glm::translate(seaweedMatrix, glm::vec3(xPosition, yPosition, zPosition));

		// Stała rotacja wodorostów wokół osi Y z lekką różnorodnością
		float baseRotation = 0.0f; // Podstawowy obrót całej grupy
		float randomRotation = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 360.0f - 180.0f; // Losowy obrót w zakresie -180.0f do 180.0f
		float rotationAngle = baseRotation + 40.0f + (i % 5) * 2.0f + randomRotation; // Delikatnie różne kąty obrotu dla każdego wodorostu
		seaweedMatrix = glm::rotate(seaweedMatrix, glm::radians(rotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));

		// Stałe skalowanie wodorostów z lekkimi różnicami
		float scaleFactor = 0.7f + (i % 3) * 0.05f; // Skala w zakresie 0.7f do 0.85f
		seaweedMatrix = glm::scale(seaweedMatrix, glm::vec3(scaleFactor, scaleFactor, scaleFactor));

		// Rysowanie pojedynczego wodorostu
		//drawObjectPBR(models::seaweed, seaweedMatrix, glm::vec3(), textures::seaweed, 0.1f, 0.0f, 10.0f);
		seaweedMatrices.push_back(seaweedMatrix);
	}
}



//render scene --------------------------------------------------------------------------------- render scene
void renderScene(GLFWwindow* window)
{


	//shadows
	renderShadows(programDepth, depthMapSunFBO, sunVP);
	// shadows od lampki ryby

	//skybox
	glClearColor(0.0f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	renderSkybox(models::skybox, glm::translate(glm::scale(glm::mat4(), glm::vec3(2.5f, 2.0f, 2.0f)), glm::vec3(3.0f, 1.0f, 0.0f)), textures::skybox);


	//time and delta time
	float time = glfwGetTime();

	updateDeltaTime(time);

	// jellyfish interaction
	if (isPlayerBlinking) {
		float shakeIntensity = 0.05f;
		cameraPos.x += shakeIntensity * sin(20.0f * blinkTimer);
		cameraPos.y += shakeIntensity * sin(25.0f * blinkTimer);
	}
	//sun
	renderSun();


	//render structures
	drawObjectPBR(models::ground, glm::mat4(), glm::vec3(), textures::ground, 0.8f, 0.0f, 30.0f);
	// np statek, jakis skarb piracki ??

	//render animals 
	// rekiny, ryby, kraby, ryba z lampką na czole
	glm::mat4 startingPos = glm::mat4();
	startingPos = glm::translate(startingPos, glm::vec3(0.0f, 5.0f, 0.0f)); // Przesunięcie
	startingPos = glm::rotate(startingPos, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // Obrót

    animateShark(startingPos);
	renderCrabs();
	renderSeashells();
	//render environment

	animateShark(startingPos);

	

	// roslinnosc, kamienie itp
	glm::mat4 modelMatrix = glm::mat4();
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, -0.8f, 0.0f)); // Przesunięcie
	modelMatrix = glm::scale(modelMatrix, glm::vec3(0.1f, 0.1f, 0.1f));    // Skalowanie
	modelMatrix = glm::rotate(modelMatrix, glm::radians(-30.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // Obrót
	drawObjectPBR(models::v_boat, modelMatrix, glm::vec3(), textures::v_boat, 0.0f, 0.0f, 30.0f);

	glm::mat4 swordModelMatrix = glm::mat4();
	swordModelMatrix = glm::translate(swordModelMatrix, glm::vec3(10.2f, 0.4f, 3.8f)); // Przesunięcie
	swordModelMatrix = glm::scale(swordModelMatrix, glm::vec3(0.03f, 0.03f, 0.03f));    // Skalowanie
	swordModelMatrix = glm::rotate(swordModelMatrix, glm::radians(87.0f), glm::vec3(1.0f, 0.0f, 0.0f));

	drawObjectPBR(models::sword, swordModelMatrix, glm::vec3(), textures::sword, 0.0f, 0.0f, 50.0f);

	glm::mat4 beastModelMatrix = glm::mat4();
	beastModelMatrix = glm::translate(beastModelMatrix, glm::vec3(6.0f, -7.7f, 5.5f)); // Przesunięcie
	beastModelMatrix = glm::scale(beastModelMatrix, glm::vec3(7.0f, 7.0f, 7.0f));    // Skalowanie

	drawObjectPBR(models::beast, beastModelMatrix, glm::vec3(), textures::beast, 0.0f, 0.0f, 50.0f);

	glm::mat4 scullModelMatrix = glm::mat4();
	scullModelMatrix = glm::translate(scullModelMatrix, glm::vec3(10.0f, 0.35f, -5.0f)); // Przesunięcie
	scullModelMatrix = glm::scale(scullModelMatrix, glm::vec3(0.002f, 0.002f, 0.002f));    // Skalowanie
	scullModelMatrix = glm::rotate(scullModelMatrix, glm::radians(-30.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	scullModelMatrix = glm::rotate(scullModelMatrix, glm::radians(-50.0f), glm::vec3(1.0f, 0.0f, 0.0f));

	drawObjectPBR(models::scull, scullModelMatrix, glm::vec3(), textures::scull, 0.0f, 0.0f, 50.0f);

	glm::mat4 remainsModelMatrix = glm::mat4();
	remainsModelMatrix = glm::translate(remainsModelMatrix, glm::vec3(8.0f, -0.05f, -3.0f)); // Przesunięcie
	remainsModelMatrix = glm::scale(remainsModelMatrix, glm::vec3(0.4f, 0.4f, 0.4f));    // Skalowanie
	remainsModelMatrix = glm::rotate(remainsModelMatrix, glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	drawObjectPBR(models::remains, remainsModelMatrix, glm::vec3(), textures::remains, 0.0f, 0.0f, 50.0f);

	glm::mat4 chestModelMatrix = glm::mat4();
	chestModelMatrix = glm::translate(chestModelMatrix, glm::vec3(-8.0f, 0.45f, -5.0f)); // Przesunięcie
	chestModelMatrix = glm::scale(chestModelMatrix, glm::vec3(1.0f, 1.0f, 1.0f));    // Skalowanie
	chestModelMatrix = glm::rotate(chestModelMatrix, glm::radians(120.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	drawObjectPBR(models::treasureChest, chestModelMatrix, glm::vec3(), textures::treasureChest, 0.0f, 0.0f, 50.0f);

	glm::mat4 goldModelMatrix = glm::mat4();
	goldModelMatrix = glm::translate(goldModelMatrix, glm::vec3(-8.0f, -0.5f, -7.2f)); // Przesunięcie
	goldModelMatrix = glm::scale(goldModelMatrix, glm::vec3(0.7f, 0.7f, 0.7f));    // Skalowanie

	drawObjectPBR(models::gold, goldModelMatrix, glm::vec3(), textures::gold, 0.0f, 0.0f, 50.0f);

	goldModelMatrix = glm::mat4();
	goldModelMatrix = glm::translate(goldModelMatrix, glm::vec3(-9.0f, -0.5f, -6.2f)); // Przesunięcie
	goldModelMatrix = glm::scale(goldModelMatrix, glm::vec3(0.5f, 0.5f, 0.5f));    // Skalowanie

	drawObjectPBR(models::gold, goldModelMatrix, glm::vec3(), textures::gold, 0.0f, 0.0f, 50.0f);

	goldModelMatrix = glm::mat4();
	goldModelMatrix = glm::translate(goldModelMatrix, glm::vec3(-7.0f, -0.5f, -4.0f)); // Przesunięcie
	goldModelMatrix = glm::scale(goldModelMatrix, glm::vec3(0.5f, 0.5f, 0.5f));    // Skalowanie

	drawObjectPBR(models::gold, goldModelMatrix, glm::vec3(), textures::gold, 0.0f, 0.0f, 50.0f);

	glm::mat4 rockModelMatrix = glm::mat4();
	rockModelMatrix = glm::translate(rockModelMatrix, glm::vec3(10.0f, -1.0f, -6.0f)); // Przesunięcie
	rockModelMatrix = glm::scale(rockModelMatrix, glm::vec3(0.006f, 0.006f, 0.006f));    // Skalowanie

	drawObjectPBR(models::rock, rockModelMatrix, glm::vec3(), textures::rock, 0.0f, 0.0f, 30.0f);

	rockModelMatrix = glm::mat4();
	rockModelMatrix = glm::translate(rockModelMatrix, glm::vec3(10.0f, -1.0f, -8.0f)); // Przesunięcie
	rockModelMatrix = glm::scale(rockModelMatrix, glm::vec3(0.006f, 0.006f, 0.006f));    // Skalowanie
	rockModelMatrix = glm::rotate(rockModelMatrix, glm::radians(50.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	drawObjectPBR(models::rock, rockModelMatrix, glm::vec3(), textures::rock, 0.0f, 0.0f, 30.0f);

	rockModelMatrix = glm::mat4();
	rockModelMatrix = glm::translate(rockModelMatrix, glm::vec3(10.0f, -3.0f, 20.0f)); // Przesunięcie
	rockModelMatrix = glm::scale(rockModelMatrix, glm::vec3(0.03f, 0.03f, 0.03f));    // Skalowanie
	rockModelMatrix = glm::rotate(rockModelMatrix, glm::radians(50.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	drawObjectPBR(models::rock, rockModelMatrix, glm::vec3(), textures::rock, 0.0f, 0.0f, 30.0f);

	rockModelMatrix = glm::mat4();
	rockModelMatrix = glm::translate(rockModelMatrix, glm::vec3(15.0f, -3.0f, 20.0f)); // Przesunięcie
	rockModelMatrix = glm::scale(rockModelMatrix, glm::vec3(0.03f, 0.03f, 0.03f));    // Skalowanie
	rockModelMatrix = glm::rotate(rockModelMatrix, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	drawObjectPBR(models::rock, rockModelMatrix, glm::vec3(), textures::rock, 0.0f, 0.0f, 30.0f);

	rockModelMatrix = glm::mat4();
	rockModelMatrix = glm::translate(rockModelMatrix, glm::vec3(20.0f, -3.5f, 20.0f)); // Przesunięcie
	rockModelMatrix = glm::scale(rockModelMatrix, glm::vec3(0.03f, 0.03f, 0.03f));    // Skalowanie
	rockModelMatrix = glm::rotate(rockModelMatrix, glm::radians(130.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	drawObjectPBR(models::rock, rockModelMatrix, glm::vec3(), textures::rock, 0.0f, 0.0f, 30.0f);

	rockModelMatrix = glm::mat4();
	rockModelMatrix = glm::translate(rockModelMatrix, glm::vec3(-10.0f, -3.0f, 24.0f)); // Przesunięcie
	rockModelMatrix = glm::scale(rockModelMatrix, glm::vec3(0.03f, 0.03f, 0.03f));    // Skalowanie
	rockModelMatrix = glm::rotate(rockModelMatrix, glm::radians(130.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	drawObjectPBR(models::rock, rockModelMatrix, glm::vec3(), textures::rock, 0.0f, 0.0f, 30.0f);

	rockModelMatrix = glm::mat4();
	rockModelMatrix = glm::translate(rockModelMatrix, glm::vec3(-14.0f, -3.0f, 24.0f)); // Przesunięcie
	rockModelMatrix = glm::scale(rockModelMatrix, glm::vec3(0.03f, 0.03f, 0.03f));    // Skalowanie
	rockModelMatrix = glm::rotate(rockModelMatrix, glm::radians(-120.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	drawObjectPBR(models::rock, rockModelMatrix, glm::vec3(), textures::rock, 0.0f, 0.0f, 30.0f);

	rockModelMatrix = glm::mat4();
	rockModelMatrix = glm::translate(rockModelMatrix, glm::vec3(-30.0f, -4.9f, 0.0f)); // Przesunięcie
	rockModelMatrix = glm::scale(rockModelMatrix, glm::vec3(0.03f, 0.03f, 0.03f));    // Skalowanie
	rockModelMatrix = glm::rotate(rockModelMatrix, glm::radians(-180.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	drawObjectPBR(models::rock, rockModelMatrix, glm::vec3(), textures::rock, 0.0f, 0.0f, 30.0f);

	rockModelMatrix = glm::mat4();
	rockModelMatrix = glm::translate(rockModelMatrix, glm::vec3(-30.0f, -6.0f, -13.0f)); // Przesunięcie
	rockModelMatrix = glm::scale(rockModelMatrix, glm::vec3(0.03f, 0.03f, 0.03f));    // Skalowanie
	rockModelMatrix = glm::rotate(rockModelMatrix, glm::radians(175.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	drawObjectPBR(models::rock, rockModelMatrix, glm::vec3(), textures::rock, 0.0f, 0.0f, 30.0f);

	rockModelMatrix = glm::mat4();
	rockModelMatrix = glm::translate(rockModelMatrix, glm::vec3(-24.0f, -5.0f, -22.0f)); // Przesunięcie
	rockModelMatrix = glm::scale(rockModelMatrix, glm::vec3(0.03f, 0.03f, 0.03f));    // Skalowanie
	rockModelMatrix = glm::rotate(rockModelMatrix, glm::radians(140.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	drawObjectPBR(models::rock, rockModelMatrix, glm::vec3(), textures::rock, 0.0f, 0.0f, 30.0f);

	rockModelMatrix = glm::mat4();
	rockModelMatrix = glm::translate(rockModelMatrix, glm::vec3(24.0f, -5.5f, -22.0f)); // Przesunięcie
	rockModelMatrix = glm::scale(rockModelMatrix, glm::vec3(0.03f, 0.03f, 0.03f));    // Skalowanie
	rockModelMatrix = glm::rotate(rockModelMatrix, glm::radians(-140.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	drawObjectPBR(models::rock, rockModelMatrix, glm::vec3(), textures::rock, 0.0f, 0.0f, 30.0f);

	rockModelMatrix = glm::mat4();
	rockModelMatrix = glm::translate(rockModelMatrix, glm::vec3(2.0f, -7.0f, -33.0f)); // Przesunięcie
	rockModelMatrix = glm::scale(rockModelMatrix, glm::vec3(0.03f, 0.03f, 0.03f));    // Skalowanie
	rockModelMatrix = glm::rotate(rockModelMatrix, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	drawObjectPBR(models::rock, rockModelMatrix, glm::vec3(), textures::rock, 0.0f, 0.0f, 30.0f);

	rockModelMatrix = glm::mat4();
	rockModelMatrix = glm::translate(rockModelMatrix, glm::vec3(33.0f, -5.5f, 0.0f)); // Przesunięcie
	rockModelMatrix = glm::scale(rockModelMatrix, glm::vec3(0.03f, 0.03f, 0.03f));    // Skalowanie
	rockModelMatrix = glm::rotate(rockModelMatrix, glm::radians(-170.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	drawObjectPBR(models::rock, rockModelMatrix, glm::vec3(), textures::rock, 0.0f, 0.0f, 30.0f);

	rockModelMatrix = glm::mat4();
	rockModelMatrix = glm::translate(rockModelMatrix, glm::vec3(33.0f, -5.5f, -10.0f)); // Przesunięcie
	rockModelMatrix = glm::scale(rockModelMatrix, glm::vec3(0.03f, 0.03f, 0.03f));    // Skalowanie
	rockModelMatrix = glm::rotate(rockModelMatrix, glm::radians(-150.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	drawObjectPBR(models::rock, rockModelMatrix, glm::vec3(), textures::rock, 0.0f, 0.0f, 30.0f);

	glm::mat4 crabModelMatrix = glm::mat4();
	crabModelMatrix = glm::translate(crabModelMatrix, glm::vec3(9.0f,-0.29f, -6.0f)); // Przesunięcie
	crabModelMatrix = glm::scale(crabModelMatrix, glm::vec3(0.03f, 0.03f, 0.03f));    // Skalowanie

	drawObjectPBR(models::crab, crabModelMatrix, glm::vec3(), textures::crab, 0.0f, 0.0f, 30.0f);

	crabModelMatrix = glm::mat4();
	crabModelMatrix = glm::translate(crabModelMatrix, glm::vec3(11.0f, -0.27f, -8.0f)); // Przesunięcie
	crabModelMatrix = glm::scale(crabModelMatrix, glm::vec3(0.03f, 0.03f, 0.03f));    // Skalowanie
	crabModelMatrix = glm::rotate(crabModelMatrix, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	drawObjectPBR(models::crab, crabModelMatrix, glm::vec3(), textures::crab, 0.0f, 0.0f, 30.0f);




	crabModelMatrix = glm::mat4();
	crabModelMatrix = glm::translate(crabModelMatrix, glm::vec3(9.0f, -0.27f, -8.0f)); // Przesunięcie
	crabModelMatrix = glm::scale(crabModelMatrix, glm::vec3(0.03f, 0.03f, 0.03f));    // Skalowanie
	crabModelMatrix = glm::rotate(crabModelMatrix, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	drawObjectPBR(models::crab, crabModelMatrix, glm::vec3(), textures::crab, 0.0f, 0.0f, 30.0f);

	crabModelMatrix = glm::mat4();
	crabModelMatrix = glm::translate(crabModelMatrix, glm::vec3(7.0f, -0.8f, -8.0f)); // Przesunięcie
	crabModelMatrix = glm::scale(crabModelMatrix, glm::vec3(0.03f, 0.03f, 0.03f));    // Skalowanie
	crabModelMatrix = glm::rotate(crabModelMatrix, glm::radians(62.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	drawObjectPBR(models::crab, crabModelMatrix, glm::vec3(), textures::crab, 0.0f, 0.0f, 30.0f);


	glm::mat4 statueModelMatrix = glm::mat4();
	statueModelMatrix = glm::translate(statueModelMatrix, glm::vec3(-10.0f, -1.25f, 10.0f)); // Przesunięcie
	statueModelMatrix = glm::scale(statueModelMatrix, glm::vec3(0.2f, 0.2f, 0.2f));    // Skalowanie
	statueModelMatrix = glm::rotate(statueModelMatrix, glm::radians(-50.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	statueModelMatrix = glm::rotate(statueModelMatrix, glm::radians(20.0f), glm::vec3(1.0f, 0.0f, 1.0f));

	drawObjectPBR(models::statue, statueModelMatrix, glm::vec3(), textures::statue, 0.0f, 0.0f, 30.0f);

	//renderSeaweed();
	for (const auto& seaweedMatrix : seaweedMatrices) {
		drawObjectPBR(models::seaweed, seaweedMatrix, glm::vec3(), textures::seaweed, 0.1f, 0.0f, 10.0f);
	}



	renderBoids();

	animateJellyfishInstances();

	//render and animate player
	animatePlayer();

	glfwSwapBuffers(window);
}



// initialization ---------------------------------------------------------------------------------------------------------------------------------------------- initialization
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	aspectRatio = width / float(height);
	glViewport(0, 0, width, height);
	WIDTH = width;
	HEIGHT = height;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // Odwrócone, ponieważ współrzędne y idą w górę
	lastX = xpos;
	lastY = ypos;

	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	// Upewnij się, że pitch nie przekracza 89 stopni, aby uniknąć dziwnych efektów
	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraDir = glm::normalize(front);

	playerDir = cameraDir;
}


// init ------------------------------------------------------------------------------------------------------- init
void init(GLFWwindow* window)
{
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Ukryj kursor i zablokuj go w oknie


	glEnable(GL_DEPTH_TEST);
	programPBR = shaderLoader.CreateProgram("shaders/shader_pbr.vert", "shaders/shader_pbr.frag");
	programTex = shaderLoader.CreateProgram("shaders/shader_pbr_tex.vert", "shaders/shader_pbr_tex.frag");
	programSun = shaderLoader.CreateProgram("shaders/shader_sun.vert", "shaders/shader_sun.frag");
	programSkybox = shaderLoader.CreateProgram("shaders/shader_skybox.vert", "shaders/shader_skybox.frag");
	programDepth = shaderLoader.CreateProgram("shaders/shader_shadow.vert", "shaders/shader_shadow.frag");

	loadAllModels();

	//init depth maps
	initDepthMap(depthMapSun, depthMapSunFBO);

	initializeBoids(100);
	addJellyfishInstance(glm::vec3(-10.0f, 5.0f, -5.0f), 0.5f, 1.0f);
	addJellyfishInstance(glm::vec3(-9.0f, 6.5f, -5.0f), 0.2f, 0.9f);
	addJellyfishInstance(glm::vec3(-11.0f, 7.0f, -7.0f), 1.0f, 0.7f);
	addJellyfishInstance(glm::vec3(-10.0f, 7.0f, -6.0f), 0.7f, 0.8f);
	addJellyfishInstance(glm::vec3(-11.0f, 5.5f, -5.0f), 0.8f, 0.7f);

	addJellyfishInstance(glm::vec3(8.0f, 1.5f, -7.0f), 0.6f, 1.2f);
	addJellyfishInstance(glm::vec3(12.0f, 2.0f, -7.0f), 0.5f, 1.0f);
	addJellyfishInstance(glm::vec3(8.0f, 2.5f, -5.0f), 0.6f, 0.8f);
	addJellyfishInstance(glm::vec3(15.0f, 3.0f, -6.0f), 0.8f, 1.0f);
	addJellyfishInstance(glm::vec3(8.0f, 2.5f, -9.0f), 0.7f, 0.7f);

	addJellyfishInstance(glm::vec3(-4.0f, 1.5f, -6.0f), 0.4f, 0.8f);
	addJellyfishInstance(glm::vec3(1.0f, 2.5f, -8.0f), 0.6f, 0.6f);
	addJellyfishInstance(glm::vec3(0.0f, 2.0f, -10.0f), 0.4f, 0.8f);
	addJellyfishInstance(glm::vec3(-2.5f, 3.0f, -12.0f), 0.8f, 0.7f);
	addJellyfishInstance(glm::vec3(2.0f, 1.5f, -10.0f), 0.4f, 0.6f);

	addJellyfishInstance(glm::vec3(-2.5f, 3.0f, -16.0f), 0.6f, 0.6f);

	addJellyfishInstance(glm::vec3(5.5f, 2.0f, -18.0f), 0.4f, 0.7f);
	addJellyfishInstance(glm::vec3(0.5f, 3.0f, -17.0f), 0.8f, 0.6f);
	addJellyfishInstance(glm::vec3(-10.0f, 3.5f, -19.0f), 0.5f, 0.7f);
	addJellyfishInstance(glm::vec3(-4.0f, 1.7f, -17.0f), 0.6f, 0.6f);
	addJellyfishInstance(glm::vec3(-17.5f, 2.5f, -12.0f), 0.4f, 0.7f);
	addJellyfishInstance(glm::vec3(-14.5f, 1.0f, -10.0f), 0.6f, 0.8f);
	addJellyfishInstance(glm::vec3(10.5f, 4.0f, -18.0f), 0.5f, 0.6f);
	addJellyfishInstance(glm::vec3(8.5f, 2.8f, -14.0f), 0.8f, 0.6f);
	addJellyfishInstance(glm::vec3(10.5f, 3.4f, -17.0f), 0.4f, 0.5f);
	addJellyfishInstance(glm::vec3(3.5f, 1.8f, -19.0f), 0.7f, 0.6f);
	addJellyfishInstance(glm::vec3(-2.5f, 2.4f, -19.0f), 0.7f, 0.5f);

	addJellyfishInstance(glm::vec3(8.5f, 2.8f, -25.0f), 0.8f, 0.6f);
	addJellyfishInstance(glm::vec3(10.5f, 3.4f, -30.0f), 0.4f, 0.5f);
	addJellyfishInstance(glm::vec3(-13.5f, 1.8f, -28.0f), 0.7f, 0.6f);
	addJellyfishInstance(glm::vec3(-6.5f, 2.4f, -29.0f), 0.7f, 0.5f);

	renderSeaweed();



}


//shutdown ---------------------------------------------------------------------------------------------------------------------------------------------------------- shutdown
void shutdown(GLFWwindow* window)
{
	shaderLoader.DeleteProgram(programPBR);
	shaderLoader.DeleteProgram(programTex);
	shaderLoader.DeleteProgram(programSun);
	shaderLoader.DeleteProgram(programSkybox);
	shaderLoader.DeleteProgram(programDepth);
}

//input processing ------------------------------------------------------------------------------------------------------------------------------------------- input processing
void processInput(GLFWwindow* window)
{
	glm::vec3 spaceshipSide = glm::normalize(glm::cross(playerDir, glm::vec3(0.f, 1.f, 0.f)));
	glm::vec3 spaceshipUp = glm::vec3(0.f, 1.f, 0.f);
	float angleSpeed = 0.05f * deltaTime * 60;
	float moveSpeed = 0.05f * deltaTime * 60;


	//exit
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}


	//motion
	glm::vec3 acceleration(0.0f);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		playerPos += playerDir * moveSpeed;

	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		playerPos -= playerDir * moveSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		playerPos += spaceshipSide * moveSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		playerPos -= spaceshipSide * moveSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		playerPos += spaceshipUp * moveSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
		playerPos -= spaceshipUp * moveSpeed;
	}


	//rotation
	/*if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
		playerDir = glm::vec3(glm::eulerAngleY(angleSpeed) * glm::vec4(playerDir, 0));
	}
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
		playerDir = glm::vec3(glm::eulerAngleY(-angleSpeed) * glm::vec4(playerDir, 0));
	}*/


	//update camera
	cameraPos = playerPos - 0.5 * playerDir + glm::vec3(0, 1, 0) * 0.2f;
	//cameraDir = playerDir;


	//exposition
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
		exposition -= 0.05;
	}
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
		exposition += 0.05;
	}

	//sunDir
	//x
	if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) {
		sunx += 0.1;
	}
	if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS) {
		sunx -= 0.1;
	}
	//y
	if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS) {
		suny += 0.1;
	}
	if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS) {
		suny -= 0.1;
	}
	//z
	if (glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS) {
		sunz += 0.1;
	}
	if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS) {
		sunz -= 0.1;
	}


	// debug info
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
		printf("spaceshipPos = glm::vec3(%ff, %ff, %ff);\n", playerPos.x, playerPos.y, playerPos.z);
		printf("spaceshipDir = glm::vec3(%ff, %ff, %ff);\n", playerDir.x, playerDir.y, playerDir.z);
		printf("sunDir = glm::vec3(%ff, %ff, %ff);\n\n", sunx, suny, suny);
	}
}

//constrain movement  ------------------------------------------------------------------------------------------------------------------------------------ constrain movement
void constrainMovement()
{
	//x pos
	if (playerPos.x > 15.0f)
	{
		playerPos.x = 15.0f;
	}
	if (playerPos.x < -15.0f)
	{
		playerPos.x = -15.0f;
	}

	//y pos
	if (playerPos.y > 12.0f)
	{
		playerPos.y = 12.0f;
	}
	if (playerPos.y < 0.0f)
	{
		playerPos.y = 0.0f;
	}

	//z pos
	if (playerPos.z > 15.0f)
	{
		playerPos.z = 15.0f;
	}
	if (playerPos.z < -15.0f)
	{
		playerPos.z = -15.0f;
	}

}



//fps limiter --------------------------------------------------------------------------------------------------------------------------------------------------fps limiter
void setMaxFPS(float fps)
{
	if (1 / deltaTime > fps)
	{
		float timeToDelay = 1 - (deltaTime * fps);
		Sleep((timeToDelay * 1000) / fps);
	}
}


//main loop -------------------------------------------------------------------------------------------------------------------------------------------------------- main loop
void renderLoop(GLFWwindow* window) {
	while (!glfwWindowShouldClose(window))
	{
		processInput(window);
		constrainMovement();

		renderScene(window);
		glfwPollEvents();

		setMaxFPS(75);
	}
}

