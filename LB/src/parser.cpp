#include "parser.h"


namespace pegtl = tao::TAO_PEGTL_NAMESPACE;


using namespace pegtl;
using namespace std;


namespace LB {

    bool debug = 0;

    /*
    * Data required to parse
    */
    std::vector<Variable *> parsed_operands;
    std::vector<std::string> parsed_operators;
    std::vector<std::string> parsed_deftypes;
    std::vector<std::string> scopeVec;
    /*
    * Grammar rules from now on.
    */


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
        name{};
    /*
    * Keywords.
    */
    // LB keywords,
    // 1) registers, labels and types

    // new LB vars

    
    struct LB_t: 
    pegtl::sor<
      name,
      number
    >{};

    struct LB_args:
    pegtl::star<
      pegtl::seq<
        seps,
        LB_t,
        seps,
        pegtl::opt<
          pegtl::one<','>
        >
      >
    >{};

    struct LB_names:
    pegtl::star<
      pegtl::seq<
        pegtl::star<pegtl::blank>,
        name,
        pegtl::star<pegtl::blank>,
        pegtl::opt<
          pegtl::one<','>
        >
      >
    >{};

    struct LB_s: 
    pegtl::sor<
      LB_t,
      label
    >{};

    struct LB_int64:
    pegtl::seq<
        seps,
        TAOCPP_PEGTL_STRING( "int64" ),
        seps,
        pegtl::star<
            pegtl::seq<
                seps,
                pegtl::one<'['>,
                seps,
                pegtl::one<']'>,
                seps
            >
        >
    >{};

    struct LB_tuple:
    TAOCPP_PEGTL_STRING( "tuple" ){};

    struct LB_code:
    TAOCPP_PEGTL_STRING( "code" ){};

    struct LB_type:
    pegtl::sor<
        LB_int64,
        LB_tuple,
        LB_code
    >{};

    struct LB_T:
    pegtl::sor<
        LB_type,
        TAOCPP_PEGTL_STRING( "void" )
    >{};




    struct LB_index:
    pegtl::seq<
      seps,
      pegtl::one<'['>,
      LB_t,
      pegtl::one<']'>
    >{};

    struct LB_var_index: //name[t][t][t]
    pegtl::seq<
      name,
      pegtl::plus<
        LB_index
      >
    >{};


    struct LB_type_vars:
    pegtl::star<
      pegtl::seq<
        seps,
        LB_type,
        seps,
        name,
        seps,
        pegtl::opt<
          pegtl::one<','>
        >
      >
    >{};


    struct function_head:
    pegtl::seq<
        seps,
        function_name,
        seps,
        pegtl::one< '(' >,
        seps,
        pegtl::opt< LB_type_vars >,
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
    struct str_print: TAOCPP_PEGTL_STRING( "print" ){}; 
    struct str_br: TAOCPP_PEGTL_STRING( "br" ){};
    struct str_return: TAOCPP_PEGTL_STRING( "return" ){};
    struct str_main: TAOCPP_PEGTL_STRING( "main" ){};
    struct str_new: TAOCPP_PEGTL_STRING( "new" ){};
    struct str_array: TAOCPP_PEGTL_STRING( "Array" ){};
    struct str_tuple: TAOCPP_PEGTL_STRING( "Tuple" ){};
    struct str_length: TAOCPP_PEGTL_STRING( "length" ){};
    struct str_continue: TAOCPP_PEGTL_STRING( "continue" ){};
    struct str_if: TAOCPP_PEGTL_STRING( "if" ){};
    struct str_while: TAOCPP_PEGTL_STRING( "while" ){};
    struct str_break: TAOCPP_PEGTL_STRING( "break" ){};

    struct LB_cond:
    pegtl::seq<
        seps,
        LB_t, 
        seps, 
        pegtl::sor<arith_op, cmp_op>,
        seps, 
        LB_t 
    >{};


