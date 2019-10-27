#include "parser.h"


namespace pegtl = tao::TAO_PEGTL_NAMESPACE;


using namespace pegtl;
using namespace std;


namespace LA {

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
    // LA keywords,
    // 1) registers, labels and types

    // new LA vars

    
    struct LA_t: 
    pegtl::sor<
      name,
      number
    >{};

    struct LA_args:
    pegtl::star<
      pegtl::seq<
        seps,
        LA_t,
        seps,
        pegtl::opt<
          pegtl::one<','>
        >
      >
    >{};

    struct LA_s: 
    pegtl::sor<
      LA_t,
      label
    >{};

    struct LA_int64:
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

    struct LA_tuple:
    TAOCPP_PEGTL_STRING( "tuple" ){};

    struct LA_code:
    TAOCPP_PEGTL_STRING( "code" ){};

    struct LA_type:
    pegtl::sor<
        LA_int64,
        LA_tuple,
        LA_code
    >{};

    struct LA_T:
    pegtl::sor<
        LA_type,
        TAOCPP_PEGTL_STRING( "void" )
    >{};




    struct LA_index:
    pegtl::seq<
      seps,
      pegtl::one<'['>,
      LA_t,
      pegtl::one<']'>
    >{};

    struct LA_var_index: //name[t][t][t]
    pegtl::seq<
      name,
      pegtl::plus<
        LA_index
      >
    >{};


    struct LA_type_vars:
    pegtl::star<
      pegtl::seq<
        seps,
        LA_type,
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
        pegtl::opt< LA_type_vars >,
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

    // 5) LA Instructions
    struct LA_def_type: pegtl::seq<LA_type, seps, name >{};
    struct LA_mov: pegtl::seq<name, seps, mov_op, seps, LA_s >{}; //name <- s
    struct LA_read_array: pegtl::seq<name, seps, mov_op, seps, LA_var_index >{}; // name <- name([t])
    struct LA_write_array: pegtl::seq<LA_var_index, seps, mov_op, seps, LA_s >{}; // name([t]) <- s
    struct LA_op: pegtl::seq<name, seps, mov_op, seps, LA_t, seps, arith_op, seps, LA_t >{};
    struct LA_cmp: pegtl::seq<name, seps, mov_op, seps, LA_t, seps, cmp_op, seps, LA_t>{};
    struct LA_return: pegtl::seq<str_return>{};
    struct LA_return_var: pegtl::seq<str_return, seps, LA_t>{};
    struct LA_label: pegtl::seq<label>{}; // Label Inst
    struct LA_br1: pegtl::seq<str_br, seps, label>{};
    struct LA_br2: pegtl::seq<str_br, seps, LA_t, seps, label, seps, label>{};
    struct LA_length: pegtl::seq<name, seps, mov_op, seps, str_length, seps, name, seps, LA_t>{};
    struct LA_call1: pegtl::seq<name, seps, pegtl::one< '(' >, seps, LA_args, seps, pegtl::one< ')' >>{};
    struct LA_call2: pegtl::seq<name, seps, mov_op, seps, name, seps, pegtl::one< '(' >, seps, LA_args, seps, pegtl::one< ')' >>{};
    struct LA_print: pegtl::seq<str_print, seps, pegtl::one< '(' >, seps, LA_args, seps, pegtl::one< ')' >>{};
    struct LA_new_array: pegtl::seq<name, seps, mov_op, seps, str_new, seps, str_array, seps, pegtl::one< '(' >, seps, LA_args, seps, pegtl::one< ')' >>{};
    struct LA_new_tuple: pegtl::seq<name, seps, mov_op, seps, str_new, seps, str_tuple, seps, pegtl::one< '(' >, seps, LA_t, seps, pegtl::one< ')' >>{};

    
    struct Instruction_rule:
    pegtl::sor<
        pegtl::seq< pegtl::at<LA_def_type>, LA_def_type >,
        pegtl::seq< pegtl::at<LA_cmp>, LA_cmp >,
        pegtl::seq< pegtl::at<LA_op>, LA_op >,
        pegtl::seq< pegtl::at<LA_read_array>, LA_read_array >,
        pegtl::seq< pegtl::at<LA_write_array>, LA_write_array >,
        pegtl::seq< pegtl::at<LA_length>, LA_length >,
        pegtl::seq< pegtl::at<LA_print>, LA_print >,
        pegtl::seq< pegtl::at<LA_call1>, LA_call1 >,
        pegtl::seq< pegtl::at<LA_call2>, LA_call2 >,
        
