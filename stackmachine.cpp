#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "typedefine.hpp"
#include "stackmachine.hpp"


// 文字列を大文字に変換する
void upper(char *str){
    int i;
    for(i=0; str[i]!='\0'; i++){
        str[i] = toupper(str[i]);
    }
}

// 空白、コンマ、改行、タブかどうかを調べる
int is_blank(char c){
    switch (c) {
        case ' ':
        case ',':
        case '\n':
        case '\t':
            return 1;
        default:
            break;
    }
    return 0;
}

// スタックの中身を出力
void dump_stack(int *stack, int sp){
    int i;
    printf("--- Stack Dump ---\n");
    for(i=0; i<sp; i++){
        printf("%d: %d\n", i, stack[i]);
    }
    printf("----------------\n");
}

// codeを実行する
void execute_code(instruction *code){
    int stack[MAX_STACK];
    int disp[MAX_LEVEL];

    instruction ireg;  // 実行する命令
    int sp = 0;   // スタックポインタ
    int ip = 0;   // インタラクションポインタ

    int i, lev, ret_val;

    // Initialize
    disp[0] = 0;
    stack[0] = 0;

    printf(">> Execute\n");

    while(1){
        if(ip >= MAX_CODE){
            printf("Error: Too many code.\n");
            return;
        }
        if(sp >= MAX_STACK){
            printf("Error: Stack Over fllow.\n");
            return;
        }

        ireg = code[ip++];
        switch(ireg.func){
            case NOP:
                continue;
            case CPY:
                for(i=0; i<ireg.u.value; i++){
                    stack[sp++] = stack[sp-2];
                }
                break;
            case LOD:
                // Push
                stack[sp++] = stack[disp[ireg.u.addr.level] + ireg.u.addr.addr];
                break;
            case LBI:
                // Push by index
                if(stack[sp-1] < 0){
                    printf("Error: Index is must be greater than or equeal 0 !!\n");
                    exit(1);
                }else if((disp[ireg.u.addr.level] + ireg.u.addr.addr + stack[sp-1]) >= sp){
                    printf("Error: Too big Index!!\n");
                    exit(1);
                }
                stack[sp-1] = stack[disp[ireg.u.addr.level] + ireg.u.addr.addr + stack[sp-1]];
                break;
            case STO:
                // Pop
                stack[disp[ireg.u.addr.level] + ireg.u.addr.addr] = stack[--sp];
                break;
            case SBI:
                // Pop by index
                if(stack[sp-1] < 0){
                    printf("Error: Index is must be greater than or equeal 0 !!\n");
                    exit(1);
                }else if((disp[ireg.u.addr.level] + ireg.u.addr.addr + stack[sp-2]) >= sp){
                    printf("Error: Too big Index!!\n");
                    exit(1);
                }
                stack[disp[ireg.u.addr.level] + ireg.u.addr.addr + stack[sp-2]] = stack[sp-1];
                sp -= 2;  // delete value and Offset
                break;
            case CAL:
                // Call
                lev = ireg.u.addr.level + 1;
                stack[sp] = disp[lev];  // disp の退避
                stack[sp+1] = ip;       // 戻り先codeの退避
                disp[lev] = sp;         // spの退避
                ip = ireg.u.addr.addr;  // サブルーチンアドレスへ
                break;
            case RET:
                // Return
                ret_val = stack[--sp];  // 戻り値を一時領域へ

                lev = ireg.u.addr.level;
                sp = disp[lev];         // sp の回復
                disp[lev] = stack[sp];  // disp の回復
                ip = stack[sp+1];       // 戻りcodeアドレスへ
                sp -= ireg.u.addr.addr; // 引数を削除

                stack[sp++] = ret_val;  // 戻り値をpush
                break;
            case LIT:
                stack[sp++] = ireg.u.value;
                break;
            case INC:
                sp += ireg.u.value;
                break;
            case JMP:
                ip = ireg.u.addr.addr;
                continue;
            case JPC:
                if(stack[--sp] != 0){
                    ip = ireg.u.addr.addr;
                }
                continue;
            case OPR:
                switch (ireg.u.opcode) {
                    // 四則演算
                    case ADD:
                        stack[sp-2] = stack[sp-2] + stack[sp-1];
                        sp--; break;
                    case SUB:
                        stack[sp-2] = stack[sp-2] - stack[sp-1];
                        sp--; break;
                    case MUL:
                        stack[sp-2] = stack[sp-2] * stack[sp-1];
                        sp--; break;
                    case DIV:
                        if(stack[sp-1] == 0){
                            printf("Error: Dived by Zero!!\n");
                            exit(1);
                        }
                        stack[sp-2] = stack[sp-2] / stack[sp-1];
                        sp--; break;
                    case MOD:
                        if(stack[sp-1] == 0){
                            printf("Error: Dived by Zero!!\n");
                            exit(1);
                        }
                        stack[sp-2] = stack[sp-2] % stack[sp-1];
                        sp--; break;

                    // ビット演算
                    case NOT:
                        stack[sp-1] = ~stack[sp-1];
                        break;
                    case OR:
                        stack[sp-2] = stack[sp-2] | stack[sp-1];
                        sp--; break;
                    case AND:
                        stack[sp-2] = stack[sp-2] & stack[sp-1];
                        sp--; break;
                    case XOR:
                        stack[sp-2] = stack[sp-2] ^ stack[sp-1];
                        sp--; break;
                    case SHL:
                        stack[sp-2] = stack[sp-2] << stack[sp-1];
                        sp--; break;
                    case SHR:
                        stack[sp-2] = stack[sp-2] >> stack[sp-1];
                        sp--; break;

                    // 比較
                    case EQ:
                        stack[sp-2] = stack[sp-2] == stack[sp-1];
                        sp--; break;
                    case NEQ:
                        stack[sp-2] = stack[sp-2] != stack[sp-1];
                        sp--; break;
                    case LT:
                        stack[sp-2] = stack[sp-2] < stack[sp-1];
                        sp--; break;
                    case LE:
                        stack[sp-2] = stack[sp-2] <= stack[sp-1];
                        sp--; break;
                    case GT:
                        stack[sp-2] = stack[sp-2] > stack[sp-1];
                        sp--; break;
                    case GE:
                        stack[sp-2] = stack[sp-2] >= stack[sp-1];
                        sp--; break;
                    case ODD:
                        stack[sp-1] = 0x1 & stack[sp-1];
                        break;
                    case WRT:
                        printf("%d", stack[--sp]);
                        break;
                    case WRL:
                        printf("\n");
                        break;
                }
                break;
            default:
                dump_stack(stack, sp);
                return;
        }
    }
}

