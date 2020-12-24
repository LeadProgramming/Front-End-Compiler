/*
* Name: David Nguyen (891362089)
* Professor's Name: Professor Anthony Le
* Date: 12/16/19
* Assignment Name: Syntax Analysis
* Assignment #: 3
*/

#include "Compiler.h"

//Helper function to create a line in the ostream.
void createline(ofstream &o) {
    o << setw(42) << setfill('-') << endl;
    o << "\n";
    o << setfill(' ');
}

//reads the input file and uses the lexer to parse the line for tokens.
void Compiler::readFile(const string &fileName) {
    ifstream myFile(fileName);
    if (myFile.is_open()) {
        string line;
        tl init_;
        init_.second = '$';
        tlp terminate;
        get<0>(terminate) = init_;
        while (getline(myFile, line)) {
            tlp_list.push_back(vector<tlp>());
            lexer(line);
            tlp_list[index].push_back(terminate);
            if (get<0>(tlp_list[index][0]).second != "$")
                push_down_LR_driver();
            index++;
        }
    } else
        throw runtime_error("ERROR: FILE CANNOT BE OPENED.");
}

//writes tokens and its corresponding lexeme to an output file.
void Compiler::writeFile(const string &fileName) {
    ofstream myFile(fileName);
    if (myFile.is_open()) {
        myFile << setw(25) << fixed << "Output";
        createline(myFile);
        myFile << endl;
        for (size_t i = 0; i < asm_list.size(); i++) {
            if(get<2>(asm_list[i]) != NULL) {
                myFile << setw(4) << get<0>(asm_list[i]) << setw(8) << get<1>(asm_list[i]) << setw(8)
                       << get<2>(asm_list[i])
                       << endl;
            } else{
                myFile << setw(4) << get<0>(asm_list[i]) << setw(8) << get<1>(asm_list[i]) << setw(8)
                       << endl;
            }
        }
        myFile << endl;
        myFile << setw(28) << "Symbol Table";
        createline(myFile);
        myFile << setw(5) << "Identifier" << setw(20) << "MemoryLocation " << setw(11) << "Type" << endl;
        for (size_t i = 0; i < sym_table.size(); i++) {
            myFile << setw(7) << get<0>(sym_table[i]) << setw(16) << get<1>(sym_table[i]) << setw(18)
                   << get<2>(sym_table[i]) << endl;
        }
        myFile << "Finished." << endl;
    } else throw runtime_error("Cannot open file!");
}

//converts the characters from the input into its column (int).
int Compiler::char_to_col(char c) {
    //comment
    if (c == '!')
        return 0;
        //operator
    else if (c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || c == '<' || c == '>' || c == '=')
        return 1;
        //space
    else if (c == ' ' || c == '\t')
        return 2;
        //separator
    else if (c == '\'' || c == '(' || c == ')' || c == '{' || c == '}' || c == '[' || c == ']' || c == ',' ||
             c == ':' || c == ';' || c == '!' || c == '#')
        return 3;
        //digits
    else if (c >= 48 && c <= 57)
        return 4;
        //letters
    else if ((c >= 65 && c <= 90) || (c >= 97 && c <= 122) || c == '$')
        return 5;
        //decimal point
    else if (c == '.');
    return 6;
}

//token -> regex -> NFSM -> DFSM  = lexer
//we can associate lexeme to its token.
void Compiler::lexer(string line) {
    int col = 0;
    string lexeme;
    bool isComment = 0;
    for (size_t i = 0; i < line.length(); i++) {
        col = char_to_col(line[i]);
        //break the loop if its a comment(0).
        if (col == 0 || isComment) {
            //first time encountering the comment tag.
            if (!isComment)
                isComment = 1;
                //second time encountering the comment tag.
            else
                isComment = 0;
            break;
        }
        //if col is a operator(1) or space or tabs(2) or separator(3), then input char terminates a token.
        if (col == 1 || col == 2 || col == 3) {
            insert_token(state, lexeme);
            lexeme.clear();
            state = 0;
        }
        //if the state is a space, then we skip it.
        if (state == 3) {
            lexeme.clear();
            state = 0;
        }
        //if the state is a operator or separator, then input char terminates a token.
        if (state == 2 || state == 4) {
            insert_token(state, lexeme);
            state = 0;
            lexeme.clear();
        }
        lexeme += line[i];
        state = table[state][col];
    }
    //if the state is not 0 and we reach the null terminator, then the null terminator terminates a token.
    if (state != 0) {
        insert_token(state, lexeme);
        lexeme.clear();
        state = 0;
    }
}

