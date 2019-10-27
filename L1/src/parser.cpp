#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <set>
#include <iterator>
#include <cstring>
#include <cctype>
#include <cstdlib>
#include <stdint.h>
#include <assert.h>

#include <tao/pegtl.hpp>
#include <tao/pegtl/analyze.hpp>
#include <tao/pegtl/contrib/raw_string.hpp>

#include "L1.h"
#include "parser.h"



namespace pegtl = tao::TAO_PEGTL_NAMESPACE;


using namespace pegtl;
using namespace std;


namespace L1 {

  bool debug = 0;

  /*
   * Data required to parse
   */
  std::vector<Operand *> parsed_operands;
  std::vector<std::string> parsed_operators;

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

  /*
   * Keywords.
   */
  // L1 keywords
    // 1) registers and labels
  struct L1_sx: TAOCPP_PEGTL_STRING( "rcx" ){};

  struct L1_a:
    pegtl::sor<
      L1_sx,
      TAOCPP_PEGTL_STRING( "rdi" ),
      TAOCPP_PEGTL_STRING( "rsi" ),
      TAOCPP_PEGTL_STRING( "rdx" ),
      TAOCPP_PEGTL_STRING( "r8" ),
      TAOCPP_PEGTL_STRING( "r9" )
    >{};

  struct L1_w:
    pegtl::sor<
      L1_a,
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

  struct L1_x:
    pegtl::sor<
      L1_w,
      TAOCPP_PEGTL_STRING( "rsp" )
    >{};

  struct L1_t:
    pegtl::sor<
      L1_x,
      number
    >{};

  struct L1_s:
    pegtl::sor<
      L1_t,
      label
    >{};

  struct L1_u:
    pegtl::sor<
      L1_w,
      label
    >{};

  struct L1_E:
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
  struct str_memxM: pegtl::seq< TAOCPP_PEGTL_STRING( "mem" ), seps, L1_x, seps, number >{};
  struct str_call: TAOCPP_PEGTL_STRING( "call" ){};
  struct str_print: TAOCPP_PEGTL_STRING( "print 1" ){}; 
  struct str_allocate: TAOCPP_PEGTL_STRING( "allocate 2" ){}; 
  struct str_arrayerror: TAOCPP_PEGTL_STRING( "array-error 2" ){};
  struct str_cjump: TAOCPP_PEGTL_STRING( "cjump" ){};
  struct str_goto: TAOCPP_PEGTL_STRING( "goto" ){};
  struct str_return: TAOCPP_PEGTL_STRING( "return" ){};

  //5) L1 Instructions 
  struct L1_movq_1: pegtl::seq<L1_w, seps, movq_op, seps, L1_s >{};
  struct L1_movq_2: pegtl::seq<L1_w, seps, movq_op, seps, str_memxM >{};
  struct L1_movq_3: pegtl::seq<str_memxM, seps, movq_op, seps, L1_s >{};
  struct L1_movq :
    pegtl::sor<
      pegtl::seq< pegtl::at<L1_movq_1>, L1_movq_1 >,
      pegtl::seq< pegtl::at<L1_movq_2>, L1_movq_2 >,
      pegtl::seq< pegtl::at<L1_movq_3>, L1_movq_3 >
    > {};

  struct L1_aopt: pegtl::seq<L1_w, seps, pegtl::sor<aop_op1,aop_op2>, seps, L1_t >{};
  struct L1_memaop_1: pegtl::seq<str_memxM, seps, aop_op1, seps, L1_t >{};
  struct L1_memaop_2: pegtl::seq<L1_w, seps, aop_op1, seps, str_memxM  >{};

  struct L1_aop :
    pegtl::sor<
      pegtl::seq< pegtl::at<L1_aopt>, L1_aopt >,
      pegtl::seq< pegtl::at<L1_memaop_1>, L1_memaop_1 >,
      pegtl::seq< pegtl::at<L1_memaop_2>, L1_memaop_2 >
    > {};

  struct L1_sop: pegtl::seq<L1_w, seps, sop_op, seps, pegtl::sor<L1_w, number> >{}; 
  struct L1_cmp: pegtl::seq<L1_w, seps, movq_op, seps, L1_t, seps, cmp_op, seps, L1_t>{};
  struct L1_cjump_1: pegtl::seq<str_cjump, seps, L1_t, seps, cmp_op, seps, L1_t, seps, label, pegtl::star<pegtl::blank>, label>{};
  struct L1_cjump_2: pegtl::seq<str_cjump, seps, L1_t, seps, cmp_op, seps, L1_t, seps, label>{};
  struct L1_goto: pegtl::seq<str_goto, seps, label>{};
  struct L1_return: pegtl::seq<str_return>{};
  struct L1_call: pegtl::seq<str_call, seps, L1_u, seps, number>{};
  struct L1_print: pegtl::seq<str_call, seps, str_print>{};
  struct L1_allocate: pegtl::seq<str_call, seps, str_allocate>{};
  struct L1_arrayerror: pegtl::seq<str_call, seps, str_arrayerror>{};
  struct L1_inc: pegtl::seq<L1_w, seps, inc_op>{};
  struct L1_dec: pegtl::seq<L1_w, seps, dec_op>{};
  struct L1_lea: pegtl::seq<L1_w, seps, lea_op, seps, L1_w, seps, L1_w, seps, L1_E>{};
  struct L1_label: pegtl::seq<label>{}; // Label Inst