// コードを表示する
void print_code(instruction *code){
    instruction ireg;
    int ip = 0;

    while(1){
        if(ip >= MAX_CODE){
            printf("Error: Too many code.\n");
            return;
        }
        ireg = code[ip++];

        printf("%6d: ", ip-1);
        switch(ireg.func){
            case CPY: printf("cpy"); break;
            case NOP: printf("nop"); break;
            case LOD: printf("lod"); break;
            case LBI: printf("lbi"); break;
            case STO: printf("sto"); break;
            case SBI: printf("sbi"); break;
            case CAL: printf("cal"); break;
            case RET: printf("ret"); break;
            case LIT: printf("lit"); break;
            case INC: printf("inc"); break;
            case JMP: printf("jmp"); break;
            case JPC: printf("jpc"); break;
            case OPR: printf("opr"); break;
            default:
                printf("END\n\n");
                return;
        }

        printf(", ");

        if(ireg.func != OPR){
            switch (ireg.func) {
                case LOD:
                case LBI:
                case STO:
                case SBI:
                case CAL:
                case RET:
                    printf("%d, %d", ireg.u.addr.level, ireg.u.addr.addr);
                    break;
                case JPC:
                case JMP:
                    printf("%d", ireg.u.addr.addr);
                    break;
                case LIT:
                case INC:
                case CPY:
                    printf("%d", ireg.u.value);
                    break;
                case NOP:
                default:
                    break;
            }

        }else{
            switch (ireg.u.opcode) {
                // 四則演算
                case ADD:
                    printf("add"); break;
                case SUB:
                    printf("sub"); break;
                case MUL:
                    printf("mul"); break;
                case DIV:
                    printf("div"); break;
                case MOD:
                    printf("mod");  break;

                // ビット演算
                case NOT:
                    printf("not");  break;
                case OR:
                    printf("or");  break;
                case AND:
                    printf("and");  break;
                case XOR:
                    printf("xor");  break;
                case SHL:
                    printf("shl");  break;
                case SHR:
                    printf("shr");  break;

                // 比較
                case EQ:
                    printf("eq");  break;
                case NEQ:
                    printf("neq"); break;
                case LT:
                    printf("lt");  break;
                case LE:
                    printf("le");  break;
                case GT:
                    printf("gt");  break;
                case GE:
                    printf("ge");  break;
                case ODD:
                    printf("ODD");  break;

                // 出力
                case WRT:
                    printf("wrt"); break;
                case WRL:
                    printf("wrl"); break;
            }
        }
        printf("\n");
    }
}

