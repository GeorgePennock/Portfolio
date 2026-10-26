#include "DrawSystem.hpp"

#include <SFML/Graphics.hpp>
#include <math.h>
#include "ExternalHeaders.h"
#include "Debug.h"
#include "DrawShelf.h"
#include "StaticDrawShelf.h"
#include "SFMLUtil.h"
#include <map>


sf::RenderWindow window;
sf::Clock uiDeltaClock;

sf::Clock deltaClock;
sf::Clock gameClock;

int DrawSystem::activeDrawCalls = 0;
float DrawSystem::targetFPS = 0;

#define _VERT(offset, x, y, rx, ry, col)\
target[startIndex + offset] = sf::Vertex({ x,y });\
target[startIndex + offset].texCoords = sf::Vector2f{ rx, ry };\
target[startIndex + offset].color = col;

#define _VERTW(offset, x, y, rx, ry)\
target[startIndex + offset] = sf::Vertex({ x,y });\
target[startIndex + offset].texCoords = sf::Vector2f{ rx, ry };

// TODO Replace with the biggest size and divide down
#define ATLAS_SEGMENT_SIZE 16

// The goat rectangle who is incharge of all draw calls
// actually u aint the goat no more dawg sorry, we got DrawShelves but i still love u
sf::RectangleShape draw;

sf::VertexArray preStatics;
sf::VertexArray postStatics;

DrawSystem* DrawSystem::ptr = nullptr;

float rotX(float x, float y, float rot);
float rotY(float x, float y, float rot);

void DrawSystem::ActuallyBakePreStatics()
{
	Debug::Log("Baking pre render statics...");
	shouldDrawPreStatics = std::size(preDrawStatics) != 0;

	std::set<int> texIdsNeeded;
	for (StaticDrawShelf* shelf : preDrawStatics)
	{
		if (!texIdsNeeded.contains(shelf->tex.identifier)) texIdsNeeded.emplace(shelf->tex.identifier);
	}
	preDrawAtlas.GenerateStaticAtlas(texIdsNeeded);

	BakeStaticsOf(preDrawStatics, preStatics, true);
}

void DrawSystem::ActuallyBakePostStatics()
{
	Debug::Log("Baking post render statics...");
	shouldDrawPostStatics = std::size(postDrawStatics) != 0;
	
	std::set<int> texIdsNeeded;
	for (StaticDrawShelf* shelf : postDrawStatics)
	{
		if (!texIdsNeeded.contains(shelf->tex.identifier)) texIdsNeeded.emplace(shelf->tex.identifier);
	}
	postDrawAtlas.GenerateStaticAtlas(texIdsNeeded);

	BakeStaticsOf(postDrawStatics, postStatics, false);
}

void DrawSystem::LoadTexture(std::string texture)
{
	sf::Texture* tex = new sf::Texture(texture);

	allTextures.emplace(texture, tex);
}

