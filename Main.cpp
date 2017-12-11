#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Network.hpp>
#include <SFML/System.hpp>
#include <SFML/OpenGL.hpp>
#include <SFML/Main.hpp>

#include <ctime>
#include <vector>
#include <memory>

#include "SpaceShip.h"
#include "Bullet.h"
#include "Asteroid.h"

using namespace sf;

int GameState = 0;
bool isPaused = false;

const int GAMEWIDTH = 2880;
const int GAMEHEIGHT = 1800;

const float PI = 3.1415926;

Vector2f bulletSize(150, 30);
float bulletVelocity = 500.f;

float shipRadius = 55.f;
float bulletRadius = 15.f;

String shiptDirState;
SpaceShip ship(shipRadius);
float shipVelocity = 500.f;
float tempShipVelocity = 0.f;
float speedInterval = 4.f;

std::vector<std::unique_ptr<Asteroid>> manyAsteroids;
std::vector<std::unique_ptr<Bullet>> manyBullets;
std::vector<Asteroid *> asteroidsAdded;
std::vector<std::vector<std::unique_ptr<Asteroid>>> grid;

float astroidVelocity = 250.f;
float sAstRadius = 35.f;
float mAstRadius = 55.f;
float bAstRadius = 85.f;

int score = 0, life = 3, level = 1;
int flashTimer = 100;
Color shellColor(239, 244, 248, 50);

RenderWindow window(VideoMode(GAMEWIDTH, GAMEHEIGHT), "Max's Asteroid!");
Text lifeTxt, scoreTxt, restartTxt, menutext, levelText;
Texture texture, pushTexture, astTexture, bulletTexture, explosion;
Sprite background, shipPush;
Texture shipSprite;
SoundBuffer buf1, buf2, buf3, buf4, buf5;
Sound shootSound, driftSound, explodeSound, crashSound, winSound;
Music backgroundMusic;

bool isInvincible = true;
int invincibleTime = 10;

void setControl(float);
void render_frame();
void render_menu();
void render_pause();
void update_state(float);
void shoot();
void create_ast();
void render_death();
void ast_get_hit(Asteroid *, int);
void make_it_invincible();
int checkGrid(Vector2f);
void collision_check(CircleShape *, CircleShape *);
bool is_collided(CircleShape *, CircleShape *);
void ast_bounce(Asteroid *, Asteroid *);
void ck_optimize();
void restart();
void respawn();
void levelUp();

class Animation
{
public:
	float Frame, speed;
	Sprite sprite;
	std::vector<IntRect> frames;

	Animation() {}

	Animation(Texture &t, int x, int y, int w, int h, int count, float Speed)
	{
		Frame = 0;
		speed = Speed;

		for (int i = 0; i<count; i++)
			frames.push_back(IntRect(x + i*w, y, w, h));

		sprite.setTexture(t);
		sprite.setOrigin(w / 2, h / 2);
		sprite.setTextureRect(frames[0]);
	}


	void update()
	{
		Frame += speed;
		int n = frames.size();
		if (Frame >= n) Frame -= n;
		if (n>0) sprite.setTextureRect(frames[int(Frame)]);
	}

	bool isEnd()
	{
		return Frame + speed >= frames.size();
	}

};

std::vector<Animation *> allExplosion;

