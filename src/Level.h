#pragma once

// Includes
//------------------------------------------------------------------------------
// Game
#include "Player.h"

// Core
#include "Core/Scene.h"
#include "Core/Group.h"
#include "Core/AssetManager.h"
#include "Core/Tiled/TiledMap.h"

#include <iostream>
#include "Overlay.h"
#include "Sprites.h"

//------------------------------------------------------------------------------
class SceneLayerRenderer
{
public:
	SceneLayerRenderer(TiledMap* tiledMap)
		: mTiledMap(tiledMap)
		, mExcludedLayers(tiledMap->LayerCount(), false)
	{ }

	void ExcludeLayerFromRendering(const std::string& layerName) 
	{
		std::optional<size_t> index = mTiledMap->GetLayerIndex(layerName);
		assert(index.has_value());
		mExcludedLayers[index.value()] = true;
	}

	void DrawLayer(size_t layerIndex, sf::RenderWindow& window, const ViewRegion& viewRegion)
	{
		if (!mExcludedLayers[layerIndex])
		{
			mTiledMap->DrawLayer(layerIndex, window, viewRegion);
		}
	}

private:
	TiledMap* mTiledMap;
	std::vector<bool> mExcludedLayers;
};

//------------------------------------------------------------------------------
class Level : public Scene
{
public:
	void Create() override
	{
		mAllSprites = CreateGroup();
		mTreeSprites = CreateGroup();
		mCollisionSprites = CreateGroup();

		AssetManager& assetManager = GetResourceLocator().GetAssetManager();

		const ApplicationConfig& config = GetResourceLocator().GetApplicationConfig();
		sf::Vector2f windowSize = sf::Vector2f(config.GetWindowSize());

		mWorldView.setSize(windowSize);
		mWorldView.setCenter(windowSize * 0.5f);

		mHUDView.setSize(windowSize);
		mHUDView.setCenter(windowSize * 0.5f);

		mTiledMap = &assetManager.GetAsset<TiledMap>("main");
		mLayerRenderer = std::make_unique<SceneLayerRenderer>(mTiledMap);
		
		// House
		for (const std::string& layerName : { "HouseFloor", "HouseFurnitureBottom" })
		{
			mLayerRenderer->ExcludeLayerFromRendering(layerName);
			for (auto& definition : mTiledMap->GetObjectDefinitions(layerName))
			{
				auto* sprite = CreateGameObject<TiledMapObjectSprite>(definition, 
																	  LAYERS.at("house bottom"));
				mAllSprites->Add(sprite);
			}
		}
		
		for (const std::string& layerName : { "HouseWalls", "HouseFurnitureTop" })
		{
			mLayerRenderer->ExcludeLayerFromRendering(layerName);
			for (auto& definition : mTiledMap->GetObjectDefinitions(layerName))
			{
				auto* sprite = CreateGameObject<TiledMapObjectSprite>(definition);
				mAllSprites->Add(sprite);
			}
		}		
		
		// Fence
		for (const std::string& layerName : { "Fence" })
		{
			mLayerRenderer->ExcludeLayerFromRendering(layerName);
			for (auto& definition : mTiledMap->GetObjectDefinitions(layerName))
			{
				auto* sprite = CreateGameObject<TiledMapObjectSprite>(definition);
				mAllSprites->Add(sprite);
				mCollisionSprites->Add(sprite);
			}
		}		

		// Trees
		for (const std::string& layerName : { "Trees" })
		{
			mLayerRenderer->ExcludeLayerFromRendering(layerName);
			for (auto& definition : mTiledMap->GetObjectDefinitions(layerName))
			{
				Tree* object = CreateGameObject<Tree>(std::move(definition), *mAllSprites);
				mAllSprites->Add(object);
				mCollisionSprites->Add(object);
				mTreeSprites->Add(object);
			}
		}

		// Wild flowers
		for (const std::string& layerName : { "Decoration" })
		{
			mLayerRenderer->ExcludeLayerFromRendering(layerName);
			for (auto& definition : mTiledMap->GetObjectDefinitions(layerName))
			{
				WildFlower* object = CreateGameObject<WildFlower>(std::move(definition));
				mAllSprites->Add(object);
				mCollisionSprites->Add(object);
			}
		}

		// Collion tiles
		for (const std::string& layerName : { "Collision" })
		{
			mLayerRenderer->ExcludeLayerFromRendering(layerName);
			for (auto& definition : mTiledMap->GetObjectDefinitions(layerName))
			{
				auto* sprite = CreateGameObject<TiledMapObjectSprite>(definition);
				mCollisionSprites->Add(sprite);
			}
		}		

		// Player
		for (auto& definition : mTiledMap->GetObjectDefinitions("Player"))
		{
			mLayerRenderer->ExcludeLayerFromRendering("Player");
			if (definition.GetName() == "Start")
			{
				mPlayer = CreateGameObject<Player>(assetManager, 
												   definition.GetPosition(),
					                               *mCollisionSprites, 
					                               *mTreeSprites);
				mAllSprites->Add(mPlayer);
			}
		}

		mOverlay = std::make_unique<Overlay>(assetManager, *mPlayer);
	}