// 1行の文字列を命令に変換する
// inst = f("lod 1 3")
// inst.func = lod, inst.u.addr.level = 1, inst.u.addr.addr = 3
instruction line_to_inst(char *line){
    instruction inst;
    int lp = 0;
    int i;
    char func[4];
    char a[MAX_LINE];
    char b[MAX_LINE];

    a[0] = '\0';
    b[0] = '\0';

    // 空白を捨てる
    while(is_blank(line[lp])) lp++;

    // 何もない行及びコメントのための";"を無視する
    if(line[lp] == '\0' || line[lp] == ';'){
        inst.func = IGN;
        return inst;
    }

    // funcを読み取る
    func[0] = line[lp++];
    func[1] = line[lp++];
    func[2] = line[lp++];
    func[3] = '\0';

    while(is_blank(line[lp])) lp++;  // 空白を捨てる

    // カンマか空白、終端文字、改行文字まで捨てる
    i = 0;
    while(1){
        if(is_blank(line[lp]) || line[lp] == '\0')
            break;
        a[i++] = line[lp++];
    }
    a[i++] = '\0';
    lp++;

    // 次の文字が終端文字じゃないときbがある可能性がある
    i = 0;
    if(line[lp] != '\0'){
        while(is_blank(line[lp])) lp++;  // 空白を捨てる

        // bがあった
        if(line[lp] != '\0'){
            while(1){
                if(is_blank(line[lp]) || line[lp] == '\0')
                    break;
                b[i++] = line[lp++];
            }
            b[i++] = '\0';
        }
    }

    // 命令
    if(strcmp("LOD", func) == 0){
        inst.func = LOD;
    }else if(strcmp("LBI", func) == 0){
        inst.func = LBI;
    }else if(strcmp("STO", func) == 0){
        inst.func = STO;
    }else if(strcmp("SBI", func) == 0){
        inst.func = SBI;
    }else if(strcmp("CAL", func) == 0){
        inst.func = CAL;
    }else if(strcmp("RET", func) == 0){
        inst.func = RET;
    }else if(strcmp("LIT", func) == 0){
        inst.func = LIT;
    }else if(strcmp("INC", func) == 0){
        inst.func = INC;
    }else if(strcmp("JMP", func) == 0){
        inst.func = JMP;
    }else if(strcmp("JPC", func) == 0){
        inst.func = JPC;
    }else if(strcmp("OPR", func) == 0){
        inst.func = OPR;
    }else if(strcmp("CPY", func) == 0){
        inst.func = CPY;
    }else if(strcmp("NOP", func) == 0){
        inst.func = NOP;
        return inst;
    }else{
        inst.func = END;
        return inst;
    }

    // オペコード
    if(inst.func == OPR){
        if(strcmp("ADD", a) == 0){      // 四則演算
            inst.u.opcode = ADD;
        }else if(strcmp("SUB", a) == 0){
            inst.u.opcode = SUB;
        }else if(strcmp("MUL", a) == 0){
            inst.u.opcode = MUL;
        }else if(strcmp("DIV", a) == 0){
            inst.u.opcode = DIV;
        }else if(strcmp("MOD", a) == 0){
            inst.u.opcode = MOD;
        }else if(strcmp("OR", a) == 0){  // ビット演算
            inst.u.opcode = OR;
        }else if(strcmp("AND", a) == 0){
            inst.u.opcode = AND;
        }else if(strcmp("NOT", a) == 0){
            inst.u.opcode = NOT;
        }else if(strcmp("XOR", a) == 0){
            inst.u.opcode = XOR;
        }else if(strcmp("SHR", a) == 0){
            inst.u.opcode = SHR;
        }else if(strcmp("SHL", a) == 0){
            inst.u.opcode = SHL;
        }else if(strcmp("EQ", a) == 0){  // 比較
            inst.u.opcode = EQ;
        }else if(strcmp("NEQ", a) == 0){
            inst.u.opcode = NEQ;
        }else if(strcmp("LT", a) == 0){
            inst.u.opcode = LT;
        }else if(strcmp("LE", a) == 0){
            inst.u.opcode = LE;
        }else if(strcmp("GT", a) == 0){
            inst.u.opcode = GT;
        }else if(strcmp("GE", a) == 0){
            inst.u.opcode = GE;
        }else if(strcmp("ODD", a) == 0){
            inst.u.opcode = ODD;
        }else if(strcmp("WRT", a) == 0){  // 出力
            inst.u.opcode = WRT;
        }else if(strcmp("WRL", a) == 0){
            inst.u.opcode = WRL;
        }else{
            // オペコードが無い
            inst.func = END;
            return inst;
        }
    }else{
        if(a[0] != '\0'){
            switch (inst.func) {
                case LOD:
                case LBI:
                case STO:
                case SBI:
                case CAL:
                case RET:
                    if(b[0] != '\0'){
                        inst.u.addr.level = atoi(a);
                        inst.u.addr.addr = atoi(b);
                    }else{
                        // bが無い
                        inst.func = END;
                        return inst;
                    }
                    break;
                case JPC:
                case JMP:
                    inst.u.addr.addr = atoi(a);
                    break;
                case LIT:
                case INC:
                case CPY:
                    inst.u.value = atoi(a);
                    break;
                case NOP:
                default:
                    break;
            }
        }else{
            // aが存在しない
            inst.func = END;
            return inst;
        }
    }

    return inst;
}

// スタックマシンの言語で書かれたファイルを引数codeに格納する
int read_code(char *filename, instruction *code){
    FILE *fp;
    char line[MAX_LINE];
    int i = 0;

    fp = fopen(filename, "r");
    if(fp == NULL){
        printf("NOT search \"%s\" file.\n", filename);
        return -1;
    }
    while(fgets(line, MAX_LINE, fp) != NULL){
        upper(line);
        code[i] = line_to_inst(line);  // エラーで inst.func = end が返る
        if(code[i].func == IGN)
            continue;
        if(code[i].func == END)
            break;
        i++;
    }

    fclose(fp);
    return 0;
}

// int main(int argc, char const *argv[]) {
//     instruction code[MAX_CODE];
//     int i = -1;
//     int is_invalid_code = 0;
//
//     if(argc == 1){
//         printf("ERROR: Plz input source's file.\n");
//         return -1;
//     }
//
//     is_invalid_code = read_code((char*)argv[1], code) == -1;
//     if(is_invalid_code){
//         return -1;
//     }
//
//     print_code(code);
//     execute_code(code);
//
//     return 0;
// }
