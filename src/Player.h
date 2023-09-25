#pragma once

#include <iostream>

#include "Core/GameObject.h"
#include "Core/ResourceLocator.h"
#include "Core/Animation.h"
#include "Core/Support.h"

class Player : public Sprite
{
public:
	Player(ResourceLocator& locator, const sf::Vector2f& position)	
		: mAnimation(locator.GetAnimationManager().Get("character")),
		  mSpeed(200),
		  mStatus("down_idle")
	{
		mAnimation.SetOriginAnchor(sf::Vector2f(0.5f, 0.5f));
		mAnimation.SetAnimationSequence(mStatus);
	}

	sf::FloatRect GetLocalBounds() const override { return mAnimation.GetGlobalBounds(); }

	void Input()
	{
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
	}

	void GetStatus()
	{
		// idle
		if (mDirection.lengthSq() != 0)
		{
			mStatus = SplitAndGetElement(mStatus, '_', 0) + "_idle";
		}

		// tool usde
	}

	void Move(const sf::Time& timestamp)
	{
		if (mDirection.lengthSq() != 0)
		{
			mDirection = mDirection.normalized();
		}
		
		MoveX(mDirection.x * timestamp.asSeconds() * mSpeed);
		MoveY(mDirection.y * timestamp.asSeconds() * mSpeed);		
	}

	void Animate(const sf::Time& timestamp)
	{
		mAnimation.SetAnimationSequence(mStatus);
		mAnimation.Upate(timestamp);
	}

	void Update(const sf::Time& timestamp) override
	{
		Input();
		GetStatus();
		Move(timestamp);
		Animate(timestamp);
	};

	void Draw(sf::RenderWindow& window) override
	{
		window.draw(mAnimation.GetSprite(), GetTransform());		
	}

private:		
	sf::Vector2f mDirection;	
	float mSpeed;
	std::string mStatus;
	Animation mAnimation;
};