/*
* Once the input char terminates a token.
* then we gather the current state and its parsed lexeme and insert it in a vector.
* The writeFile() function will write the elements to an output file.
*/
void Compiler::insert_token(int st, string lex) {
    tl tk_lx;
    tlp tok_lex_pro;
    if (st == 5 || st == 8) {
        tk_lx.first = "Integer";
        tk_lx.second = lex;
        get<0>(tok_lex_pro) = tk_lx;
        tlp_list[index].push_back(tok_lex_pro);
    } else if (st == 6 || st == 11 || st == 12) {
        bool checkKW = false;
        //binary search because its faster.
        int leftSide = 0;
        int rightSide = kw.size() - 1;
        for (; leftSide <= rightSide;) {
            int index = floor((leftSide + rightSide) / 2);
            if (kw[index] < lex)
                leftSide = index + 1;
            else if (kw[index] > lex)
                rightSide = index - 1;
            else {
                checkKW = true;
                break;
            }
        }
        if (checkKW) {
            tk_lx.first = "Keyword";
            tk_lx.second = lex;
            get<0>(tok_lex_pro) = tk_lx;
            tlp_list[index].push_back(tok_lex_pro);
        } else {
            tk_lx.first = "Identifier";
            tk_lx.second = lex;
            get<0>(tok_lex_pro) = tk_lx;
            tlp_list[index].push_back(tok_lex_pro);
        }
    } else if (st == 2) {
        tk_lx.first = "Operator";
        tk_lx.second = lex;
        get<0>(tok_lex_pro) = tk_lx;
        tlp_list[index].push_back(tok_lex_pro);
    } else if (st == 4) {
        tk_lx.first = "Separator";
        tk_lx.second = lex;
        get<0>(tok_lex_pro) = tk_lx;
        tlp_list[index].push_back(tok_lex_pro);
    } else if (st == 9 || st == 10) {
        tk_lx.first = "Float";
        tk_lx.second = lex;
        get<0>(tok_lex_pro) = tk_lx;
        tlp_list[index].push_back(tok_lex_pro);
    }
}

//----------------------------------------------------------------Syntax Analyzer-----------------------------------------------------------------//
//---------------------------------------------------------LR Parser----------------------------------------------------//

int Compiler::convert_token(const string &tok) {
    if (tok == "if")
        return 0;
    else if (tok == "{")
        return 1;
    else if (tok == "}")
        return 2;
    else if (tok == "else")
        return 3;
    else if (tok == "while")
        return 4;
    else if (tok == "id")
        return 5;
    else if (tok == "int")
        return 6;
    else if (tok == "float")
        return 7;
    else if (tok == "bool")
        return 8;
    else if (tok == ",")
        return 9;
    else if (tok == ";")
        return 10;
    else if (tok == "=")
        return 11;
    else if (tok == "+")
        return 12;
    else if (tok == "-")
        return 13;
    else if (tok == "<")
        return 14;
    else if (tok == ">")
        return 15;
    else if (tok == "*")
        return 16;
    else if (tok == "/")
        return 17;
    else if (tok == "(")
        return 18;
    else if (tok == ")")
        return 19;
    else if (tok == "num")
        return 20;
    else if (tok == "$")
        return 21;
    else if (tok == "S")
        return 22;
    else if (tok == "D")
        return 23;
    else if (tok == "N")
        return 24;
    else if (tok == "M")
        return 25;
    else if (tok == "A")
        return 26;
    else if (tok == "E")
        return 27;
    else if (tok == "T")
        return 28;
    else if (tok == "F")
        return 29;
    else
        return -1;
}

string Compiler::convert2production(int i) {
    if (i >= 1 && i <= 4)
        return "S";
    else if (i == 5)
        return "D";
    else if (i >= 6 && i <= 8)
        return "N";
    else if (i >= 9 && i <= 10)
        return "M";
    else if (i == 11)
        return "A";
    else if (i >= 12 && i <= 16)
        return "E";
    else if (i >= 17 && i <= 19)
        return "T";
    else if (i >= 20 && i <= 22)
        return "F";
    else
        return NULL;
}