    // 5) LB Instructions
    struct LB_def_type: pegtl::seq<LB_type, seps, LB_names >{};
    struct LB_mov: pegtl::seq<name, seps, mov_op, seps, LB_s >{}; //name <- s
    struct LB_read_array: pegtl::seq<name, seps, mov_op, seps, LB_var_index >{}; // name <- name([t])
    struct LB_write_array: pegtl::seq<LB_var_index, seps, mov_op, seps, LB_s >{}; // name([t]) <- s
    struct LB_op: pegtl::seq<name, seps, mov_op, seps, LB_cond>{};
    struct LB_return: pegtl::seq<str_return>{};
    struct LB_return_var: pegtl::seq<str_return, seps, LB_t>{};
    struct LB_label: pegtl::seq<label>{}; // Label Inst
    struct LB_br1: pegtl::seq<str_br, seps, label>{};
    struct LB_if: pegtl::seq<str_if, seps, pegtl::one< '(' >, seps, LB_cond, seps, pegtl::one< ')'>, seps, label, seps, label>{};
    struct LB_while: pegtl::seq<str_while, seps, pegtl::one< '(' >, seps, LB_cond, seps, pegtl::one< ')'>, seps, label, seps, label>{};
    struct LB_break: pegtl::seq<str_break>{};
    struct LB_continue: pegtl::seq<str_continue>{};
    struct LB_scopeOpen: pegtl::seq<pegtl::one< '{' >>{};
    struct LB_scopeClose: pegtl::seq<pegtl::one< '}' >>{};
    struct LB_scope;
    struct LB_length: pegtl::seq<name, seps, mov_op, seps, str_length, seps, name, seps, LB_t>{};
    struct LB_call1: pegtl::seq<name, seps, pegtl::one< '(' >, seps, LB_args, seps, pegtl::one< ')' >>{};
    struct LB_call2: pegtl::seq<name, seps, mov_op, seps, name, seps, pegtl::one< '(' >, seps, LB_args, seps, pegtl::one< ')' >>{};
    struct LB_print: pegtl::seq<str_print, seps, pegtl::one< '(' >, seps, LB_args, seps, pegtl::one< ')' >>{};
    struct LB_new_array: pegtl::seq<name, seps, mov_op, seps, str_new, seps, str_array, seps, pegtl::one< '(' >, seps, LB_args, seps, pegtl::one< ')' >>{};
    struct LB_new_tuple: pegtl::seq<name, seps, mov_op, seps, str_new, seps, str_tuple, seps, pegtl::one< '(' >, seps, LB_t, seps, pegtl::one< ')' >>{};

    
    struct Instruction_rule:
    pegtl::sor<
        pegtl::seq< pegtl::at<LB_def_type>, LB_def_type >,
        pegtl::seq< pegtl::at<LB_op>, LB_op >,
        pegtl::seq< pegtl::at<LB_read_array>, LB_read_array >,
        pegtl::seq< pegtl::at<LB_write_array>, LB_write_array >,
        pegtl::seq< pegtl::at<LB_length>, LB_length >,
        pegtl::seq< pegtl::at<LB_print>, LB_print >,
        pegtl::seq< pegtl::at<LB_call1>, LB_call1 >,
        pegtl::seq< pegtl::at<LB_call2>, LB_call2 >,

        pegtl::seq< pegtl::at<LB_if>, LB_if >,
        pegtl::seq< pegtl::at<LB_while>, LB_while >,
        pegtl::seq< pegtl::at<LB_break>, LB_break >,
        pegtl::seq< pegtl::at<LB_continue>, LB_continue >,
        pegtl::seq< pegtl::at<LB_scope>, LB_scope >,

        pegtl::seq< pegtl::at<LB_new_array>, LB_new_array >,
        pegtl::seq< pegtl::at<LB_new_tuple>, LB_new_tuple >,
        pegtl::seq< pegtl::at<LB_return_var>, LB_return_var >,
        pegtl::seq< pegtl::at<LB_return>, LB_return >,  
        pegtl::seq< pegtl::at<LB_br1>, LB_br1 >,
        pegtl::seq< pegtl::at<LB_label>, LB_label >, 
        pegtl::seq< pegtl::at<LB_mov>, LB_mov >  
    > {};


    struct Instructions_rule:
    pegtl::star<
      pegtl::seq<
        seps,
        Instruction_rule,
        seps
      >
    > {};

    struct LB_scope:
   	pegtl::seq<
   		LB_scopeOpen,
   		seps,
   		Instructions_rule,
   		seps,
   		LB_scopeClose
   	>{};


