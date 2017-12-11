#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Network.hpp>
#include <SFML/System.hpp>
#include <SFML/OpenGL.hpp>
#include <SFML/Main.hpp>

using namespace sf;

class Bullet : public sf::CircleShape
{
private:
	float velocity;
	Vector2f direction;
	Clock clock;

public:
	Bullet(Clock);
	void setDirection(Vector2f);
	Vector2f getDirection();
	void addVelocity();
	void setVelocity(float);
	void moveBullet(float);
	bool isDead(float);
	~Bullet();
};