  struct Instruction_rule:
    pegtl::sor<
      pegtl::seq< pegtl::at<L1_cmp>, L1_cmp >,
      pegtl::seq< pegtl::at<L1_movq>, L1_movq >,
      pegtl::seq< pegtl::at<L1_aop>, L1_aop >,
      pegtl::seq< pegtl::at<L1_sop>, L1_sop >,
      pegtl::seq< pegtl::at<L1_cjump_1>, L1_cjump_1 >,
      pegtl::seq< pegtl::at<L1_cjump_2>, L1_cjump_2 >,       
      pegtl::seq< pegtl::at<L1_goto>, L1_goto >,
      pegtl::seq< pegtl::at<L1_print>, L1_print >,
      pegtl::seq< pegtl::at<L1_allocate>, L1_allocate >,
      pegtl::seq< pegtl::at<L1_arrayerror>, L1_arrayerror >,
      pegtl::seq< pegtl::at<L1_call>, L1_call >,
      pegtl::seq< pegtl::at<L1_inc>, L1_inc >,
      pegtl::seq< pegtl::at<L1_dec>, L1_dec >,
      pegtl::seq< pegtl::at<L1_lea>, L1_lea >,
      pegtl::seq< pegtl::at<L1_return>, L1_return >,
      pegtl::seq< pegtl::at<L1_label>, L1_label >    
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
      pegtl::one< ')' >
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

  /*
   * Actions attached to grammar rules.
   */
  template< typename Rule >
  struct action : pegtl::nothing< Rule > {};

  template<> struct action < label > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      if (p.entryPointLabel.empty()){
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


  //PARSE Operands & Operators
  //Operands = L1_x | N | Label | str_memxM
  template<> struct action < L1_w > {  //set of all registers
    template< typename Input >
    static void apply( const Input & in, Program & p ){
    auto newOp = new Operand();
    std::string w = in.string();
    newOp->op_type = GPREG;
    newOp->str = in.string();
    parsed_operands.push_back(newOp);
    }
  };

  template<> struct action < TAOCPP_PEGTL_STRING( "rsp" ) > {  // rsp not included in L1_w
    template< typename Input >
    static void apply( const Input & in, Program & p ){
    auto newOp = new Operand();
    newOp->op_type = GPREG;
    newOp->str = in.string();
    parsed_operands.push_back(newOp);
    }
  };
  
  template<> struct action < str_memxM > { //Pop out x and M from parsed_operands before pushing memxM in
    template< typename Input >
    static void apply( const Input & in, Program & p ){  
      auto newOp = new Operand();
      newOp->op_type = MEM;
      newOp->str = in.string(); 
      parsed_operands.pop_back();
      parsed_operands.pop_back();
      parsed_operands.push_back(newOp); 
      
    }
  };
  template<> struct action < number > { //a number can be an operand too
    template< typename Input >
    static void apply( const Input & in, Program & p ){
      auto newOp = new Operand();
      newOp->op_type = NUM;
      newOp->str = in.string(); 
      parsed_operands.push_back(newOp);
    }
  };

  template<> struct action < L1_E > {
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
  template<> struct action < L1_movq > {
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

  template<> struct action < L1_aop > {
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

  template<> struct action < L1_sop > {
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

  template<> struct action < L1_cmp > {
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

  template<> struct action < L1_cjump_1 > {
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

  template<> struct action < L1_cjump_2 > {
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

  template<> struct action < L1_return > {
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
  template<> struct action < L1_label > {
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

  template<> struct action < L1_goto > {
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

  template<> struct action < L1_call > {
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

  template<> struct action < L1_print > {
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

  template<> struct action < L1_allocate > {
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

  template<> struct action < L1_arrayerror > {
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

  template<> struct action < L1_lea > {
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

  template<> struct action < L1_inc > {
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

  template<> struct action < L1_dec > {
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
    /*
     * Parse.
     */
    file_input< > fileInput(fileName);
    Program p;
    parse< grammar, action >(fileInput, p);

    return p;
  }
}