int main()
{
	std::srand(std::time(0));

	Font font;

	font.loadFromFile("arial.TTF");
	texture.loadFromFile("background.jpg");

	pushTexture.loadFromFile("shipPush.png");
	shipPush.setTexture(pushTexture);

	astTexture.loadFromFile("Asteroid.png");
	bulletTexture.loadFromFile("Fireball.png");
	explosion.loadFromFile("explosion.png");

	background.setScale(Vector2f(1.5, 1.5));
	background.setTexture(texture);
	
	lifeTxt.setFont(font);
	lifeTxt.setCharacterSize(50);
	lifeTxt.setFillColor(sf::Color::Red);
	lifeTxt.setStyle(Text::Bold);
	lifeTxt.setPosition(20, GAMEHEIGHT - 80);

	levelText.setFont(font);
	levelText.setCharacterSize(50);
	levelText.setFillColor(sf::Color::Red);
	levelText.setStyle(Text::Bold);
	levelText.setPosition(20, 80);

	scoreTxt.setFont(font);
	scoreTxt.setCharacterSize(50);
	scoreTxt.setFillColor(sf::Color::Red);
	scoreTxt.setStyle(Text::Bold);
	scoreTxt.setPosition(GAMEWIDTH - 250, GAMEHEIGHT - 80);

	restartTxt.setFont(font);
	restartTxt.setCharacterSize(50);
	restartTxt.setFillColor(sf::Color::Red);
	restartTxt.setStyle(Text::Bold);
	restartTxt.setPosition(GAMEWIDTH / 3.5, GAMEHEIGHT / 2 - 50);

	menutext.setFont(font);
	menutext.setCharacterSize(50);
	menutext.setFillColor(sf::Color::Red);
	menutext.setStyle(Text::Bold);
	menutext.setPosition(GAMEWIDTH / 3.5, GAMEHEIGHT / 2 - 50);

	ship.setPosition(Vector2f(GAMEWIDTH/2, GAMEHEIGHT /2 ));
	ship.setOrigin(Vector2f(shipRadius, shipRadius));
	shipSprite.loadFromFile("ship.png");
	ship.setTexture(&shipSprite);

	buf1.loadFromFile("shoot.wav");
	buf2.loadFromFile("drifting.wav");
	buf3.loadFromFile("explode.wav");
	buf4.loadFromFile("crash.wav");
	buf5.loadFromFile("win.wav");

	shootSound.setBuffer(buf1);
	driftSound.setBuffer(buf2);
	explodeSound.setBuffer(buf3);
	crashSound.setBuffer(buf4);
	winSound.setBuffer(buf5);

	backgroundMusic.openFromFile("background.flac");
	backgroundMusic.setVolume(30);
	backgroundMusic.play();
	backgroundMusic.setLoop(true);

	Clock clock, countClock;

	create_ast();

	while (window.isOpen())
	{
		Event event;
		while (window.pollEvent(event))
		{
			if (event.type == Event::Closed)
				window.close();
		}

		float dt = clock.restart().asSeconds();

		switch (GameState)
		{
		case 0:
			render_menu();
			break;
		case 1:
			if (isPaused)
			{
				render_pause();
			}
			else
			{
				setControl(dt);
				update_state(dt);
				render_frame();
			}
			break;
		case 2:
			window.close();
			break;
		case 3:
			render_death();
			break;
		default:
			break;
		}
	}
	return 0;
}

void render_menu()
{
	window.clear();

	menutext.setString("Welcome, press \"Enter\" to start, or press \"P\" to exit");
	if (Keyboard::isKeyPressed(Keyboard::Return))
	{
		GameState = 1;
	}
	else if (Keyboard::isKeyPressed(Keyboard::P))
	{
		GameState = 2;
	}

	window.draw(menutext);
	window.display();
}

void render_pause()
{
	window.clear();

	menutext.setString("Press \"Enter\" to re-start, \"r\" to resume, or press \"P\" to exit");
	if (Keyboard::isKeyPressed(Keyboard::Return))
	{
		isPaused = false;
		restart();
	}
	else if (Keyboard::isKeyPressed(Keyboard::R))
	{
		isPaused = false;
	}
	else if (Keyboard::isKeyPressed(Keyboard::P))
	{
		GameState = 2;
	}

	window.draw(menutext);
	window.display();
}

void render_death()
{
	window.clear();
	restartTxt.setString("You scored " + std::to_string(score) + " points, press \"Enter\" to restart, or \"ESC\" to exit.");
	window.draw(restartTxt);
	window.display();

	if (Keyboard::isKeyPressed(Keyboard::Return))
	{
		restart();
	}
	else if (Keyboard::isKeyPressed(Keyboard::Escape))
	{
		GameState = 2;
	}
}