void DrawSystem::BakeStaticsOf(std::set<StaticDrawShelf*>& shelves, sf::VertexArray& target, bool isPreDraw)
{
	// todo fix

	const int TRIS = 6;
	int size = std::size(shelves);
	int triCount = std::size(shelves) * TRIS;
	target = sf::VertexArray(sf::PrimitiveType::Triangles, triCount);

	int incr = 0;
	for (StaticDrawShelf* shelf : shelves)
	{
		int startIndex = incr * TRIS;
		Vectorf pos = shelf->pos; float rot = shelf->rot; Vectorf size = shelf->siz; bool c = shelf->piv; 
		Recti rect = (isPreDraw ? preDrawAtlas : postDrawAtlas).rects[shelf->tex.identifier];

		Debug::Log("rect");
		Debug::Log(rect.AsString());

		float x = c ? pos.x - (size.x / 2.0f) : pos.x;
		float y = c ? pos.y - (size.y / 2.0f) : pos.y;
		float xw = c ? pos.x + (size.x / 2.0f) : (pos.x + size.x);
		float yh = c ? pos.y + (size.y / 2.0f) : (pos.y + size.y);

		float ox = pos.x;
		float oy = pos.y;

		float lx = x - ox;
		float ly = y - oy;
		float lxw = xw - ox;
		float lyh = yh - oy;

		float xtl = rotX(lx, ly, rot) + ox;
		float ytl = rotY(lx, ly, rot) + oy;
		float xtr = rotX(lxw, ly, rot) + ox;
		float ytr = rotY(lxw, ly, rot) + oy;
		float xbl = rotX(lx,lyh, rot) + ox;
		float ybl = rotY(lx,lyh, rot) + oy;
		float xbr = rotX(lxw,lyh, rot) + ox;
		float ybr = rotY(lxw,lyh, rot) + oy;

		float rx = static_cast<float>(rect.x());
		float ry = static_cast<float>(rect.y());
		float rxw = rx + static_cast<float>(rect.w());
		float ryh = ry + static_cast<float>(rect.h());

		// dawg do u like my macro (i do) (thank you) (#talkingtomyself) (#mindfullness)

		_VERTW(0, xtl, ytl, rx, ry);
		_VERTW(1, xtr, ytr, rxw, ry);
		_VERTW(2, xbl, ybl, rx, ryh);
		_VERTW(3, xtr, ytr, rxw, ry);
		_VERTW(4, xbl, ybl, rx, ryh);
		_VERTW(5, xbr, ybr, rxw, ryh);

		incr++;
	}
}



DrawSystem::DrawSystem(std::string title, unsigned int windowWidth, unsigned int windowHeight)
{
	isInitialised = false;
	wantsToBakePostStatics = false;
	wantsToBakePreStatics = false;

	uiDeltaClock = sf::Clock();
	gameClock = sf::Clock();
	deltaClock = sf::Clock();

	preDrawEvent = Event();
	uiPreDrawEvent = Event();

	draw = sf::RectangleShape();

	window = sf::RenderWindow(sf::VideoMode({ windowWidth, windowHeight }), title);

	preDrawAtlas.sys = this; postDrawAtlas.sys = this;
	preDrawAtlas.GenerateStaticAtlas({});
	postDrawAtlas.GenerateStaticAtlas({});

	if (!ImGui::SFML::Init(window))
	{
		Debug::LogError("Could not initialise IMGUI!");
		window.close();
		return;
	}

	ptr = this;

	isInitialised = true;
}

DrawSystem::~DrawSystem()
{
	for (auto kvp : drawableInstances)
	{
		if (kvp.second.shape != nullptr) delete kvp.second.shape;
	}

	for (auto kvp : allTextures)
	{
		if (kvp.second != nullptr) delete kvp.second;
	}
}

bool DrawSystem::IsOpen() { return window.isOpen(); }

void DrawSystem::ForceClose()
{
	if (IsOpen()) window.close();
}

bool DrawSystem::IsInitialised() { return isInitialised; }

void DrawSystem::RegisterShelf(DrawShelf* shelf)
{
	drawables.insert(shelf);
	sf::RectangleShape* drawRectangle = new sf::RectangleShape();

	// Complete decouple of the DrawShelf from sfml, pass change events through here
	DrawInst inst{};
	inst.shape = drawRectangle;
	inst.draw = shelf;
	inst.sys = this;
	inst.activeTexture = nullptr;

	shelf->posChanged.Subscribe(std::bind_front(&DrawInst::UpdPos, inst)); inst.UpdPos(shelf->position);
	shelf->rotChanged.Subscribe(std::bind_front(&DrawInst::UpdRot, inst)); inst.UpdRot(shelf->rotation);
	shelf->pivotChanged.Subscribe(std::bind_front(&DrawInst::UpdPiv, inst)); inst.UpdPiv(shelf->renderCentered);
	shelf->sizeChanged.Subscribe(std::bind_front(&DrawInst::UpdSiz, inst)); inst.UpdSiz(shelf->size);
	shelf->texChanged.Subscribe(std::bind_front(&DrawInst::UpdTex, inst)); inst.UpdTex(shelf->texture);

	drawableInstances.emplace(shelf, inst);
}

