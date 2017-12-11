#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Network.hpp>
#include <SFML/System.hpp>
#include <SFML/OpenGL.hpp>
#include <SFML/Main.hpp>

using namespace sf;

class Asteroid : public CircleShape
{
private:
	float velocity;
	Vector2f direction;
	float radius;
public:
	Asteroid();
	void setVelocity(float);
	void moveAsteroid(float);
	void setDirection(Vector2f);
	Vector2f getDirection();
	void moveAst(float);
	~Asteroid();
};