void render_frame()
{
	window.clear();
	window.draw(background);

	if (tempShipVelocity == shipVelocity)
	{
		window.draw(shipPush);
	}

	window.draw(ship);
	for (size_t i = 0; i < manyBullets.size(); ++i)
	{
		window.draw(*manyBullets[i]);
	}
	for (size_t i = 0; i < manyAsteroids.size(); i++)
	{
		window.draw(*manyAsteroids[i]);
	}

	for (size_t i = 0; i < allExplosion.size(); i++)
	{
		window.draw(allExplosion[i]->sprite);
	}

	window.draw(lifeTxt);
	window.draw(scoreTxt);
	window.draw(levelText);

	window.display();
}

void update_state(float dt)
{
	if (Keyboard::isKeyPressed(Keyboard::I))
	{
		make_it_invincible();
	}

	if (Keyboard::isKeyPressed(Keyboard::Escape))
	{
		isPaused = true;
	}

	lifeTxt.setString("Life: " + std::to_string(life));
	scoreTxt.setString("Score: " + std::to_string(score));
	levelText.setString("Level: " + std::to_string(level));

	if (life <= 0)
	{
		GameState = 3;
	}

	if (Keyboard::isKeyPressed(Keyboard::P))
	{
		ship.setPosition(Vector2f(GAMEWIDTH / 2, GAMEHEIGHT / 2));
		astroidVelocity += 100;
		manyAsteroids.clear();
		manyBullets.clear();
		create_ast();
	}

	float shipX = ship.getPosition().x;
	float shipY = ship.getPosition().y;
	Vector2f curPos = ship.getPosition();
	Vector2i mousePos = Mouse::getPosition(window);

	// flip ship
	if (shipY + shipRadius <= 0 && Keyboard::isKeyPressed(Keyboard::W))
	{
		ship.setPosition(Vector2f(shipX, GAMEHEIGHT - shipRadius));
	}
	else if (shipY + shipRadius >= GAMEHEIGHT && Keyboard::isKeyPressed(Keyboard::S))
	{
		ship.setPosition(Vector2f(shipX, -shipRadius));
	}
	else if (shipX + shipRadius <= 0 && Keyboard::isKeyPressed(Keyboard::A))
	{
		ship.setPosition(Vector2f(GAMEWIDTH - shipRadius, shipY));
	}
	else if (shipX + shipRadius >= GAMEWIDTH && Keyboard::isKeyPressed(Keyboard::D))
	{
		ship.setPosition(Vector2f(-shipRadius, shipY));
	}
	
	float rotation = atan2(mousePos.y - curPos.y, mousePos.x - curPos.x) * 180 / PI;
	
	float aaa = (rotation + 90) * PI / 180;
	ship.setRotation(rotation + 90);
	shipPush.setOrigin(Vector2f(35, 10));
	shipPush.setPosition(Vector2f(shipX - shipRadius * sin(aaa), shipY + shipRadius * cos(aaa)));
	shipPush.setRotation(rotation + 90);

	// bullets path
	for (int i = manyBullets.size() - 1; i >= 0 ; i--)
	{
		float bulletX = manyBullets[i]->getPosition().x;
		float bulletY = manyBullets[i]->getPosition().y;

		if (manyBullets[i]->isDead(dt))
		{
			manyBullets.erase(manyBullets.begin() + i);
			continue;
		}
		manyBullets[i]->moveBullet(dt);

		if (bulletY + bulletRadius <= 0)
		{
			manyBullets[i]->setPosition(Vector2f(bulletX, GAMEHEIGHT - bulletRadius - 1));
		}
		else if (bulletY + bulletRadius >= GAMEHEIGHT)
		{
			manyBullets[i]->setPosition(Vector2f(bulletX, -bulletRadius + 1));
		}
		else if (bulletX + bulletRadius <= 0)
		{
			manyBullets[i]->setPosition(Vector2f(GAMEWIDTH - bulletRadius - 1, bulletY));
		}
		else if (bulletX + bulletRadius >= GAMEWIDTH)
		{
			manyBullets[i]->setPosition(Vector2f(-bulletRadius + 1, bulletY));
		}
	}

	// astroid path
	for (int i = 0; i < manyAsteroids.size(); i++)
	{
		float astX = manyAsteroids[i]->getPosition().x;
		float astY = manyAsteroids[i]->getPosition().y;
		float astRadius = manyAsteroids[i]->getRadius();

		if (astY + astRadius <= 0)
		{
			manyAsteroids[i]->setPosition(Vector2f(astX, GAMEHEIGHT - astRadius - 1));
		}
		else if (astY + astRadius >= GAMEHEIGHT)
		{
			manyAsteroids[i]->setPosition(Vector2f(astX, -astRadius + 1));
		}
		else if (astX + astRadius <= 0)
		{
			manyAsteroids[i]->setPosition(Vector2f(GAMEWIDTH - astRadius - 1, astY));
		}
		else if (astX + astRadius >= GAMEWIDTH)
		{
			manyAsteroids[i]->setPosition(Vector2f(-astRadius + 1, astY));
		}
		manyAsteroids[i]->moveAst(dt);
	}

	ck_optimize();

	if (manyAsteroids.size() == 0)
	{
		levelUp();
	}

	if (allExplosion.size() > 0)
	{
		for (int i = 0; i < allExplosion.size(); i++)
		{
			if (allExplosion[i]->isEnd())
			{
				allExplosion.erase(allExplosion.begin() + i);
				continue;
			}
			allExplosion[i]->update();
		}
	}
}

