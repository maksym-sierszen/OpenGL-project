#ifndef MODELS_HPP
#define MODELS_HPP

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
	Core::RenderContext trout;
	Core::RenderContext nemo;
	Core::RenderContext shark;
	Core::RenderContext anglerfish;
	Core::RenderContext crab;

	Core::RenderContext jellyfish;

	


	//user

	//environment
	Core::RenderContext v_boat;

	Core::RenderContext seashell;
	Core::RenderContext seaweed;

	Core::RenderContext treasureChest;
	Core::RenderContext rock;
	Core::RenderContext statue;
	Core::RenderContext gold;
	Core::RenderContext remains;
	Core::RenderContext scull;
	Core::RenderContext beast;
	Core::RenderContext sword;

}

//textures ----------------------------------------------------------------------------------------------------------------------------------------------------- textures
namespace textures
{
	//structures

	GLuint sphere;
	GLuint ground;
	GLuint skybox;

	//animals
	GLuint trout;
	GLuint nemo;
	GLuint shark;
	GLuint crab;
	GLuint jellyfish;
	GLuint anglerfish;
	


	//user

	//environment
	GLuint v_boat;
	GLuint seashell;
	GLuint seaweed;
	GLuint treasureChest;
	GLuint rock;
	GLuint statue;
	GLuint gold;
	GLuint remains;
	GLuint scull;
	GLuint beast;
	GLuint sword;

}

//paths ----------------------------------------------------------------------------------------------------------------------------------------------------------path
namespace objects_paths
{
	//structures
	std::string sphere = "./models/structures/sphere";
	std::string ground = "./models/structures/ground";
	std::string skybox = "./models/structures/skybox";

	//animals
	std::string trout = "./models/environment/trout";
	std::string nemo = "./models/environment/nemo";
	std::string shark = "./models/environment/shark";
	std::string crab = "./models/animals/crab";
	std::string jellyfish = "./models/environment/jellyfish";
	std::string anglerfish = "./models/animals/anglerfish";


	//user
	

	//environment
	std::string v_boat = "./models/environment/v_boat";
	std::string seashell = "./models/environment/seashell";
	std::string seaweed = "./models/environment/seaweed";
	std::string treasureChest = "./models/environment/treasureChest";
	std::string rock = "./models/environment/rock";
	std::string statue = "./models/environment/statue";
	std::string gold = "./models/environment/gold";
	std::string remains = "./models/environment/remains";
	std::string scull = "./models/environment/scull";
	std::string beast = "./models/environment/beast";
	std::string sword = "./models/environment/sword";

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
		"models/structures/skybox/textures/uw_ft.jpg",
		"models/structures/skybox/textures/uw_bk.jpg",
		"models/structures/skybox/textures/uw_up.jpg",
		"models/structures/skybox/textures/uw_dn.jpg",
		"models/structures/skybox/textures/uw_rt.jpg",
		"models/structures/skybox/textures/uw_lf.jpg"
	};
	for (unsigned int i = 0; i < 6; i++)
	{
		unsigned char* image = SOIL_load_image(filepaths[i], &w, &h, 0, SOIL_LOAD_RGBA);
		std::cout << "Trying to load texture: " << filepaths[i] << std::endl;
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
	loadModelToContext(objects_paths::v_boat, models::v_boat, textures::v_boat);
	loadModelToContext(objects_paths::trout, models::trout, textures::trout);
	loadModelToContext(objects_paths::nemo, models::nemo, textures::nemo);
	loadModelToContext(objects_paths::shark, models::shark, textures::shark);

	loadModelToContext(objects_paths::seashell, models::seashell, textures::seashell);
	loadModelToContext(objects_paths::crab, models::crab, textures::crab);
	loadModelToContext(objects_paths::seaweed, models::seaweed, textures::seaweed);

	loadModelToContext(objects_paths::jellyfish, models::jellyfish, textures::jellyfish);
	loadModelToContext(objects_paths::treasureChest, models::treasureChest, textures::treasureChest);
	loadModelToContext(objects_paths::rock, models::rock, textures::rock);
	loadModelToContext(objects_paths::crab, models::crab, textures::crab);
	loadModelToContext(objects_paths::statue, models::statue, textures::statue);
	loadModelToContext(objects_paths::gold, models::gold, textures::gold);
	loadModelToContext(objects_paths::remains, models::remains, textures::remains);
	loadModelToContext(objects_paths::scull, models::scull, textures::scull);
	loadModelToContext(objects_paths::beast, models::beast, textures::beast);
	loadModelToContext(objects_paths::sword, models::sword, textures::sword);
	loadModelToContext(objects_paths::anglerfish, models::anglerfish, textures::anglerfish);
}

#endif // MODELS_HPP

