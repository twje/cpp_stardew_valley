#pragma once

#include <iostream>
#include <unordered_map>
#include <vector>

#include "Core/GameObject.h"
#include "Core/ResourceLocator.h"
#include "Core/Animation/Animation.h"
#include "Core/Support.h"
#include "Core/ItemPicker.h"
#include "Core/AssetManager.h"
#include "Core/Animation/AnimationPlayer.h"

#include "Settings.h"

// --------------------------------------------------------------------------------
enum class TimerId
{
	TOOL_USE = 0,
	TOOL_SWITCH = 1,
	SEED_USE = 2,
	SEED_SWITCH = 3
};

// --------------------------------------------------------------------------------
class IPlayerObserver
{
public:
	virtual void ToolChanged(std::string tool) { }
	virtual void SeedChanged(std::string seed) { }
};

// --------------------------------------------------------------------------------
class PlayerSubject
{
public:
	void Subscribe(IPlayerObserver* observer) { mObservers.emplace_back(observer); }

	void NotifyToolChanged(std::string tool)
	{
		for (auto& observer : mObservers)
		{
			observer->ToolChanged(tool);
		}
	}

	void NotifySeedChanged(std::string seed)
	{
		for (auto& observer : mObservers)
		{
			observer->SeedChanged(seed);
		}
	}

private:
	std::vector<IPlayerObserver*> mObservers;
};

// --------------------------------------------------------------------------------
class Player : public Sprite, public PlayerSubject
{
public:
	Player(AssetManager& assetManager, const sf::Vector2f& position)
		: mAnimationPlayer(assetManager.GetAsset<Animation>("character")),
		  mSpeed(300),
		  mStatus("down_idle"),
		  mToolPicker({ "hoe", "axe", "water"}),
		  mSeedPicker({ "corn", "tomato" })
	{		
		mAnimationPlayer.SetAnimationSequence(mStatus);
		SetPosition(position);
		SetOrigin(sf::Vector2f(0.5f, 0.5f));

		mTimers.emplace(TimerId::TOOL_USE, Timer(sf::milliseconds(350), std::bind(&Player::UseTool, this)));
		mTimers.emplace(TimerId::TOOL_SWITCH, Timer(sf::milliseconds(200)));
		mTimers.emplace(TimerId::SEED_USE, Timer(sf::milliseconds(350), std::bind(&Player::UseSeed, this)));
		mTimers.emplace(TimerId::SEED_SWITCH, Timer(sf::milliseconds(200)));
	}

	virtual sf::FloatRect GetLocalBoundsInternal() const override
	{
		return mAnimationPlayer.GetSprite().getLocalBounds();
	}

	virtual sf::FloatRect GetGlobalBoundsInternal() const override
	{
		return mAnimationPlayer.GetSprite().getGlobalBounds();
	}

	virtual const sf::Drawable& GetDrawable() const override
	{
		return mAnimationPlayer.GetSprite();
	}

	uint16_t GetDepth() const override { return LAYERS.at("main"); }
	std::string GetActiveTool() const { return mToolPicker.GetItem(); }
	std::string GetActiveSeed() const { return mSeedPicker.GetItem(); }

	void UseTool()
	{
		std::cout << mToolPicker.GetItem() << std::endl;
	}

	void UseSeed()
	{
		std::cout << mSeedPicker.GetItem() << std::endl;
	}

	void Input()
	{
		if (!mTimers[TimerId::TOOL_USE].IsActive())
		{
			// directions
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
			{
				mDirection.y = -1;
				mStatus = "up";
			}
			else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
			{
				mDirection.y = 1;
				mStatus = "down";
			}
			else
			{
				mDirection.y = 0;
			}

			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
			{
				mDirection.x = 1;
				mStatus = "right";
			}
			else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
			{
				mDirection.x = -1;
				mStatus = "left";
			}
			else
			{
				mDirection.x = 0;
			}

			// tool use
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space))
			{
				mTimers[TimerId::TOOL_USE].Start();
				mDirection = sf::Vector2f();
			}

			// change tool
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q) && !mTimers[TimerId::TOOL_SWITCH].IsActive())
			{
				mTimers[TimerId::TOOL_SWITCH].Start();
				mToolPicker.Next();
				NotifyToolChanged(mToolPicker.GetItem());
			}

			// seed use
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl))
			{
				mTimers[TimerId::SEED_USE].Start();
				mDirection = sf::Vector2f();
			}

			// change seed
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E) && !mTimers[TimerId::SEED_SWITCH].IsActive())
			{
				mTimers[TimerId::SEED_SWITCH].Start();
				mSeedPicker.Next();
				NotifySeedChanged(mSeedPicker.GetItem());
			}
		}
	}

	void GetStatus()
	{
		// idle
		if (mDirection.lengthSq() == 0)
		{
			mStatus = SplitAndGetElement(mStatus, '_', 0) + "_idle";
		}

		// tool use
		if (mTimers[TimerId::TOOL_USE].IsActive())
		{
			mStatus = SplitAndGetElement(mStatus, '_', 0) + "_" + mToolPicker.GetItem();
		}
	}

	void UpdateTimers(const sf::Time& timestamp)
	{
		for (auto& pair : mTimers) {
			pair.second.Update(timestamp);
			if (pair.second.IsFinished())
			{
				pair.second.Reset();
			}
		}
	}

	void Move(const sf::Time& timestamp)
	{
		if (mDirection.lengthSq() != 0)
		{
			mDirection = mDirection.normalized();
		}

		float lerpFactor = mSpeed * timestamp.asSeconds();

		mTargetPosition.x = mDirection.x * lerpFactor;
		mTargetPosition.y = mDirection.y * lerpFactor;
		
		mCurrentPosition.x += 0.1f * (mTargetPosition.x - mCurrentPosition.x);
		mCurrentPosition.y += 0.1f * (mTargetPosition.y - mCurrentPosition.y);

		MoveX(int(mTargetPosition.x));
		MoveY(int(mTargetPosition.y));
	}

	void Animate(const sf::Time& timestamp)
	{
		mAnimationPlayer.SetAnimationSequence(mStatus);
		mAnimationPlayer.Upate(timestamp);
	}

	void Update(const sf::Time& timestamp) override
	{
		Input();
		GetStatus();
		UpdateTimers(timestamp);
		Move(timestamp);
		Animate(timestamp);
	};

private:
	sf::Vector2f mDirection;
	float mSpeed;
	std::string mStatus;
	AnimationPlayer mAnimationPlayer;
	std::unordered_map<TimerId, Timer> mTimers;
	std::string mSelectedTool;
	ItemPicker<std::string> mToolPicker;
	ItemPicker<std::string> mSeedPicker;

	sf::Vector2f mCurrentPosition;
	sf::Vector2f mTargetPosition;
};