void setControl(float dt)
{
	if (Keyboard::isKeyPressed(Keyboard::A))
	{
		driftSound.play();
		shiptDirState = "A";
		tempShipVelocity = shipVelocity;
		if (Keyboard::isKeyPressed(Keyboard::W))
		{
			shiptDirState = "AW";
			ship.move(-shipVelocity * dt / 2, -shipVelocity * dt / 2);
		}
		else if (Keyboard::isKeyPressed(Keyboard::S))
		{
			shiptDirState = "AS";
			ship.move(-shipVelocity * dt / 2, shipVelocity * dt / 2);
		}
		else
		{
			ship.move(-shipVelocity * dt, 0.f);
		}
	}
	else if (Keyboard::isKeyPressed(Keyboard::D))
	{
		driftSound.play();
		shiptDirState = "D";
		tempShipVelocity = shipVelocity;
		if (Keyboard::isKeyPressed(Keyboard::S))
		{
			shiptDirState = "DS";
			ship.move(shipVelocity * dt / 2, shipVelocity * dt / 2);
		}
		else if (Keyboard::isKeyPressed(Keyboard::W))
		{
			shiptDirState = "DW";
			ship.move(shipVelocity * dt / 2, -shipVelocity * dt / 2);
		}
		else
		{
			ship.move(shipVelocity * dt, 0.f);
		}
	}
	else if (Keyboard::isKeyPressed(Keyboard::W))
	{
		driftSound.play();
		shiptDirState = "W";
		tempShipVelocity = shipVelocity;
		
		if (Keyboard::isKeyPressed(Keyboard::A))
		{
			shiptDirState = "WA";
			ship.move(-shipVelocity * dt / 2, -shipVelocity * dt / 2);
		}
		else if (Keyboard::isKeyPressed(Keyboard::D))
		{
			shiptDirState = "WD";
			ship.move(shipVelocity * dt / 2, -shipVelocity * dt / 2);
		}
		else
		{
			ship.move(0.f, -shipVelocity * dt);
		}
	}
	else if (Keyboard::isKeyPressed(Keyboard::S))
	{
		driftSound.play();
		shiptDirState = "S";
		tempShipVelocity = shipVelocity;

		if (Keyboard::isKeyPressed(Keyboard::A))
		{
			shiptDirState = "SA";
			ship.move(-shipVelocity * dt / 2, shipVelocity * dt / 2);
		}
		else if (Keyboard::isKeyPressed(Keyboard::D))
		{
			shiptDirState = "SD";
			ship.move(shipVelocity * dt / 2, shipVelocity * dt / 2);
		}
		else
		{
			ship.move(0.f, shipVelocity * dt);
		}
	}
	else
	{
		driftSound.stop();
		if (shiptDirState == "A")
		{
			if (tempShipVelocity - speedInterval < 0)
			{
				tempShipVelocity = 0;
			}
			tempShipVelocity -= speedInterval;
			ship.move(-tempShipVelocity * dt, 0.f);
		}
		else if (shiptDirState == "AW")
		{
			if (tempShipVelocity - speedInterval < 0)
			{
				tempShipVelocity = 0;
			}
			tempShipVelocity -= speedInterval;
			ship.move(-tempShipVelocity * dt / 2, -tempShipVelocity * dt / 2);
		}
		else if (shiptDirState == "AS")
		{
			if (tempShipVelocity - speedInterval < 0)
			{
				tempShipVelocity = 0;
			}
			tempShipVelocity -= speedInterval;
			ship.move(-tempShipVelocity * dt / 2, tempShipVelocity * dt / 2);
		}
		else if (shiptDirState == "D")
		{
			if (tempShipVelocity - speedInterval < 0)
			{
				tempShipVelocity = 0;
			}
			tempShipVelocity -= speedInterval;
			ship.move(tempShipVelocity * dt, 0.f);	
		}
		else if (shiptDirState == "DS")
		{
			if (tempShipVelocity - speedInterval < 0)
			{
				tempShipVelocity = 0;
			}
			tempShipVelocity -= speedInterval;
			ship.move(tempShipVelocity * dt / 2, tempShipVelocity * dt / 2);
		}
		else if (shiptDirState == "DW")
		{
			if (tempShipVelocity - speedInterval < 0)
			{
				tempShipVelocity = 0;
			}
			tempShipVelocity -= speedInterval;
			ship.move(tempShipVelocity * dt / 2, -tempShipVelocity * dt / 2);
		}
		else if (shiptDirState == "W")
		{
			if (tempShipVelocity - speedInterval < 0)
			{
				tempShipVelocity = 0;
			}
			tempShipVelocity -= speedInterval;
			ship.move(0.f, -tempShipVelocity * dt);
		}
		else if (shiptDirState == "WA")
		{
			if (tempShipVelocity - speedInterval < 0)
			{
				tempShipVelocity = 0;
			}
			tempShipVelocity -= speedInterval;
			ship.move(-tempShipVelocity * dt / 2, -tempShipVelocity * dt / 2);
		}
		else if (shiptDirState == "WD")
		{
			if (tempShipVelocity - speedInterval < 0)
			{
				tempShipVelocity = 0;
			}
			tempShipVelocity -= speedInterval;
			ship.move(tempShipVelocity * dt / 2, -tempShipVelocity * dt / 2);
		}
		else if (shiptDirState == "S")
		{
			if (tempShipVelocity - speedInterval < 0)
			{
				tempShipVelocity = 0;
			}
			tempShipVelocity -= speedInterval;
			ship.move(0.f, tempShipVelocity * dt);
		}
		else if (shiptDirState == "SA")
		{
			if (tempShipVelocity - speedInterval < 0)
			{
				tempShipVelocity = 0;
			}
			tempShipVelocity -= speedInterval;
			ship.move(-tempShipVelocity * dt / 2, tempShipVelocity * dt / 2);
		}
		else if (shiptDirState == "SD")
		{
			if (tempShipVelocity - speedInterval < 0)
			{
				tempShipVelocity = 0;
			}
			tempShipVelocity -= speedInterval;
			ship.move(tempShipVelocity * dt / 2, tempShipVelocity * dt / 2);
		}
		else
		{
			shiptDirState = "";
		}
	}

	if (Mouse::isButtonPressed(Mouse::Left))
	{
		shoot();
	}
}