void DrawSystem::DeregisterShelf(DrawShelf* shelf)
{
	drawables.erase(shelf);
	if (drawableInstances.contains(shelf))
	{
		delete drawableInstances[shelf].shape;
		drawableInstances.erase(shelf);
	}
}

void DrawSystem::RegisterStaticShelf(StaticDrawShelf* shelf, StaticOrder order)
{
	if (order == Post) postDrawStatics.insert(shelf);
	else if (order == Pre) preDrawStatics.insert(shelf);
	BakeStatics(order);
}

void DrawSystem::DeregisterStaticShelf(StaticDrawShelf* shelf, StaticOrder order)
{
	if (order == Post) postDrawStatics.erase(shelf);
	else if (order == Pre) preDrawStatics.erase(shelf);
	BakeStatics(order);
}

void DrawSystem::DrawLine(Vectorf from, Vectorf to, int thickness, int hue)
{

}

void DrawSystem::DrawBox(Vectorf from, Vectorf to, int thickness, int hue)
{
	DrawLine({ from.x, from.y }, {from.x, to.y}, thickness, hue);
	DrawLine({ from.x, from.y }, {to.x, from.y}, thickness, hue);
	DrawLine({ from.x, to.y }, {to.x, to.y}, thickness, hue);
	DrawLine({ to.x, from.y }, {to.x, to.y}, thickness, hue);
}

void DrawSystem::DrawCircle(Vectorf from, float radius, int density, int thickness, int hue)
{
}

void DrawSystem::DrawLineStrip(std::vector<Vectorf> points, bool connectEndToStart, int thickness, int hue)
{
}


void DrawSystem::DrawShelves()
{
	// Draw Pre Statics
	if (shouldDrawPreStatics)
	{
		sf::RenderStates states{};
		states.texture = preDrawAtlas.texture;
		window.draw(preStatics, states);
		activeDrawCalls++;
	}

	for (DrawShelf* s : drawables)
	{
		sf::RectangleShape* toDraw = drawableInstances[s].shape;
		//Debug::Log(InverseTranslatef(toDraw->getSize()).AsString());
		activeDrawCalls++;
		window.draw(*toDraw);
	}

	if (shouldDrawPostStatics)
	{
		sf::RenderStates states{};
		states.texture = postDrawAtlas.texture;
		window.draw(postStatics, states);
		activeDrawCalls++;
	}
	// Draw Post Statics
}

void DrawSystem::BakeStatics(StaticOrder staticOrder)
{
	if (staticOrder == Pre) wantsToBakePreStatics = true;
	else if (staticOrder == Post) wantsToBakePostStatics = true;
}

Event* DrawSystem::OnPreDrawEvent()
{
	return &preDrawEvent;
}

Event* DrawSystem::OnUiPreDrawEvent()
{
	return &uiPreDrawEvent;
}

bool DrawSystem::DrawButton(Vectorf origin, Vectorf size, int hue)
{
	draw.setPosition(Translatef(origin));
	draw.setOrigin(Translatef(size / 2));
	draw.setSize(Translatef(size));
	draw.setFillColor(sf::Color(hue));

	activeDrawCalls++;
	window.draw(draw);

	// Using ImGui so its a local mouse position
	sf::Vector2i mPos = ImGui::GetMousePos();
	Vectorf mousePosition{ static_cast<float>(mPos.x), static_cast<float>(mPos.y) };

	if (Rectf(origin - (size / 2.f), size).InBounds(mousePosition))
	{
		// Using ImGui so i can easily just have a "mouse clicked once" event
		return ImGui::IsMouseClicked(ImGuiMouseButton_Left);
	}
	return false;
}

