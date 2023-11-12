#pragma once

#include <memory>

#include "Core/AssetManager.h"

// --------------------------------------------------------------------------------
class TextureLoader : public AssetLoader
{
public:
	virtual std::unique_ptr<Asset> Load(const std::string& fileName, AssetManager& assetManager) override;
};

// --------------------------------------------------------------------------------
class SpritesheetLoader : public AssetLoader
{
public:
	virtual std::unique_ptr<Asset> Load(const std::string& filePath, AssetManager& assetManager) override;
};
