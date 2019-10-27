#include "parser.h"



namespace pegtl = tao::TAO_PEGTL_NAMESPACE;


using namespace pegtl;
using namespace std;


namespace L3 {

  bool debug = 0;

  /*
   * Data required to parse
   */
  int inst_position = 0;
  std::vector<Variable *> parsed_operands;
  std::vector<std::string> parsed_operators;
  bool is_Func_only = false;
  /*
   * Grammar rules from now on.
   */
  struct name:
    pegtl::seq<
      pegtl::plus<
        pegtl::sor<
          pegtl::alpha,
          pegtl::one< '_' >
        >
      >,
      pegtl::star<
        pegtl::sor<
          pegtl::alpha,
          pegtl::one< '_' >,
          pegtl::digit
        >
      >
    > {};

  struct label:
    pegtl::seq<
      pegtl::one<':'>,
      name
    > {};

  struct number:
    pegtl::seq<
      pegtl::opt<
        pegtl::sor<
          pegtl::one< '-' >,
          pegtl::one< '+' >
        >
      >,
      pegtl::plus<
        pegtl::digit
      >
    >{};

  struct function_name:
    label {};

  

  struct comment:
    pegtl::disable<
      TAOCPP_PEGTL_STRING( "//" ),
      pegtl::until< pegtl::eolf >
    > {};

  struct seps:
    pegtl::star<
      pegtl::sor<
        pegtl::ascii::space,
        comment
      >
    > {};



  // new L3 vars
  struct L3_var:
    pegtl::seq<
      pegtl::one<'%'>,
      name
    >{};

    struct L3_vars:
    pegtl::star<
      pegtl::seq<
        seps,
        L3_var,
        seps,
        pegtl::opt<
          pegtl::one<','>
        >
      >
    >{};

    
  /*
   * Keywords.
   */
  // L3 keywords,
    // 1) registers and labels
  

  struct L3_t: 
    pegtl::sor<
      L3_var,
      number
    >{};


  struct L3_args:
    pegtl::star<
      pegtl::seq<
        seps,
        L3_t,
        seps,
        pegtl::opt<
          pegtl::one<','>
        >
      >
    >{};
  struct L3_s: 
    pegtl::sor<
      L3_t,
      label
    >{};

  struct L3_u: 
    pegtl::sor<
      L3_var,
      label
    >{};

  struct function_head:
    pegtl::seq<
        seps,
        function_name,
        seps,
        pegtl::one< '(' >,
        seps,
        pegtl::opt< L3_vars >,
        seps,
        pegtl::one< ')' >
    >{};

  // 2) Operators. 
  struct mov_op: TAOCPP_PEGTL_STRING( "<-" ){};
  struct add_op: TAOCPP_PEGTL_STRING( "+" ){};
  struct sub_op: TAOCPP_PEGTL_STRING( "-" ){};
  struct mul_op: TAOCPP_PEGTL_STRING( "*" ){};
  struct and_op: TAOCPP_PEGTL_STRING( "&" ){};
  struct lt_op: pegtl::one< '<' >{};
  struct le_op: TAOCPP_PEGTL_STRING( "<=" ){};
  struct eq_op: pegtl::one< '=' >{};
  struct gt_op: pegtl::one< '>' >{};
  struct ge_op: TAOCPP_PEGTL_STRING( ">=" ){};
  struct sal_op: TAOCPP_PEGTL_STRING( "<<" ){};
  struct sar_op: TAOCPP_PEGTL_STRING( ">>" ){};

  // 3) grouping some operators:
  struct arith_op: pegtl::sor<
  add_op,
  sub_op,
  mul_op,
  and_op,
  sal_op,
  sar_op
  >{};

  struct cmp_op: pegtl::sor<
    pegtl::seq<pegtl::at<le_op>,le_op>,
    pegtl::seq< pegtl::at<lt_op>,lt_op>,
    pegtl::seq< pegtl::at<eq_op>,eq_op>,
    pegtl::seq<pegtl::at<ge_op>,ge_op>,
    pegtl::seq< pegtl::at<gt_op>,gt_op>
  >{};

