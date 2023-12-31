#pragma once

// Includes
//------------------------------------------------------------------------------
// Core
#include "Core/AssetManager.h"

// System
#include <memory>

// Forward Declarations
// --------------------------------------------------------------------------------
class Texture;
class Spritesheet;

// --------------------------------------------------------------------------------
class TextureLoader : public AssetLoader<Texture>
{
public:
	virtual std::unique_ptr<Asset> Load(AssetFileDescriptor<Texture> descriptor) override;
	virtual std::unique_ptr<Asset> Load(AssetMemoryDescriptor<Texture> descriptor) override;

private:
	std::unique_ptr<Asset> Load(const std::string& filePath);
};

// --------------------------------------------------------------------------------
class SpritesheetLoader : public AssetLoader<Spritesheet>
{
public:
	virtual std::unique_ptr<Asset> Load(AssetFileDescriptor<Spritesheet> descriptor) override;
	virtual std::unique_ptr<Asset> Load(AssetMemoryDescriptor<Spritesheet> descriptor) override;
};
