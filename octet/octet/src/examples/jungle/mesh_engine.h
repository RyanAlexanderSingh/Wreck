////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Triangle example: The most basic OpenGL application
//
// Level: 0
//
// Demonstrates:
//   Basic framework app
//   Shaders
//   Basic Matrices
//
//


namespace octet {


  class MeshEngine{
	  ReadFile file;
	  HandleMesh *mesh;
    visual_scene *cur_scene;

	  // this array consists the number of trees that should be generated of each species of tree
	  dynarray<int> numberOfSpecies;
	  dynarray<int> trees; // type of trees
	  string finalAxiom;
	  time_t timer;
	  enum Type { BUSH, LEAFTREE, BRANCHTREE, A } treeType;
    enum Terrain { SAND, GRASS, ROCK };
    Catmull_Rom_Spline flower;
    octet::RandomNumberGenerator* _rand;

    // Barrier which limits collada models
    int _colladaModelCount;

    bool shouldGenerateColladaModel() const {
      // In case a negative _colladaModelCount is defined, it implies
      // infinite model generation, else if a postive value is defined,
      // generation should stop when 0 is reached.
      return _colladaModelCount != 0;
    };

  public:
	  MeshEngine () :
      _colladaModelCount(-1) {
    }

    void reset(int colladaModelCount) {
      _colladaModelCount = colladaModelCount;
    };

	  void init ( dynarray<int> &num, dynarray<int> &types, HandleMesh &mesh_, dynarray<vec3> &pos, visual_scene* scene ) {
		  mesh = &mesh_;
      cur_scene = scene;
		  //mesh.init();
		  numberOfSpecies = num;
		  trees = types;
		  //srand((unsigned int) time(NULL));
      _rand = (&octet::LinearCongruential::getInstance());

      flower.init(mesh_);
		 
		  createObjects( pos );
	  }

	  // read a file depending on the input/region
	  void readFile() {
		  file.reset();
		  if (treeType == BUSH)
			  file.openFiles(app_utils::get_path("assets/jungle/source1.txt")); 
		  else if (treeType == BRANCHTREE)
			  file.openFiles(app_utils::get_path("assets/jungle/source2.txt")); 
		  else if (treeType == LEAFTREE)
			  file.openFiles(app_utils::get_path("assets/jungle/source3.txt")); 
		  else if (treeType == A)
			  file.openFiles(app_utils::get_path("assets/jungle/source4.txt")); 
	  }

	  void translateTrees ( int i ) {
		  if ( trees[i] == BUSH )
			  treeType = BUSH;
		  else if ( trees[i] == BRANCHTREE)
			  treeType = BRANCHTREE;
		  else if ( trees[i] == LEAFTREE)
			  treeType = LEAFTREE;
		  else if ( trees[i] == A)
			  treeType = A;
	  }

    int terrainByHeight ( float y ) {
      // y < -0.5 = sand
      if (y < -0.5) return SAND;
      // y > -0.5 & < 1.5 = grass
      if (y < 1.5) return GRASS;
      // y > 1.5 = rock
      else return ROCK;
    }

