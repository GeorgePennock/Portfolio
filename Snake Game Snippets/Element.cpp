#include "Element.h"
#include "Map.h"

void Element::SetColour(sf::Color color)
{
	shape.setFillColor(color);
}

void Element::Texture(const sf::Texture* texture)
{
	//Debugging::Log((std::string)"Trying to texture");
	shape.setTexture(texture);
}

void Element::SetUV(sf::IntRect rect)
{
	shape.setTextureRect(rect);
}

void Element::DrawShape(sf::RenderWindow& renderWindow)
{
	shape.setPosition(Map::Translate(position) + shape.getGeometricCenter());
	shape.setOrigin(shape.getGeometricCenter());
	shape.setRotation(sf::degrees(angle));
	shape.setScale({ scale,scale });
	renderWindow.draw(shape);
}

Element::Element()
{
	shape = sf::RectangleShape(Map::GetTileSizeAsVector2f());
	SetColour(sf::Color::White);
}

void Element::ResizeElement()
{
	shape.setSize(Map::GetTileSizeAsVector2f());
}

Position Element::GetPosition()
{
	return position;
}

bool Element::IsActive() const
{
	return active;
}

void Element::SetActive(bool value)
{
	active = value;
}

void Element::Draw(sf::RenderWindow& renderWindow)
{
	DrawShape(renderWindow);
}

void Element::OnPlay()
{
}

void Element::OnTick()
{
}

void Element::ResetElement()
{
}
