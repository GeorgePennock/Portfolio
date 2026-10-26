#include "UISplitImage.h"
#include "Debugging.h"
#include "AssetPack.h"
#include "Utility.h"

void UISplitImage::GenerateVertices(Position position, Position size, char boundLeft, char boundRight, char boundUp, char boundDown, float scalar)
{
	// This is my custom solution for generating split images, similar to unity's sprite splitting system
	// It lets you define borders, and in the interior it streches the texture rect, whilst on the exterior it keeps it uniform
	// Meant for buttons and what not

	float scaledBoundLeft = boundLeft * scalar;
	float scaledBoundRight = boundRight * scalar;
	float scaledBoundUp = boundUp * scalar;
	float scaledBoundDown = boundDown * scalar;

	unsigned int uBoundLeft = static_cast<unsigned int>(boundLeft);
	unsigned int uBoundRight = static_cast<unsigned int>(boundRight);
	unsigned int uBoundUp = static_cast<unsigned int>(boundUp);
	unsigned int uBoundDown = static_cast<unsigned int>(boundDown);

	sf::Vector2u imageSize = backupTexture.getSize();

	// Inner Vertices
	GEN_VERTEX_T(innerTopLeft, scaledBoundLeft, scaledBoundUp, sf::Vector2f(uBoundLeft, uBoundUp));
	GEN_VERTEX_T(innerTopRight, size.X - scaledBoundRight, scaledBoundUp, sf::Vector2f(imageSize.x - uBoundRight, uBoundUp));
	GEN_VERTEX_T(innerBottomLeft, scaledBoundLeft, size.Y - scaledBoundDown, sf::Vector2f(uBoundLeft, imageSize.y - uBoundDown));
	GEN_VERTEX_T(innerBottomRight, size.X - scaledBoundRight, size.Y - scaledBoundDown, sf::Vector2f(imageSize.x - uBoundRight, imageSize.y - uBoundDown));

	// Top Left Vertices
	GEN_VERTEX_T(outerTopLeft, 0, 0, sf::Vector2f(0, 0));
	GEN_VERTEX_T(outerTopLeftOffsetRight, scaledBoundLeft, 0, sf::Vector2f(uBoundLeft, 0));
	GEN_VERTEX_T(outerTopLeftOffsetDown, 0, scaledBoundUp, sf::Vector2f(0, uBoundUp));

	// Top Right Vertices
	GEN_VERTEX_T(outerTopRight, size.X, 0, sf::Vector2f(imageSize.x, 0));
	GEN_VERTEX_T(outerTopRightOffsetLeft, size.X - scaledBoundRight, 0, sf::Vector2f(imageSize.x - uBoundRight, 0));
	GEN_VERTEX_T(outerTopRightOffsetDown, size.X, scaledBoundUp, sf::Vector2f(imageSize.x, uBoundUp));

	// Bottom Left Vertices
	GEN_VERTEX_T(outerBottomLeft, 0, size.Y, sf::Vector2f(0, imageSize.y));
	GEN_VERTEX_T(outerBottomLeftOffsetRight, scaledBoundLeft, size.Y, sf::Vector2f(uBoundLeft, imageSize.y));
	GEN_VERTEX_T(outerBottomLeftOffsetUp, 0, size.Y - scaledBoundDown, sf::Vector2f(0, imageSize.y - uBoundUp));

	// Bottom Right Vertices
	GEN_VERTEX_T(outerBottomRight, size.X, size.Y, sf::Vector2f(imageSize.x, imageSize.y));
	GEN_VERTEX_T(outerBottomRightOffsetLeft, size.X - scaledBoundRight, size.Y, sf::Vector2f(imageSize.x - uBoundRight, imageSize.y));
	GEN_VERTEX_T(outerBottomRightOffsetUp, size.X, size.Y - scaledBoundDown, sf::Vector2f(imageSize.x, imageSize.y - uBoundUp));

	// Center
	GEN_TRIANGLE(vertexArray, 0, innerTopLeft, innerTopRight, innerBottomLeft);
	GEN_TRIANGLE(vertexArray, 3, innerBottomLeft, innerBottomRight, innerTopRight);

	// Top Left
	GEN_TRIANGLE(vertexArray, 6, outerTopLeft, outerTopLeftOffsetRight, outerTopLeftOffsetDown);
	GEN_TRIANGLE(vertexArray, 9, outerTopLeftOffsetDown, outerTopLeftOffsetRight, innerTopLeft);

	// Top Right
	GEN_TRIANGLE(vertexArray, 12, outerTopRightOffsetLeft, outerTopRight, innerTopRight);
	GEN_TRIANGLE(vertexArray, 15, innerTopRight, outerTopRight, outerTopRightOffsetDown);

	// Bottom Left
	GEN_TRIANGLE(vertexArray, 18, outerBottomLeftOffsetUp, innerBottomLeft, outerBottomLeft);
	GEN_TRIANGLE(vertexArray, 21, outerBottomLeft, innerBottomLeft, outerBottomLeftOffsetRight);

	// Bottom Right
	GEN_TRIANGLE(vertexArray, 24, innerBottomRight, outerBottomRightOffsetUp, outerBottomRightOffsetLeft);
	GEN_TRIANGLE(vertexArray, 27, outerBottomRightOffsetLeft, outerBottomRightOffsetUp, outerBottomRight);

	// Left Side
	GEN_TRIANGLE(vertexArray, 30, outerTopLeftOffsetDown, innerTopLeft, innerBottomLeft);
	GEN_TRIANGLE(vertexArray, 33, innerBottomLeft, outerTopLeftOffsetDown, outerBottomLeftOffsetUp);

	// Right Side
	GEN_TRIANGLE(vertexArray, 36, innerTopRight, outerTopRightOffsetDown, outerBottomRightOffsetUp);
	GEN_TRIANGLE(vertexArray, 39, outerBottomRightOffsetUp, innerTopRight, innerBottomRight);

	// Top Side
	GEN_TRIANGLE(vertexArray, 42, outerTopLeftOffsetRight, outerTopRightOffsetLeft, innerTopRight);
	GEN_TRIANGLE(vertexArray, 45, innerTopRight, outerTopLeftOffsetRight, innerTopLeft);

	// Bottom Side
	GEN_TRIANGLE(vertexArray, 48, innerBottomLeft, innerBottomRight, outerBottomRightOffsetLeft);
	GEN_TRIANGLE(vertexArray, 51, outerBottomRightOffsetLeft, innerBottomLeft, outerBottomLeftOffsetRight);

}