	  void createObjects ( dynarray<vec3> &pos ) {
		  translateTrees(0);
		  readFile();
		  createSequence ();

      dynarray<Branch> tree;

		  for( unsigned int j = 0; j < numberOfSpecies.size(); j++) { // for every input in numberOfSpecies array which actuall says how many differenst species of trees we have
			  for ( int i = 0; i < numberOfSpecies[j]; i++){ // number of trees of that species
				  mesh->loadNewObject(pos[i].x(), pos[i].y(), pos[i].z(), 0.5f, 3.5f );

          // Small chance to have something other than a procedurally generated tree
          if (shouldGenerateColladaModel() && randf(_rand,0.0f,1.0f) < 0.2f) {
            switch(terrainByHeight(pos[i].y())) {
              case(GRASS):
                addColladaToScene("assets/jungle/collada/OneGiraffeLOD1.dae", pos[i]);
              break;
              case(SAND):
                addColladaToScene("assets/jungle/collada/bananatree.dae", pos[i]);
              break;
              case(ROCK):
                addColladaToScene("assets/jungle/collada/leaf%20grass.dae", pos[i]);
              break;
            }
          } else {
            // Clear the tree dynamic array
            tree.reset();
            // Create a tree structure and store the result in the 'tree' parameter
				    createTreeStructure(finalAxiom, tree);
            addToMesh(tree);
          }

          // Have flowers/grass on the grass terrain
          if (shouldGenerateColladaModel() && terrainByHeight(pos[i].y()) == GRASS) {
            // Tree or grass
            if (randf(_rand,0.0f,1.0f) < 0.5f) {
              // Random amount of flowers, 1 to 3
              for (int k = rand()%3; k>0; k--) {
                // Position the flower in a random toroid centered around the tree
                // with outer radius 4 and inner radius 2
                vec3 randVec = vec3(randf(_rand,-4.0f,4.0f),0.0f, randf(_rand,-4.0f,4.0f));
                while ((randVec-pos[i]).length() < 2.0f) randVec = vec3(randf(_rand,-4.0f,4.0f),0.0f, randf(_rand,-4.0f,4.0f));
                vec3 collPos = pos[i]+randVec;
                float randomN = randf(_rand,0.0f,1.0f);
                if (randomN < 0.25f) {
                  addColladaToScene("assets/jungle/collada/plant%20m.dae", collPos);
                } else if (randomN < 0.5f) {
                  addColladaToScene("assets/jungle/collada/plant%20m2.dae", collPos);
                } else {
                  addColladaToScene("assets/jungle/collada/flower1.dae", collPos);
                }
              }
            } else {
              // Position the grass region beneath the trees
              addColladaToScene("assets/jungle/collada/leaf%20grass(region).dae", pos[i]);
            }
          }

          // Have a few rocks on the sand or grass on grass terrain 
          if (shouldGenerateColladaModel() && i > 0) {
            // Position between two trees
            vec3 collPos = (pos[i]+pos[i-1])/2;
            // Rocks on the sand
            if (terrainByHeight(collPos.y()) == SAND && randf(_rand,0.0f,1.0f) < 0.2f) {
              if (randf(_rand,0.0f,1.0f) < 0.5f) {
                addColladaToScene("assets/jungle/collada/rock1.dae", collPos);
              } else {
                addColladaToScene("assets/jungle/collada/rock2.dae", collPos);
              }
            // Grass on the grass
            } else if (terrainByHeight(collPos.y()) == GRASS) {
              addColladaToScene("assets/jungle/collada/trop_grass(region).dae", pos[i]);
            }
          }
				  
          //mesh->loadNewObject(pos[i].x(), pos[i].y(), pos[i].z()+5, 0.5f, 3.5f );
          //manageRendering(finalAxiom);

				  if ( file.isStochastic() ) {
					  readFile();
					  createSequence ();
				  }
			  }
			  if ( j < numberOfSpecies.size() - 1 ) {
				  translateTrees(j+1);
				  readFile();
				  createSequence ();
			  }
		  }
		  //mesh.meshReady();
	  }

