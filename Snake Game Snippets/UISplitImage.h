#pragma once
#include "UIElement.h"

class UISplitImage : public UIElement
{
private:
	sf::VertexArray vertexArray;
	sf::Transformable transform;

	short int texture;

	char boundUp;
	char boundDown;
	char boundLeft;
	char boundRight;
	float scalar;

	void GenerateVertices(Position position, Position size, char boundLeft, char boundRight, char boundUp, char boundDown, float scalar);

protected:
	void PositionChanged(Position position) override;
	void SizeChanged(Position size) override;
	void AngleChanged(float angle) override;

public:
	sf::Texture backupTexture;
	sf::Texture* image;

	// Must have the same size otherwise texture will display wrong
	void ChangeTexture(sf::Texture& texture);


	UISplitImage(Position position, Position size, short int path, char boundLeft, char boundRight, char boundUp, char boundDown, float scalar);
	UISplitImage(Position position, Position size, std::string path, char boundLeft, char boundRight, char boundUp, char boundDown, float scalar);

	void Draw(sf::RenderWindow& renderWindow) override;
	void TextureUpdate() override;
};