        pegtl::seq< pegtl::at<LA_new_array>, LA_new_array >,
        pegtl::seq< pegtl::at<LA_new_tuple>, LA_new_tuple >,
        pegtl::seq< pegtl::at<LA_return_var>, LA_return_var >,
        pegtl::seq< pegtl::at<LA_return>, LA_return >,  
        pegtl::seq< pegtl::at<LA_br2>, LA_br2 >,
        pegtl::seq< pegtl::at<LA_br1>, LA_br1 >,
        pegtl::seq< pegtl::at<LA_label>, LA_label >, 
        pegtl::seq< pegtl::at<LA_mov>, LA_mov >  
    > {};


    struct Instructions_rule:
    pegtl::plus<
      pegtl::seq<
        seps,
        Instruction_rule,
        seps
      >
    > {};


    struct Function_rule:
    pegtl::seq<
      seps,
      LA_T,
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

    template<> struct action < LA_type > {
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
    template<> struct action < LA_mov > { // var <- s 
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

    template<> struct action < LA_def_type > { // type var -- we can just ignore these
    template< typename Input >
    static void apply( const Input & in, Program & p ){
        auto currentF = p.functions.back();
        auto newInst = new Instruction();
        auto newInst2 = new Instruction();
        auto def = parsed_operands.back();
        parsed_operands.pop_back();

        newInst->operands.push_back(def);
        newInst->Inst = in.string();
        newInst->op = DEFINE;
        if(debug)cout<< "DEF type " << def->defType <<" "<<def->str << endl;
        if(def->defType == "tuple") currentF->tupleDefs.push_back(def->str);
        if(def->defType == "code") currentF->codeDefs.push_back(def->str);
        currentF->instructions.push_back(newInst);
        std::size_t arrayDec = newInst->Inst.find("[]");
        std::size_t tupleDec = newInst->Inst.find("tuple");
        if ((arrayDec!=std::string::npos) || (tupleDec!=std::string::npos)){
            newInst2->op = MOV;
            auto dest = new Variable();
            dest->op_type = def->op_type;
            dest->str =  def->str;
            auto src = new Variable();
            src->op_type = NUM;
            src->dontEncode = true;
            src->str = "0";
            newInst2->operands.push_back(dest);
            newInst2->operands.push_back(src);
            newInst2->Inst = dest->str + " <- " + src->str;
            currentF->instructions.push_back(newInst2);
            if(debug) cout<< "DEF " << newInst2->Inst << endl;
        }
        }
    }; 

    template<> struct action < LA_read_array > {
    template< typename Input >
    static void apply( const Input & in, Program & p ){
        auto currentF = p.functions.back();
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
        currentF->instructions.push_back(newInst);
        }
    };

    template<> struct action < LA_write_array > {
    template< typename Input >
    static void apply( const Input & in, Program & p ){
        auto currentF = p.functions.back();
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
        currentF->instructions.push_back(newInst);
        }
    };

    template<> struct action < LA_op > {
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
        if(debug) cout<< "ARITH_OP " + newInst->operands[0]->str + " <- " + newInst->operands[1]->str + " " + arith + " " + newInst->operands[2]->str << endl;
        currentF->instructions.push_back(newInst);
        }
    };
    
    template<> struct action < LA_cmp > {
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
        if(debug) cout<< "CMP_OP " + newInst->operands[0]->str + " <- " + newInst->operands[1]->str + " " + cmpop + " " + newInst->operands[2]->str << endl;
        currentF->instructions.push_back(newInst);
        }
    };

    template<> struct action < LA_return > {
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

    template<> struct action < LA_return_var > {
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

    template<> struct action < LA_label > {
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

    template<> struct action < LA_br1 > {
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

    template<> struct action < LA_br2 > { //br t label1 babel2
    template< typename Input >
    static void apply( const Input & in, Program & p){
        auto currentF = p.functions.back();
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
        currentF->instructions.push_back(newInst);
        }
    };

    template<> struct action < LA_call1 > {
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

    template<> struct action < LA_call2 > {
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

    template<> struct action < LA_print > {
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

    //LA_length
    template<> struct action < LA_length > { //var <- length var t
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

    //LA_new_array
    template<> struct action < LA_new_array > { //var <- new Array(args)
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

    //LA_new_tuple
    template<> struct action < LA_new_tuple > { //var <- new Tuple(t)
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