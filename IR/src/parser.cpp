#include "parser.h"



namespace pegtl = tao::TAO_PEGTL_NAMESPACE;


using namespace pegtl;
using namespace std;


namespace IR {

    bool debug = 0;

    /*
    * Data required to parse
    */
    std::vector<Variable *> parsed_operands;
    std::vector<std::string> parsed_operators;
    std::vector<std::string> parsed_deftypes;
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


    /*
    * Keywords.
    */
    // IR keywords,
    // 1) registers, labels and types

    // new IR vars
    struct IR_var:
    pegtl::seq<
      pegtl::one<'%'>,
      name
    >{};
    
    struct IR_t: 
    pegtl::sor<
      IR_var,
      number
    >{};

    struct IR_args:
    pegtl::star<
      pegtl::seq<
        seps,
        IR_t,
        seps,
        pegtl::opt<
          pegtl::one<','>
        >
      >
    >{};

    struct IR_s: 
    pegtl::sor<
      IR_t,
      label
    >{};

    struct IR_u: 
    pegtl::sor<
      IR_var,
      label
    >{};

    struct IR_int64:
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

    struct IR_tuple:
    TAOCPP_PEGTL_STRING( "tuple" ){};

    struct IR_code:
    TAOCPP_PEGTL_STRING( "code" ){};

    struct IR_type:
    pegtl::sor<
        IR_int64,
        IR_tuple,
        IR_code
    >{};

    struct IR_T:
    pegtl::sor<
        IR_type,
        TAOCPP_PEGTL_STRING( "void" )
    >{};




    struct IR_index:
    pegtl::seq<
      seps,
      pegtl::one<'['>,
      IR_t,
      pegtl::one<']'>
    >{};

    struct IR_var_index: //var[t][t][t]
    pegtl::seq<
      IR_var,
      pegtl::plus<
        IR_index
      >
    >{};


    struct IR_type_vars:
    pegtl::star<
      pegtl::seq<
        seps,
        IR_type,
        seps,
        IR_var,
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
        pegtl::opt< IR_type_vars >,
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
    struct str_arrayerror: TAOCPP_PEGTL_STRING( "array-error" ){};
    struct str_br: TAOCPP_PEGTL_STRING( "br" ){};
    struct str_return: TAOCPP_PEGTL_STRING( "return" ){};
    struct str_define: TAOCPP_PEGTL_STRING( "define" ){};
    struct str_main: TAOCPP_PEGTL_STRING( ":main" ){};
    struct str_new: TAOCPP_PEGTL_STRING( "new" ){};
    struct str_array: TAOCPP_PEGTL_STRING( "Array" ){};
    struct str_tuple: TAOCPP_PEGTL_STRING( "Tuple" ){};
    struct str_length: TAOCPP_PEGTL_STRING( "length" ){};

