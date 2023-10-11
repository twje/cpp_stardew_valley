#include "Core/TiledMapLoader.h"

#include <fstream>
#include <iostream>
#include <cassert>

#include "Core/TiledMap.h"

// --------------------------------------------------------------------------------
/*static*/ std::unique_ptr<TiledMap> TiledMapLoader::Load(const fs::path& filePath)
{
    assert(HasFileExtension(filePath, ".json"));

    json parentNode;
    LoadJson(filePath.string(), parentNode);

    std::unique_ptr<TiledMap> tiledMap = LoadMapSettings(parentNode);
    LoadLayers(*tiledMap.get(), parentNode);
    LoadTileSets(filePath.parent_path(), *tiledMap.get(), parentNode);

	return tiledMap;
}

// --------------------------------------------------------------------------------
/*static*/ std::unique_ptr<TiledMap> TiledMapLoader::LoadMapSettings(json& parentNode)
{
    uint32_t width = ExtractUInt32(parentNode, "width");
    uint32_t height = ExtractUInt32(parentNode, "height");
    uint32_t tileWidth = ExtractUInt32(parentNode, "tilewidth");
    uint32_t tileHeight = ExtractUInt32(parentNode, "tileheight");

    return std::make_unique<TiledMap>(width, height, tileWidth, tileHeight);
}

// --------------------------------------------------------------------------------
/*static*/ void TiledMapLoader::LoadLayers(TiledMap& tiledMap, json& parentNode)
{
    for (const json& layerNode : parentNode["layers"])
    {
        const std::string& layerName = ExtractString(layerNode,"name");
        const std::string& layerType = ExtractString(layerNode, "type");
        if (layerType == "tilelayer")
        {
            uint32_t layerWidth = ExtractUInt32(layerNode, "width");
            uint32_t layerHeight = ExtractUInt32(layerNode, "height");

            std::vector<uint32_t> tiles;
            for (const json& tileNode : layerNode["data"])
            {
                tiles.emplace_back(tileNode.get<uint32_t>());
            }

            TiledLayer layer(layerName, layerWidth, layerHeight, std::move(tiles));
            tiledMap.AddLayer(std::move(layer));
        }
    }
}

// --------------------------------------------------------------------------------
/*static*/ void TiledMapLoader::LoadTileSets(const fs::path& directoryPath, TiledMap& tiledMap, json& parentNode)
{
    for (json tilesetNode : parentNode["tilesets"])
    {
        uint32_t firstGid = ExtractUInt32(tilesetNode, "firstgid");

        // Parse data
        json tilesetData;
        if (tilesetNode.contains("source"))
        {
            const std::string& sourcefilePath = ExtractString(tilesetNode, "source");
            fs::path filePath = directoryPath / fs::path(sourcefilePath);
            LoadJson(filePath, tilesetData);
        }
        else
        {
            tilesetData = tilesetNode;
        }

        uint32_t columns = ExtractUInt32(tilesetData, "columns");
        if (columns > 0)
        {
            uint32_t imageWidth = ExtractUInt32(tilesetData, "imagewidth");
            uint32_t imageHeight = ExtractUInt32(tilesetData, "imageheight");
            uint32_t tileWidth = ExtractUInt32(tilesetData, "tilewidth");
            uint32_t tileHeight = ExtractUInt32(tilesetData, "tileheight");
            uint32_t margin = ExtractUInt32(tilesetData, "margin");
            uint32_t spacing = ExtractUInt32(tilesetData, "spacing");
            uint32_t tileCount = ExtractUInt32(tilesetData, "tilecount");
            const std::string& name = ExtractString(tilesetData, "name");

            TiledSet tiledSet(firstGid, columns, imageHeight, imageWidth,
                              tileWidth, tileHeight, margin, spacing,
                              tileCount, name);

            tiledMap.AddTileset(std::move(tiledSet));
        }
        else
        {
            // TileSet is a collection of images
        }
    }
}

// --------------------------------------------------------------------------------
/*static*/ void TiledMapLoader::LoadJson(fs::path filePath, json& outJson)
{
    std::ifstream ifs(filePath);
    if (!ifs.is_open())
    {
        throw std::runtime_error("Error opening file: " + filePath.string());
    }
    try
    {
        ifs >> outJson;
    }
    catch (const json::parse_error& e)
    {
        throw std::runtime_error("Error parsing JSON: " + std::string(e.what()));
    }
}

// --------------------------------------------------------------------------------
/*static*/ uint32_t TiledMapLoader::ExtractUInt32(const json& parentNode, const std::string& key)
{
    if (!parentNode.contains(key))
    {
        throw std::invalid_argument("Invalid JSON structure: missing key - " + key);
    }
    return parentNode[key].get<uint32_t>();
}

// --------------------------------------------------------------------------------
/*static*/ std::string TiledMapLoader::ExtractString(const json& parentNode, const std::string& key)
{
    if (!parentNode.contains(key))
    {
        throw std::invalid_argument("Invalid JSON structure: missing key - " + key);
    }
    return parentNode[key].get<std::string>();
}

// --------------------------------------------------------------------------------
/*static*/ bool TiledMapLoader::HasFileExtension(fs::path filePath, const std::string& extension)
{
    if (filePath.has_extension())
    {
        return filePath.extension() == extension;
    }
    return false;
}