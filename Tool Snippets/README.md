# Tool Snippets (C#)
These are just snippets / examples of different parts of the project.

## Saveable Behaviours
     SaveableBehaviour.cs
     Entity.cs
     Chunks.cs
     GameSave.cs
I created a system for saving an entire unity MonoBehaviour class using reflection. Inheriting from the SaveableBehaviour class allowed you to use the [Save] Attribute. Marking a property or variable with a SaveAttribute would cause it to be added to the class's save data.

A saved class would store whatever parameters you specified, aswell as its type. You can then specify how a saved variable is loaded back into the world depending on its type. I made this to integrate into a chunking system, where I would deload regions of the map that are not near the player.

## Smart Enemies
    LivingEntity.cs
    SmartEntity.cs
    EntityBrain.cs
    EntityStateMachine.cs

    CritterEntityBrain.cs
    CritterEntityStateMachine.cs
For one of our game projects, I wanted an easy and adaptable way of creating AI systems that can track the player and make decisions.

I went with a StateMachine setup split across a few different components. The SmartEntity would have a reference to an EntityBrain. The entity brain would contain an EntityStateMachine which was incharge of making all the decisions in relation to the entity. 

The entity brain would be incharge of showing settings like how far away the player has to be. I've also linked an example implementation called the CritterEntity.

## Unity Editor Map Window
     MapEditorWindow.cs
I created alot of Unity Editor tools for level designers to use, hoping to streamline their development and make it easier to build the map.

One of these systems was a Map Editor Window that let you pick specific chunks to load from a map overlay. Each part of the map was stored in its own scene file, making it hard to find and load the right parts of the map that you wanted. This system let you select each of these chunks from an image of what the map looked like.
## Unity Editor Transform Tools
     VineCurveEditorWindow.cs
     PrefabScatterEditorWindow.cs
I created lots of different transformation tools to use for level designers to easily place and populate enviroments in our game.

One system I made let you create a Line renderer and curve it with a custom height and arc, then bake that curve into a mesh.

Another system I made let you scatter and place items easily. Items had to be stored in a wrapper class where they were not instantiated yet, so it made it hard to place them down in batches. This tool handled it for you, and also allowed you to align it to the surface and randomly rotate placed items.
# Tool Snippets (C++)
These are just snippets / examples of different parts of the project.

## Texture Atlasing
     DrawSystem.hpp
     DrawSystem.cpp
I developed a small texture atlasing system which would automatically bunch textures into one main file. This would help reduce load of the GPU so that it doesnt have to change textures as often.

You can also use this image for a singular draw call of unmoving images, which can combine hundreds of draw calls into a singular one, with the only caveat being that you cant move elements indivually anymore.

(You can find the texture atlating within the DrawSystem, its implementation is located at the bottom of the .cpp file).