	  // translate a sequence in a tree
    // DEPRECATED
	  void manageRendering ( string axiom ) {
			for ( int i = 0; i < axiom.size(); i++){
				if ( file.existInVariables(axiom[i]) ) {  
					if ( axiom[i] == 'F' ) {
						mesh->pushCube();
						mesh->translate( 0.0f, mesh->getHeight(), 0.0f );					
					}
					// it will cause problem the leaf height because it is not the same as branch height and here
					// we translate brunch height
					else if (axiom[i] == 'X' ) {
            flower.set( static_cast<float>(rand()%2+1)/2.0f, 10, static_cast<float>(rand()%1+1)/2.0f, 3, "rose", 120.0f, mesh->getModelToWorldMatrix(), static_cast<float>(rand()%4+1), static_cast<float>(rand()%2+1), 0.0f );  // noise: 4, 2, 0
            flower.set( static_cast<float>(rand()%3+1)/2.0f, 10, static_cast<float>(-rand()%1+1)/2.0f, 1, "leaf", 45.0f, mesh->getModelToWorldMatrix(), static_cast<float>(rand()%2+1), static_cast<float>(rand()%1+1), 0.0f );  // noise: 2, 1.5, 0

		
						//mesh->pushLeafs();
						//mesh.translate( 0.0f, mesh.getHeight() / 2.0f, 0.0f );
					}	
          else if (axiom[i] == 'P' ) {
            flower.set( 2.0f, 10, 0.5f, 1, "leaf", 0.0f, mesh->getModelToWorldMatrix(), 1.0f, 1.0f,  .0f );
            flower.set(  1.0f, 10, 0.5f, 1, "leaf", 50.0f, mesh->getModelToWorldMatrix(), 1.0f, 2.0f,  .0f );
            flower.set(  1.0f, 10, 0.5f, 1, "leaf", -50.0f, mesh->getModelToWorldMatrix(), 1.0f, 2.0f,  .0f );
            flower.set(  1.0f, 10, 0.5f, 1, "leaf", 100.0f, mesh->getModelToWorldMatrix(), 1.0f, 2.0f,  .0f );
            flower.set(  0.5f, 10, 0.5f, 1, "leaf", -100.0f, mesh->getModelToWorldMatrix(), 1.0f, 2.0f,  .0f );
          }
          else if ( axiom[i] == 'T' ) {
            flower.set( 1.0f, 10, 0.0f, 3, "rose", 120.0f, mesh->getModelToWorldMatrix(), 4.0f, 2.0f, .0f );
            flower.set( 0.5f, 10, 0.5f, 1, "rose", 45.0f, mesh->getModelToWorldMatrix(), 4.0f, 2.0f, -1.0f );
            flower.set( 0.5f, 10, 0.5f, 1, "rose", -45.0f, mesh->getModelToWorldMatrix(), 4.0f, 2.0f, -1.0f );
            flower.set( 0.5f, 10, 0.5f, 1, "rose", 180.0f, mesh->getModelToWorldMatrix(), 4.0f, 2.0f, -1.0f );
            //flower.set(  4, 10, 0.0f, 1, "leaf", 45.0f, mesh->getModelToWorldMatrix(), 2.0f, 1.5f,  1.0f );
          }

				}
				else if ( axiom[i] == '+' ) {
					mesh->rotateZ( file.getAngle());
				}
				else if ( axiom[i] == '-' ) {
					mesh->rotateZ( -file.getAngle());
				}
				else if ( axiom[i] == '[' ) {
					mesh->pushModelMatrix();
				}
				else if ( axiom[i] == ']' ) {
					mesh->getModelToWorldMatrix() = mesh->getLastModelToWorldmatrix();
					mesh->popModelMatrix();
				}
				else if ( axiom[i] == '^' ) {
					mesh->rotateY( -file.getAngle() );
				}
				else if ( axiom[i] == '&' ) {
					mesh->rotateY( file.getAngle() );
				}
				else if ( axiom[i] == '<' ) {
					mesh->rotateX( file.getAngle() );
				}
				else if ( axiom[i] == '>' ) {
					mesh->rotateX( -file.getAngle() );
				}
				else if ( axiom[i] == '|' ) {
					mesh->rotateX( 180.0f );
				}
			}
		}

    // Flower data structure that will be used to draw a flower
    struct FlowerStruct {
      char type;
      mat4t position;
    };

    // Branch data structure that will be used to draw a tree
    struct Branch {
      // Size of the branch
      int size;
      // http://en.wikipedia.org/wiki/Strahler_number
      // Related to the initial radius of the branch
      int strahlerNumber;
      // Related to the final radius of the branch
      int childBiggestStrahlerNumber;
      // Who is it linked to
      int parentID;
      //dynarray<int> childrenIDs;
      // Where is its starting position
      mat4t transform0;
      // Where is its ending position and rotation
      mat4t transform1;
      // Where are its flowers positioned
      dynarray<FlowerStruct> flowers;
    };

