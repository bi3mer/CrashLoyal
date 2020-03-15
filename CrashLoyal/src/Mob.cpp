#include "Mob.h"

#include <memory>
#include <limits>
#include <stdlib.h>
#include <stdio.h>
#include "Building.h"
#include "Waypoint.h"
#include "GameState.h"
#include "Point.h"

int Mob::previousUUID;

Mob::Mob() 
	: pos(-10000.f,-10000.f)
	, nextWaypoint(NULL)
	, targetPosition(new Point)
	, state(MobState::Moving)
	, uuid(Mob::previousUUID + 1)
	, attackingNorth(true)
	, health(-1)
	, targetLocked(false)
	, target(NULL)
	, lastAttackTime(0)
{
	Mob::previousUUID += 1;
}

void Mob::Init(const Point& pos, bool attackingNorth)
{
	health = GetMaxHealth();
	this->pos = pos;
	this->attackingNorth = attackingNorth;
	findClosestWaypoint();
}

std::shared_ptr<Point> Mob::getPosition() {
	return std::make_shared<Point>(this->pos);
}

bool Mob::findClosestWaypoint() {
	std::shared_ptr<Waypoint> closestWP = GameState::waypoints[0];
	float smallestDist = std::numeric_limits<float>::max();

	for (std::shared_ptr<Waypoint> wp : GameState::waypoints) {
		//std::shared_ptr<Waypoint> wp = GameState::waypoints[i];
		// Filter out any waypoints that are "behind" us (behind is relative to attack dir
		// Remember y=0 is in the top left
		if (attackingNorth && wp->pos.y > this->pos.y) {
			continue;
		}
		else if ((!attackingNorth) && wp->pos.y < this->pos.y) {
			continue;
		}

		float dist = this->pos.dist(wp->pos);
		if (dist < smallestDist) {
			smallestDist = dist;
			closestWP = wp;
		}
	}
	std::shared_ptr<Point> newTarget = std::shared_ptr<Point>(new Point);
	this->targetPosition->x = closestWP->pos.x;
	this->targetPosition->y = closestWP->pos.y;
	this->nextWaypoint = closestWP;
	
	return true;
}

void Mob::moveTowards(std::shared_ptr<Point> moveTarget, double elapsedTime) {
	Point movementVector;
	movementVector.x = moveTarget->x - this->pos.x;
	movementVector.y = moveTarget->y - this->pos.y;
	movementVector.normalize();

	if (mobCollisionPoint.isZero() == false)
	{
		this->updateMovementVector(&movementVector, &mobCollisionPoint, 2.0);
	}

	if (buildingCollisionPoint.isZero() == false)
	{
		this->updateMovementVector(&movementVector, &buildingCollisionPoint, 0.01);
	}

	if (riverCollisionPoint.isZero() == false)
	{
		this->updateMovementVector(&movementVector, &riverCollisionPoint, 0.01);
	}

	movementVector *= (float)this->GetSpeed();
	movementVector *= (float)elapsedTime;
	pos += movementVector;
}


void Mob::findNewTarget() {
	// Find a new valid target to move towards and update this mob
	// to start pathing towards it

	if (!findAndSetAttackableMob()) { findClosestWaypoint(); }
}

// Have this mob start aiming towards the provided target
// TODO: impliment true pathfinding here
void Mob::updateMoveTarget(std::shared_ptr<Point> target) {
	this->targetPosition->x = target->x;
	this->targetPosition->y = target->y;
}

void Mob::updateMoveTarget(Point target) {
	this->targetPosition->x = target.x;
	this->targetPosition->y = target.y;
}


// Movement related
//////////////////////////////////
// Combat related

int Mob::attack(int dmg) {
	this->health -= dmg;
	return health;
}

bool Mob::findAndSetAttackableMob() {
	// Find an attackable target that's in the same quardrant as this Mob
	// If a target is found, this function returns true
	// If a target is found then this Mob is updated to start attacking it
	for (std::shared_ptr<Mob> otherMob : GameState::mobs) {
		if (otherMob->attackingNorth == this->attackingNorth) { continue; }

		bool imLeft    = this->pos.x     < (SCREEN_WIDTH / 2);
		bool otherLeft = otherMob->pos.x < (SCREEN_WIDTH / 2);

		bool imTop    = this->pos.y     < (SCREEN_HEIGHT / 2);
		bool otherTop = otherMob->pos.y < (SCREEN_HEIGHT / 2);
		if ((imLeft == otherLeft) && (imTop == otherTop)) {
			// If we're in the same quardrant as the otherMob
			// Mark it as the new target
			this->setAttackTarget(otherMob);
			return true;
		}
	}
	return false;
}

// TODO Move this somewhere better like a utility class
int randomNumber(int minValue, int maxValue) {
	// Returns a random number between [min, max). Min is inclusive, max is not.
	return (rand() % maxValue) + minValue;
}

void Mob::setAttackTarget(std::shared_ptr<Attackable> newTarget) {
	this->state = MobState::Attacking;
	target = newTarget;
}

