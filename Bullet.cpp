#include "Bullet.h"

Bullet::Bullet(Clock bulletClock)
{
	this->clock = bulletClock;
}

void Bullet::setDirection(Vector2f newDirection)
{
	this->direction = newDirection;
}

Vector2f Bullet::getDirection()
{
	return this->direction;
}

void Bullet::setVelocity(float velocity)
{
	this->velocity = velocity;
}

void Bullet::addVelocity()
{
	this->velocity += 30.f;
}

void Bullet::moveBullet(float dt)
{
	this->setPosition(this->getPosition() + this->direction * this->velocity * dt);
}

bool Bullet::isDead(float dt)
{
	return this->clock.getElapsedTime().asSeconds() >= 3;
}

Bullet::~Bullet()
{

}