    // It reads the L-System final axiom and generates
    // a tree structure made out of branches
    void createTreeStructure(string axiom, dynarray<Branch>& tree) {
      // We start at the base of the tree
      int currentBranch = 0;

      // The base branch has this characteristics
      Branch initial;
      // Start with no size
      initial.size = 0;
      // This can only be determined after the whole tree is available
      // Ending nodes (leaves) have strahlerNumber == 1
      initial.strahlerNumber = 1;
      // And they don't have children, childBiggestStrahlerNumber == 0
      initial.childBiggestStrahlerNumber = 0;
      // The first branch don't have a parent
      initial.parentID = -1;
      // The initial position is set to the initial position
      initial.transform0 = mesh->getModelToWorldMatrix();
      initial.transform1 = mesh->getModelToWorldMatrix();
      
      // Add the root branch to the tree
      tree.push_back(initial);

      for ( int i = 0; i < axiom.size(); i++ ) {
        //printf("Handle %c B%d sz%d\n",axiom[i], currentBranch, tree.size());
				if ( file.existInVariables(axiom[i]) ) {  
					if ( axiom[i] == 'F' ) {
            // grow current branch
            tree[currentBranch].size++;
						// modify ending transform accordingly
						tree[currentBranch].transform1.translate( 0.0f, mesh->getHeight(), 0.0f );					
					}
					else if (axiom[i] == 'X' || axiom[i] == 'P' || axiom[i] == 'T' || axiom[i] == 'L') {
            // Add a leaf to the current branch using the current rotation
            FlowerStruct flowerS;
            flowerS.position = tree[currentBranch].transform1;
            flowerS.type = axiom[i];
            tree[currentBranch].flowers.push_back(flowerS);
					}
				}
				else if ( axiom[i] == '+' ) {
          // modify current branch rotation
					tree[currentBranch].transform0.rotateZ( file.getAngle());
          tree[currentBranch].transform1.rotateZ( file.getAngle());

				}
				else if ( axiom[i] == '-' ) {
          // modify current branch rotation
					tree[currentBranch].transform0.rotateZ( -file.getAngle());
          tree[currentBranch].transform1.rotateZ( -file.getAngle());

				}
				else if ( axiom[i] == '[' ) {
					// Create a new branch which starting position is the same as its parent end
          initial.transform0 = tree[currentBranch].transform1;
          initial.transform1 = tree[currentBranch].transform1;
          // The parentID is set to the current branch, before it changes
          initial.parentID = currentBranch;
          // The next branch is added as a child of the current branch
          //tree[currentBranch].childrenIDs.push_back(tree.size());
          // The current branch changes to the last element of the tree
          currentBranch = tree.size();
          // The branch is added to the tree
          tree.push_back(initial);

				}
				else if ( axiom[i] == ']' ) {
					// The current branch is the parent of the last current branch
          // Notice that this makes the transform go back to the parent's transform as well
          currentBranch = tree[currentBranch].parentID;

				}
				else if ( axiom[i] == '^' ) {
          // modify current branch rotation
					tree[currentBranch].transform0.rotateY( -file.getAngle() );
          tree[currentBranch].transform1.rotateY( -file.getAngle() );

				}
				else if ( axiom[i] == '&' ) {
          // modify current branch rotation
					tree[currentBranch].transform0.rotateY( file.getAngle() );
          tree[currentBranch].transform1.rotateY( file.getAngle() );

				}
				else if ( axiom[i] == '<' ) {
          // modify current branch rotation
					tree[currentBranch].transform0.rotateX( file.getAngle() );
          tree[currentBranch].transform1.rotateX( file.getAngle() );

				}
				else if ( axiom[i] == '>' ) {
          // modify current branch rotation
					tree[currentBranch].transform0.rotateX( -file.getAngle() );
          tree[currentBranch].transform1.rotateX( -file.getAngle() );

				}
				else if ( axiom[i] == '|' ) {
          // modify current branch rotation
					tree[currentBranch].transform0.rotateX( 180.0f );
          tree[currentBranch].transform1.rotateX( 180.0f );

				}
			}

      // Traverse the tree using post-order method to determine strahler number
      for ( unsigned int i = tree.size()-1; i > 0; i-- ) {
        // Continue the for loop if this branch only has leaves
        if (tree[i].size == 0) continue;

        int parentID = tree[i].parentID;
        if ( tree[parentID].strahlerNumber < tree[i].strahlerNumber ) {
          // Parent's number must be at least equal to its child
          tree[parentID].strahlerNumber = tree[i].strahlerNumber;
        } else if ( tree[parentID].strahlerNumber == tree[i].strahlerNumber ) {
          for (unsigned int j = tree.size()-1; j > 0; j-- ) {
            // Continue the for loop if i and j are the same branches
            // Or if this branch only has leaves
            if (i == j || tree[j].size == 0) continue;
            // Check if i and j are siblings
            if ( parentID == tree[j].parentID ) {
              // And have the same number
              if ( tree[i].strahlerNumber == tree[j].strahlerNumber ) {
                // The parents number must be one number bigger then its children's
                // if at least two have the same number
                tree[parentID].strahlerNumber = tree[i].strahlerNumber+1;
                tree[parentID].childBiggestStrahlerNumber = tree[i].strahlerNumber;
                break;
              } else if ( tree[i].strahlerNumber < tree[j].strahlerNumber ) {
                // Or be equal to the child's biggest number
                tree[parentID].strahlerNumber = tree[j].strahlerNumber;
                tree[parentID].childBiggestStrahlerNumber = tree[j].strahlerNumber;
                break;
              }
              //printf("     %d and %d are siblings because of %d\n", i, j, parentID);
            }
          }
        }
        //printf("BRANCH %d, size %d, parent %d, leaves %d, SN %d\n", i, tree[i].size, tree[i].parentID, tree[i].leaves.size(), tree[i].strahlerNumber);
      }
      //printf("BRANCH %d, size %d, parent %d, leaves %d, SN %d\n", 0, tree[0].size, tree[0].parentID, tree[0].leaves.size(), tree[0].strahlerNumber);
      //system("PAUSE");
    }