    // 5) IR Instructions
    struct IR_def_type: pegtl::seq<IR_type, seps, IR_var >{};
    struct IR_mov: pegtl::seq<IR_var, seps, mov_op, seps, IR_s >{}; //var <- s
    struct IR_read_array: pegtl::seq<IR_var, seps, mov_op, seps, IR_var_index >{}; // var <- var([t])
    struct IR_write_array: pegtl::seq<IR_var_index, seps, mov_op, seps, IR_s >{}; // var([t]) <- s
    struct IR_op: pegtl::seq<IR_var, seps, mov_op, seps, IR_t, seps, arith_op, seps, IR_t >{};
    struct IR_cmp: pegtl::seq<IR_var, seps, mov_op, seps, IR_t, seps, cmp_op, seps, IR_t>{};
    struct IR_return: pegtl::seq<str_return>{};
    struct IR_return_var: pegtl::seq<str_return, seps, IR_t>{};
    struct IR_label: pegtl::seq<label>{}; // Label Inst
    struct IR_br1: pegtl::seq<str_br, seps, label>{};
    struct IR_br2: pegtl::seq<str_br, seps, IR_t, seps, label, seps, label>{};
    struct IR_length: pegtl::seq<IR_var, seps, mov_op, seps, str_length, seps, IR_var, seps, IR_t>{};
    struct IR_call1: pegtl::seq<str_call, seps, IR_u, seps, pegtl::one< '(' >, seps, IR_args, seps, pegtl::one< ')' >>{};
    struct IR_call2: pegtl::seq<IR_var, seps, mov_op, seps, str_call, seps, IR_u, seps, pegtl::one< '(' >, seps, IR_args, seps, pegtl::one< ')' >>{};
    struct IR_print: pegtl::seq<str_call, seps, str_print, seps, pegtl::one< '(' >, seps, IR_args, seps, pegtl::one< ')' >>{};
    struct IR_arrayerror: pegtl::seq<str_call, seps, str_arrayerror, seps, pegtl::one< '(' >, seps, IR_args, seps, pegtl::one< ')' >>{};
    struct IR_new_array: pegtl::seq<IR_var, seps, mov_op, seps, str_new, seps, str_array, seps, pegtl::one< '(' >, seps, IR_args, seps, pegtl::one< ')' >>{};
    struct IR_new_tuple: pegtl::seq<IR_var, seps, mov_op, seps, str_new, seps, str_tuple, seps, pegtl::one< '(' >, seps, IR_t, seps, pegtl::one< ')' >>{};

    
    struct Instruction_rule:
    pegtl::sor<
        pegtl::seq< pegtl::at<IR_def_type>, IR_def_type >,
        pegtl::seq< pegtl::at<IR_cmp>, IR_cmp >,
        pegtl::seq< pegtl::at<IR_op>, IR_op >,
        pegtl::seq< pegtl::at<IR_read_array>, IR_read_array >,
        pegtl::seq< pegtl::at<IR_write_array>, IR_write_array >,
        pegtl::seq< pegtl::at<IR_length>, IR_length >,
        pegtl::seq< pegtl::at<IR_mov>, IR_mov >,
        pegtl::seq< pegtl::at<IR_call1>, IR_call1 >,
        pegtl::seq< pegtl::at<IR_call2>, IR_call2 >,
        pegtl::seq< pegtl::at<IR_print>, IR_print >,
        pegtl::seq< pegtl::at<IR_arrayerror>, IR_arrayerror >,
        pegtl::seq< pegtl::at<IR_new_array>, IR_new_array >,
        pegtl::seq< pegtl::at<IR_new_tuple>, IR_new_tuple >
    > {};

    struct Te_rule:
    pegtl::sor<
        pegtl::seq< pegtl::at<IR_return_var>, IR_return_var >,
        pegtl::seq< pegtl::at<IR_return>, IR_return >,  
        pegtl::seq< pegtl::at<IR_br1>, IR_br1 >,
        pegtl::seq< pegtl::at<IR_br2>, IR_br2 >
    >{};

    struct Instructions_rule:
    pegtl::plus<
      pegtl::seq<
        seps,
        Instruction_rule,
        seps
      >
    > {};

    struct BasicBlock_rule: //bb
    pegtl::seq<
        IR_label,
        seps,
        pegtl::opt< Instructions_rule>,
        seps,
        Te_rule
    >{};

    struct BasicBlocks_rule: //bbs
    pegtl::plus<
      pegtl::seq<
        seps,
        BasicBlock_rule,
        seps
      >
    > {};

