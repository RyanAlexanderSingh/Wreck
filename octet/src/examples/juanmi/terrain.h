////////////////////////////////////////////////////////////////////////////////
//
// (C) Juanmi Huertas Delgado 2014
//
// 	PanchitoObject represent the runner of the game
//		This class is required by PanchitoJog
//

namespace octet {
	enum {	NUM_MAX_BLOCKS = 4,
			LENGTH_BLOCK = 10,
			WIDTH_BLOCK = 10,
			HEIGTH_BLOCK = 1,
	};
	class terrainBlock{
		//The position where the block is
		mat4t modelToWorld;
		scene_node* blockNode;
		mesh_box ground;

		//Terrain has colors (they will have materials, but for now, just color)
		//And we have the ground, and the artifacts attached
		//vec4 colorGround;
		//vec4 colorArtifacts;

	public:
		terrainBlock(){
			blockNode = 0;
		}

		~terrainBlock(){
			delete blockNode;
		}

		//this is called to create a new terrainBlock from the emptyness
		void init(float x, float z){
			//First we ad the colors!
			//colorGround = vec4(1,1,0,1);
			//colorArtifacts = vec4(0,0,1,1);
			//and now we get ready the position. Every single terrainBlock will be at height (y) = 0
			modelToWorld.loadIdentity();
			modelToWorld.translate(x,0,z);
			//the node that will hold the block
			if(blockNode = 0)
				blockNode = new scene_node(modelToWorld,atom_);
		}

		void LoadToScene(ref<visual_scene> scene){
			//First the materials
			material *materialGround = new material(vec4(1,1,0,1));
			material *materialArtifacts = new material(vec4(0,0,1,1));

			//Then the ground (floor)
			mesh_box *ground = new mesh_box(vec3(WIDTH_BLOCK,HEIGTH_BLOCK,LENGTH_BLOCK-2));
			mat4t groundPosition;
			groundPosition.loadIdentity();
			groundPosition.translate(0,0,0);
			blockNode = new scene_node(groundPosition,atom_);
			scene->add_mesh_instance(new mesh_instance(blockNode,ground,materialGround));
		}

		void runBlock(float distance){
			blockNode->translate(vec3(0,0,distance));
		}

		void render(){
			//Render the ground
			//Render all the artifacts
		}

		scene_node * getNode(){
			return blockNode;
		}
	};

	class terrain{
		//The position where the terrain is, this will be fixed
		mat4t modelToWorld;

		//The blocks of the terrain, those will move
		double_list<scene_node*> blockNodes;
		terrainBlock blocks [NUM_MAX_BLOCKS];

		//some variables for blocks "running" effect
		float currentFirstBlockPosition;
		int currentFirstBlock;
		int numberBlocks;

		//Scene for drawing items
		ref<visual_scene> app_scene;

		void add_block(){
			blocks[numberBlocks].init(0.f,0.f);
			//blocks[numberBlocks].LoadToScene(app_scene);
			++numberBlocks;
		}

		void remove_block(){
			blockNodes.erase(blockNodes.begin());
		}
	public:
		terrain(){
		}

		~terrain(){
		}

		// This will initializate the terrain
		void init(visual_scene * scene){
			app_scene = scene;
			currentFirstBlockPosition = 0;
			numberBlocks = 0;
			for(int i=0; i<NUM_MAX_BLOCKS; ++i)
				add_block();
		}

		// This function will make the ground to "run". 
		// It will take into account the current speed of Panchito
		void run_terrainBlocks(float currentSpeed){
			/*double_list<terrainBlock>::iterator it(blocks.begin());
			while(it != blocks.end()){
				it->runBlock(currentSpeed);
				it++;
			}
			currentFirstBlockPosition -= currentSpeed;
			if(currentFirstBlockPosition <= - LENGTH_BLOCK){
				currentFirstBlockPosition = 0;
				remove_block();
				add_block();
			}*/
		}

		void render(){
			/*//FILL HERE HOW TO RENDER THIS SCENE!!!!
			//Render all blocks of terrain
			for (int i = 0; i < NUM_MAX_BLOCKS; ++i){
				blockNodes
			}*/
		}
	};
}