    struct Function_rule:
    pegtl::seq<
      seps,
      LB_T,
      seps,
      function_head,
      seps,
      LB_scope,
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
        auto newretType = parsed_deftypes.back();
        parsed_deftypes.pop_back();
        newF->name = in.string();
        newF->retType = newretType;
        p.functions.push_back(newF);
        }
    };

    template<> struct action < function_head > {
        template< typename Input >
        static void apply( const Input & in, Program & p){
            auto currentF = p.functions.back();
            while(!parsed_operands.empty()){
                auto op = parsed_operands.back();
                currentF->arguments.push_back(op); // reversed order of original arguments - handled later
                parsed_operands.pop_back();
                }
            }
        };

    //Operands = name | N | Label
    template<> struct action < name > {
        template< typename Input >
        static void apply( const Input & in, Program & p){
            auto newOp = new Variable();
            newOp->op_type = VAR;
            newOp->str = in.string();

            if(!parsed_deftypes.empty()){
                newOp->defType = parsed_deftypes.back();
                parsed_deftypes.pop_back();
            }
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
            parsed_operands.pop_back();
            parsed_operands.push_back(newOp);
        }
    };

    //defType void | int64[] | tuple | code

    template<> struct action < LB_type > {
        template< typename Input >
        static void apply( const Input & in, Program & p){
            parsed_deftypes.push_back(in.string());
        }
    };

    template<> struct action < TAOCPP_PEGTL_STRING( "void" ) > {
        template< typename Input >
        static void apply( const Input & in, Program & p){
            parsed_deftypes.push_back(in.string());
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
    template<> struct action < LB_mov > { // var <- s 
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
        newInst->Inst = in.string();
        if(debug) cout<< "MOV " << newInst->operands[0]->str + " <- " + newInst->operands[1]->str << endl;
        currentF->instructions.push_back(newInst);
        }
    }; 

    template<> struct action < LB_def_type > { 
    template< typename Input >
    static void apply( const Input & in, Program & p ){
            auto currentF = p.functions.back();
            for(auto oprnd : parsed_operands){
                auto newInst = new Instruction();
                oprnd->defType = parsed_operands[0]->defType;
                newInst->operands.push_back(oprnd);
                newInst->Inst = parsed_operands[0]->defType + " " + oprnd->str;
                newInst->op = DEFINE;
                currentF->instructions.push_back(newInst);
                if(debug){
                    cout<< "DEF " << newInst->operands[0]->defType <<" " << newInst->operands[0]->str<<endl;     
                } 
            }
            parsed_operands.clear();  
        }  
    }; 

    template<> struct action < LB_read_array > {
    template< typename Input >
    static void apply( const Input & in, Program & p ){
        auto currentF = p.functions.back();
        auto newInst = new Instruction();
        newInst->op = READ_ARRAY_TUPLE;
        

        auto dest = parsed_operands[0];
        newInst->operands.push_back(dest);
        parsed_operands.erase(parsed_operands.begin());

        auto src = parsed_operands[0];
        newInst->operands.push_back(src);
        parsed_operands.erase(parsed_operands.begin());


        for(auto oprnd : parsed_operands){
          newInst->indices.push_back(oprnd);
        }
        parsed_operands.clear();
        
        newInst->Inst = in.string();
        if(debug){ 
            cout<< "READ_ARRAY_TUPLE" << "<- ";
            for(auto var : newInst->operands){
                cout<< var->str + " ";
            }
            cout<< "INDICES:" << " ";
            for(auto idx : newInst->indices){
                cout<< idx->str + " ";
            }
            cout<<endl;
        }
        currentF->instructions.push_back(newInst);
        }
    };

    template<> struct action < LB_write_array > {
    template< typename Input >
    static void apply( const Input & in, Program & p ){
        auto currentF = p.functions.back();
        auto newInst = new Instruction();
        newInst->op = WRITE_ARRAY_TUPLE;

        auto dest = parsed_operands[0];
        newInst->operands.push_back(dest);
        parsed_operands.erase(parsed_operands.begin());

        auto src = parsed_operands.back();
        newInst->operands.push_back(src);
        parsed_operands.pop_back();

        for(auto oprnd : parsed_operands){
          newInst->indices.push_back(oprnd);
        }
        parsed_operands.clear();

        newInst->Inst = in.string();
        if(debug){ 
            cout<< "WRITE_ARRAY_TUPLE" << " ";
            for(auto var : newInst->operands){
                cout<< var->str + "<- ";
            }
            cout<< "INDICES:" << " ";
            for(auto idx : newInst->indices){
                cout<< idx->str + " ";
            }
            cout<<endl;
        }
        currentF->instructions.push_back(newInst);
        }
    };

    std::map<std::string, Operator> LB_arithMap = {
        {"+", ADD}, 
        {"-", SUB}, 
        {"*", MUL}, 
        {"&", AND}, 
        {"<", LT}, 
        {"<=", LE}, 
        {"=", EQ},
        {">",GT }, 
        {">=",GE },
        {"<<",SAL}, 
        {">>",SAR}
        };

    template<> struct action < LB_op > {
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

        newInst->op = LB_arithMap[arith];

        newInst->Inst = in.string();
        if(debug) cout<< "ARITH_OP " + newInst->operands[0]->str + " <- " + newInst->operands[1]->str + " " + arith + " " + newInst->operands[2]->str << endl;
        currentF->instructions.push_back(newInst);
        }
    };
    
    template<> struct action < LB_return > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        auto currentF = p.functions.back();
        auto newInst = new Instruction();
        newInst->op = RET;
        newInst->Inst = in.string();
        if(debug) cout<< "RET " << in.string() << endl;
        currentF->instructions.push_back(newInst);
        }
    };

    template<> struct action < LB_continue > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        auto currentF = p.functions.back();
        auto newInst = new Instruction();
        newInst->op = CONTINUE;
        newInst->Inst = in.string();
        if(debug) cout<< "CONTINUE " << in.string() << endl;
        currentF->instructions.push_back(newInst);
        }
    };

    template<> struct action < LB_break > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        auto currentF = p.functions.back();
        auto newInst = new Instruction();
        newInst->op = BREAK;
        newInst->Inst = in.string();
        if(debug) cout<< "BREAK " << in.string() << endl;
        currentF->instructions.push_back(newInst);
        }
    };

    template<> struct action < LB_scopeOpen > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        auto currentF = p.functions.back();
        auto newInst = new Instruction();
        newInst->op = SCOPE_OPEN;
        newInst->Inst = in.string();
        scopeVec.push_back(in.string());
        if(debug) cout<< "SCOPE_OPEN " << in.string() << endl;
        currentF->instructions.push_back(newInst);
        }
    };

    template<> struct action < LB_scopeClose > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        auto currentF = p.functions.back();
        auto newInst = new Instruction();
        newInst->op = SCOPE_CLOSE;
        newInst->Inst = in.string();
        if(debug) cout<< "SCOPE_CLOSE " << in.string() << endl;
        currentF->instructions.push_back(newInst);
        }
        
    };


    template<> struct action < LB_return_var > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        auto currentF = p.functions.back();
        auto retVal = parsed_operands.back();
        parsed_operands.pop_back();
        auto newInst = new Instruction();
        newInst->operands.push_back(retVal);
        newInst->op = RETVAR;
        newInst->Inst = in.string();
        if(debug) cout<< "RETVAR return " << newInst->operands[0]->str << endl;
        currentF->instructions.push_back(newInst);
        }
    };

    template<> struct action < LB_label > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        auto currentF = p.functions.back();
        auto lbl = parsed_operands.back();
        parsed_operands.clear();
        auto newInst = new Instruction();
        newInst->operands.push_back(lbl);
        newInst->op = LBL;
        newInst->Inst = in.string();
        if(debug) cout<< "LABEL " << newInst->operands[0]->str << endl;
        currentF->instructions.push_back(newInst);
        }
    };

    template<> struct action < LB_br1 > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        auto currentF = p.functions.back();
        auto newInst = new Instruction();
        auto brLabel = parsed_operands.back();
        parsed_operands.clear();
        newInst->op = JUMP;
        newInst->Inst = in.string();
        newInst->operands.push_back(brLabel);
        if(debug) cout<< "JUMP br " << newInst->operands[0]->str << endl;
        currentF->instructions.push_back(newInst);
        }
    };

    template<> struct action < LB_if > { //if (t op t) label1 babel2
    template< typename Input >
    static void apply( const Input & in, Program & p){
        auto currentF = p.functions.back();
        auto newInst = new Instruction();
        auto Label2 = parsed_operands.back();
        parsed_operands.pop_back();
        auto Label1 = parsed_operands.back();
        parsed_operands.pop_back();
        auto Var2 = parsed_operands.back();
        parsed_operands.pop_back();
        auto Var1 = parsed_operands.back();
        parsed_operands.pop_back();

        std::string arith = parsed_operators.back();
        parsed_operators.pop_back();
        newInst->op = IF;
        newInst->cond = LB_arithMap[arith];

        newInst->Inst = in.string();
        newInst->operands.push_back(Var1);
        newInst->operands.push_back(Var2);
        newInst->operands.push_back(Label1);
        newInst->operands.push_back(Label2);
        if(debug) cout<< "IF if " << newInst->operands[0]->str + " " + arith + " " + newInst->operands[1]->str + " " + newInst->operands[2]->str + " " + newInst->operands[3]->str<< endl;
        currentF->instructions.push_back(newInst);
        }
    };

    template<> struct action < LB_while > { //while (t op t) label1 babel2
    template< typename Input >
    static void apply( const Input & in, Program & p){
        auto currentF = p.functions.back();
        auto newInst = new Instruction();
        auto Label2 = parsed_operands.back();
        parsed_operands.pop_back();
        auto Label1 = parsed_operands.back();
        parsed_operands.pop_back();
        auto Var2 = parsed_operands.back();
        parsed_operands.pop_back();
        auto Var1 = parsed_operands.back();
        parsed_operands.pop_back();

        std::string arith = parsed_operators.back();
        parsed_operators.pop_back();
        newInst->op = WHILE;
        newInst->cond = LB_arithMap[arith];

        newInst->Inst = in.string();
        newInst->operands.push_back(Var1);
        newInst->operands.push_back(Var2);
        newInst->operands.push_back(Label1);
        newInst->operands.push_back(Label2);
        if(debug) cout<< "WHILE while " << newInst->operands[0]->str + " " + arith + " " + newInst->operands[1]->str + " " + newInst->operands[2]->str + " " + newInst->operands[3]->str<< endl;
        currentF->instructions.push_back(newInst);
        }
    };

    template<> struct action < LB_call1 > {
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

    template<> struct action < LB_call2 > {
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

    template<> struct action < LB_print > {
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

    //LB_length
    template<> struct action < LB_length > { //var <- length var t
    template< typename Input >
    static void apply( const Input & in, Program & p){
        auto currentF = p.functions.back();
        auto newInst = new Instruction();
        auto src_t = parsed_operands.back();
        parsed_operands.pop_back();
        auto src_var = parsed_operands.back();
        parsed_operands.pop_back();
        auto dest_var = parsed_operands.back();
        parsed_operands.pop_back();
        newInst->op = LENGTH;
        newInst->Inst = in.string();
        newInst->operands.push_back(dest_var);
        newInst->operands.push_back(src_var);
        newInst->operands.push_back(src_t);
        if(debug) cout<< "LENGTH " << newInst->operands[0]->str + " <- length " + newInst->operands[1]->str + " " + newInst->operands[2]->str << endl;
        currentF->instructions.push_back(newInst);
        }
    };

    //LB_new_array
    template<> struct action < LB_new_array > { //var <- new Array(args)
    template< typename Input >
    static void apply( const Input & in, Program & p ){
        auto currentF = p.functions.back();
        auto newInst = new Instruction();
        newInst->op = NEWARRAY;

        auto dest = parsed_operands[0];
        newInst->operands.push_back(dest);
        parsed_operands.erase(parsed_operands.begin());

        for(auto oprnd : parsed_operands){
          newInst->indices.push_back(oprnd);
        }
        parsed_operands.clear();
        
        newInst->Inst = in.string();
        if(debug){ 
          cout<< "ARRAY " <<  newInst->operands[0]->str << "<- new Array ";
          for(auto var : newInst->indices){
            cout<< var->str + " ";
          }
          cout<<endl;
        }
        currentF->instructions.push_back(newInst);        
        }
    };

    //LB_new_tuple
    template<> struct action < LB_new_tuple > { //var <- new Tuple(t)
    template< typename Input >
    static void apply( const Input & in, Program & p ){
        auto currentF = p.functions.back();
        auto newInst = new Instruction();
        auto src_t = parsed_operands.back();
        parsed_operands.pop_back();
        auto dest_var = parsed_operands.back();
        parsed_operands.pop_back();
        newInst->op = NEWTUPLE;
        newInst->Inst = in.string();
        newInst->operands.push_back(dest_var);
        newInst->indices.push_back(src_t);
        if(debug) cout<< "TUPLE " << newInst->operands[0]->str + " <- new Tuple " + newInst->indices[0]->str << endl;
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
        return p;
    }
}