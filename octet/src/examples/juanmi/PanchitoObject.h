////////////////////////////////////////////////////////////////////////////////
//
// (C) Juanmi Huertas Delgado 2014
//
// 	PanchitoObject represent the runner of the game
//		This class is required by PanchitoJog
//

namespace octet {
	class PanchitoObject{
		//where is Panchito! (this position matches with the bottom of the hat, 
		//						and the top of his body) CENTER of Panchito
		mat4t modelToWorld;
		scene_node *panchitoNode;
		scene_node *hatNode;
		scene_node *bodyNode;

		//First version, Panchito is simple right now! (2 cylinders (hat) + 1 box (body)!)
		float halfWidth;
		float halfHeight;

		//Panchito is easy to draw, he has two colors!
		vec4 colorHat;
		vec4 colorBody;

		//Panchito is jogging! These variables defines its jumping and tilting
		float jump;
		float tilt;
		int jumpTics;
		int directionJump;
		int tiltTics;
		int directionTilt;

	public:
		PanchitoObject() {
			panchitoNode = 0;
			hatNode = 0;
			bodyNode = 0;
		}

		~PanchitoObject() {
			delete panchitoNode;
			delete hatNode;
			delete bodyNode;
		}

		//This is called to initializate Panchito in a position
		void init(float x, float y, float z, float scale){
			//He will have a fixed color, and fix sizes, so we init them
			colorHat = vec4(0,0,0,1); //the hat is black!
			colorBody = vec4(1,0,0,1); //the body is red!!
			halfWidth = 1.f*scale;
			halfHeight = 1.f*scale;
			//and now we take x and y (position of Panchito), that can be fixed outside
			modelToWorld.loadIdentity();
			modelToWorld.translate(x, y, z);
			//This is the node that holds all panchito. hat, body, arms, legs ar children of that node
			if(panchitoNode == 0)
				panchitoNode = new scene_node(modelToWorld,atom_);
			modelToWorld.loadIdentity();
			if(hatNode == 0)
				hatNode = new scene_node(modelToWorld,atom_);
			if(hatNode == 0)
				bodyNode = new scene_node(modelToWorld,atom_);
			//this variables are for Panchito's jogging
			jump = 0.01f;
			jumpTics = 5;
			directionJump = -1;
			tilt = 0.01f;
			tiltTics = 10;
			directionTilt = 1;
		}

		//This method load Panchito to the current scene
		void LoadToScene(ref<visual_scene> scene){
			//First the materials
			material *materialBody = new material(colorBody);
			material *materialHat = new material(colorHat); 
			
			//Then Panchito itself
			//First the body
			mesh_sphere *body = new mesh_sphere(vec3(0,0,0),halfHeight);
			mat4t bodyPosition;
			bodyPosition.loadIdentity();
			bodyPosition.translate(0,-halfWidth*0.5f,0);
			scene_node *nodeBody = new scene_node(bodyPosition,atom_);
			panchitoNode->add_child(nodeBody);
			scene->add_mesh_instance(new mesh_instance(nodeBody,body,materialBody));

			//Then the hat (bottom and top)
			mesh_cylinder *hatBottom = new mesh_cylinder(zcylinder(),mat4t(),30);
			mat4t hatBottomPosition;
			hatBottomPosition.loadIdentity();
			hatBottomPosition.translate(0,.2f*halfHeight,0);
			hatBottomPosition.scale(4, .025f, 4);
			hatBottomPosition.rotate(90, 1, 0, 0);
			scene_node *nodeHatBottom = new scene_node(hatBottomPosition,atom_);
			hatNode->add_child(nodeHatBottom);
			scene->add_mesh_instance(new mesh_instance(nodeHatBottom,hatBottom,materialHat));
			
			mesh_cylinder *hatTop = new mesh_cylinder(zcylinder(), mat4t(), 30);
			mat4t hatTopPosition;
			hatTopPosition.loadIdentity();
			hatTopPosition.translate(0, .8f*halfHeight, 0);
			hatTopPosition.scale(2.5f, 3, 2.5f);
			hatTopPosition.rotate(90, 1, 0, 0);
			scene_node *nodeHatTop = new scene_node(hatTopPosition,atom_);
			hatNode->add_child(nodeHatTop);
			hatNode->rotate(10,vec3(1,0,0));
			scene->add_mesh_instance(new mesh_instance(nodeHatTop,hatTop,materialHat));

			panchitoNode->add_child(hatNode);
			scene->add_child(panchitoNode);
		}

		void animateJogging(){
			panchitoNode->translate(vec3(0,directionJump*jump,0));
			scene_node * child = panchitoNode->get_child(1);
			child->translate(vec3(0,directionJump*jump*0.5f,0));
			child = 0;
			--jumpTics;
			if(jumpTics == 0){
				jumpTics = 10;
				directionJump = -directionJump;
			}
			panchitoNode->translate(vec3(directionTilt*tilt,0,0));
			--tiltTics;
			if(tiltTics == 0){
				tiltTics = 20;
				directionTilt = -directionTilt;
			}
		}
	};
}