Vectorf DrawSystem::TEMP_MouseDelta()
{
	return tempMouseDelta;
}

bool DrawSystem::TEMP_MouseStatus()
{
	return sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
}

void DrawSystem::PreRender()
{
	window.clear();

	Debug::DefineImGui();

	if (wantsToBakePostStatics)
	{
		wantsToBakePostStatics = false;
		ActuallyBakePostStatics();
	}

	if (wantsToBakePreStatics)
	{
		wantsToBakePreStatics = false;
		ActuallyBakePreStatics();
	}

	activeDrawCalls = 0;
}

void DrawSystem::Render()
{
	// Fire predraw at the literal last moment
	OnPreDrawEvent()->Fire();

	ImVec2 v2 = ImGui::GetMousePos();
	tempMouseDelta = Vectorf(v2.x, v2.y) - oldMouse;
	oldMouse = { v2.x,v2.y };

	OnUiPreDrawEvent()->Fire();

	// Draw
	ImGui::SFML::Render(window);
	window.display();
}

void DrawSystem::PollEvents()
{
	while (const std::optional event = window.pollEvent())
	{
		ImGui::SFML::ProcessEvent(window, event.value());

		if (event->is<sf::Event::Closed>())
			window.close();
	}
	
	ImGui::SFML::Update(window, uiDeltaClock.restart());

}

void DrawSystem::SetTargetFPS(int fps)
{
	window.setFramerateLimit(fps);

	targetFPS = fps == 0 ? std::numeric_limits<float>::max() : static_cast<float>(fps);
}

float DrawSystem::GetTargetFPS()
{
	return targetFPS;
}

int DrawSystem::GetDrawCallCount() { return activeDrawCalls; }

float DrawSystem::TimeSinceLastSample()
{
	return deltaClock.restart().asSeconds();
}

float DrawSystem::Time()
{
	return gameClock.getElapsedTime().asSeconds();
}

DrawSystem* DrawSystem::I()
{
	return ptr;
}

TexId DrawSystem::LoadImage(std::string file, Recti subRect)
{
	if (!allTextures.contains(file)) LoadTexture(file);
	TexId returnValue = TexId(); returnValue.identifier = ++texIdIncrPtr;
	Tex tex{};

	tex.texture = allTextures[file];
	if (tex.subrect == Recti{ 0, 0, 0, 0 })
	{
		tex.subrect = Recti(Vectori{ 0,0 }, InverseTranslateutoi(allTextures[file]->getSize()));
	}
	else tex.subrect = subRect;
	isAnimatedTextures.emplace(returnValue.identifier, false);
	allImages.emplace(returnValue.identifier, tex);
	texFileNames.emplace(returnValue.identifier, file);

	return returnValue;
}

TexId DrawSystem::LoadAnimation(std::string file, Recti subRect, int xTiles, int yTiles, AnimDirection animation)
{
	if (!allTextures.contains(file)) LoadTexture(file);
	TexId returnValue = TexId(); returnValue.identifier = ++texIdIncrPtr;
	TexAnim tex{};

	tex.texture = allTextures[file];
	if (tex.subrect == Recti{ 0, 0, 0, 0 })
	{
		tex.subrect = Recti(Vectori{ 0,0 }, InverseTranslateutoi(allTextures[file]->getSize()));
	}
	else tex.subrect = subRect;
	tex.xTiles = xTiles;
	tex.yTiles = yTiles;
	tex.direction = animation;

	isAnimatedTextures.emplace(returnValue.identifier, true);
	allAnimations.emplace(returnValue.identifier, tex);
	texFileNames.emplace(returnValue.identifier, file);
	
	return returnValue;
}

float rotX(float x, float y, float rot)
{
	return (x * std::cos(sf::degrees(rot).asRadians())) - (y * std::sin(sf::degrees(rot).asRadians()));
}

