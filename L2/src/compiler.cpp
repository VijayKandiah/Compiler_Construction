#include "parser.h"
#include "code_generator.h"
#include "liveness.h"
#include "spiller.h"
#include "graph_color.h"

using namespace std;

void print_help (char *progName){
  std::cerr << "Usage: " << progName << " [-v] [-g 0|1] [-O 0|1|2] [-s] [-l 0|1] [-i] SOURCE" << std::endl;
  return ;
}

int main(
  int argc, 
  char **argv
  ){
  auto enable_code_generator = true;
  auto spill_only = false;
  auto interference_only = false;
  int32_t liveness_only = 0;
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
  while ((opt = getopt(argc, argv, "vg:O:sl:i")) != -1) {
    switch (opt){

      case 'l':
        liveness_only = strtoul(optarg, NULL, 0);
        break ;

      case 'i':
        interference_only = true;
        break ;

      case 's':
        spill_only = true;
        break ;

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
  
  L2::Program p;

  if (spill_only){
    /* 
     * Parse an L2 function and the spill arguments.
     */
 	p = L2::parse_spill(argv[optind]);
  } else if (liveness_only){

    /*
     * Parse an L2 function.
     */
  	p = L2::parse_function(argv[optind]);


  } else if (interference_only){

    /*
     * Parse an L2 function.
     */
  	p = L2::parse_function(argv[optind]);

  } else {

    /* 
     * Parse the L2 program.
     */
    p = L2::parse_file(argv[optind]);
  }

  if (verbose){
    cout<< "(" << p.entryPointLabel <<endl;
    for (auto f : p.functions){
      cout<< "\t (" << f->name<<endl;
      cout<< "\t\t" << f->arguments << " " << f->locals <<endl;
      for (auto i : f->instructions)
        cout << "\t\t" << i->Inst <<endl;
      cout<< "\t )" <<endl;
    }
    cout<< ")" <<endl;
  }
  /*
   * Special cases.
   */
  if (spill_only){
    /*
     * Spill.
     */
  	for (auto f: p.functions){
  		if(verbose){
		   	cout<< "(" << f->name<<endl;
			cout<< "\t" << f->arguments << " " << f->locals <<endl;
			for (auto i : f->instructions)
				cout << "\t" << i->Inst <<endl; 
			cout<< ")" <<endl;
			cout<<f->spilled_variable<<endl;
			cout<<f->spill_replace<<endl;
		}
  		L2::spill_print(f);
	}
    return 0;
  }



  if (liveness_only){
  /*
   * Liveness test.
   */
    for (auto f: p.functions){
      L2::liveness_print(f);
      if (verbose){
          cout<< "\t (" << f->name<<endl;
          cout<< "\t\t" << f->arguments << " " << f->locals <<endl;
          for (auto i : f->instructions){
              cout << "\t\t" << i->Inst <<endl;
          }
          cout<< "\t )" <<endl;
      }
    }
    return 0;
  }


  if (interference_only){
    /*
    * Interference graph test.
    */
    for (auto f: p.functions){
    	L2::interference_test(f);
    }
    return 0;
  }

  /*
   * Generate the target code.
   */
  if (enable_code_generator){


    L2::generate_code(p, verbose); //generate L1 code

    return 0;
  }

  return 0;
}