void shoot()
{
	shootSound.play();
	Clock bulletClock;

	Bullet *oneBullet = new Bullet(bulletClock);

	float rotation = ship.getRotation() * PI / 180;

	oneBullet->setRadius(bulletRadius);
	oneBullet->setTexture(&bulletTexture);
	//oneBullet->setFillColor(Color(100, 100, 200));
	oneBullet->setOrigin(Vector2f(bulletRadius, bulletRadius));
	oneBullet->setPosition(ship.getPosition().x + 80 * sin(rotation), ship.getPosition().y - 80 * cos(rotation));
	oneBullet->setDirection(Vector2f(sin((rotation)), -cos((rotation))));
	oneBullet->setVelocity(bulletVelocity); 

	manyBullets.push_back(move(std::unique_ptr<Bullet>(oneBullet)));
}

void create_ast()
{
	for (size_t i = 0; i < 12; i++)
	{
		int randomNum = std::rand();
		int thisRadius = randomNum % 3;

		Asteroid *oneAsteroid = new Asteroid();

		switch (thisRadius)
		{
		case 0:
			oneAsteroid->setRadius(sAstRadius);
			oneAsteroid->setOrigin(Vector2f(sAstRadius, sAstRadius));
			break;
		case 1:
			oneAsteroid->setRadius(mAstRadius);
			oneAsteroid->setOrigin(Vector2f(sAstRadius, sAstRadius));
			break;
		case 2:
			oneAsteroid->setRadius(bAstRadius);
			oneAsteroid->setOrigin(Vector2f(sAstRadius, sAstRadius));
			break;
		default:
			break;
		}
		
		oneAsteroid->setTexture(&astTexture);
		oneAsteroid->setVelocity(astroidVelocity);

		if (i<3)
		{
			oneAsteroid->setDirection(Vector2f(sin(randomNum), cos(randomNum)));
			oneAsteroid->setPosition(randomNum % GAMEWIDTH, 1);
		}
		else if (i >= 3 && i < 7)
		{
			oneAsteroid->setDirection(Vector2f(sin(randomNum), cos(randomNum)));
			oneAsteroid->setPosition(randomNum % GAMEWIDTH, GAMEHEIGHT - 1);
		}
		else if (i > 7 && i < 10)
		{
			oneAsteroid->setDirection(Vector2f(sin(randomNum), cos(randomNum)));
			oneAsteroid->setPosition(1, randomNum % GAMEHEIGHT);
		}
		else
		{
			oneAsteroid->setDirection(Vector2f(sin(randomNum), cos(randomNum)));
			oneAsteroid->setPosition(GAMEWIDTH - 1, randomNum % GAMEHEIGHT);
		}

		//grid[checkGrid(oneAsteroid->getPosition())].push_back(move(std::unique_ptr<Asteroid>(oneAsteroid)));
		manyAsteroids.push_back(move(std::unique_ptr<Asteroid>(oneAsteroid)));
	}
}

