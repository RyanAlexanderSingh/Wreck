////////////////////////////////////////////////////////////////////////////////
//
// Space Monkey Mafia - Procedural Jungle Generation
//
// URL: https://www.doc.gold.ac.uk/~mas01at/proj2013/?page_id=4
//
// Artemis Tsouflidou     [ma303at]
// Brian Gatt             [ma301bg]
// Stefana Ovesia         [ma301so]
// Gustavo Arcanjo Silva  [io301gas]
//
////////////////////////////////////////////////////////////////////////////////

#include "../../octet.h"

// Reference: http://stackoverflow.com/questions/1644868/c-define-macro-for-debug-printing
#ifdef _DEBUG
  #define DEBUG_PRINT(fmt, ...) do{ printf(fmt, __VA_ARGS__); } while(0)
#endif
#ifndef _DEBUG
  #define DEBUG_PRINT(fmt, ...)
#endif

namespace octet {
namespace jungle {

  /** 
   *  Command Line Arguments
   */
  class CmdLineArgs {
  private:
    const char* _directory;
    int _seed;
    float _cycle;
    int _modelCount;
    
  public:
    CmdLineArgs() :
      _directory("../../"),
      _seed(-1),
      _cycle(30.f),
      _modelCount(-1) {
    };

    /**
      *  @return the --directory|-d command line argument if specified or NULL
      **/
    const char* getDirectory() const {
      return _directory;
    };
      
    /**
      *  @return the --seed|-s command line argument if specified or -1 if not
      **/
    int getSeed() const {
      return _seed;
    };
      
    /**
      *  @return the --cycle|-c command line argument if specified or the default value
      **/
    float getCycle() const {
      return _cycle;
    };
    
    /**
      *  @return the --model|-m command line argument if specified or the default value
      **/
    int getModelCount() const {
      return _modelCount;
    };

    static CmdLineArgs parse(int argc, char* argv[]) {
      CmdLineArgs args;
        
      int i = 0;
      while (i < argc) {
        if (
            (::strcmp(argv[i], "--directory") == 0 || ::strcmp(argv[i], "-d") == 0) &&
            argc >= i + 1
          ) {
          args._directory = argv[++i];
        } else if (
            (::strcmp(argv[i], "--seed") == 0 || ::strcmp(argv[i], "-s") == 0) &&
            argc >= i + 1
          ) {
          args._seed = ::atoi(argv[++i]);
        } else if (
            (::strcmp(argv[i], "--cycle") == 0 || ::strcmp(argv[i], "-c") == 0) &&
            argc >= i + 1
          ) {
          args._cycle = (float) ::atof(argv[++i]);
        } else if (
            (::strcmp(argv[i], "--model") == 0 || ::strcmp(argv[i], "-m") == 0) &&
            argc >= i + 1
          ) {
          args._modelCount = ::atoi(argv[++i]);
        }

        ++i;
      };

      return args;
    };
  };

} // namespace jungle
} // namespace octet

#include "jungle.h"

int main(int argc, char **argv) {
  octet::jungle::CmdLineArgs args = octet::jungle::CmdLineArgs::parse(argc, argv);

  octet::app_utils::prefix(args.getDirectory());
  octet::app::init_all(argc, argv);

  octet::jungle_app app(argc, argv, args);
  app.init();

  octet::app::run_all_apps();
}

#undef DEBUG_PRINT