    void addToMesh (dynarray<Branch>& tree) {
      for ( unsigned int i = 0; i < tree.size(); i++ ) {
        // If the size is 0, this branch only has leaves
        if (tree[i].size > 0) {
          mesh->position(tree[i].transform0);
          // The number of points in the circle matches the rotation angle so that y-rotated cones maintain alignment
          int pointsPerCircle = 360 / ((int) file.getAngle());
          // Minimum of 10 points per circle
          if (pointsPerCircle < 10) pointsPerCircle *= (int)10/pointsPerCircle;
          mesh->pushCone(tree[i].strahlerNumber, tree[i].childBiggestStrahlerNumber, tree[i].size, pointsPerCircle);
        }
        for ( unsigned int j = 0; j < tree[i].flowers.size(); j++ ) {
          addFlowerToMesh(tree[i].flowers[j]);
        }
      }
    }

    void addColladaToScene (const char *filename, vec3 &pos) {
      --_colladaModelCount;

      // Don't add if too many mesh instances
      if (cur_scene->get_num_mesh_instances() > 90) return;

      FILE *file = fopen(app_utils::get_path(filename), "rb");
      char buf[8];
      visual_scene *app_scene = 0;
      if (file && fread(buf, 1, sizeof(buf), file) && !memcmp(buf, "octet", 5)) {
        fclose(file);
      } else {
        if (file) fclose(file);
        collada_builder builder;
        if (!builder.load_xml(filename)) {
          printf("\nERROR: could not open %s\nThis is likely a problem with paths.\n", filename);
          return;
        }

        resource_dict dict;
        builder.get_resources(dict);
        app_scene = dict.get_visual_scene(builder.get_default_scene());

        assert(app_scene);

        //app_scene->create_default_camera_and_lights();

        //dict.set_active_scene(app_scene);

        //visual_scene *cur_scene = dict.get_active_scene();

        DEBUG_PRINT("Add a collada %s\n", filename);
        app_scene->loadIdentity();
        app_scene->translate(pos);
        app_scene->rotate(randf(_rand,0.0f,360.0f),vec3(0,1,0));
        // Get every mesh in the scene of the collada file
        for (unsigned i = 0; i != app_scene->get_num_mesh_instances(); ++i) {
          DEBUG_PRINT("Mesh %d: \n",i);
          // Get the mesh instance from the collada scene
          mesh_instance *mi = app_scene->get_mesh_instance(i);
          // Using custom collada alpha bump shader
          // Add mesh instance to current scene
          cur_scene->add_mesh_instance(mi);
        }
      }
    }