void ast_get_hit(Asteroid *theAst, int index)
{
	Animation *playAnim = new Animation(explosion, 0, 0, 192, 192, 64, 0.6);
	playAnim->sprite.setPosition(theAst->getPosition());
	allExplosion.push_back(playAnim);
	explodeSound.play();

	if (theAst->getRadius() == bAstRadius)
	{
		theAst->setRadius(mAstRadius);
		theAst->setDirection(Vector2f(1, 1));
		theAst->setVelocity(astroidVelocity);
		float theAstX = theAst->getPosition().x;
		float theAstY = theAst->getPosition().y;

		Asteroid *dividedAsteroid = new Asteroid();
		dividedAsteroid->setRadius(mAstRadius);
		dividedAsteroid->setTexture(&astTexture);
		dividedAsteroid->setVelocity(astroidVelocity);
		dividedAsteroid->setDirection(Vector2f(1, 0));
		dividedAsteroid->setPosition(theAstX, theAstY);

		asteroidsAdded.push_back(dividedAsteroid);
	}
	else if (theAst->getRadius() == mAstRadius)
	{
		theAst->setRadius(sAstRadius);
		theAst->setDirection(Vector2f(1, 1));
		theAst->setVelocity(astroidVelocity);
		float theAstX = theAst->getPosition().x;
		float theAstY = theAst->getPosition().y;

		Asteroid *dividedAsteroid = new Asteroid();
		dividedAsteroid->setRadius(sAstRadius);
		dividedAsteroid->setTexture(&astTexture);
		dividedAsteroid->setVelocity(astroidVelocity);
		dividedAsteroid->setDirection(Vector2f(1, 0));
		dividedAsteroid->setPosition(theAstX, theAstY);

		asteroidsAdded.push_back(dividedAsteroid);
	}
	else if (theAst->getRadius() == sAstRadius)
	{
		manyAsteroids.erase(manyAsteroids.begin() + index);
		score++;
	}
}

