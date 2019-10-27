#include "parser.h"
#include "code_generator.h"


using namespace std;

void print_help (char *progName){
  std::cerr << "Usage: " << progName << " [-v] [-g 0|1] [-O 0|1|2]   SOURCE" << std::endl;
  return ;
}

int main(
  int argc, 
  char **argv
  ){
  auto enable_code_generator = true;
  int32_t optLevel = 0;

  /* 
   * Check the compiler arguments.
   */
  bool verbose = false;
  if( argc < 2 ) {
    print_help(argv[0]);
    return 1;
  }
  int32_t opt;
  while ((opt = getopt(argc, argv, "vg:O:")) != -1) {
    switch (opt){

      case 'O':
        optLevel = strtoul(optarg, NULL, 0);
        break ;

      case 'g':
        enable_code_generator = (strtoul(optarg, NULL, 0) == 0) ? false : true ;
        break ;

      case 'v':
        verbose = true;
        break ;

      default:
        print_help(argv[0]);
        return 1;
    }
  }

  /*
   * Parse the input file.
   */
  
  IR::Program p;
    /* 
     * Parse the IR program.
     */
    p = IR::parseFile(argv[optind]);

  if (verbose){
    cout<< "(" <<endl;
    for (auto f : p.functions){
      cout<< "\t (" << f->name<<endl;
      for (auto bb : f->bblocks){
        cout<< "\t\t"<<bb->entry_label->Inst<<"\t"<<endl;
        for(auto i : bb->instructions){
          cout << "\t\t" << i->Inst<<"\t"<<endl;
        }
        cout<< "\t\t"<<bb->exit->Inst<<"\t"<<endl;
      }
      cout<< "\t )" <<endl;
    }
    cout<< ")" <<endl;
  }




  /*
   * Generate the target code.
   */
  if (enable_code_generator){
    //TODO

    IR::generateCode(p); 

    return 0;
  }

  return 0;
}