float rotY(float x, float y, float rot)
{
	return (y * std::cos(sf::degrees(rot).asRadians())) + (x * std::sin(sf::degrees(rot).asRadians()));
}

void DrawSystem::DrawInst::UpdPos(Vectorf pos)
{
	shape->setPosition(Translatef(pos));
}

void DrawSystem::DrawInst::UpdRot(float rot)
{
	shape->setRotation(sf::degrees(rot));
}

void DrawSystem::DrawInst::UpdSiz(Vectorf siz)
{
	Debug::Log("Updating size to" + siz.AsString());
	shape->setSize(Translatef(siz));
}

void DrawSystem::DrawInst::UpdPiv(bool centered)
{
	if (centered)
	{
		shape->setOrigin(Translatef(draw->GetSize() / 2.0f));
		return;
	}
	shape->setOrigin({ 0,0 });
}

void DrawSystem::DrawInst::UpdTex(TexId texId)
{
	if (!sys->isAnimatedTextures.contains(texId.identifier)) return;
	bool isAnimated = sys->isAnimatedTextures[texId.identifier];

	if (isAnimated)
	{
		Debug::LogFatal("Animated textures not implemented.");
		return;
	}

	Tex targetTexture = sys->allImages[texId.identifier];

	bool hasActualTextureChanged = targetTexture.texture != activeTexture;
	
	if (hasActualTextureChanged)
	{
		activeTexture = targetTexture.texture;
		shape->setTexture(activeTexture);
	}

	shape->setTextureRect(TranslateRecti(targetTexture.subrect));
}

bool TexId::operator==(const TexId& other) const
{
	return this->identifier == other.identifier;
}

// Texture atlasing things now

struct Pack
{
	int size;
	std::string fullString = "";

	Pack()
	{
		this->size = 1;
		this->fullString = "bad";
	}

	Pack(int rect, std::string string)
	{
		this->size = rect;
		this->fullString = string;
	}
};

struct Quad
{
	int dimensions;

	Quad* subquads[4];

	bool isPack;
	Pack pack;

	Quad(Pack pack)
	{
		isPack = true;
		this->pack = pack;

		// surely... surely this doesnt work...
		///subquads[0, 1, 2, 3] = nullptr;

		subquads[0] = nullptr;
		subquads[1] = nullptr;
		subquads[2] = nullptr;
		subquads[3] = nullptr;
	}

	Quad(Quad* a, Quad* b, Quad* c, Quad* d)
	{
		isPack = false;
		this->pack = Pack();
		this->subquads[0] = a;
		this->subquads[1] = b;
		this->subquads[2] = c;
		this->subquads[3] = d;
	}

	Quad() 
	{

		isPack = false;
		this->pack = Pack();
		subquads[0] = nullptr;
		subquads[1] = nullptr;
		subquads[2] = nullptr;
		subquads[3] = nullptr;
	}
};

void SampleQuad(sf::Image* image, Quad* targetQuad, sf::Vector2u topLeft, std::map<std::string, sf::Image*>& imageDictionary, std::map<std::string, Recti>* rects);

