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

  class ReadFile  {
    std::FILE *file;
    string axiom;
    // TODO: remove, unused
	  string constants;
	  int iterations;
	  float angle;
	  dynarray<char> var;
	  dynarray<string> rule;
    // TODO: remove, unused
	  dynarray<int> positions;
	  dynarray<float> probabilities;

	  enum { MAX_LINE = 30 };

    public:
      
    ReadFile () { }

	  // read the specified file
	  void openFiles ( const char *filename) {

		  file = fopen(filename, "r"); 
	      if ( file != NULL ) {
		    DEBUG_PRINT("file opened\n");
		    readData(file);
		    fclose(file);
	      }
	      else
		    DEBUG_PRINT("file error\n");
	  }
	
	  void readData ( FILE *in ) {
		
		  static char stackBuf[MAX_LINE];
		  char *buf = &(stackBuf[0]); 
		  while ( fgets( buf, MAX_LINE, in) != NULL ) {
			  if ( strncmp(buf, "axiom", 5) == 0) {
				  axiom = fgets( buf, MAX_LINE, in); 
			  }
			  else if ( strncmp(buf, "constants", 9) == 0) {
				  constants = fgets( buf, MAX_LINE, in);
			  }
			  else if ( strncmp(buf, "variables", 9) == 0) {
				  fgets( buf, MAX_LINE, in);
				  var.push_back(buf[0]);   // this thing has lots of trush inside!!!!!!!!!!!
			  }
			  else if ( strncmp(buf, "rules", 5) == 0) {
				  rule.push_back(fgets( buf, MAX_LINE, in));
			  }
			  else if ( strncmp(buf, "iterations", 10) == 0) {
				  iterations = atoi(fgets( buf, MAX_LINE, in));
			  }
			  else if ( strncmp(buf, "angle", 5) == 0) {
				  angle = (float)atof(fgets( buf, MAX_LINE, in));
			  }
			  else if (strncmp(buf, "positions", 8) == 0){
				  positions.push_back( atoi(fgets( buf, MAX_LINE, in)) );
				  positions.push_back( atoi(fgets( buf, MAX_LINE, in)) );
			  }
			  else if ( strncmp(buf, "probability", 5) == 0) {
				  probabilities.push_back((float)atof(fgets( buf, MAX_LINE, in)));
			  }
		  }
	  }


	  void reset () {		
		  var.reset();
		  rule.reset();
		  positions.reset();
		  probabilities.reset();
	  }

	  // select the right rule to replace a variable
	  string selectRule ( char c, int random ) {
		  float temp = 0.0f;

		  for ( unsigned int i = 0; i < var.size(); i++ ){
			  if (c == var[i]){
				  // check the probability value in that position ( i )
				  if ( i < probabilities.size() ) {
					  temp += probabilities[i]*100.0f;
					  if ( random <= temp)
						  return rule[i];
				  }
			  }
				
		  }
		  // if no rule was selected by probabilities then return the first one
		  // this happens when in one file a variable has multiple rules but the other variable doesnt
		  // so it has no probability and no rule is selected from the above code
		  for ( unsigned int i = 0; i < var.size(); i++ ) {
			  if (c == var[i])
				  return rule[i];
		  }
		  DEBUG_PRINT("Maybe there is an error, the first rule was selected\n");
		  return "";
	  }
	  // return true if a tree is stochastic
    bool isStochastic () {
      /* First approach
      //char temp = var[0]; // unused?
		  for ( unsigned int i = 0; i < var.size()-1; i++) {
			  for( unsigned int j = i; j < var.size()-1; j++) {
				  if ( var[i] == var[j+1] )
					  return true;
			  }
		  }
		  return false;
      */
      // Second approach
      // If there is anything in the probabilites array (it is not empty),
      // then there is stochasticity
      return !probabilities.empty();
	  }

	  int getPosition ( int i ) { return positions[i]; }
	  string getAxiom () { return axiom; }
	  float getAngle () { return angle; }
	  string getConstants () { return constants; }
	  char getVariables ( int temp ) { return var[temp]; }
	  string getRules ( int temp ) { return rule[temp]; }
	  int sizeOfVariables () { return var.size(); }
	  int getIterations () { return iterations; }
	  int& setIterations ( ) { return iterations; }
	  float& setAngle () { return angle; }
	  bool existInVariables ( char temp ) {
		  for ( int j = 0; j < sizeOfVariables(); j++){
			  if ( temp == getVariables(j))
				  return 1;
		  }
		  return 0;
	  }
  };
 
}


