#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Network.hpp>
#include <SFML/System.hpp>
#include <SFML/OpenGL.hpp>
#include <SFML/Main.hpp>

using namespace sf;

class SpaceShip : public sf::CircleShape
{
private:
	float velocity;

public:
	SpaceShip(float);
	void setVelocity(float);
	void moveShip(float);
	~SpaceShip();
};



