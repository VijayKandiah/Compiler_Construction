#include "parser.h"



namespace pegtl = tao::TAO_PEGTL_NAMESPACE;


using namespace pegtl;
using namespace std;


namespace L2 { // L2 version

  bool debug = 0;

  /*
   * Data required to parse
   */
  std::vector<Operand *> parsed_operands;
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

  struct argument_number:
    number {};

  struct local_number:
    number {} ;



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

  // new L2 vars
  struct L2_var:
    pegtl::seq<
      pegtl::one<'%'>,
      name
    >{};

  struct SpillReplace:
    L2_var {};

  struct SpilledVariable:
    L2_var {} ;

  /*
   * Keywords.
   */
  // L2 keywords, also as L1 keywords
    // 1) registers and labels
  struct L2_sx:
    pegtl::sor<
      TAOCPP_PEGTL_STRING( "rcx" ),
      L2_var
    >{};

  struct L2_a: // also as L1_a
    pegtl::sor<
      L2_sx,
      TAOCPP_PEGTL_STRING( "rdi" ),
      TAOCPP_PEGTL_STRING( "rsi" ),
      TAOCPP_PEGTL_STRING( "rdx" ),
      TAOCPP_PEGTL_STRING( "r8" ),
      TAOCPP_PEGTL_STRING( "r9" )
    >{};

  struct L2_w: // also as L1_w
    pegtl::sor<
      L2_a,
      TAOCPP_PEGTL_STRING( "rax" ),
      TAOCPP_PEGTL_STRING( "rbx" ),
      TAOCPP_PEGTL_STRING( "rbp" ),
      TAOCPP_PEGTL_STRING( "r10" ),
      TAOCPP_PEGTL_STRING( "r11" ),
      TAOCPP_PEGTL_STRING( "r12" ),
      TAOCPP_PEGTL_STRING( "r13" ),
      TAOCPP_PEGTL_STRING( "r14" ),
      TAOCPP_PEGTL_STRING( "r15" )
    >{};

  struct L2_x: // also as L1_x
    pegtl::sor<
      L2_w,
      TAOCPP_PEGTL_STRING( "rsp" )
    >{};

  struct L2_t: // also as L1_t
    pegtl::sor<
      L2_x,
      number
    >{};

  struct L2_s: // also as L1_s
    pegtl::sor<
      L2_t,
      label
    >{};

  struct L2_u: // also as L1_u
    pegtl::sor<
      L2_w,
      label
    >{};

  struct L2_E: // also as L1_E
    pegtl::sor<
      pegtl::one< '1' >,
      pegtl::one< '2' >,
      pegtl::one< '4' >,
      pegtl::one< '8' >
    >{};

    // 2) Operators. 
  struct movq_op: TAOCPP_PEGTL_STRING( "<-" ){};
  struct addq_op: TAOCPP_PEGTL_STRING( "+=" ){};
  struct subq_op: TAOCPP_PEGTL_STRING( "-=" ){};
  struct imulq_op: TAOCPP_PEGTL_STRING( "*=" ){};
  struct andq_op: TAOCPP_PEGTL_STRING( "&=" ){};
  struct dec_op: TAOCPP_PEGTL_STRING( "--" ){};
  struct inc_op: TAOCPP_PEGTL_STRING( "++" ){};
  struct lq_op: pegtl::one< '<' >{};
  struct leq_op: TAOCPP_PEGTL_STRING( "<=" ){};
  struct eq_op: pegtl::one< '=' >{};
  struct salq_op: TAOCPP_PEGTL_STRING( "<<=" ){};
  struct sarq_op: TAOCPP_PEGTL_STRING( ">>=" ){};

    // 3) grouping some operators:
  struct lea_op: pegtl::string< '@' >{}; // rdi @ rdi rsi 4 : set rdi to rdi + (rsi * 4)
  struct aop_op1: pegtl::sor<
    addq_op,
    subq_op
  >{};
  struct aop_op2: pegtl::sor<
    imulq_op,
    andq_op
  >{};    
        
  struct sop_op: pegtl::sor<
    salq_op,
    sarq_op
  >{};