    void addColladaToMesh (const char *filename) {
      FILE *file = fopen(app_utils::get_path(filename), "rb");
      char buf[8];
      visual_scene *app_scene = 0;
      if (file && fread(buf, 1, sizeof(buf), file) && !memcmp(buf, "octet", 5)) {
        fclose(file);
      } else {
        if (file) fclose(file);
        collada_builder builder;
        if (!builder.load_xml(filename)) {
          printf("\nERROR: could not open %s\nThis is likely a problem with paths.\n", filename);
          return;
        }

        resource_dict dict;
        builder.get_resources(dict);
        app_scene = dict.get_visual_scene(builder.get_default_scene());

        assert(app_scene);

        //app_scene->create_default_camera_and_lights();

        //dict.set_active_scene(app_scene);

        //visual_scene *cur_scene = dict.get_active_scene();

        DEBUG_PRINT("Add a collada %s\n", filename);
        // Get every mesh in the scene of the collada file
        for (unsigned i = 0; i != app_scene->get_num_mesh_instances(); ++i) {
          DEBUG_PRINT("Mesh %d: \n",i);
          // Get the mesh instance from the scene and the mesh from the instance
          mesh_instance *mi = app_scene->get_mesh_instance(i);

          Mesh octMesh = mesh->getMesh();

          scene::mesh *m = mi->get_mesh();

          unsigned j;
          unsigned k;

          // Make sure both drawing modes are the same
          if (m->get_mode() == GL_TRIANGLES) {
            for (k = 0; k != m->get_num_slots() ; ++k ) {
              // Add vertices
              if (m->get_attr(k) == attribute_pos) {
                for (j = 0; j != m->get_num_vertices() ; ++j) {
                  vec3 v = m->get_value(k,j);
                  mesh->pushVertex(v);
                  // TODO: fix?
                  // We assume that the index is in ascending order always
                  mesh->pushIndex();
                }
              }
              // Add UV
              else if (m->get_attr(k) == attribute_uv) {
                for (j = 0; j != m->get_num_vertices() ; ++j) {
                  vec4 v = m->get_value(k,j);
                  mesh->pushUV(v[0],v[1]);
                }
              }
              DEBUG_PRINT("%d Vertices\n",j);
              DEBUG_PRINT("%d Slot has attribute %d\n",k,m->get_attr(k));
            }
          }
          DEBUG_PRINT("%d Slots\n", k);
        }
      }

    }

