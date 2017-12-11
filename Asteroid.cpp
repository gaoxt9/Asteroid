#include "Asteroid.h"

Asteroid::Asteroid()
{

}

void Asteroid::setVelocity(float newVel)
{
	this->velocity = newVel;
}

void Asteroid::moveAsteroid(float dt)
{
	this->setPosition(this->getPosition() + Vector2f(0, velocity * dt));
}

void Asteroid::setDirection(Vector2f newDirection)
{
	this->direction = newDirection;
}

Vector2f Asteroid::getDirection()
{
	return this->direction;
}

void Asteroid::moveAst(float dt)
{
	this->setPosition(this->getPosition() + this->direction * this->velocity * dt);
}

Asteroid::~Asteroid()
{
}