   // 4) more strings to parse
  struct str_call: TAOCPP_PEGTL_STRING( "call" ){};
  struct str_print: TAOCPP_PEGTL_STRING( "print" ){}; 
  struct str_allocate: TAOCPP_PEGTL_STRING( "allocate" ){}; 
  struct str_arrayerror: TAOCPP_PEGTL_STRING( "array-error" ){};
  struct str_br: TAOCPP_PEGTL_STRING( "br" ){};
  struct str_return: TAOCPP_PEGTL_STRING( "return" ){};
  struct str_load: TAOCPP_PEGTL_STRING( "load" ){};
  struct str_store: TAOCPP_PEGTL_STRING( "store" ){};
  struct str_define: TAOCPP_PEGTL_STRING( "define" ){};
  struct str_main: TAOCPP_PEGTL_STRING( ":main" ){};



 // 5) L3 Instructions
    struct L3_mov: pegtl::seq<L3_var, seps, mov_op, seps, L3_s >{};
    struct L3_op: pegtl::seq<L3_var, seps, mov_op, seps, L3_t, seps, arith_op, seps, L3_t >{};
    struct L3_cmp: pegtl::seq<L3_var, seps, mov_op, seps, L3_t, seps, cmp_op, seps, L3_t>{};
    struct L3_load: pegtl::seq<L3_var, seps, mov_op, seps, str_load, seps, L3_var>{};
    struct L3_store: pegtl::seq<str_store, seps, L3_var, seps, mov_op, seps, L3_s>{};
    struct L3_return: pegtl::seq<str_return>{};
    struct L3_return_var: pegtl::seq<str_return, seps, L3_t>{};
    struct L3_label: pegtl::seq<label>{}; // Label Inst
    struct L3_br1: pegtl::seq<str_br, seps, label>{};
    struct L3_br2: pegtl::seq<str_br, seps, L3_var, seps, label>{};
    struct L3_call1: pegtl::seq<str_call, seps, L3_u, seps, pegtl::one< '(' >, seps, L3_args, seps, pegtl::one< ')' >>{};
    struct L3_print: pegtl::seq<str_call, seps, str_print, seps, pegtl::one< '(' >, seps, L3_args, seps, pegtl::one< ')' >>{};
    struct L3_allocate: pegtl::seq<L3_var, seps, mov_op, seps, str_call, seps, str_allocate, seps, pegtl::one< '(' >, seps, L3_args, seps, pegtl::one< ')' >>{};
    struct L3_arrayerror: pegtl::seq<str_call, seps, str_arrayerror, seps, pegtl::one< '(' >, seps, L3_args, seps, pegtl::one< ')' >>{};
    struct L3_call2: pegtl::seq<L3_var, seps, mov_op, seps, str_call, seps, L3_u, seps, pegtl::one< '(' >, seps, L3_args, seps, pegtl::one< ')' >>{};

  struct Instruction_rule:
    pegtl::sor<
        pegtl::seq< pegtl::at<L3_cmp>, L3_cmp >,
        pegtl::seq< pegtl::at<L3_op>, L3_op >,
        pegtl::seq< pegtl::at<L3_mov>, L3_mov >,
        pegtl::seq< pegtl::at<L3_load>, L3_load >,
        pegtl::seq< pegtl::at<L3_store>, L3_store >,
        pegtl::seq< pegtl::at<L3_return_var>, L3_return_var >,
        pegtl::seq< pegtl::at<L3_return>, L3_return >,       
        pegtl::seq< pegtl::at<L3_label>, L3_label >,
        pegtl::seq< pegtl::at<L3_br1>, L3_br1 >,
        pegtl::seq< pegtl::at<L3_br2>, L3_br2 >,
        pegtl::seq< pegtl::at<L3_call1>, L3_call1 >,
        pegtl::seq< pegtl::at<L3_call2>, L3_call2 >,
        pegtl::seq< pegtl::at<L3_print>, L3_print >,
        pegtl::seq< pegtl::at<L3_arrayerror>, L3_arrayerror >,
        pegtl::seq< pegtl::at<L3_allocate>, L3_allocate >
    > {};

  struct Instructions_rule:
    pegtl::plus<
      pegtl::seq<
        seps,
        Instruction_rule,
        seps
      >
    > { };

  struct Function_rule:
    pegtl::seq<
      seps,
      str_define,
      seps,
      function_head,
      seps,
      pegtl::one< '{' >,
      seps,
      Instructions_rule,
      seps,
      pegtl::one< '}' >,
      seps
    > {};

  struct Functions_rule:
    pegtl::plus<
      seps,
      Function_rule,
      seps
    > {};


  struct grammar :
    pegtl::must<
      Functions_rule
    > {};

