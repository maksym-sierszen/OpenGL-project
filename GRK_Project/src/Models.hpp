#include <string>
#include <filesystem>

//models ------------------------------------------------------------------------------------------------------------------------------------------------------------- models
namespace models
{
	//structures

	Core::RenderContext sphere;
	Core::RenderContext ground;
	Core::RenderContext skybox;

	//animals

	//user

	//environment
	

}

//textures ----------------------------------------------------------------------------------------------------------------------------------------------------- textures
namespace textures
{
	//structures

	GLuint sphere;
	GLuint ground;
	GLuint skybox;

	//animals

	//user

	//environment

}

//paths ----------------------------------------------------------------------------------------------------------------------------------------------------------path
namespace objects_paths
{
	//structures
	std::string sphere = "./models/structures/sphere";
	std::string ground = "./models/structures/ground";
	std::string skybox = "./models/structures/skybox";

	//animals


	//user
	

	//environment

}

//get element path ------------------------------------------------------------------------------------------------------------------ get element path

std::string getModelPath(std::string path)
{
	std::string model_path = "";
	for (const auto& entry : std::filesystem::directory_iterator(path))
	{
		std::string file = entry.path().generic_u8string();
		if (file.substr(file.length() - 4, file.length()) == ".obj")
		{
			model_path = file;
		}
	}

	return model_path;
}

std::string getMaterialPath(std::string path)
{
	std::string material_path = "";
	for (const auto& entry : std::filesystem::directory_iterator(path))
	{
		std::string file = entry.path().generic_u8string();
		if (file.substr(file.length() - 4, file.length()) == ".mtl")
		{
			material_path = file;
		}
	}

	return material_path;
}

std::string getTexturePath(std::string path)
{
	std::string texture_path = "";
	for (const auto& entry : std::filesystem::directory_iterator(path))
	{
		std::string file = entry.path().generic_u8string();
		if (file.substr(file.length() - 4, file.length()) == ".png")
		{
			texture_path = file;
		}
	}

	return texture_path;
}


//load models and their textures ------------------------------------------------------------------------------------------------------------ load models and their textures
void loadModelToContext(std::string pathObject, Core::RenderContext& context, GLuint& texture)
{
	Assimp::Importer import;

	const aiScene* scene = import.ReadFile(getModelPath(pathObject), aiProcess_Triangulate | aiProcess_CalcTangentSpace);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
		return;
	}
	context.initFromAssimpMesh(scene->mMeshes[0]);

	if (getTexturePath(pathObject) != "")
	{
		texture = Core::LoadTexture(getTexturePath(pathObject).c_str());
	}
}

//load skybox and it's texture -------------------------------------------------------------------------------------------------------------- load skybox and it's texture
void loadSkyboxWithTextures(std::string pathObject, Core::RenderContext& context)
{
	loadModelToContext(pathObject, context, textures::skybox);

	int w, h;

	glGenTextures(1, &textures::skybox);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textures::skybox);

	const char* filepaths[6] = {
		"models/structures/skybox/textures/side1.png",
		"models/structures/skybox/textures/side2.png",
		"models/structures/skybox/textures/top.jpg",
		"models/structures/skybox/textures/bottom.jpg",
		"models/structures/skybox/textures/side3.png",
		"models/structures/skybox/textures/side4.jpg"
	};
	for (unsigned int i = 0; i < 6; i++)
	{
		unsigned char* image = SOIL_load_image(filepaths[i], &w, &h, 0, SOIL_LOAD_RGBA);
		if (image) {
			glTexImage2D(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image
			);
		}
		else {
			std::cout << "Failed to load texture: " << filepaths[i] << std::endl;
		}
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

//init loading ----------------------------------------------------------------------------------------------------------------------------------------- init loading
void loadAllModels()
{
	//load structures and their textures
	loadModelToContext(objects_paths::sphere, models::sphere, textures::sphere);
	loadModelToContext(objects_paths::ground, models::ground, textures::ground);
	

	

	//load user and his texture


	//load skybox and it's textures
	loadSkyboxWithTextures(objects_paths::skybox, models::skybox);

	//load environment objects and their textures 

}