bool Mob::targetInRange() {
	float range = this->GetSize(); // TODO: change this for ranged units
	float totalSize = range + target->GetSize();
	return this->pos.insideOf(*(target->getPosition()), totalSize);
}
// Combat related
////////////////////////////////////////////////////////////
// Collisions

bool Mob::isCollision(float x2, float y2, float s2)
{	
	float x1 = pos.x;
	float y1 = pos.y;
	float s1 = this->GetSize();

	return abs(x1 - x2) < s1 + s2 && abs(y1 - y2) < s1 + s2;
}

std::vector<ObjectData> Mob::checkCollision() 
{
	std::vector<ObjectData> objects;
	for (std::shared_ptr<Mob> otherMob : GameState::mobs) 
	{
		// don't collide with yourself
		if (this->sameMob(otherMob)) { continue; }

		if (isCollision(otherMob->pos.x, otherMob->pos.y, otherMob->GetSize()))
		{
			ObjectData obj;
			obj.pos = otherMob->pos;
			obj.size = otherMob->GetSize();
			obj.weight = 1.0;
			objects.push_back(obj);
		}
	}

	for (std::shared_ptr<Building> building : GameState::buildings)
	{
		Point p = building->getPoint();
		int size = building->GetSize();

		if (isCollision(p.x, p.y, size))
		{
			ObjectData obj;
			obj.pos = p;
			obj.size = size;
			obj.weight = 1000.0;
		}
	}

	// @TODO: river

	return objects;
}

Point Mob::processCollision(std::vector<ObjectData> objects) 
{
	Point target(0,0);

	for (ObjectData obj : objects)
	{
		Point p(this->pos.x - obj.pos.x, this->pos.y - obj.pos.y);
		p.normalize();

		target.x += p.x * obj.weight;
		target.y += p.y * obj.weight;
	}

	if (target.isZero() == false)
	{
		target.normalize();
		target.x += this->pos.x;
		target.y += this->pos.y;
	}

	return target;
}

void Mob::handleCollisions()
{
	// mobs
	std::vector<ObjectData> objects;
	for (std::shared_ptr<Mob> otherMob : GameState::mobs)
	{
		// don't collide with yourself
		if (this->sameMob(otherMob)) { continue; }

		if (isCollision(otherMob->pos.x, otherMob->pos.y, otherMob->GetSize()))
		{
			ObjectData obj;
			obj.pos = otherMob->pos;
			obj.size = otherMob->GetSize();
			obj.weight = 1.0;
			objects.push_back(obj);
		}
	}

	mobCollisionPoint = processCollision(objects);

	// buildings
	objects.clear();
	for (std::shared_ptr<Building> building : GameState::buildings)
	{
		Point p = building->getPoint();
		int size = building->GetSize() / 1.7; // gave best looking result

		if (isCollision(p.x, p.y, size))
		{
			ObjectData obj;
			obj.pos = p;
			obj.size = size;
			obj.weight = 1;

			objects.push_back(obj);
		}
	}

	buildingCollisionPoint = processCollision(objects);

	// @TODO: river
}

// Collisions
///////////////////////////////////////////////
// Procedures

void Mob::attackProcedure(double elapsedTime) {
	if (this->target == nullptr || this->target->isDead()) {
		this->targetLocked = false;
		this->target = nullptr;
		this->state = MobState::Moving;
		return;
	}

	if (targetInRange()) {
		if (this->lastAttackTime >= this->GetAttackTime()) {
			std::cout << "attacking!" << std::endl;
			// If our last attack was longer ago than our cooldown
			this->target->attack(this->GetDamage());
			this->lastAttackTime = 0; // lastAttackTime is incremented in the main update function
			return;
		}
	}
	else {
		// If the target is not in range
		moveTowards(target->getPosition(), elapsedTime);
	}
}

void Mob::moveProcedure(double elapsedTime) 
{
	if (targetPosition)
	{
		moveTowards(targetPosition, elapsedTime);

		// Check for collisions
		if (this->nextWaypoint->pos.insideOf(this->pos, (this->GetSize() + WAYPOINT_SIZE))) {
			std::shared_ptr<Waypoint> trueNextWP = this->attackingNorth ?
												   this->nextWaypoint->upNeighbor :
												   this->nextWaypoint->downNeighbor;
			setNewWaypoint(trueNextWP);
		}

		// Fighting otherMob takes priority always
		findAndSetAttackableMob();

	} else {
		// if targetPosition is nullptr
		findNewTarget();
	}
}

void Mob::updateMovementVector(Point* movementVector, Point* collision, float weight)
{
	Point point;
	point.x = collision->x - this->pos.x;
	point.y = collision->y - this->pos.y;
	point.normalize();

	// we don't want a collision to have as much weight as movement.
	point.x /= weight;
	point.y /= weight;

	movementVector->x += point.x;
	movementVector->y += point.y;
	movementVector->normalize();
}

void Mob::update(double elapsedTime) 
{
	handleCollisions();

	switch (this->state) {
	case MobState::Attacking:
		this->attackProcedure(elapsedTime);
		break;
	case MobState::Moving:
	default:
		this->moveProcedure(elapsedTime);
		break;
	}

	this->lastAttackTime += (float)elapsedTime;
}