  struct cmp_op: pegtl::sor<
    pegtl::seq<pegtl::at<leq_op>,leq_op>,
    pegtl::seq< pegtl::at<lq_op>,lq_op>,
    pegtl::seq< pegtl::at<eq_op>,eq_op>
  >{};

  // 4) more strings to parse
  struct str_memxM: pegtl::seq< TAOCPP_PEGTL_STRING( "mem" ), seps, L2_x, seps, number >{};
  struct str_call: TAOCPP_PEGTL_STRING( "call" ){};
  struct str_print: TAOCPP_PEGTL_STRING( "print 1" ){}; 
  struct str_allocate: TAOCPP_PEGTL_STRING( "allocate 2" ){}; 
  struct str_arrayerror: TAOCPP_PEGTL_STRING( "array-error 2" ){};
  struct str_cjump: TAOCPP_PEGTL_STRING( "cjump" ){};
  struct str_goto: TAOCPP_PEGTL_STRING( "goto" ){};
  struct str_return: TAOCPP_PEGTL_STRING( "return" ){};
  // new L2 stack argument
  struct str_stackArgM: pegtl::seq< TAOCPP_PEGTL_STRING( "stack-arg" ), seps, number >{};

  //5) L2 Instructions, also as L1 Instructions
  struct L2_movq_1: pegtl::seq<L2_w, seps, movq_op, seps, L2_s >{};
  struct L2_movq_2: pegtl::seq<L2_w, seps, movq_op, seps, str_memxM >{};
  struct L2_movq_3: pegtl::seq<str_memxM, seps, movq_op, seps, L2_s >{};
  // new L2 movq
  struct L2_movq_4: pegtl::seq<L2_w, seps, movq_op, seps, str_stackArgM >{};
  struct L2_movq : // add L2_movq_4
    pegtl::sor<
      pegtl::seq< pegtl::at<L2_movq_1>, L2_movq_1 >,
      pegtl::seq< pegtl::at<L2_movq_2>, L2_movq_2 >,
      pegtl::seq< pegtl::at<L2_movq_3>, L2_movq_3 >,
      pegtl::seq< pegtl::at<L2_movq_4>, L2_movq_4 > // L2 stack argument
    > {};

  struct L2_aopt: pegtl::seq<L2_w, seps, pegtl::sor<aop_op1,aop_op2>, seps, L2_t >{};
  struct L2_memaop_1: pegtl::seq<str_memxM, seps, aop_op1, seps, L2_t >{};
  struct L2_memaop_2: pegtl::seq<L2_w, seps, aop_op1, seps, str_memxM  >{};

  struct L2_aop :
    pegtl::sor<
      pegtl::seq< pegtl::at<L2_aopt>, L2_aopt >,
      pegtl::seq< pegtl::at<L2_memaop_1>, L2_memaop_1 >,
      pegtl::seq< pegtl::at<L2_memaop_2>, L2_memaop_2 >
    > {};

  struct L2_sop: pegtl::seq<L2_w, seps, sop_op, seps, pegtl::sor<L2_w, number> >{}; //won't parse sx :/
  struct L2_cmp: pegtl::seq<L2_w, seps, movq_op, seps, L2_t, seps, cmp_op, seps, L2_t>{};
  struct L2_cjump_1: pegtl::seq<str_cjump, seps, L2_t, seps, cmp_op, seps, L2_t, seps, label, pegtl::star<pegtl::blank>, label>{};
  struct L2_cjump_2: pegtl::seq<str_cjump, seps, L2_t, seps, cmp_op, seps, L2_t, seps, label>{};
  struct L2_goto: pegtl::seq<str_goto, seps, label>{};
  struct L2_return: pegtl::seq<str_return>{};
  struct L2_call: pegtl::seq<str_call, seps, L2_u, seps, number>{};
  struct L2_print: pegtl::seq<str_call, seps, str_print>{};
  struct L2_allocate: pegtl::seq<str_call, seps, str_allocate>{};
  struct L2_arrayerror: pegtl::seq<str_call, seps, str_arrayerror>{};
  struct L2_inc: pegtl::seq<L2_w, seps, inc_op>{};
  struct L2_dec: pegtl::seq<L2_w, seps, dec_op>{};
  struct L2_lea: pegtl::seq<L2_w, seps, lea_op, seps, L2_w, seps, L2_w, seps, L2_E>{};
  struct L2_label: pegtl::seq<label>{}; // Label Inst