void UISplitImage::PositionChanged(Position position)
{
	transform.setPosition(position.AsVector2f());
}

void UISplitImage::SizeChanged(Position size)
{
	transform.setScale({ 1,1 });
}

void UISplitImage::AngleChanged(float angle)
{
	transform.setRotation(sf::degrees(angle));
}

void UISplitImage::ChangeTexture(sf::Texture& texture)
{
	image = &texture;
}


UISplitImage::UISplitImage(Position position, Position size, short int path, char boundLeft, char boundRight, char boundUp, char boundDown, float scalar)
{
	Configure(position, size);
	if (!backupTexture.loadFromFile(AssetPack::Get(path)))
	{
		Debugging::Error((std::string)"Failed to load image " + AssetPack::Get(path));
		Visible = false;
	}
	image = &backupTexture;
	texture = path;

	vertexArray = sf::VertexArray(sf::PrimitiveType::Triangles, 9 * 6);


	GenerateVertices(position, size, boundLeft, boundRight, boundUp, boundDown, scalar);

	Visible = true;
	RecieveInputs = true;

	this->boundLeft = boundLeft;
	this->boundRight = boundRight;
	this->boundUp = boundUp;
	this->boundDown = boundDown;
	this->scalar = scalar;
}

UISplitImage::UISplitImage(Position position, Position size, std::string path, char boundLeft, char boundRight, char boundUp, char boundDown, float scalar)
{
	Configure(position, size);
	if (!backupTexture.loadFromFile(path))
	{
		Debugging::Error((std::string)"Failed to load image " + path);
		Visible = false;
	}
	image = &backupTexture;
	texture = 0;

	vertexArray = sf::VertexArray(sf::PrimitiveType::Triangles,9 * 6);


	GenerateVertices(position, size, boundLeft, boundRight, boundUp, boundDown, scalar);

	Visible = true;
	RecieveInputs = true;

	this->boundLeft = boundLeft;
	this->boundRight = boundRight;
	this->boundUp = boundUp;
	this->boundDown = boundDown;
	this->scalar = scalar;
}

void UISplitImage::Draw(sf::RenderWindow& renderWindow)
{
	if (!Visible) return;

	sf::RenderStates states = sf::RenderStates(image);
	states.transform = transform.getTransform();
	renderWindow.draw(vertexArray,states);
	
	//sf::RectangleShape shape;
	//shape.setPosition(transform.getPosition());
	//shape.setSize(this->GetSize().AsVector2f());
	//shape.setOutlineThickness(1);
	//shape.setFillColor(sf::Color::Transparent);
	//shape.setOutlineColor(sf::Color::Red);
	//
	//renderWindow.draw(shape);
	//
	//shape.move(sf::Vector2f(boundLeft * scalar, boundUp * scalar));
	//shape.setSize(this->GetSize().AsVector2f() - sf::Vector2f((boundLeft + boundRight) * scalar, (boundUp + boundDown) * scalar));
	//shape.setOutlineColor(sf::Color::Blue);
	//
	//renderWindow.draw(shape);

}

void UISplitImage::TextureUpdate()
{
	if (texture == 0) return;

	if (!backupTexture.loadFromFile(AssetPack::Get(texture)))
	{
		Debugging::Error((std::string)"Failed to load image " + AssetPack::Get(texture));
		Visible = false;
	}
	image = &backupTexture;
}

