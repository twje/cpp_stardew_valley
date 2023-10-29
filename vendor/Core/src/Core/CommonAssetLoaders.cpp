#include <SFML/Graphics.hpp>

#include "Core/CommonAssetLoaders.h"
#include "Core/Animation/Animation.h"
#include "Core/Animation/AnimationSequence.h"
#include "Core/Spritesheet.h"

// --------------------------------------------------------------------------------
std::unique_ptr<AssetBase> TextureLoader::Load(const std::string& fileName, AssetManager& assetManager)
{
	auto texture = std::make_unique<sf::Texture>();
	if (!texture->loadFromFile(fileName))
	{
		throw std::runtime_error("Failed to load texture: " + fileName);
	}
	return std::make_unique<Asset<sf::Texture>>(std::move(texture));
}

// --------------------------------------------------------------------------------
std::unique_ptr<AssetBase> SpritesheetLoader::Load(const std::string& filePath, AssetManager& assetManager)
{
	auto spritesheet = std::make_unique<Spritesheet>();
	spritesheet->LoadFromFile(filePath, assetManager);
	return std::make_unique<Asset<Spritesheet>>(std::move(spritesheet));
}