  /*
   * Actions attached to grammar rules.
   */
  template< typename Rule >
  struct action : pegtl::nothing< Rule > {};


  template<> struct action < function_name > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      auto newF = new Function();
      inst_position = 0;
      newF->name = in.string();
      p.functions.push_back(newF);
    }
  };

  template<> struct action < function_head > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        auto currentF = p.functions.back();
        while(!parsed_operands.empty()){
            auto op = parsed_operands.back();
            currentF->arguments.push_back(op); // reversed order of original arguments!
            parsed_operands.pop_back();
            }
        }
    };   


  //Operands = var | N | Label
  template<> struct action < L3_var > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        auto newOp = new Variable();
        newOp->op_type = VAR;
        newOp->str = in.string();
        parsed_operands.push_back(newOp);
        }
    };


  template<> struct action < number > {
    template< typename Input >
    static void apply( const Input & in, Program & p ){
      auto newOp = new Variable();
      newOp->op_type = NUM;
      long num = stoll(in.string());
      newOp->str = to_string(num); 
      parsed_operands.push_back(newOp);
    }
  };

  template<> struct action < label > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        auto newOp = new Variable();
        newOp->op_type = LABEL;
        newOp->str = in.string();
        parsed_operands.push_back(newOp);
    }
  };

  //PARSE OPERATORS, arith_op | cmp_op
  //arith_op 
  template<> struct action < arith_op > {
    template< typename Input >
    static void apply( const Input & in, Program & p ){
        parsed_operators.push_back(in.string());
        }
    };

 // cmp_op
  template<> struct action < cmp_op > {
    template< typename Input >
    static void apply( const Input & in, Program & p ){
        parsed_operators.push_back(in.string());
        }
    };


  //Instructions
  template<> struct action < L3_mov > {
    template< typename Input >
    static void apply( const Input & in, Program & p ){
        auto currentF = p.functions.back();
        auto src = parsed_operands.back();
        parsed_operands.pop_back();
        auto dest = parsed_operands.back();
        parsed_operands.pop_back();
        auto newInst = new Instruction();
        newInst->operands.push_back(dest);
        newInst->operands.push_back(src);
        newInst->op = MOV;
        newInst->pos = inst_position;
        inst_position++;
        newInst->Inst = in.string();
        if(debug) cout<< "MOV " << newInst->operands[0]->str + " <- " + newInst->operands[1]->str << endl;
        currentF->instructions.push_back(newInst);

        }
    };  

  template<> struct action < L3_op > {
    template< typename Input >
    static void apply( const Input & in, Program & p ){
        auto currentF = p.functions.back();
        auto src1 = parsed_operands.back();
        parsed_operands.pop_back();
        auto src2 = parsed_operands.back();
        parsed_operands.pop_back();
        auto dest = parsed_operands.back();
        parsed_operands.pop_back();
        auto newInst = new Instruction();
        newInst->operands.push_back(dest);
        newInst->operands.push_back(src2);
        newInst->operands.push_back(src1);
        std::string arith = parsed_operators.back();

        parsed_operators.pop_back();

        switch (arith[0]){
        case '+': newInst->op = ADD; break;
        case '-': newInst->op = SUB; break;
        case '*': newInst->op = MUL; break;
        case '&': newInst->op = AND; break;
        case '<': newInst->op = SAL; break;
        case '>': newInst->op = SAR; break;
        }
        newInst->Inst = in.string();
        newInst->pos = inst_position;
        inst_position++;
        if(debug) cout<< "ARITH_OP " + newInst->operands[0]->str + " <- " + newInst->operands[1]->str + " " + arith + " " + newInst->operands[2]->str << endl;
        currentF->instructions.push_back(newInst);
        }
    };

  template<> struct action < L3_cmp > {
    template< typename Input >
    static void apply( const Input & in, Program & p ){
        auto currentF = p.functions.back();
        auto src1 = parsed_operands.back();
        parsed_operands.pop_back();
        auto src2 = parsed_operands.back();
        parsed_operands.pop_back();
        auto dest = parsed_operands.back();
        parsed_operands.pop_back();
        auto newInst = new Instruction();
        newInst->operands.push_back(dest);
        newInst->operands.push_back(src2);
        newInst->operands.push_back(src1);
        std::string cmpop = parsed_operators.back();

        parsed_operators.pop_back();

        if (cmpop == "<") newInst->op = LT;
        if (cmpop == "<=") newInst->op = LE;  
        if (cmpop == "=") newInst->op = EQ;
        if (cmpop == ">") newInst->op = GT;
        if (cmpop == ">=") newInst->op = GE; 

        newInst->Inst = in.string();
        newInst->pos = inst_position;
        inst_position++;
        if(debug) cout<< "CMP_OP " + newInst->operands[0]->str + " <- " + newInst->operands[1]->str + " " + cmpop + " " + newInst->operands[2]->str << endl;
        currentF->instructions.push_back(newInst);
        }
    };

  template<> struct action < L3_load > {
    template< typename Input >
    static void apply( const Input & in, Program & p ){
        auto currentF = p.functions.back();
        auto src = parsed_operands.back();
        parsed_operands.pop_back();
        auto dest = parsed_operands.back();
        parsed_operands.pop_back();
        auto newInst = new Instruction();
        newInst->operands.push_back(dest);
        newInst->operands.push_back(src);
        newInst->op = LOAD;
        newInst->Inst = in.string();
        newInst->pos = inst_position;
        inst_position++;
        if(debug) cout<< "LOAD " << newInst->operands[0]->str + " <- load " + newInst->operands[1]->str << endl;
        currentF->instructions.push_back(newInst);
        }
    };

  template<> struct action < L3_store > {
    template< typename Input >
    static void apply( const Input & in, Program & p ){
        auto currentF = p.functions.back();
        auto src = parsed_operands.back();
        parsed_operands.pop_back();
        auto dest = parsed_operands.back();
        parsed_operands.pop_back();
        auto newInst = new Instruction();
        newInst->operands.push_back(dest);
        newInst->operands.push_back(src);
        newInst->op = STORE;
        newInst->Inst = in.string();
        newInst->pos = inst_position;
        inst_position++;
        if(debug) cout<< "STORE store " << newInst->operands[0]->str + " <- " + newInst->operands[1]->str << endl;
        currentF->instructions.push_back(newInst);
        }
    };

  template<> struct action < L3_return > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        auto currentF = p.functions.back();
        auto newInst = new Instruction();
        newInst->op = RET;
        newInst->Inst = in.string();
        newInst->pos = inst_position;
        inst_position++;
        if(debug) cout<< "RET " << in.string() << endl;
        currentF->instructions.push_back(newInst);
        }
    };

  template<> struct action < L3_return_var > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        auto currentF = p.functions.back();
        auto retVal = parsed_operands.back();
        parsed_operands.pop_back();
        auto newInst = new Instruction();
        newInst->operands.push_back(retVal);
        newInst->op = RETVAR;
        newInst->Inst = in.string();
        newInst->pos = inst_position;
        inst_position++;
        if(debug) cout<< "RETVAR return " << newInst->operands[0]->str << endl;
        currentF->instructions.push_back(newInst);
        }
    };

  template<> struct action < L3_label > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        auto currentF = p.functions.back();
        auto lbl = parsed_operands.back();
        parsed_operands.pop_back();
        auto newInst = new Instruction();
        newInst->operands.push_back(lbl);
        newInst->op = LBL;
        newInst->Inst = in.string();
        newInst->pos = inst_position;
        inst_position++;
        if(debug) cout<< "LABEL " << newInst->operands[0]->str << endl;
        currentF->instructions.push_back(newInst);
        }
    };

  template<> struct action < L3_br1 > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        auto currentF = p.functions.back();
        auto newInst = new Instruction();
        auto brLabel = parsed_operands.back();
        parsed_operands.pop_back();
        newInst->op = GOTO;
        newInst->Inst = in.string();
        newInst->pos = inst_position;
        inst_position++;
        newInst->operands.push_back(brLabel);
        if(debug) cout<< "GOTO br " << newInst->operands[0]->str << endl;
        currentF->instructions.push_back(newInst);
        }
    };

    template<> struct action < L3_br2 > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        auto currentF = p.functions.back();
        auto newInst = new Instruction();
        auto brLabel = parsed_operands.back();
        parsed_operands.pop_back();
        auto brVar = parsed_operands.back();
        parsed_operands.pop_back();
        newInst->op = CJUMP;
        newInst->Inst = in.string();
        newInst->operands.push_back(brVar);
        newInst->operands.push_back(brLabel);
        newInst->pos = inst_position;
        inst_position++;
        if(debug) cout<< "CJUMP br " << newInst->operands[0]->str + " " + newInst->operands[1]->str << endl;
        currentF->instructions.push_back(newInst);
        }
    };


  template<> struct action < L3_call1 > {
    template< typename Input >
    static void apply( const Input & in, Program & p ){
        auto currentF = p.functions.back();
        auto newInst = new Instruction();
        newInst->op = CALL;

        auto callee = parsed_operands[0];
        newInst->operands.push_back(callee);
        parsed_operands.erase(parsed_operands.begin());

        for(auto oprnd : parsed_operands){
          newInst->operands.push_back(oprnd);
        }
        parsed_operands.clear();
        
        newInst->Inst = in.string();
        newInst->pos = inst_position;
        inst_position++;
        if(debug){ 
          cout<< "CALL" << " ";
          for(auto var : newInst->operands){
            cout<< var->str<< " ";
          }
          cout<<endl;
        }
        currentF->instructions.push_back(newInst);
        }
    };

  template<> struct action < L3_call2 > {
    template< typename Input >
    static void apply( const Input & in, Program & p ){
        auto currentF = p.functions.back();
        auto newInst = new Instruction();
        newInst->op = CALL_RETVAR;

        auto dest = parsed_operands[0];
        newInst->operands.push_back(dest);
        parsed_operands.erase(parsed_operands.begin());

        auto callee = parsed_operands[0];
        newInst->operands.push_back(callee);
        parsed_operands.erase(parsed_operands.begin());


        for(auto oprnd : parsed_operands){
          newInst->operands.push_back(oprnd);
        }
        parsed_operands.clear();
        
        newInst->Inst = in.string();
        newInst->pos = inst_position;
        inst_position++;
        if(debug){ 
          cout<< "CALL" << " ";
          for(auto var : newInst->operands){
            cout<< var->str + " ";
          }
          cout<<endl;
        }
        currentF->instructions.push_back(newInst);
        }
    };

  template<> struct action < L3_allocate > {
    template< typename Input >
    static void apply( const Input & in, Program & p ){
        auto currentF = p.functions.back();
        auto newInst = new Instruction();
        newInst->op = ALLOCATE;

        auto dest = parsed_operands[0];
        newInst->operands.push_back(dest);
        parsed_operands.erase(parsed_operands.begin());

        for(auto oprnd : parsed_operands){
          newInst->operands.push_back(oprnd);
        }
        parsed_operands.clear();

        newInst->Inst = in.string();
        newInst->pos = inst_position;
        inst_position++;
        if(debug){ 
          cout<< "ALLOCATE" << " ";
          for(auto var : newInst->operands){
            cout<< var->str + " ";
          }
          cout<<endl;
        }
        currentF->instructions.push_back(newInst);
        }
    };

  template<> struct action < L3_print > {
    template< typename Input >
    static void apply( const Input & in, Program & p ){
        auto currentF = p.functions.back();
        auto newInst = new Instruction();
        newInst->op = PRINT;

        for(auto oprnd : parsed_operands){
          newInst->operands.push_back(oprnd);
        }
        parsed_operands.clear();

        newInst->Inst = in.string();
        newInst->pos = inst_position;
        inst_position++;
        if(debug){ 
          cout<< "PRINT" << " ";
          for(auto var : newInst->operands){
            cout<< var->str + " ";
          }
          cout<<endl;
        }
        currentF->instructions.push_back(newInst);
        }
    };


  template<> struct action < L3_arrayerror > {
    template< typename Input >
    static void apply( const Input & in, Program & p ){
        auto currentF = p.functions.back();
        auto newInst = new Instruction();
        newInst->op = ARRAYERROR;

        for(auto oprnd : parsed_operands){
          newInst->operands.push_back(oprnd);
        }
        parsed_operands.clear();

        newInst->Inst = in.string();
        newInst->pos = inst_position;
        inst_position++;
        if(debug){ 
          cout<< "ARRAYERROR" << " ";
          for(auto var : newInst->operands){
            cout<< var->str + " ";
          }
          cout<<endl;
        }
        currentF->instructions.push_back(newInst);
        }
    };

  Program parseFile (char *fileName){
    /*
     * Check the grammar for some possible issues.
     */
    pegtl::analyze< grammar >();
    
    // Parse.
     
    file_input< > fileInput(fileName);
    Program p;
    parse< grammar, action >(fileInput, p);

    for(auto f : p.functions){
      std::reverse(f->arguments.begin(), f->arguments.end());
    }
    
    return p;
  }

}