void DrawSystem::Atlas::GenerateStaticAtlas(std::set<int> texIds)
{
	bool needsRegenerating = false;
	for (int id : texIds)
	{
		if (!sys->allImages.contains(id))
		{
			continue;
		}
		if (!existingTextures.contains(sys->allImages[id].texture))
		{
			needsRegenerating = true;
			break;
		}
	}

	if (!needsRegenerating) return;

	Debug::Log("Generating static atlas...");

	// Texture sheets can be used for certain things yeah k
	//std::unordered_map<std::string, sf::Texture*> textures;

	rects.clear();

	sf::Image* generatedImage{};

	std::map<std::string, sf::Image*> seenFiles;

	std::map<int, std::vector<Pack>> dimensions;

	int count = 0;

	std::set<std::string> ts;

	for (int id : texIds)
	{
		if (!ts.contains(sys->texFileNames[id])) ts.emplace(sys->texFileNames[id]);
	}

	for (std::string fileName : ts)
	{
		count++;

		Debug::Log(std::string("Loading image ").append(fileName));

		sf::Image* loadedImage = new sf::Image(fileName);
		seenFiles.emplace(fileName, loadedImage);
		// calc shape thingies
		sf::Vector2u imageSize = loadedImage->getSize();
		int highest = std::max(imageSize.x, imageSize.y);

		int dimension = ATLAS_SEGMENT_SIZE;

		if (dimension <= 0) dimension = 1; // prevent infinite loop
		while (dimension < highest)
		{
			dimension *= 2;
			Debug::Log(dimension);
		}

		if (!dimensions.contains(dimension)) dimensions.emplace(dimension, std::vector<Pack>());

		dimensions[dimension].push_back(Pack(dimension, fileName));
	}

	if (count == 0)
	{
		Debug::LogWarning("No viable image files in path");
		return;
	}

	// Building quads

	// For garbage collection
	std::vector<Quad*> clean;

	// The active quad layer
	std::vector<Quad*> currentQuadContext;

	for (auto pair : dimensions)
	{
		Debug::Log("Dimensionally searching " + std::to_string(pair.first));

		// Populate currentQuadContext with this layer's quads
		for (Pack pack : pair.second)
		{
			Debug::Log("Inpsecting pack " + pack.fullString);
			Quad* quad = new Quad(pack);
			quad->dimensions = pack.size;

			clean.push_back(quad);
			currentQuadContext.push_back(quad);
		}

		int length = std::size(currentQuadContext);
		int numberOfSquares = std::floor(static_cast<float>(length) /4.f);

		std::vector<Quad*> newQuadContext;

		for (int i = 0; i < numberOfSquares; i++)
		{
			Debug::Log("Developing quads");
			Quad* newQuad = new Quad(currentQuadContext[(i * 4) + 0], currentQuadContext[(i * 4) + 1], currentQuadContext[(i * 4) + 2], currentQuadContext[(i * 4) + 3]);
			newQuad->dimensions = pair.first * 2;

			clean.push_back(newQuad);
			newQuadContext.push_back(newQuad);
		}

		if (length != (numberOfSquares * 4))
		{
			Debug::Log("Developing unfinished quad");
			Quad* newQuad = new Quad();

			for (int i = 0; i < length % 4; i++)
			{
				// i for some reason need to tell it this? even tho the default constructor is being ran

				newQuad->isPack = false;
				newQuad->subquads[i] = currentQuadContext[i];
				newQuad->dimensions = pair.first * 2;
			}
			clean.push_back(newQuad);
			newQuadContext.push_back(newQuad);
		}

		Debug::Log("Prior Length: " + std::to_string(length));

		Debug::Log("Current size: " + std::to_string(std::size(newQuadContext)));
		currentQuadContext = newQuadContext;
	}

	if (std::size(currentQuadContext) > 1)
	{
		Quad* finalTopQuad = new Quad();
		int l = std::size(currentQuadContext);

		for (int i = 0; i < l; i++)
		{
			// i for some reason need to tell it this? even tho the default constructor is being ran

			finalTopQuad->isPack = false;
			finalTopQuad->subquads[i] = currentQuadContext[i];
			finalTopQuad->dimensions = currentQuadContext[0]->dimensions * 2;
		}
		clean.push_back(finalTopQuad);
		
		currentQuadContext.clear(); currentQuadContext.push_back(finalTopQuad);
	}

	if (std::size(currentQuadContext) != 1)
	{
		// bad news
		Debug::Log(std::size(currentQuadContext));
		Debug::LogFatal("BROKEN CATASTROPHICALLY");
	}

	// for some reason my quad context is clearing by here?

	Quad* topLevelQuad = currentQuadContext[0];

	int textureSize = topLevelQuad->dimensions;

	if (generatedImage != nullptr) delete generatedImage;
	Debug::Log(textureSize);
	generatedImage = new sf::Image(sf::Vector2u(textureSize, textureSize), sf::Color((char)255, (char)0, (char)255, (char)0));

	sf::Vector2u topLeft{ 0,0 };

	std::map<std::string, Recti> dawgiranoutofnames;

	SampleQuad(generatedImage, topLevelQuad, topLeft, seenFiles, &dawgiranoutofnames);

	bool printed = generatedImage->saveToFile(std::filesystem::current_path().append("debugImageCache.png"));

	if (printed)
	{
		Debug::Log("Generated Image atlas file.");
	}
	else
	{
		Debug::LogError("Couldnt generate image atlas file :(");
	}

	// Finally
	for (int id : texIds)
	{
		std::string str = sys->texFileNames[id];
		if (dawgiranoutofnames.contains(str))
		{
			rects.emplace(id, dawgiranoutofnames[str]);
		}
	}

	if (image != nullptr) delete image;
	image = generatedImage;
	if (texture != nullptr) delete texture;
	texture = new sf::Texture(); texture->loadFromImage(*generatedImage);

	for (Quad* quad : clean)
	{
		if (quad != nullptr)
		{
			delete quad;
			quad = nullptr;
		}
	}

	for (auto file : seenFiles)
	{
		//allImages.emplace(file.first, new Image(file.first, seenFiles[file.first]));
		delete file.second;
	}

	//for (auto file : dawgiranoutofnames)
	//{
	//	allImages.insert({ file.first, new Image(file.first,file.second) });
	//}

}

