#include "SpaceShip.h"

SpaceShip::SpaceShip(float r)
{
	this->setRadius(r);
}

void SpaceShip::setVelocity(float newVel)
{
	this->velocity = newVel;
}

void SpaceShip::moveShip(float dt)
{
	this->setPosition(this->getPosition() + Vector2f(0, velocity * dt));
}

SpaceShip::~SpaceShip()
{

}