int Compiler::num_of_pop(string s) {
    if (s == "1" || s == "2" || s == "6" || s == "7" || s == "8" || s == "10" || s == "16" || s == "19" || s == "21" ||
        s == "22")
        return 1;
    else if (s == "5" || s == "9" || s == "12" || s == "13" || s == "14" || s == "15" || s == "17" || s == "18" ||
             s == "20")
        return 3;
    else if (s == "11")
        return 4;
    else if (s == "4")
        return 5;
    else if (s == "3")
        return 9;
    else
        return -1;
}

string Compiler::check_id_num(int i) {
    if (get<0>(tlp_list[index][i]).first == "Identifier" || get<0>(tlp_list[index][i]).first == "Keyword") {
        if (get<0>(tlp_list[index][i]).second == "int")
            return "int";
        else if (get<0>(tlp_list[index][i]).second == "float")
            return "float";
        else if (get<0>(tlp_list[index][i]).second == "bool")
            return "bool";
        else if (get<0>(tlp_list[index][i]).second == "while")
            return "while";
        else if (get<0>(tlp_list[index][i]).second == "if")
            return "if";
        else if (get<0>(tlp_list[index][i]).second == "else")
            return "else";
        else
            return "id";
    } else if (get<0>(tlp_list[index][i]).first == "Float" || get<0>(tlp_list[index][i]).first == "Integer")
        return "num";
    else
        return get<0>(tlp_list[index][i]).second;
};

void Compiler::push_down_LR_driver() {
    stack<string> s;
    s.push("0");
    int i = 0;
    while (1) {
        //current state (top of stack)
        string curr = s.top();
        //incoming token
        string incoming = check_id_num(i);
        string X = SLR_table[stoi(s.top())][convert_token(incoming)];
        cout << "Top of Stack: " << s.top() << " : " << " Incoming: " << incoming << endl;
        cout << X << endl;
        control(X, i);
        if (X[0] == 's') {
            X.erase(X.begin());
            s.push(incoming);
            s.push(X);
            i++;
        } else if (X[0] == 'r') {
            X.erase(X.begin());
            for (int i = 0; i < num_of_pop(X) * 2; i++)
                s.pop();
            string temp_peek = s.top();
            s.push(convert2production(stoi(X)));
            s.push(SLR_table[stoi(temp_peek)][convert_token(s.top())]);
        } else if (X == "accepted") {
            break;
        } else {
            throw runtime_error("Bad Input!");
        }
    }
}