    struct Function_rule:
    pegtl::seq<
      seps,
      str_define,
      seps,
      IR_T,
      seps,
      function_head,
      seps,
      pegtl::one< '{' >,
      seps,
      BasicBlocks_rule,
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
                if(op->defType == "tuple") currentF->tupleDefs.push_back(op->str);
                if(op->defType == "code") currentF->codeDefs.push_back(op->str);
                currentF->arguments.push_back(op); // reversed order of original arguments - handled later
                parsed_operands.pop_back();
                }
            }
        };

    //Operands = var | N | Label
    template<> struct action < IR_var > {
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
            parsed_operands.push_back(newOp);
        }
    };

    //defType void | int64[] | tuple | code

    template<> struct action < IR_type > {
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
    template<> struct action < IR_mov > { // var <- s 
    template< typename Input >
    static void apply( const Input & in, Program & p ){
        auto currentF = p.functions.back();
        auto bb = currentF->bblocks.back();
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
        bb->instructions.push_back(newInst);
        }
    }; 

    template<> struct action < IR_def_type > { // type var -- we can just ignore these
    template< typename Input >
    static void apply( const Input & in, Program & p ){
        auto currentF = p.functions.back();
        auto bb = currentF->bblocks.back();
        auto newInst = new Instruction();
        auto def = parsed_operands.back();
        parsed_operands.pop_back();
        newInst->operands.push_back(def);
        newInst->Inst = in.string();
        newInst->op = DEFINE;
        if(debug)cout<< "DEF type " << def->defType <<" "<<def->str << endl;
        if(def->defType == "tuple") currentF->tupleDefs.push_back(def->str);
        if(def->defType == "code") currentF->codeDefs.push_back(def->str);
        bb->instructions.push_back(newInst);
        }
    }; 

    template<> struct action < IR_read_array > {
    template< typename Input >
    static void apply( const Input & in, Program & p ){
        auto currentF = p.functions.back();
        auto bb = currentF->bblocks.back();
        auto newInst = new Instruction();
        newInst->op = READ_ARRAY;

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
            cout<< "READ_ARRAY" << "<- ";
            for(auto var : newInst->operands){
                cout<< var->str + " ";
            }
            cout<< "INDICES:" << " ";
            for(auto idx : newInst->indices){
                cout<< idx->str + " ";
            }
            cout<<endl;
        }
        bb->instructions.push_back(newInst);
        }
    };

    template<> struct action < IR_write_array > {
    template< typename Input >
    static void apply( const Input & in, Program & p ){
        auto currentF = p.functions.back();
        auto bb = currentF->bblocks.back();
        auto newInst = new Instruction();
        newInst->op = WRITE_ARRAY;

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
            cout<< "WRITE_ARRAY" << " ";
            for(auto var : newInst->operands){
                cout<< var->str + "<- ";
            }
            cout<< "INDICES:" << " ";
            for(auto idx : newInst->indices){
                cout<< idx->str + " ";
            }
            cout<<endl;
        }
        bb->instructions.push_back(newInst);
        }
    };

    template<> struct action < IR_op > {
    template< typename Input >
    static void apply( const Input & in, Program & p ){
        auto currentF = p.functions.back();
        auto bb = currentF->bblocks.back();
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
        if(debug) cout<< "ARITH_OP " + newInst->operands[0]->str + " <- " + newInst->operands[1]->str + " " + arith + " " + newInst->operands[2]->str << endl;
        bb->instructions.push_back(newInst);
        }
    };
    
    template<> struct action < IR_cmp > {
    template< typename Input >
    static void apply( const Input & in, Program & p ){
        auto currentF = p.functions.back();
        auto bb = currentF->bblocks.back();
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
        if(debug) cout<< "CMP_OP " + newInst->operands[0]->str + " <- " + newInst->operands[1]->str + " " + cmpop + " " + newInst->operands[2]->str << endl;
        bb->instructions.push_back(newInst);
        }
    };

    template<> struct action < IR_return > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        auto currentF = p.functions.back();
        auto bb = currentF->bblocks.back();
        auto newInst = new Instruction();
        newInst->op = RET;
        newInst->Inst = in.string();
        if(debug) cout<< "RET " << in.string() << endl;
        bb->exit = newInst;
        }
    };

    template<> struct action < IR_return_var > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        auto currentF = p.functions.back();
        auto bb = currentF->bblocks.back();
        auto retVal = parsed_operands.back();
        parsed_operands.pop_back();
        auto newInst = new Instruction();
        newInst->operands.push_back(retVal);
        newInst->op = RETVAR;
        newInst->Inst = in.string();
        if(debug) cout<< "RETVAR return " << newInst->operands[0]->str << endl;
        bb->exit = newInst;
        }
    };

    template<> struct action < IR_label > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        auto currentF = p.functions.back();
        auto bb = new BasicBlock();
        auto lbl = parsed_operands.back();
        parsed_operands.pop_back();
        auto newInst = new Instruction();
        newInst->operands.push_back(lbl);
        newInst->op = LBL;
        newInst->Inst = in.string();
        if(debug) cout<< "LABEL " << newInst->operands[0]->str << endl;
        bb->entry_label = newInst;
        currentF->bblocks.push_back(bb);
        }
    };

    template<> struct action < IR_br1 > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        auto currentF = p.functions.back();
        auto bb = currentF->bblocks.back();
        auto newInst = new Instruction();
        auto brLabel = parsed_operands.back();
        parsed_operands.pop_back();
        newInst->op = JUMP;
        newInst->Inst = in.string();
        newInst->operands.push_back(brLabel);
        if(debug) cout<< "JUMP br " << newInst->operands[0]->str << endl;
        bb->exit = newInst;
        }
    };

    template<> struct action < IR_br2 > { //br t label1 babel2
    template< typename Input >
    static void apply( const Input & in, Program & p){
        auto currentF = p.functions.back();
        auto bb = currentF->bblocks.back();
        auto newInst = new Instruction();
        auto brLabel2 = parsed_operands.back();
        parsed_operands.pop_back();
        auto brLabel1 = parsed_operands.back();
        parsed_operands.pop_back();
        auto brVar = parsed_operands.back();
        parsed_operands.pop_back();
        newInst->op = CJUMP;
        newInst->Inst = in.string();
        newInst->operands.push_back(brVar);
        newInst->operands.push_back(brLabel1);
        newInst->operands.push_back(brLabel2);
        if(debug) cout<< "CJUMP br " << newInst->operands[0]->str + " " + newInst->operands[1]->str + " " + newInst->operands[2]->str << endl;
        bb->exit = newInst;
        }
    };

    template<> struct action < IR_call1 > {
    template< typename Input >
    static void apply( const Input & in, Program & p ){
        auto currentF = p.functions.back();
        auto bb = currentF->bblocks.back();
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
        bb->instructions.push_back(newInst);
        }
    };

    template<> struct action < IR_call2 > {
    template< typename Input >
    static void apply( const Input & in, Program & p ){
        auto currentF = p.functions.back();
        auto bb = currentF->bblocks.back();
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
        bb->instructions.push_back(newInst);
        }
    };

    template<> struct action < IR_print > {
    template< typename Input >
    static void apply( const Input & in, Program & p ){
        auto currentF = p.functions.back();
        auto bb = currentF->bblocks.back();
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
        bb->instructions.push_back(newInst);
        }
    };

    template<> struct action < IR_arrayerror > {
    template< typename Input >
    static void apply( const Input & in, Program & p ){
        auto currentF = p.functions.back();
        auto bb = currentF->bblocks.back();
        auto newInst = new Instruction();
        newInst->op = ARRAYERROR;

        for(auto oprnd : parsed_operands){
          newInst->operands.push_back(oprnd);
        }
        parsed_operands.clear();

        newInst->Inst = in.string();
        if(debug){ 
          cout<< "ARRAYERROR" << " ";
          for(auto var : newInst->operands){
            cout<< var->str + " ";
          }
          cout<<endl;
        }
        bb->instructions.push_back(newInst);
        }
    };


    //IR_length
    template<> struct action < IR_length > { //var <- length var t
    template< typename Input >
    static void apply( const Input & in, Program & p){
        auto currentF = p.functions.back();
        auto bb = currentF->bblocks.back();
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
        bb->instructions.push_back(newInst);
        }
    };

    //IR_new_array
    template<> struct action < IR_new_array > { //var <- new Array(args)
    template< typename Input >
    static void apply( const Input & in, Program & p ){
        auto currentF = p.functions.back();
        auto bb = currentF->bblocks.back();
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
        bb->instructions.push_back(newInst);        
        }
    };

    //IR_new_tuple
    template<> struct action < IR_new_tuple > { //var <- new Tuple(t)
    template< typename Input >
    static void apply( const Input & in, Program & p ){
        auto currentF = p.functions.back();
        auto bb = currentF->bblocks.back();
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
        bb->instructions.push_back(newInst);
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