DrawSystem::Atlas::~Atlas()
{
	if (texture != nullptr) delete texture;
	if (image != nullptr) delete image;
}

void SampleQuad(sf::Image* image, Quad* quad, sf::Vector2u topLeft, std::map<std::string, sf::Image*>& imageDictionary, std::map<std::string, Recti>* rects)
{
	if (quad->isPack)
	{
		// End of chain
		std::filesystem::path path = quad->pack.fullString;
		Debug::Log(path);
		Debug::Log(imageDictionary[quad->pack.fullString]);

		rects->emplace(quad->pack.fullString, Recti(Vectori{ static_cast<int>(topLeft.x),static_cast<int>(topLeft.y) }, InverseTranslateutoi(imageDictionary[quad->pack.fullString]->getSize())));

		image->copy(*imageDictionary[quad->pack.fullString], topLeft);
		return;
	}

	if (quad->subquads[0] != nullptr)
	{
		sf::Vector2u origin = topLeft + sf::Vector2u{ 0, 0 };
		Debug::Log(quad->subquads[0]->pack.fullString);
		SampleQuad(image, quad->subquads[0], origin, imageDictionary, rects);
	}
	if (quad->subquads[1] != nullptr)
	{
		sf::Vector2u origin = topLeft + sf::Vector2u{ static_cast<unsigned int>(quad->dimensions / 2), 0 };
		Debug::Log(quad->subquads[1]->pack.fullString);
		SampleQuad(image, quad->subquads[1], origin, imageDictionary, rects);
	}
	if (quad->subquads[2] != nullptr)
	{
		sf::Vector2u origin = topLeft + sf::Vector2u{ 0, static_cast<unsigned int>(quad->dimensions / 2) };
		Debug::Log(quad->subquads[2]->pack.fullString);
		SampleQuad(image, quad->subquads[2], origin, imageDictionary, rects);
	}
	if (quad->subquads[3] != nullptr)
	{
		sf::Vector2u origin = topLeft + sf::Vector2u{ static_cast<unsigned int>(quad->dimensions / 2), static_cast<unsigned int>(quad->dimensions / 2) };
		Debug::Log(quad->subquads[3]->pack.fullString);
		SampleQuad(image, quad->subquads[3], origin, imageDictionary, rects);
	}
}