  struct Instruction_rule:
    pegtl::sor<
      pegtl::seq< pegtl::at<L2_cmp>, L2_cmp >,
      pegtl::seq< pegtl::at<L2_movq>, L2_movq >,
      pegtl::seq< pegtl::at<L2_aop>, L2_aop >,
      pegtl::seq< pegtl::at<L2_sop>, L2_sop >,
      pegtl::seq< pegtl::at<L2_cjump_1>, L2_cjump_1 >,
      pegtl::seq< pegtl::at<L2_cjump_2>, L2_cjump_2 >,       
      pegtl::seq< pegtl::at<L2_goto>, L2_goto >,
      pegtl::seq< pegtl::at<L2_print>, L2_print >,
      pegtl::seq< pegtl::at<L2_allocate>, L2_allocate >,
      pegtl::seq< pegtl::at<L2_arrayerror>, L2_arrayerror >,
      pegtl::seq< pegtl::at<L2_call>, L2_call >,
      pegtl::seq< pegtl::at<L2_inc>, L2_inc >,
      pegtl::seq< pegtl::at<L2_dec>, L2_dec >,
      pegtl::seq< pegtl::at<L2_lea>, L2_lea >,
      pegtl::seq< pegtl::at<L2_return>, L2_return >,
      pegtl::seq< pegtl::at<L2_label>, L2_label >    
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
      pegtl::one< '(' >,
      seps,
      function_name,
      seps,
      argument_number,
      seps,
      local_number,
      seps,
      Instructions_rule,
      seps,
      pegtl::one< ')' >,
      seps
    > {};

  struct Spill_rule:
    pegtl::seq<
      seps,
      pegtl::one< '(' >,
      seps,
      function_name,
      seps,
      argument_number,
      seps,
      local_number,
      seps,
      Instructions_rule,
      seps,
      pegtl::one< ')' >,
      seps,
      SpilledVariable,
      seps,
      SpillReplace,
      seps
    > {};

  struct Functions_rule:
    pegtl::plus<
      seps,
      Function_rule,
      seps
    > {};

  struct entry_point_rule:
    pegtl::seq<
      seps,
      pegtl::one< '(' >,
      seps,
      label,
      seps,
      Functions_rule,
      seps,
      pegtl::one< ')' >,
      seps
    > { };

  struct grammar :
    pegtl::must<
      entry_point_rule
    > {};

  struct grammar_func :
    pegtl::must<
      Function_rule
    > {};

  struct grammar_spill :
    pegtl::must<
      Spill_rule
    > {};

  /*
   * Actions attached to grammar rules.
   */
  template< typename Rule >
  struct action : pegtl::nothing< Rule > {};

