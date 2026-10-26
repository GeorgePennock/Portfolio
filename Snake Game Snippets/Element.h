#pragma once

#include "Position.h"
#include "SFML/Graphics.hpp"

#include "Debugging.h"

class Element // This could be using sf::Drawable or sf::Sprite but i want a custom solution that works w. the grid
{
private:
	// States whether the Element recieves events for tick & draw
	// Managed by the map
	bool active = true;

protected:
	sf::RectangleShape shape; // I want this to be private: UPDATE: Nevermind its handy to have exposed
	// I dont think i can tho

	Position position;
	float angle = 0;
	float scale = 1;

	void SetColour(sf::Color color);
	void Texture(const sf::Texture* texture);
	void SetUV(sf::IntRect rect);

	// TODO
	// I want to private the shape class and create custom methods for DrawElement, Rotate, Scale with options to rotate around center or scale around center

	void DrawShape(sf::RenderWindow& renderWindow);

public:
	Element();
	
	// Warning - Overrides the scale factor!
	void ResizeElement();
	Position GetPosition();

	bool IsActive() const;
	void SetActive(bool);

	virtual void OnPlay();
	virtual void OnTick();
	virtual void ResetElement(); // Used for game restarting - sets it back to its default state
	virtual void Draw(sf::RenderWindow& renderWindow);

};