void ast_bounce (Asteroid *ast1, Asteroid *ast2)
{
	Vector2f aaa = ast1->getPosition() - ast2->getPosition();
	aaa = aaa / sqrt(pow(aaa.x, 2) + pow(aaa.y, 2));
	ast1->setDirection(aaa);
	ast2->setDirection(-aaa);
}

void make_it_invincible()
{
	if (flashTimer == 0)
	{
		ship.setTexture(&shipSprite);
		flashTimer = 100;
	}
	else if (flashTimer % 2 == 0)
	{
		ship.setFillColor(shellColor);
		flashTimer--;
	}
	else
	{
		ship.setFillColor(shellColor);
		flashTimer--;
	}
}

int checkGrid(Vector2f ptPos)
{
	int xUnit = GAMEWIDTH / 3;
	int yUnit = GAMEHEIGHT / 3;

	int col = ptPos.x / xUnit;
	int row = ptPos.y / yUnit;

	if (col >= 3)
	{
		col = 2;
	}

	if (col <= 0)
	{
		col = 0;
	}

	if (row >= 3)
	{
		row = 2;
	}

	if (row <= 0)
	{
		row = 0;
	}

	return (row + 1) + col * 3 - 1;
}

void collision_check(CircleShape *obj1, CircleShape *obj2)
{
	Vector2f onePos = obj1->getPosition();
	Vector2f twoPos = obj2->getPosition();

	float distance = sqrt(pow(twoPos.x-onePos.x, 2) + pow(twoPos.y - onePos.y, 2));

	if (is_collided(obj1, obj2))
	{
		if (dynamic_cast<SpaceShip *>(obj1) != nullptr || dynamic_cast<SpaceShip *>(obj2) != nullptr)
		{
			life--;
		}
		else if (dynamic_cast<Bullet *>(obj1) != nullptr && dynamic_cast<Asteroid *>(obj2) != nullptr)
		{
			Bullet * theBullet = dynamic_cast<Bullet *>(obj1);
			Asteroid * theAsteroid = dynamic_cast<Asteroid *>(obj2);
			for (size_t i = manyBullets.size() - 1; i >=0 ; i--)
			{
				if (manyBullets[i].get() == theBullet)
				{
					//manyBullets.erase(manyBullets.begin() + i);
					break;
				}
			}
			for (size_t i = manyAsteroids.size() - 1; i >= 0; i--)
			{
				if (manyAsteroids[i].get() == theAsteroid)
				{
					for (size_t j = grid[checkGrid(theAsteroid->getPosition())].size() - 1; j >= 0; j--)
					{
						if (grid[checkGrid(theAsteroid->getPosition())][j].get() == theAsteroid)
						{
							//ast_get_hit(theAsteroid, i);
							break;
						}
					}
					break;
				}
			}
		}
		else if (dynamic_cast<Asteroid *>(obj1) != nullptr && dynamic_cast<Bullet *>(obj2) != nullptr)
		{
			Bullet * theBullet = dynamic_cast<Bullet *>(obj2);
			Asteroid * theAsteroid = dynamic_cast<Asteroid *>(obj1);
			for (size_t i = manyBullets.size() - 1; i >= 0; i--)
			{
				if (manyBullets[i].get() == theBullet)
				{
					manyBullets.erase(manyBullets.begin() + i);
					break;
				}
			}
			for (size_t i = manyAsteroids.size() - 1; i >= 0; i--)
			{
				if (manyAsteroids[i].get() == theAsteroid)
				{
					for (size_t j = grid[checkGrid(theAsteroid->getPosition())].size() - 1; j >= 0; j--)
					{
						if (grid[checkGrid(theAsteroid->getPosition())][j].get() == theAsteroid)
						{
							grid[checkGrid(theAsteroid->getPosition())].erase(grid[checkGrid(theAsteroid->getPosition())].begin() + j);
							break;
						}
					}
					//manyAsteroids.erase(manyAsteroids.begin() + i);
					break;
				}
			}
		}
		else if (dynamic_cast<Asteroid *>(obj1) != nullptr && dynamic_cast<Asteroid *>(obj2) != nullptr)
		{
			ast_bounce(dynamic_cast<Asteroid *>(obj1), dynamic_cast<Asteroid *>(obj2));
		}
	}
}