    void addFlowerToMesh (FlowerStruct& flowerS) {
      if (flowerS.type == 'X') {
      	flower.set( static_cast<float>(rand()%2+1), 10, static_cast<float>(rand()%1+1), 3, "rose", 120.0f, flowerS.position, static_cast<float>(rand()%4+1), static_cast<float>(rand()%2+1), 0.0f );  // noise: 4, 2, 0
        flower.set( static_cast<float>(rand()%3+2), 10, static_cast<float>(-rand()%1+1), 1, "leaf", 45.0f, flowerS.position, static_cast<float>(rand()%2+1), static_cast<float>(rand()%1+1), 0.0f );  // noise: 2, 1.5, 0
      } else if (flowerS.type == 'P') {
        flower.set( 3, 10, 0.5f, 1, "leaf", 0.0f, flowerS.position, 2.0f, 5.0f,  -1.0f );
      } else if (flowerS.type == 'T') {
        flower.set( 2.0f, 10, 0.0f, 3, "rose", 120.0f, flowerS.position, 4.0f, 2.0f, .0f );
        flower.set( 1.0f, 10, 1.5f, 1, "rose", 45.0f, flowerS.position, 4.0f, 2.0f, -1.0f );
        flower.set( 1.0f, 10, 1.5f, 1, "rose", -45.0f, flowerS.position, 4.0f, 2.0f, -1.0f );
        flower.set( 1.0f, 10, 1.5f, 1, "rose", 180.0f, flowerS.position, 4.0f, 2.0f, -1.0f );
        //flower.set(  4, 10, 0.0f, 1, "leaf", 45.0f, mesh->getModelToWorldMatrix(), 2.0f, 1.5f,  1.0f );
      }
      else if ( flowerS.type == 'L' ) {
        flower.set( 2.0f, 10, 3.0f, 4, "rose", 90.0f, flowerS.position, 2.0f, 2.0f, 1.0f );        
        flower.set( static_cast<float>(rand()%3+1), 10, static_cast<float>(-rand()%1+1), 1, "leaf", 45.0f, flowerS.position, static_cast<float>(rand()%2+1), static_cast<float>(rand()%1+1), 0.0f );  // noise: 2, 1.5, 0
      } else if (flowerS.type == 'P') {
        flower.set( static_cast<float>(rand()%3+1), 10, 0.5f, 1, "leaf", 0.0f, flowerS.position, static_cast<float>(rand()%2+1), static_cast<float>(rand()%5+1),  -1.0f );
      } else if (flowerS.type == 'T') {
        flower.set( 2.0f, 10, 0.5f, 3, "rose", 120.0f, flowerS.position, 4.0f, static_cast<float>(rand()%2+1), -0.5f );
        flower.set( 1.0f, 10, 1.0f, 1, "rose", 45.0f, flowerS.position, 4.0f, static_cast<float>(rand()%2+1), -1.0f );
        flower.set( 1.0f, 10, 1.0f, 1, "rose", -45.0f, flowerS.position, 4.0f, static_cast<float>(rand()%2+1), -1.0f );
        flower.set( 1.0f, 10, 1.0f, 1, "rose", 180.0f, flowerS.position, 4.0f, static_cast<float>(rand()%2+1), -1.0f );
        //flower.set(  4, 10, 0.0f, 1, "leaf", 45.0f, mesh->getModelToWorldMatrix(), 2.0f, 1.5f,  1.0f );
      }
      else if ( flowerS.type == 'L' ) {
        flower.set( static_cast<float>(rand()%3+1), 10, static_cast<float>(rand()%2+1), 4, "rose", 90.0f, flowerS.position, static_cast<float>(rand()%2+1), static_cast<float>(rand()%2+1), 1.0f );        
      }
    }

	  string chooseRule( int i, char c ) {
		  int temp = rand()%100;
		  if ( !file.isStochastic() ) // if not stochastic
			  return file.getRules(i);
		  else {                      // if it is stochastic
			  return file.selectRule(c, temp); // select rule depending on propabilities
		  }
		  
	  }

	  // replace its variable with the right rule
	  string ManageSequenceRules ( char temp ) {
		string s;
		s = &temp;
		for ( int i = 0; i < file.sizeOfVariables(); i++) {
			
			if ( temp == file.getVariables(i)) 
				return chooseRule( i, temp );
		}
	    if (temp == '\n')
			return "";
		else		
			return s;		
	}

	  // create the sequence that will be rendered
	  void createSequence () {
		  string axiom = file.getAxiom();
		  string seq;
		  for ( int i=0; i<file.getIterations(); i++) {
			  for ( unsigned int j=0; j<strlen(axiom); j++) {
				  seq += ManageSequenceRules(axiom[j]); //tree.getRules();
			  }
			  axiom = seq;
			  seq = "";
		  }
		  if (file.getIterations() <= 0)
			  axiom = "";
		  finalAxiom = axiom;
      //printf("FINAL: %s", axiom);
      //system("PAUSE");
	  }

	  void render ( texture_shader &shader, mat4t &cameraToWorld) {
		  mesh->render(shader, cameraToWorld);
	  }

  };


}
 