//------------------------------------------------------Intermediate Code Generation------------------------------------------------------------------//
void Compiler::control(string state, int i) {
    //------------------------------Functions for inserting values into Symbol Table & ASM-------------------------------------//
    if (state == "r6") {
        curr_DT = "integer";
        store_symbol(get<0>(tlp_list[index][i]).second, mem_addr, curr_DT);
        mem_addr++;
    } else if (state == "r7") {
        curr_DT = "float";
        store_symbol(get<0>(tlp_list[index][i]).second, mem_addr, curr_DT);
        mem_addr++;
    } else if (state == "r8") {
        curr_DT = "bool";
        store_symbol(get<0>(tlp_list[index][i]).second, mem_addr, curr_DT);
        mem_addr++;
    } else if (curr_DT == "integer" && state == "s25") {
        store_symbol(get<0>(tlp_list[index][i]).second, mem_addr, curr_DT);
        mem_addr++;
    } else if (curr_DT == "float" && state == "s25") {
        store_symbol(get<0>(tlp_list[index][i]).second, mem_addr, curr_DT);
        mem_addr++;
    } else if (curr_DT == "bool" && state == "s25") {
        store_symbol(get<0>(tlp_list[index][i]).second, mem_addr, curr_DT);
        mem_addr++;
    }
        //resets the flags for the next line.
    else if (state == "accepted") {
        curr_DT = "";
        assignment = false;
        conditional = false;
        addition = false;
        subtraction = false;
        multiplication = false;
        division = false;
        temp_1 = 0;
        temp_2 = 0;
        temp_3 = 0;
        matched_id = false;
        jmp_line = 0;
    }
        //--------------------------------ASM_list--------------------------------//
        //    A -> id = E;     //
    else if (state == "s28") {
        //checks to see if the current identifier is in the symbol table.
        if (check_symbol(get<0>(tlp_list[index][i]).second)) {
            matched_id = true;
            if (conditional) {
                gen_instr("PUSHM", temp_1);
            } else if (addition || subtraction || multiplication || division) {
                //the assignment production is recorded so remove them via popcorns.
                asm_list.pop_back();
                asm_list.pop_back();
                line -= 2;
                gen_instr("PUSHM", temp_2);
                gen_instr("PUSHM", temp_1);
                if (addition)
                    gen_instr("ADD", NULL);
                else if (subtraction)
                    gen_instr("SUB", NULL);
                else if (multiplication)
                    gen_instr("MUL", NULL);
                else if (division)
                    gen_instr("DIV", NULL);
                gen_instr("POPM", temp_3);
            }
        } else {
            throw runtime_error(" Identifier: " + get<0>(tlp_list[index][i]).second + " was never declared.");
        }
    } else if (state == "s29") {
        assignment = true;
    }
        // E -> E + T | E - T
    else if (state == "s33") {
        addition = true;
    } else if (state == "s35") {
        subtraction = true;
    }
        // T -> T * F | T / F
    else if (state == "s42") {
        multiplication = true;
    } else if (state == "s44") {
        division = true;
    }
        //----------------------------Assignment statements (objects & numbers)----------------------------------------------//
    else if (state == "s50" && matched_id && assignment) {
        gen_instr("PUSHI", stoi(get<0>(tlp_list[index][i]).second));
        gen_instr("POPM", temp_1);
    } else if ((state == "r21" && matched_id && assignment) &&
               (!addition && !subtraction && !multiplication && !division)) {
        gen_instr("PUSHM", temp_1);
        gen_instr("POPM", temp_2);
    }
        //--------------------------------------------------------------------------------------------------------------------//
        //    S -> if E { S } else { S }   //
        //    S -> while E { S }           //
    else if (state == "s4") {
        conditional = true;
    }
    else if (state == "s13") {
        gen_instr("LABEL", NULL);
        jmp_line = line;
        conditional = true;
    } else if (state == "r14") {
        gen_instr("LES", NULL);
        conditional = false;
    } else if (state == "r15") {
        gen_instr("GRT", NULL);
        conditional = false;
    } else if (state == "s6") {
        gen_instr("JUMPZ", line + 3);
    }else if (state == "s15"){
        gen_instr("JUMPZ",line+4);
    }else if (state == "s17") {
        gen_instr("JUMP", jmp_line - 1);
    }
}

void Compiler::gen_instr(string op, int oprnd) {
    as asme;
    get<0>(asme) = line;
    get<1>(asme) = op;
    get<2>(asme) = oprnd;
    asm_list.push_back(asme);
    line++;
}

void Compiler::store_symbol(string id, int mem, string type) {
    st sym;
    get<0>(sym) = id;
    get<1>(sym) = mem;
    get<2>(sym) = type;
    sym_table.push_back(sym);
}

bool Compiler::check_symbol(string sym) {
    for (size_t i = 0; i < sym_table.size(); i++) {
        if (get<0>(sym_table[i]) == sym) {
            if (assignment) {
                if (addition || subtraction || multiplication || division) {
                    temp_3 = temp_2;
                }
                temp_2 = temp_1;
            }
            temp_1 = get<1>(sym_table[i]);
            return true;
        }
    }
    return false;
}


/*
bool Compiler::check_type(int type1, int type2){
    //comparing memory addresses.
    if(type1 >= 5000 && type2 >= 5000) {
        string first;
        string second;
        for (int i = 0; i < sym_table.size(); i++) {
            if (type1 == get<1>(sym_table[i])) {
                first = get<2>(sym_table[i]);
            }
            if (type2 == get<1>(sym_table[i])) {
                second = get<2>(sym_table[i]);
            }
        }
        if (first.empty() || second.empty())
            return false;
        else {
            if (first == second) {
                return true;
            } else
                return false;
        }
    }
    //dereferencing mem addr to compare values.
    else{

    }

}
*/