void ck_optimize()
{
	for (int i = manyAsteroids.size() - 1; i >= 0; i--)
	{
		if (is_collided(manyAsteroids[i].get(), &ship))
		{
			Animation *playAnim = new Animation(explosion, 0, 0, 192, 192, 64, 0.6);
			playAnim->sprite.setPosition(ship.getPosition());
			allExplosion.push_back(playAnim);
			crashSound.play();

			life--;
			respawn();
			break;
		}

		for (int k = manyAsteroids.size() - 1; k >= 0; k--)
		{
			if (is_collided(manyAsteroids[i].get(), manyAsteroids[k].get()))
			{
				ast_bounce(manyAsteroids[i].get(), manyAsteroids[k].get());
			}
		}

		if (manyBullets.size() != 0)
		{
			for (int j = manyBullets.size() - 1; j >= 0 ; j--)
			{
				if (is_collided(manyAsteroids[i].get(), manyBullets[j].get()))
				{
					manyBullets.erase(manyBullets.begin() + j);
					ast_get_hit(manyAsteroids[i].get(), i);
					break;
				}
			}
		}
	}

	if (asteroidsAdded.size() > 0)
	{
		manyAsteroids.insert(manyAsteroids.end(), asteroidsAdded.begin(), asteroidsAdded.end());
		asteroidsAdded.clear();
	}
}

bool is_collided(CircleShape *obj1, CircleShape *obj2)
{
	if (obj1 == obj2)
	{
		return false;
	}

	Vector2f onePos = obj1->getPosition();
	Vector2f twoPos = obj2->getPosition();

	float distance = sqrt(pow(twoPos.x - onePos.x, 2) + pow(twoPos.y - onePos.y, 2));

	return distance <= (obj1->getRadius() + obj2->getRadius());
}

void restart() 
{
	astroidVelocity = 250;
	level = 1;
	score = 0;
	GameState = 1;
	life = 3;
	manyBullets.clear();
	manyAsteroids.clear();
	allExplosion.clear();
	create_ast();
	ship.setPosition(Vector2f(GAMEWIDTH / 2, GAMEHEIGHT / 2));
}

void respawn()
{
	manyBullets.clear();
	ship.setPosition(Vector2f(GAMEWIDTH / 2, GAMEHEIGHT / 2));
}

void levelUp()
{
	winSound.play();
	level++;
	manyBullets.clear();
	manyAsteroids.clear();
	allExplosion.clear();
	astroidVelocity += 50;
	create_ast();
	ship.setPosition(Vector2f(GAMEWIDTH / 2, GAMEHEIGHT / 2));
}