	void Update(const sf::Time& timestamp) override
	{
		mTiledMap->Update(timestamp);

		for (GameObject* gameObject : *mAllSprites)
		{
			gameObject->Update(timestamp);
		}

		mWorldView.setCenter(mPlayer->GetCenter());
	}

	sf::FloatRect GetViewRegion(const sf::View& view)
	{
		sf::Vector2f center = view.getCenter();
		sf::Vector2f size = view.getSize();

		sf::Vector2f topLeft((center.x - size.x / 2), (center.y - size.y / 2));
		sf::Vector2f bottomRight((center.x + size.x / 2), (center.y + size.y / 2));

		return sf::FloatRect(topLeft, bottomRight - topLeft);
	}

	virtual void Draw(sf::RenderWindow& window)
	{
		window.setView(mWorldView);

		mAllSprites->Sort(SpriteCompareFunc);

		const ViewRegion viewRegion = GetViewRegion();
		for (size_t layerIndex = 0; layerIndex < mTiledMap->LayerCount(); layerIndex++)
		{									
			for (GameObject* gameObject : *mAllSprites)
			{
				if (gameObject->GetDepth() == layerIndex)
				{
					window.draw(*gameObject);
				}
			}
			mLayerRenderer->DrawLayer(layerIndex, window, viewRegion);
		}
		//DebugDrawHitboxes(window);
		//DrawPlayerTargetPosition(window);

		window.setView(mHUDView);
		mOverlay->Draw(window);
	}

	void DebugDrawHitboxes(sf::RenderWindow& window)
	{
		for (GameObject* gameObject : *mCollisionSprites)
		{
			DrawRect(window, static_cast<Sprite*>(gameObject)->GetHitbox(), sf::Color::Red);
			DrawRect(window, static_cast<Sprite*>(gameObject)->GetGlobalBounds(), sf::Color::Blue);
		}
	}

	void DrawPlayerTargetPosition(sf::RenderWindow& window)
	{
		float radius = 5.0f;
		sf::Vector2f offset(-radius, -radius);

		sf::CircleShape point(radius);
		point.setPosition(mPlayer->GetTargetPosition() + offset);
		point.setFillColor(sf::Color::Red);

		window.draw(point);
	}

	virtual void OnWindowResize(const sf::Vector2u& size)
	{
		mWorldView.setSize(sf::Vector2f(size));
		mHUDView.setSize(sf::Vector2f(size));
	}

private:
	static bool SpriteCompareFunc(const GameObject* object1, const GameObject* object2)
	{		
		float y1 = static_cast<const Sprite*>(object1)->GetCenter().y;
		float y2 = static_cast<const Sprite*>(object2)->GetCenter().y;

		return y1 < y2;
	}

	ViewRegion GetViewRegion()
	{
		sf::Vector2f halfSize = mWorldView.getSize() / 2.0f;
		sf::Vector2f position = mWorldView.getCenter() - halfSize;
		sf::Vector2f size = halfSize * 2.0f;
		
		return { mTiledMap->GetTileSize(), mTiledMap->GetMapSize(), { position, size } };
	}

	Player* mPlayer;
	Generic* mGround;

	Group* mAllSprites{ nullptr };
	Group* mTreeSprites{ nullptr };
	Group* mCollisionSprites{ nullptr };

	std::unique_ptr<Overlay> mOverlay;
	sf::View mWorldView;
	sf::View mHUDView;

	TiledMap* mTiledMap;
	std::unique_ptr<SceneLayerRenderer> mLayerRenderer;
};