  template<> struct action < label > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      if (p.entryPointLabel.empty() && !is_Func_only){
        p.entryPointLabel = in.string();
      } else { 
        auto newOp = new Operand();
        newOp->op_type = LABEL;
        newOp->str = in.string();
        parsed_operands.push_back(newOp);
      }
    }
  };

  template<> struct action < function_name > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      auto newF = new Function();
      newF->name = in.string();
      p.functions.push_back(newF);
    }
  };

  template<> struct action < argument_number > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      currentF->arguments = std::stoll(in.string());
    }
  };

  template<> struct action < local_number > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      currentF->locals = std::stoll(in.string());
    }
  };

  template<> struct action < SpilledVariable > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      currentF->spilled_variable = in.string();
    }
  };

  template<> struct action < SpillReplace > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      currentF->spill_replace = in.string();
    }
  };    

  //PARSE OPERANDS, AOP, SOP, CMPOP 
  //Operands = L2_x | N | Label | str_memxM
  template<> struct action < L2_w > {  //set of all registers
    template< typename Input >
    static void apply( const Input & in, Program & p ){
    auto newOp = new Operand();
    std::string w = in.string();
    if(w[0] == 'r')
      newOp->op_type = GPREG;
    else 
      newOp->op_type = VAR;
    newOp->str = in.string();
    parsed_operands.push_back(newOp);
    }
  };

  template<> struct action < TAOCPP_PEGTL_STRING( "rsp" ) > {  //set of all registers
    template< typename Input >
    static void apply( const Input & in, Program & p ){
    auto newOp = new Operand();
    newOp->op_type = GPREG;
    newOp->str = in.string();
    parsed_operands.push_back(newOp);
    }
  };
  
  template<> struct action < str_memxM > {
    template< typename Input >
    static void apply( const Input & in, Program & p ){ 
      auto newOp = new Operand();
      auto M = parsed_operands.back();
      newOp->Mvalue = std::stoll(M->str);
      parsed_operands.pop_back();
      auto src = parsed_operands.back();
      newOp->X = src->str;
      parsed_operands.pop_back();
      if(src->op_type == GPREG)
        newOp->op_type = MEMREG;
      if(src->op_type == VAR)
        newOp->op_type = MEMVAR;
      newOp->str = in.string();
      parsed_operands.push_back(newOp);  
    }
  };
  
  // new L2 action: stack argument
  template<> struct action < str_stackArgM > {
    template< typename Input >
    static void apply( const Input & in, Program & p ){
      auto newOp = new Operand();
      newOp->op_type = SARG;
      newOp->str = in.string(); 
      auto M = parsed_operands.back();
      newOp->Mvalue = std::stoll(M->str);
      parsed_operands.pop_back();
      parsed_operands.push_back(newOp);
    }
  };


  template<> struct action < number > {
    template< typename Input >
    static void apply( const Input & in, Program & p ){
      auto newOp = new Operand();
      newOp->op_type = NUM;
      newOp->str = in.string(); 
      parsed_operands.push_back(newOp);
    }
  };

  template<> struct action < L2_E > {
    template< typename Input >
    static void apply( const Input & in, Program & p ){
      auto newOp = new Operand();
      newOp->op_type = NUM;
      newOp->str = in.string(); 
      parsed_operands.push_back(newOp);
    }
  };

  //AOP 
  template<> struct action < aop_op1 > {
    template< typename Input >
    static void apply( const Input & in, Program & p ){
      parsed_operators.push_back(in.string());
    }
  };

  template<> struct action < aop_op2 > {
    template< typename Input >
    static void apply( const Input & in, Program & p ){
      parsed_operators.push_back(in.string());
    }
  };

  //SOP
  template<> struct action < sop_op > {
    template< typename Input >
    static void apply( const Input & in, Program & p ){
      parsed_operators.push_back(in.string());
    }
  };

  //CMP_OP
  template<> struct action < cmp_op > {
    template< typename Input >
    static void apply( const Input & in, Program & p ){
      parsed_operators.push_back(in.string());
    }
  };

  //Instructions
  template<> struct action < L2_movq > {
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
      newInst->op = MOVQ;
      newInst->Inst = in.string();
      if(debug) cout<< "MOVQ " << dest->str + " <- " + src->str << endl;
      currentF->instructions.push_back(newInst);
    }
  };

  template<> struct action < L2_aop > {
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
      std::string arith = parsed_operators.back();

      parsed_operators.pop_back();

      switch (arith[0]){
        case '+': newInst->op = ADDQ; break;
        case '-': newInst->op = SUBQ; break;
        case '*': newInst->op = IMULQ; break;
        case '&': newInst->op = ANDQ; break;
      }
      newInst->Inst = in.string();
      if(debug) cout<< "AOP " + dest->str + " " + arith + " " + src->str << endl;
      currentF->instructions.push_back(newInst);
    }
  };

  template<> struct action < L2_sop > {
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
      std::string shift = parsed_operators.back();
      parsed_operators.pop_back();
      switch (shift[0]){
        case '<': newInst->op = SALQ; break;
        case '>': newInst->op = SARQ; break;
      }
      newInst->Inst = in.string();
      if(debug) cout<< "SOP " + dest->str + " " + shift + " " + src->str << endl;
      currentF->instructions.push_back(newInst);
    }
  };



  template<> struct action < L2_cmp > {
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
      if (cmpop == "<") newInst->op = LQ;
      if (cmpop == "<=") newInst->op = LEQ;  
      if (cmpop == "=") newInst->op = EQ;
      newInst->Inst = in.string();
      if(debug) cout<< "CMP " + dest->str + " <- " + src2->str + " " + cmpop +  " " + src1->str<< endl;
      currentF->instructions.push_back(newInst);
    }
  };

  template<> struct action < L2_cjump_1 > {
    template< typename Input >
    static void apply( const Input & in, Program & p ){
      auto currentF = p.functions.back();
      auto flabel = parsed_operands.back();
      parsed_operands.pop_back();
      auto tlabel = parsed_operands.back();
      parsed_operands.pop_back();
      auto src1 = parsed_operands.back();
      parsed_operands.pop_back();
      auto src2 = parsed_operands.back();
      parsed_operands.pop_back();
      
      auto newInst = new Instruction();
      newInst->operands.push_back(src2);
      newInst->operands.push_back(src1);
      newInst->operands.push_back(tlabel);
      newInst->operands.push_back(flabel);

      std::string cmpop = parsed_operators.back();
      parsed_operators.pop_back();
      if (cmpop == "<") newInst->op = JL;
      if (cmpop == "<=") newInst->op = JLE;  
      if (cmpop == "=") newInst->op = JE;
      newInst->Inst = in.string();
      if(debug) cout<< "CJUMP1 " + src2->str + " " + cmpop +  " " + src1->str + " "  << tlabel->str + " " + flabel->str << endl;
      currentF->instructions.push_back(newInst);
    }
  };

  template<> struct action < L2_cjump_2 > {
    template< typename Input >
    static void apply( const Input & in, Program & p ){
      auto currentF = p.functions.back();
      auto tlabel = parsed_operands.back();
      parsed_operands.pop_back();
      auto src1 = parsed_operands.back();
      parsed_operands.pop_back();
      auto src2 = parsed_operands.back();
      parsed_operands.pop_back();
      
      auto newInst = new Instruction();
      newInst->operands.push_back(src2);
      newInst->operands.push_back(src1);
      newInst->operands.push_back(tlabel);

      std::string cmpop = parsed_operators.back();
      parsed_operators.pop_back();
      if (cmpop == "<") newInst->op = JL;
      if (cmpop == "<=") newInst->op = JLE;  
      if (cmpop == "=") newInst->op = JE;
      newInst->Inst = in.string();
      if(debug) cout<< "CJUMP2 " + src2->str + " " + cmpop +  " " + src1->str + " "  << tlabel->str << endl;
      currentF->instructions.push_back(newInst);
    }
  };

  template<> struct action < L2_return > {
      template< typename Input >
      static void apply( const Input & in, Program & p){
        auto currentF = p.functions.back();
        auto i = new Instruction();
        i->op = RETQ;
        i->Inst = in.string();
        if(debug) cout<< "RETQ " << in.string() << endl;
        currentF->instructions.push_back(i);
      }
  };

  //inst_label 
  template<> struct action < L2_label > {
      template< typename Input >
      static void apply( const Input & in, Program & p){
        auto currentF = p.functions.back();
        parsed_operands.pop_back();
        auto i = new Instruction();
        i->op = LBL;
        i->Inst = in.string();
        if(debug) cout << "LBL " << in.string() << endl;
        currentF->instructions.push_back(i);
      }
  };

  template<> struct action < L2_goto > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto i = new Instruction();
      auto gotolabel = parsed_operands.back();
      parsed_operands.pop_back();
      i->op = GOTO;
      i->Inst = in.string();
      i->operands.push_back(gotolabel);
      if(debug) cout << "GOTO " + gotolabel->str << endl;
      currentF->instructions.push_back(i);
    }
  };

  template<> struct action < L2_call > {
    template< typename Input >
    static void apply( const Input & in, Program & p ){
      auto currentF = p.functions.back();
      auto n = parsed_operands.back();
      parsed_operands.pop_back();
      auto dest = parsed_operands.back();
      parsed_operands.pop_back();
      auto newInst = new Instruction();
      newInst->operands.push_back(dest);
      newInst->operands.push_back(n);
      newInst->op = CALL;
      newInst->Inst = in.string();
      if(debug) cout<< "CALL" << " " + dest->str + " " << n->str << endl;
      currentF->instructions.push_back(newInst);
    }
  };

  template<> struct action < L2_print > {
    template< typename Input >
    static void apply( const Input & in, Program & p ){
      auto currentF = p.functions.back();
      auto newInst = new Instruction();
      newInst->op = PRINT;
      newInst->Inst = in.string();
      if(debug) cout<< "PRINT "<< in.string() << endl;
      currentF->instructions.push_back(newInst);
    }
  };

  template<> struct action < L2_allocate > {
    template< typename Input >
    static void apply( const Input & in, Program & p ){
      auto currentF = p.functions.back();
      auto newInst = new Instruction();
      newInst->op = ALLOCATE;
      newInst->Inst = in.string();
      if(debug) cout<< "ALLOCATE "<< in.string() << endl;
      currentF->instructions.push_back(newInst);
    }
  };  

  template<> struct action < L2_arrayerror > {
    template< typename Input >
    static void apply( const Input & in, Program & p ){
      auto currentF = p.functions.back();
      auto newInst = new Instruction();
      newInst->op = ARRAYERROR;
      newInst->Inst = in.string();
      if(debug) cout<< "ARRAYERROR "<< in.string() << endl;
      currentF->instructions.push_back(newInst);
    }
  };   

  template<> struct action < L2_lea > {
    template< typename Input >
    static void apply( const Input & in, Program & p ){
      auto currentF = p.functions.back();
      auto e = parsed_operands.back();
      parsed_operands.pop_back();
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
      newInst->operands.push_back(e);
      newInst->op = LEA;
      newInst->Inst = in.string();
      if(debug) cout<< "LEA " + dest->str  << " @ " + src2->str + " " + src1->str  + " " + e->str  << endl;
      currentF->instructions.push_back(newInst);
    }
  };

  template<> struct action < L2_inc > {
    template< typename Input >
    static void apply( const Input & in, Program & p ){
      auto currentF = p.functions.back();
      auto op = parsed_operands.back();
      parsed_operands.pop_back();        
      auto newInst = new Instruction();
      newInst->operands.push_back(op);
      newInst->op = INC;
      newInst->Inst = in.string();
      if(debug) cout<< "INC " + op->str  << "++" << endl;
      currentF->instructions.push_back(newInst);
    }
  };    

  template<> struct action < L2_dec > {
    template< typename Input >
    static void apply( const Input & in, Program & p ){
      auto currentF = p.functions.back();
      auto op = parsed_operands.back();
      parsed_operands.pop_back();        
      auto newInst = new Instruction();
      newInst->operands.push_back(op);
      newInst->op = DEC;
      newInst->Inst = in.string();
      if(debug) cout<< "DEC " + op->str  << "--" << endl;
      currentF->instructions.push_back(newInst);
    }
  };

  Program parse_file (char *fileName){
    /*
     * Check the grammar for some possible issues.
     */
    pegtl::analyze< grammar >();
    
    // Parse.
     
    file_input< > fileInput(fileName);
    Program p;
    parse< grammar, action >(fileInput, p);

    return p;
  }

  Program parse_function (char *fileName){
    /*
     * Check the grammar for some possible issues.
     */
    pegtl::analyze< grammar_func >();
    is_Func_only = true;
    // Parse.
     
    file_input< > fileInput(fileName);
    Program p;
    parse< grammar_func, action >(fileInput, p);

    return p;
  }

  Program parse_spill (char *fileName){
    /*
     * Check the grammar for some possible issues.
     */
    pegtl::analyze< grammar_spill >();
    is_Func_only = true;
    // Parse.
     
    file_input< > fileInput(fileName);
    Program p;
    parse< grammar_spill, action >(fileInput, p);

    return p;
  }
}
