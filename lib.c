#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <stdbool.h>
#define OBJ_FILE_NAME "myobj.o"
#define TEMP_OBJ_FILE_NAME "tmp_myobj"
#define TEMP_OBJ_FILE_NAME_END ".\\tmp_myobj_end"
#define TEMP_OBJ_FILE_NAME_DATA "tmp_myobj_data"
#define TEMP_OBJ_FILE_RELOCATIONS "tmp_myobj_relocations"
#define TEMP_OBJ_FILE_SYMBOLS "tmp_myobj_symbols"

struct var_var {
    char name[1024];
    char value[1024];
    char type[1024];
    bool isNull;
};
bool debug = true;

int vars = 0;
char words[4][1024][1024] = {{
    {"begin"}, // 1
    {"var"}, // 2
    {"end"}, // 3
    {"if"}, // 4
    {"while"}, // 5
    {"do"}, // 6
    {"to"}, // 7
    {"else"}, // 8
    {"as"}, // 9
    {"read"}, // 10
    {"write"}, // 11
    {"then"}, // 12
    {"for"}, // 13
//    {"for"}, // 14
//    {"program"}, // 15
//    {"var"}, // 16
//    {"displ"}, // 17
//    {"else"}, // 18
//    {"enter"} // 19
                             },
                             { // delimiters
                              {"NEQ"},// 1
                              {"EQV"},// 2
                              {"LOWT"},// 3
                              {"LOWE"},// 4
                               {"GRT"},// 5
                               {"GRE"},//  6
                               {"*"},//7 umn
                               {"/"},// 8 del
                               {"&&"},// 9
                               {"("},// 10
                               {")"},// 11
                               {"."},// 12
                               {";"},// 13
                               {":"},// 14
                               {"+"},//  15
                               {"-"},// 16
                               {"^"},// 17
                               {"#"},// 18
                               {"@"},// 19
                                     {"&"},// 20
                                     {"{"},// 21
                                     {"}"},// 22
                                     {","},// 23
                                     {"+"},// 24 add
                                     {'\n'},
                                     },
                             {{112},},{'a', 'a'},};

char map[1024];
// TEMPORARY
char map_2[1024];

char prohibited[] = {';', '=', '>', '<', '?', ':', '\0'};

char file_view[] = {'%', 'd', ',', '%', 'd', ';'};

struct var_var def_vars[1024];
int vars_count = 0;

// NOT MINE
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char* readFileToString(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET); // Возвращаемся в начало файла

    char* buffer = (char*)malloc(fileSize + 1);
    if (buffer == NULL) {
        perror("Memory allocation failed");
        fclose(file);
        return NULL;
    }

    fread(buffer, 1, fileSize, file);
    buffer[fileSize] = '\0'; // Добавляем нуль-терминатор

    fclose(file);
    return buffer;
}

void file_write(const char *filepath, const char *data) {
    FILE *fp = fopen(filepath, "ab");
    if (fp == NULL) {
        perror("РћС€РёР±РєР° РїСЂРё РѕС‚РєСЂС‹С‚РёРё С„Р°Р№Р»Р°");
        return;
    }

    if (data != NULL) {
        if (fputs(data, fp) == EOF) {
            perror("РћС€РёР±РєР° РїСЂРё Р·Р°РїРёСЃРё РІ С„Р°Р№Р»");
        }
    } else {
        fprintf(stderr, "РћС€РёР±РєР°: РґР°РЅРЅС‹Рµ РґР»СЏ Р·Р°РїРёСЃРё СЂР°РІРЅС‹ NULL\n");
    }

    fclose(fp);
}

double ieee754ToDouble(uint64_t bits) {
    uint64_t sign = (bits >> 63) & 0x1;
    uint64_t exponent = (bits >> 52) & 0x7FF;
    uint64_t mantissa = bits & 0xFFFFFFFFFFFFF;

    // Вычисление фактической экспоненты
    int64_t actual_exponent = exponent - 1023;

    // Вычисление мантиссы
    double mantissa_value = 1.0; // Имплицитная единица
    for (int i = 0; i < 52; i++) {
        if (mantissa & (1ULL << (51 - i))) {
            mantissa_value += (1.0 / (1ULL << (i + 1)));
        }
    }

    // Вычисление окончательного значения
    double result = (sign ? -1 : 1) * mantissa_value * (1ULL << actual_exponent);
    return result;
}

typedef union {
    double value;
    uint64_t bits;
} DoubleUnion;


char **str_split(const char *str, char delimiter) {
    int i, j, k;
    size_t len = strlen(str);
    int substring_count = 1;

    for (i = 0; i < len; i++) {
        if (str[i] == delimiter) {
            substring_count++;
        }
    }
    char **result = (char **) malloc(substring_count * sizeof(char *));
    if (!result) {
        return NULL;
    }

    i = 0;
    j = 0;
    for (k = 0; k < substring_count; k++) {
        int start = i;
        while (i < len && str[i] != delimiter) {
            i++;
        }
        int substr_len = i - start;
        result[k] = (char *) malloc((substr_len + 1) * sizeof(char));
        if (!result[k]) {
            for (int m = 0; m < k; m++) {
                free(result[m]);
            }
            free(result);
            return NULL;
        }
        strncpy(result[k], &str[start], substr_len);
        result[k][substr_len] = '\0'; // Добавляем завершающий нулевой символ
        i++; // Пропускаем разделитель
    }

    return result;
}


#define MAX 100

typedef struct {
    char items[MAX];
    int top;
} Stack;

void initStack(Stack *s) {
    s->top = -1;
}

int isFull(Stack *s) {
    return s->top == MAX - 1;
}

int isEmpty(Stack *s) {
    return s->top == -1;
}

void push(Stack *s, char item) {
    if (!isFull(s)) {
        s->items[++(s->top)] = item;
    }
}

char pop(Stack *s) {
    if (!isEmpty(s)) {
        return s->items[(s->top)--];
    }
    return '\0';
}

char peek(Stack *s) {
    if (!isEmpty(s)) {
        return s->items[s->top];
    }
    return '\0';
}

int precedence(char op) {
    switch (op) {
        case '+':
        case '-':
            return 1;
        case '*':
        case '/':
            return 2;
        case '^':
            return 3;
        default:
            return 0;
    }
}

void infixToPostfix(const char *infix, char *postfix) {
    Stack s;
    initStack(&s);
    int i, j = 0;

    for (i = 0; infix[i] != '\0'; i++) {

        if (isalnum(infix[i]) || infix[i] == '.') {
            postfix[j++] = infix[i]; // Если операнд, добавляем в выходную строку
        } else if (infix[i] == '(') {
            push(&s, infix[i]); // Если '(', помещаем в стек
        } else if (infix[i] == ')') {
            while (!isEmpty(&s) && peek(&s) != '(') {
                postfix[j++] = pop(&s); // Извлекаем из стека до '('
            }
            pop(&s); // Удаляем '(' из стека
        }else { // Оператор
            postfix[j++] = ' ';
            while (!isEmpty(&s) && precedence(peek(&s)) >= precedence(infix[i])) {
                postfix[j++] = pop(&s);
            }
            push(&s, infix[i]);
        }
    }

    while (!isEmpty(&s)) {
        postfix[j++] = pop(&s);
    }
    postfix[j] = '\0'; // Завершаем строку
}

void get_buffer_from_token(const char *tokens, char* _buffer, char* _buffer2){
    char buffer[2][100];
    memset(buffer, 0, sizeof(buffer));
    bool first = true;
    int j = 0;
    for (int i = 0; tokens[i] != '\0'; i++) {
        if (tokens[i] == ',') {
            first = false;
            j = 0;
            continue;
        }
        if (first)buffer[0][j] = tokens[i];
        else buffer[1][j] = tokens[i];
        j++;
    }
    j = 0;
    strcpy(_buffer, buffer[0]);
    strcpy(_buffer2, buffer[1]);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void append_bytes_from_file(const char *source_file_path, const char *destination_file_path) {
    // Открываем исходный файл для чтения
    FILE *source_file = fopen(source_file_path, "rb");
    if (source_file == NULL) {
        perror("err append_bytes_from_file");
        return;
    }

    // Открываем файл назначения для добавления
    FILE *destination_file = fopen(destination_file_path, "ab");
    if (destination_file == NULL) {
        perror("err append_bytes_from_file");
        fclose(source_file);
        return;
    }

    // Чтение и запись байтов
    unsigned char buffer[1024]; // Буфер для чтения данных
    size_t bytes_read;

    while ((bytes_read = fread(buffer, sizeof(unsigned char), sizeof(buffer), source_file)) > 0) {
        fwrite(buffer, sizeof(unsigned char), bytes_read, destination_file);
    }

    // Закрываем файлы
    fclose(source_file);
    fclose(destination_file);
}

void appendToFile(const char *filename, const void *data, size_t dataSize) {
    FILE *file = fopen(filename, "ab");
    if (file == NULL) {
        perror("err appendToFile\n");
        return;
    }
//    if(debug)printf("%x writed to file %s \n", *(const int *)data, filename);
    size_t writtenBytes = fwrite(data, 1, dataSize, file);
    if (writtenBytes != dataSize) {
        perror("err appendToFile\n");
    }

    fclose(file);
}
void appendToFile_little_en(const char *filename, const void *data, size_t size) {
    FILE *file = fopen(filename, "ab");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }
//    if(debug)printf("%x writed to file %s \n", *(const int *)data, filename);

    fwrite(data, sizeof(uint8_t), size, file);
    fclose(file);
}

long getFileSize(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        return -1;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fclose(file);

    return size;
}


struct hex_values{
    char name[1024];
    unsigned char addr;
    long long value;
};

typedef struct IMAGE_FILE_HEADER {
    uint32_t Machine;
    uint32_t NumberOfSections;
    uint32_t TimeDateStamp;
    uint32_t PointerToSymbolTable;
    uint32_t NumberOfSymbols;
    uint32_t SizeOfOptionalHeader;
    uint32_t Characteristics;
} IMAGE_FILE_HEADER, *PIMAGE_FILE_HEADER;

typedef struct  {
    uint32_t physical_address;        // 4 байта
    uint32_t virtual_address;         // 4 байта
    uint32_t size_of_raw_data;       // 4 байта
    uint32_t file_pointer_to_raw_data; // 4 байта
    uint32_t file_pointer_to_relocation_table; // 4 байта
    uint32_t file_pointer_to_line_numbers; // 4 байта
    uint16_t number_of_relocations;   // 2 байта
    uint16_t number_of_line_numbers;  // 2 байта
    uint32_t flags;                   // 4 байта
}COFFHeader;

typedef struct {
    uint32_t r_offset;
    uint32_t r_symbol;
    uint16_t r_type;
} RelocationEntry;

typedef struct {
    union {
        char   s_name[8];
        uint32_t s_strx;
    };
    uint32_t s_value;
    uint16_t s_section;
    uint16_t s_type;
    uint8_t  s_storage_class;
    uint8_t  s_num_aux;
} SymbolEntry;

struct hex_values hex_values_l[1024];



void correct_hex_presentation(char *digit, char *filename) {
    long long decimalNumber = 0;
    if(strcmp(digit, "false")==0){
        decimalNumber = -2;
    }else if(strcmp(digit, "true")==0){
        decimalNumber = 1;
    }else{
        decimalNumber = atoll(digit);
    }

    FILE *file = fopen(filename, "ab");
    if (file == NULL) {
        perror("Ошибка при открытии файла");
        return;
    }
    unsigned char hex_bytes[4];
    for (int i = 0; i < 4; i++) {
        hex_bytes[i] = (decimalNumber >> (i * 8)) & 0xFF;
    }

    // Запись байтов в файл
    fwrite(hex_bytes, sizeof(unsigned char), sizeof(hex_bytes), file);

    fclose(file);
}

void add_relocations_section(RelocationEntry relocationEntry){
    appendToFile_little_en(TEMP_OBJ_FILE_RELOCATIONS, &relocationEntry.r_offset, 4);
    appendToFile_little_en(TEMP_OBJ_FILE_RELOCATIONS, &relocationEntry.r_symbol, 4);
    appendToFile_little_en(TEMP_OBJ_FILE_RELOCATIONS, &relocationEntry.r_type, 2);
}

void write_bytes_to_file_position(const char *filename, long position, char *data, size_t length) {
    FILE *file = fopen(filename, "r+b");
    if (file == NULL) {
        perror("err write_bytes_to_file_position");
        return;
    }

    long long decimalNumber = atoll(data);
    unsigned char hex_bytes[length];
    for (int i = 0; i < length; i++) {
        hex_bytes[i] = (decimalNumber >> (i * 8)) & 0xFF;
    }

    if (fseek(file, position, SEEK_SET) != 0) {
        perror("err write_bytes_to_file_position");
        fclose(file);
        return;
    }

    size_t written = fwrite(hex_bytes, sizeof(uint8_t), length, file);
    if (written != length) {
        perror("err write_bytes_to_file_position");
    }

    fclose(file);
}

int machine_templates(char *op, char *value, int index_vars, int relocation_count){

    RelocationEntry relocationEntry;

    int hex = 0x0;
    if(strcmp(op, "mov_var_rax_legacy") == 0){
        hex = 0x48;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
        hex = 0xc7;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
        hex = 0xc0;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
    }else if(strcmp(op, "mov_rax_addr") == 0){
        hex = 0x48;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
        hex = 0x89;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
        hex = 0x05;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);

        for(int i = 0; i < index_vars; i++){
            if(strcmp(hex_values_l[i].name, value) == 0){
                char adr[100];
                memset(adr, '\0', 100);
                sprintf(adr, "%d", hex_values_l[i].addr);
                long code_count = getFileSize(TEMP_OBJ_FILE_NAME);
                correct_hex_presentation(adr, TEMP_OBJ_FILE_NAME);

                relocationEntry.r_offset = code_count;
                relocationEntry.r_symbol = index_vars + 4;
                relocationEntry.r_type = 0x0004;
                add_relocations_section(relocationEntry);
                relocation_count++;

                break;
            }
        }

    }else if(strcmp(op, "mov_rbx_addr") == 0){
        hex = 0x48;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
        hex = 0x89;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
        hex = 0x1d;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
        for(int i = 0; i < index_vars; i++){
            if(strcmp(hex_values_l[i].name, value) == 0){
                char adr[100];
                memset(adr, '\0', 100);
                sprintf(adr, "%d", hex_values_l[i].addr);
                long code_count = getFileSize(TEMP_OBJ_FILE_NAME);
                correct_hex_presentation(adr, TEMP_OBJ_FILE_NAME);


                relocationEntry.r_offset = code_count;
                relocationEntry.r_symbol = index_vars + 4;
                relocationEntry.r_type = 0x0004;
                add_relocations_section(relocationEntry);
                relocation_count++;

                break;
            }
        }

    }else if(strcmp(op, "mov_rdx_addr") == 0){
        hex = 0x48;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
        hex = 0x89;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
        hex = 0x15;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
        for(int i = 0; i < index_vars; i++){
            if(strcmp(hex_values_l[i].name, value) == 0){
                char adr[100];
                memset(adr, '\0', 100);
                sprintf(adr, "%d", hex_values_l[i].addr);
                long code_count = getFileSize(TEMP_OBJ_FILE_NAME);
                correct_hex_presentation(adr, TEMP_OBJ_FILE_NAME);

                relocationEntry.r_offset = code_count;
                relocationEntry.r_symbol = index_vars + 4;
                relocationEntry.r_type = 0x0004;
                add_relocations_section(relocationEntry);
                relocation_count++;

                break;
            }
        }
    }else if(strcmp(op, "mov_addr_rax") == 0){
        bool found = false;
        for(int i = 0; i < index_vars; i++){
            if(strcmp(hex_values_l[i].name, value) == 0){

                hex = 0x48;
                appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
                hex = 0x8b;
                appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
                hex = 0x05;
                appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
//                long code_count = (getFileSize(TEMP_OBJ_FILE_NAME)/16)*10;

                char adr[100];
                memset(adr, '\0', 100);
                sprintf(adr, "%d", hex_values_l[i].addr);
                long code_count = getFileSize(TEMP_OBJ_FILE_NAME);
                correct_hex_presentation(adr, TEMP_OBJ_FILE_NAME);
                found = true;

//                relocationEntry.r_offset = hex_values_l[i].addr + (code_count_sub - code_count - 2) + code_count;
                relocationEntry.r_offset = code_count;
                relocationEntry.r_symbol = index_vars + 4;
                relocationEntry.r_type = 0x0004;
                relocation_count++;
                add_relocations_section(relocationEntry);
                break;
            }
        }
        if(!found){

            hex = 0x48;
            appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
            hex = 0xC7;
            appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
            hex = 0xC0;
            appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);

            correct_hex_presentation(value, TEMP_OBJ_FILE_NAME);
        }
    }else if(strcmp(op, "add_addr_rax") == 0){

        bool found = false;
        for(int i = 0; i < index_vars; i++){
            if(strcmp(hex_values_l[i].name, value) == 0){

                hex = 0x48;
                appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
                hex = 0x03;
                appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
                hex = 0x05;
                appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);

                char adr[100];
                memset(adr, '\0', 100);
                sprintf(adr, "%d", hex_values_l[i].addr);
                long code_count = getFileSize(TEMP_OBJ_FILE_NAME);
                correct_hex_presentation(adr, TEMP_OBJ_FILE_NAME);
                found = true;

                relocationEntry.r_offset = code_count;
                relocationEntry.r_symbol = index_vars + 4;
                relocationEntry.r_type = 0x0004;
                add_relocations_section(relocationEntry);
                relocation_count++;

                break;
            }
        }
        if(!found){

            hex = 0x48;
            appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
            hex = 0x83;
            appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
            hex = 0xC0;
            appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);

            correct_hex_presentation(value, TEMP_OBJ_FILE_NAME);
        }
    }else if(strcmp(op, "mov_addr_rbx") == 0){

        bool found = false;
        for(int i = 0; i < index_vars; i++){
            if(strcmp(hex_values_l[i].name, value) == 0){

                hex = 0x48;
                appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
                hex = 0x8B;
                appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
                hex = 0x1D;
                appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);

                char adr[100];
                memset(adr, '\0', 100);
                sprintf(adr, "%d", hex_values_l[i].addr);
                long code_count = getFileSize(TEMP_OBJ_FILE_NAME);
                correct_hex_presentation(adr, TEMP_OBJ_FILE_NAME);
                found = true;

                relocationEntry.r_offset = code_count;
                relocationEntry.r_symbol = index_vars + 4;
                relocationEntry.r_type = 0x0004;
                add_relocations_section(relocationEntry);
                relocation_count++;

                break;
            }
        }
        if(!found){

            hex = 0x48;
            appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
            hex = 0xC7;
            appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
            hex = 0xC3;
            appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
//            add_relocations_section(relocationEntry);

            correct_hex_presentation(value, TEMP_OBJ_FILE_NAME);
        }
    }else if(strcmp(op, "mov_addr_rcx") == 0){

        bool found = false;
        for(int i = 0; i < index_vars; i++){
            if(strcmp(hex_values_l[i].name, value) == 0){

                hex = 0x48;
                appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
                hex = 0x8B;
                appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
                hex = 0x0D;
                appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);

                char adr[100];
                memset(adr, '\0', 100);
                sprintf(adr, "%d", hex_values_l[i].addr);
                long code_count = getFileSize(TEMP_OBJ_FILE_NAME);
                correct_hex_presentation(adr, TEMP_OBJ_FILE_NAME);
                found = true;

                relocationEntry.r_offset = code_count;
                relocationEntry.r_symbol = index_vars + 4;
                relocationEntry.r_type = 0x0004;
                add_relocations_section(relocationEntry);
                relocation_count++;

                break;
            }
        }
        if(!found){

            hex = 0x48;
            appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
            hex = 0xC7;
            appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
            hex = 0xC1;
            appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
//            add_relocations_section(relocationEntry);

            correct_hex_presentation(value, TEMP_OBJ_FILE_NAME);
        }
    }else if(strcmp(op, "mov_addr_rdx") == 0){

        bool found = false;
        for(int i = 0; i < index_vars; i++){
            if(strcmp(hex_values_l[i].name, value) == 0){

                hex = 0x48;
                appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
                hex = 0x8B;
                appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
                hex = 0x15;
                appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);

                char adr[100];
                memset(adr, '\0', 100);
                sprintf(adr, "%d", hex_values_l[i].addr);
                long code_count = getFileSize(TEMP_OBJ_FILE_NAME);
                correct_hex_presentation(adr, TEMP_OBJ_FILE_NAME);
                found = true;

                relocationEntry.r_offset = code_count;
                relocationEntry.r_symbol = index_vars + 4;
                relocationEntry.r_type = 0x0004;
                add_relocations_section(relocationEntry);
                relocation_count++;

                break;
            }
        }
        if(!found){

            hex = 0x48;
            appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
            hex = 0xC7;
            appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
            hex = 0xC2;
            appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
//            add_relocations_section(relocationEntry);

            correct_hex_presentation(value, TEMP_OBJ_FILE_NAME);
        }
    }else if(strcmp(op, "mov_addr_rcx") == 0){

        bool found = false;
        for(int i = 0; i < index_vars; i++){
            if(strcmp(hex_values_l[i].name, value) == 0){

                hex = 0x48;
                appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
                hex = 0x8B;
                appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
                hex = 0x15;
                appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);

                char adr[100];
                memset(adr, '\0', 100);
                sprintf(adr, "%d", hex_values_l[i].addr);
                long code_count = getFileSize(TEMP_OBJ_FILE_NAME);
                correct_hex_presentation(adr, TEMP_OBJ_FILE_NAME);
                found = true;

                relocationEntry.r_offset = code_count;
                relocationEntry.r_symbol = index_vars + 4;
                relocationEntry.r_type = 0x0004;
                add_relocations_section(relocationEntry);
                relocation_count++;

                break;
            }
        }
        if(!found){

            hex = 0x48;
            appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
            hex = 0xC7;
            appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
            hex = 0xC1;
            appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);

            correct_hex_presentation(value, TEMP_OBJ_FILE_NAME);
        }
    }else if(strcmp(op, "mov_addr_rbp") == 0){

        bool found = false;
        for(int i = 0; i < index_vars; i++){
            if(strcmp(hex_values_l[i].name, value) == 0){

                hex = 0x48;
                appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
                hex = 0x8B;
                appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
                hex = 0x2D;
                appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);

                char adr[100];
                memset(adr, '\0', 100);
                sprintf(adr, "%d", hex_values_l[i].addr);
                long code_count = getFileSize(TEMP_OBJ_FILE_NAME);
                correct_hex_presentation(adr, TEMP_OBJ_FILE_NAME);
                found = true;

                relocationEntry.r_offset = code_count;
                relocationEntry.r_symbol = index_vars + 4;
                relocationEntry.r_type = 0x0004;
                add_relocations_section(relocationEntry);
                relocation_count++;

                break;
            }
        }
        if(!found){

            hex = 0x48;
            appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
            hex = 0xC7;
            appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
            hex = 0xC5;
            appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);

            correct_hex_presentation(value, TEMP_OBJ_FILE_NAME);
        }
    }else if(strcmp(op, "add_addr_rbx") == 0){

        bool found = false;
        for(int i = 0; i < index_vars; i++){
            if(strcmp(hex_values_l[i].name, value) == 0){

                hex = 0x48;
                appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
                hex = 0x03;
                appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
                hex = 0x03;
                appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);

                char adr[100];
                memset(adr, '\0', 100);
                sprintf(adr, "%d", hex_values_l[i].addr);
                long code_count = getFileSize(TEMP_OBJ_FILE_NAME);
                correct_hex_presentation(adr, TEMP_OBJ_FILE_NAME);
                found = true;

                relocationEntry.r_offset = code_count;
                relocationEntry.r_symbol = index_vars + 4;
                relocationEntry.r_type = 0x0004;
                add_relocations_section(relocationEntry);
                relocation_count++;

                break;
            }
        }
        if(!found){

            hex = 0x48;
            appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
            hex = 0x81;
            appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
            hex = 0xC3;
            appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);

            correct_hex_presentation(value, TEMP_OBJ_FILE_NAME);
        }
    }else if(strcmp(op, "add_rbx_addr") == 0){
        for(int i = 0; i < index_vars; i++){
            if(strcmp(hex_values_l[i].name, value) == 0){

                hex = 0x48;
                appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
                hex = 0x01;
                appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
                hex = 0x1D;
                appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);

                char adr[100];
                memset(adr, '\0', 100);
                sprintf(adr, "%d", hex_values_l[i].addr);
                long code_count = getFileSize(TEMP_OBJ_FILE_NAME);
                correct_hex_presentation(adr, TEMP_OBJ_FILE_NAME);

                relocationEntry.r_offset = code_count;
                relocationEntry.r_symbol = index_vars + 4;
                relocationEntry.r_type = 0x0004;
                add_relocations_section(relocationEntry);
                relocation_count++;

                break;
            }
        }
    }else if(strcmp(op, "lea_addr_double") == 0){

//        bool found = false;
//        for(int i = 0; i < index_vars; i++){
//            if(strcmp(hex_values_l[i].name, value) == 0){

                hex = 0x48;
                appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
                hex = 0x8d;
                appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
                hex = 0x05;
                appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);

                char adr[100];
                memset(adr, '\0', 100);
                int count_data_set = (index_vars * 8);

                sprintf(adr, "%d", count_data_set);
                long code_count = getFileSize(TEMP_OBJ_FILE_NAME);
                correct_hex_presentation(adr, TEMP_OBJ_FILE_NAME);

                relocationEntry.r_offset = code_count;
                relocationEntry.r_symbol = index_vars + 4;
                relocationEntry.r_type = 0x0004;
                add_relocations_section(relocationEntry);
                relocation_count++;

    }else if(strcmp(op, "lea_addr_int") == 0){

        hex = 0x48;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
        hex = 0x8d;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
        hex = 0x05;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);

        char adr[100];
        memset(adr, '\0', 100);
        int count_data_set = (index_vars * 8) + 8;

        sprintf(adr, "%d", count_data_set);
        long code_count = getFileSize(TEMP_OBJ_FILE_NAME);
        correct_hex_presentation(adr, TEMP_OBJ_FILE_NAME);

        relocationEntry.r_offset = code_count;
        relocationEntry.r_symbol = index_vars + 4;
        relocationEntry.r_type = 0x0004;
        add_relocations_section(relocationEntry);
        relocation_count++;

    }else if(strcmp(op, "lea_addr_bool") == 0){
        char adr[100];

        relocation_count = machine_templates("mov_addr_rax", "1", index_vars, relocation_count);

        relocation_count = machine_templates("cmp_rax_rdx", 0x00, index_vars, relocation_count);

        relocation_count = machine_templates("je", 0x00, index_vars, relocation_count);

        hex = 0x05;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);

        relocation_count = machine_templates("jmp", 0x00, index_vars, relocation_count);
        memset(adr, '\0', 100);

        sprintf(adr,"%d", 12);
        write_bytes_to_file_position(TEMP_OBJ_FILE_NAME, getFileSize(TEMP_OBJ_FILE_NAME), adr, 4);

        hex = 0x48;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
        hex = 0x8d;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
        hex = 0x05;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);

        memset(adr, '\0', 100);
        int count_data_set = (index_vars * 8) + (8*2);

        sprintf(adr, "%d", count_data_set);
        long code_count = getFileSize(TEMP_OBJ_FILE_NAME);
        correct_hex_presentation(adr, TEMP_OBJ_FILE_NAME);

        relocationEntry.r_offset = code_count;
        relocationEntry.r_symbol = index_vars + 4;
        relocationEntry.r_type = 0x0004;
        add_relocations_section(relocationEntry);
        relocation_count++;

        relocation_count = machine_templates("jmp", 0x00, index_vars, relocation_count);
        memset(adr, '\0', 100);
        sprintf(adr,"%d", 7);
        write_bytes_to_file_position(TEMP_OBJ_FILE_NAME, getFileSize(TEMP_OBJ_FILE_NAME), adr, 4);

        hex = 0x48;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
        hex = 0x8d;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
        hex = 0x05;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);

        memset(adr, '\0', 100);
        count_data_set = (index_vars * 8) + (8*3);

        sprintf(adr, "%d", count_data_set);
        code_count = getFileSize(TEMP_OBJ_FILE_NAME);
        correct_hex_presentation(adr, TEMP_OBJ_FILE_NAME);

        relocationEntry.r_offset = code_count;
        relocationEntry.r_symbol = index_vars + 4;
        relocationEntry.r_type = 0x0004;
        add_relocations_section(relocationEntry);
        relocation_count++;

    }else if(strcmp(op, "add_rbx_rax") == 0){
        hex = 0x48;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
        hex = 0x01;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
        hex = 0xD8;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
    }else if(strcmp(op, "add_rbx_rdx") == 0){
        hex = 0x48;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
        hex = 0x01;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
        hex = 0xDA;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
    }else if(strcmp(op, "add_rax_rcx") == 0){
        hex = 0x48;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
        hex = 0x01;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
        hex = 0xC1;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
    }else if(strcmp(op, "add_rcx_rbx") == 0){
        hex = 0x48;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
        hex = 0x01;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
        hex = 0xCB;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
    }else if(strcmp(op, "mov_rbx_rax") == 0){
        hex = 0x48;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
        hex = 0x89;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
        hex = 0xD8;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
    }else if(strcmp(op, "mov_rax_rbx") == 0){
        hex = 0x48;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
        hex = 0x89;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
        hex = 0xC3;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
    }else if(strcmp(op, "mov_rbx_rcx") == 0){
        hex = 0x48;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
        hex = 0x89;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
        hex = 0xD9;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
    }else if(strcmp(op, "add_rax_rbx") == 0){
        hex = 0x48;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
        hex = 0x01;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
        hex = 0xC3;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
    }else if(strcmp(op, "sub_rax_rbx") == 0){
        hex = 0x48;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
        hex = 0x29;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
        hex = 0xC3;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
    }else if(strcmp(op, "sub_rax_rcx") == 0){
        hex = 0x48;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
        hex = 0x29;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
        hex = 0xC1;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
    }else if(strcmp(op, "sub_rcx_rax") == 0){
        hex = 0x48;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
        hex = 0x29;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
        hex = 0xC8;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
    }else if(strcmp(op, "mul_rax") == 0){
        hex = 0x48;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
        hex = 0xF7;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
        hex = 0xE0;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
    }else if(strcmp(op, "mul_rcx") == 0){
        hex = 0x48;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
        hex = 0xF7;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
        hex = 0xE1;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
    }else if(strcmp(op, "div_rax") == 0){
        hex = 0x48;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
        hex = 0xF7;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
        hex = 0xF0;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
    }else if(strcmp(op, "div_rcx") == 0){
        hex = 0x48;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
        hex = 0xF7;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
        hex = 0xF1;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
    }else if(strcmp(op, "je") == 0){
        hex = 0x74;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
    }else if(strcmp(op, "jne") == 0){
        hex = 0x75;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
    }else if(strcmp(op, "jge") == 0){
        hex = 0x7d;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
    }else if(strcmp(op, "jle") == 0){
        hex = 0x7e;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
    }else if(strcmp(op, "jl") == 0){
        hex = 0x7c;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
    }else if(strcmp(op, "jg") == 0){
        hex = 0x7f;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
    }else if(strcmp(op, "jmp") == 0){
        hex = 0xE9;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
    }else if(strcmp(op, "cmp_rax_rbx") == 0){
        hex = 0x48;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
        hex = 0x39;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
        hex = 0xC3;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
    }else if(strcmp(op, "cmp_rbx_rax") == 0){
        hex = 0x48;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
        hex = 0x39;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
        hex = 0xD8;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
    }else if(strcmp(op, "cmp_rax_rdx") == 0){
        hex = 0x48;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
        hex = 0x39;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
        hex = 0xC2;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
    }else if(strcmp(op, "cmp_addr_rdx") == 0){

        bool found = false;
        for(int i = 0; i < index_vars; i++){
            if(strcmp(hex_values_l[i].name, value) == 0){

                hex = 0x48;
                appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
                hex = 0x3B;
                appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
                hex = 0x15;
                appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);

                char adr[100];
                memset(adr, '\0', 100);
                sprintf(adr, "%d", hex_values_l[i].addr);
                long code_count = getFileSize(TEMP_OBJ_FILE_NAME);
                correct_hex_presentation(adr, TEMP_OBJ_FILE_NAME);
                found = true;

                relocationEntry.r_offset = code_count;
                relocationEntry.r_symbol = index_vars + 4;
                relocationEntry.r_type = 0x0004;
                add_relocations_section(relocationEntry);
                relocation_count++;

                break;
            }
        }
        if(!found){

            hex = 0x48;
            appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
            hex = 0x83;
            appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
            hex = 0xF9;
            appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);

            correct_hex_presentation(value, TEMP_OBJ_FILE_NAME);
        }

    }else if(strcmp(op, "cmp_addr_rcx") == 0){

        bool found = false;
        for(int i = 0; i < index_vars; i++){
            if(strcmp(hex_values_l[i].name, value) == 0){

                hex = 0x48;
                appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
                hex = 0x3B;
                appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
                hex = 0xEC;
                appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);

                char adr[100];
                memset(adr, '\0', 100);
                sprintf(adr, "%d", hex_values_l[i].addr);
                long code_count = getFileSize(TEMP_OBJ_FILE_NAME);
                correct_hex_presentation(adr, TEMP_OBJ_FILE_NAME);
                found = true;

                relocationEntry.r_offset = code_count;
                relocationEntry.r_symbol = index_vars + 4;
                relocationEntry.r_type = 0x0004;
                add_relocations_section(relocationEntry);
                relocation_count++;

                break;
            }
        }
        if(!found){

            hex = 0x48;
            appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
            hex = 0x83;
            appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
            hex = 0xFA;
            appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
//            add_relocations_section(relocationEntry);

            correct_hex_presentation(value, TEMP_OBJ_FILE_NAME);
        }
    }else if(strcmp(op, "cmp_addr_rbp") == 0){

        for(int i = 0; i < index_vars; i++){
            if(strcmp(hex_values_l[i].name, value) == 0){

                hex = 0x48;
                appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
                hex = 0x3B;
                appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
                hex = 0x2D;
                appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);

                char adr[100];
                memset(adr, '\0', 100);
                sprintf(adr, "%d", hex_values_l[i].addr);
                long code_count = getFileSize(TEMP_OBJ_FILE_NAME);
                correct_hex_presentation(adr, TEMP_OBJ_FILE_NAME);

                relocationEntry.r_offset = code_count;
                relocationEntry.r_symbol = index_vars + 4;
                relocationEntry.r_type = 0x0004;
                add_relocations_section(relocationEntry);
                relocation_count++;

                break;
            }
        }

    }else if(strcmp(op, "mov_rax_rcx") == 0){
        hex = 0x48;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
        hex = 0x89;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
        hex = 0xC1;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
    }else if(strcmp(op, "not_rax") == 0){
        hex = 0x48;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
        hex = 0xF7;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
        hex = 0xD0;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
    }else if(strcmp(op, "not_rbx") == 0){
        hex = 0x48;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
        hex = 0xF7;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
        hex = 0xD3;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
    }else if(strcmp(op, "xor_rdx_rdx") == 0){
        hex = 0x48;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
        hex = 0x31;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
        hex = 0xD2;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
    }else if(strcmp(op, "print") == 0){
        hex = 0xE8;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
        long code_count = getFileSize(TEMP_OBJ_FILE_NAME);
        hex = 0x00;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
        hex = 0x00;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
        hex = 0x00;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
        hex = 0x00;
        appendToFile(TEMP_OBJ_FILE_NAME, &hex, 1);
        relocationEntry.r_offset = code_count;
        relocationEntry.r_symbol = index_vars + 9;
        relocationEntry.r_type = 0x0004;
        add_relocations_section(relocationEntry);
        relocation_count++;
    }
    if(debug)printf("Комманда ассемблера %s, значение преехода в регистр: %s\n", op, value);
    return relocation_count;
}


int block_count = 0;


struct IMAGE_FILE_HEADER fileHeader;
COFFHeader text;
COFFHeader data;
COFFHeader bss;

void add_coff_header(){
    appendToFile(OBJ_FILE_NAME, &fileHeader.Machine, 2);
    appendToFile(OBJ_FILE_NAME, &fileHeader.NumberOfSections, 2);
    appendToFile(OBJ_FILE_NAME, &fileHeader.TimeDateStamp, 4);
    appendToFile(OBJ_FILE_NAME, &fileHeader.PointerToSymbolTable, 4);
    appendToFile(OBJ_FILE_NAME, &fileHeader.NumberOfSymbols, 4);
    appendToFile(OBJ_FILE_NAME, &fileHeader.SizeOfOptionalHeader, 2);
    appendToFile(OBJ_FILE_NAME, &fileHeader.Characteristics, 2);
}
void add_section_header(COFFHeader header){
    appendToFile_little_en(OBJ_FILE_NAME, &header.physical_address, 4);
    appendToFile_little_en(OBJ_FILE_NAME, &header.virtual_address, 4);
    appendToFile_little_en(OBJ_FILE_NAME, &header.size_of_raw_data, 4);
    appendToFile_little_en(OBJ_FILE_NAME, &header.file_pointer_to_raw_data, 4);
    appendToFile_little_en(OBJ_FILE_NAME, &header.file_pointer_to_relocation_table, 4);
    appendToFile_little_en(OBJ_FILE_NAME, &header.file_pointer_to_line_numbers, 4);
    appendToFile_little_en(OBJ_FILE_NAME, &header.number_of_relocations, 2);
    appendToFile_little_en(OBJ_FILE_NAME, &header.number_of_line_numbers, 2);
    appendToFile_little_en(OBJ_FILE_NAME, &header.flags, 4);
}

void add_data_section(struct hex_values hex){
    appendToFile_little_en(TEMP_OBJ_FILE_NAME_DATA, &hex.value, 8);
}

void add_symbol_table(SymbolEntry symbolEntry) {

    appendToFile_little_en(TEMP_OBJ_FILE_SYMBOLS, &symbolEntry.s_name, strlen(symbolEntry.s_name));
    unsigned long long size_str = strlen(symbolEntry.s_name);
    uint32_t null_hex = 0x00;
    while (size_str < 8){
        size_str++;
        appendToFile_little_en(TEMP_OBJ_FILE_SYMBOLS, &null_hex, 1);
    }
    appendToFile_little_en(TEMP_OBJ_FILE_SYMBOLS, &symbolEntry.s_value, 4);
    appendToFile_little_en(TEMP_OBJ_FILE_SYMBOLS, &symbolEntry.s_section, 2);
    appendToFile_little_en(TEMP_OBJ_FILE_SYMBOLS, &symbolEntry.s_type, 2);
    appendToFile_little_en(TEMP_OBJ_FILE_SYMBOLS, &symbolEntry.s_storage_class, 1);
    appendToFile_little_en(TEMP_OBJ_FILE_SYMBOLS, &symbolEntry.s_num_aux, 1);
}


void competition(char *path, uint32_t data_rem){
    long size = getFileSize(path);

    while((size % 16)!= 0){
        appendToFile_little_en(path, &data_rem, 1);
        size = getFileSize(path);
    }
}

int read_def_vars(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Ошибка открытия файла");
        return -1;
    }

    int count = 0;
    char line[1000];
    fgets(line, sizeof(line), file);

    int on_off = 0;
    char* token = strtok(line, ";");
    char buffer[1024];
    memset(buffer, '\0', 1024);
    int index_buffer = 0;

    while (token != NULL) {
        if(on_off == 0){
            strcpy(def_vars[count].name, token);
            on_off++;
        }else if(on_off == 1){
            strcpy(def_vars[count].value, token);
            on_off++;
        }else if(on_off == 2){
            strcpy(def_vars[count].type, token);
            def_vars[count].isNull = false;
            on_off = 0;
            count++;
        }
        token = strtok(NULL, ";");
    }

    fclose(file);

    FILE *file_2 = fopen("./ind", "r");
    if (file == NULL) {
        perror("Ошибка открытия файла");
        return -1;
    }

    int index_words = 0;
    char line_2[1000];
    fgets(line_2, sizeof(line_2), file_2);

    token = strtok(line_2, ";");
    while (token != NULL) {
        strcpy(words[2][index_words++], token);
        token = strtok(NULL, ";");
    }
    fclose(file_2);

    FILE *file_3 = fopen("./values", "r");
    if (file == NULL) {
        perror("Ошибка открытия файла");
        return -1;
    }

    int index_values = 0;
    char line_3[1000];
    fgets(line_3, sizeof(line_3), file_3);

    token = strtok(line_3, ";");
    while (token != NULL) {
        strcpy(words[3][index_values++], token);
        token = strtok(NULL, ";");
    }
    fclose(file_3);

    return count;
}

int index_vars = 0;

int main() {

    int raw_data = 0x0;
    int byte_count = 0;
    char result[4068];
    memset(result, '\0', 4068);

    fclose(fopen(OBJ_FILE_NAME, "w"));
    fclose(fopen(TEMP_OBJ_FILE_NAME, "w"));
    fclose(fopen(TEMP_OBJ_FILE_SYMBOLS, "w"));
    fclose(fopen(TEMP_OBJ_FILE_NAME_DATA, "w"));
    fclose(fopen(TEMP_OBJ_FILE_RELOCATIONS, "w"));

    char template[200];
    int relocation_count = 0;
    memset(template, '\0', sizeof(template));
    char hexval[1024];

    int cmp_flag = 0;
    int not_flag = 0;

    memset(hexval, '\0', 1024);

//    copyFile("C:\\Users\\rain\\CLionProjects\\CTest\\template.bin", OBJ_FILE_NAME);
    unsigned char hex_local = 0x25;
    appendToFile(TEMP_OBJ_FILE_NAME, &hex_local, 1);
    hex_local = 0x66;
    appendToFile(TEMP_OBJ_FILE_NAME, &hex_local, 1);
    hex_local = 0x0A;
    appendToFile(TEMP_OBJ_FILE_NAME, &hex_local, 1);
    hex_local = 0x55;
    appendToFile(TEMP_OBJ_FILE_NAME, &hex_local, 1);
    hex_local = 0x48;
    appendToFile(TEMP_OBJ_FILE_NAME, &hex_local, 1);
    hex_local = 0x89;
    appendToFile(TEMP_OBJ_FILE_NAME, &hex_local, 1);
    hex_local = 0xE5;
    appendToFile(TEMP_OBJ_FILE_NAME, &hex_local, 1);
    hex_local = 0x48;
    appendToFile(TEMP_OBJ_FILE_NAME, &hex_local, 1);
    hex_local = 0x83;
    appendToFile(TEMP_OBJ_FILE_NAME, &hex_local, 1);
    hex_local = 0xEC;
    appendToFile(TEMP_OBJ_FILE_NAME, &hex_local, 1);
    hex_local = 0x30;
    appendToFile(TEMP_OBJ_FILE_NAME, &hex_local, 1);
    byte_count+=11;

    vars_count = read_def_vars("./varibles");
    printf("var count: %d\n", vars_count);
    // init variables
    int max_hex_addr = 0;
    for (int i = 0; i < vars_count; i++) {
        if (strcmp(def_vars[i].value, "true") == 0) {
            hex_values_l[index_vars].value = 1;
        } else if (strcmp(def_vars[i].value, "false") == 0) {
            hex_values_l[index_vars].value = -2;
        } else {
            if (strcmp(def_vars[i].value, "") != 0) {
                sprintf(hexval, "%04x", atoi(def_vars[i].value));

                char *endPtr;
                long long decimalNumber = atoll(def_vars[i].value);
                size_t numBytes = sizeof(decimalNumber);

                char hex_tmp[1024];
                memset(hex_tmp, '\0', 1024);
                sprintf(hex_tmp, "%llu", decimalNumber);
                hex_values_l[index_vars].value = decimalNumber;
            } else {
                hex_values_l[index_vars].value = 0;
            }
        }

        strcpy(hex_values_l[index_vars].name, def_vars[i].name);
        hex_values_l[index_vars].addr = max_hex_addr;
        max_hex_addr += 8;
        index_vars++;
    }
    printf("var count %d\n", index_vars);

    // IMPORTANT COMMENTS
    strcpy(map_2, readFileToString("./table"));
    char *tokens = strtok(map_2, ";");


    bool is_condition_start = false;
    bool begin_find = false;
    bool has_oper = false;
    bool has_next = false;
    bool has_next_while = false;
    bool cycle_for = false;
    bool cycle_while = false;
    bool has_displ = false;
    bool wait_assign = false;
    int while_relocation = 0;
    int code_align = 0;
    int temporary_code_size = 0;
    int temporary_code_size_else = 0;
    char cycle_while_str[100];
    memset(cycle_while_str, '\0', sizeof(cycle_while_str));

    char tmp_block_name[20];
    char tmp_block_next_name[20];
    char format[5] = ".LC0";

    memset(tmp_block_name, '\0', sizeof(tmp_block_name));

    char stack[100];
    memset(stack, '\0', sizeof(stack));

    char infix[MAX], postfix[MAX];
    memset(postfix, '\0', sizeof(postfix));
    memset(infix, '\0', sizeof(infix));


    char variable[100];
    memset(variable, '\0', sizeof(variable));
    char variable_for[100];
    memset(variable_for, '\0', sizeof(variable_for));
    while (tokens != NULL) {
        if(strcmp(tokens, "2,13") == 0 && !begin_find) {
            begin_find = true;
            tokens = strtok(NULL, ";");
//            tokens = strtok(NULL, ";");
        }
        if(begin_find) {
            char buffer[2][100];
            memset(buffer[0], '\0', sizeof(buffer[0]));
            memset(buffer[1], '\0', sizeof(buffer[1]));

            //                   {                         begin                       var
            if(strcmp(tokens, "2,21")==0 || strcmp(tokens, "1,1")==0 || strcmp(tokens, "1,2")==0){
                tokens = strtok(NULL, ";");
            }
//            printf("tokens %s\n", tokens);

            if (is_condition_start) {
//                                   ;                             to                            do
                if (strcmp(tokens, "2,13") == 0 || strcmp(tokens, "1,7") == 0 || strcmp(tokens, "1,6") == 0) {
                    has_displ = false;
                    is_condition_start = false;
                    infix[strcspn(infix, "\n")] = 0;
                    printf("infix %s\n",infix);
                    infixToPostfix(infix, postfix);
                    if(debug)printf("Val:%s\n", postfix);
                    char stack_asm[100][100];
                    char s_a[10][1024];
                    memset(stack_asm, '\0', sizeof(stack_asm));

                    int index_stack = 0;
                    int v_index = 0;

                    memset(template, '\0', sizeof(template));
                    bool was_val = false;

                    bool has_condition = false;
                    for (int i = 0; *(postfix + i); ++i) {
                        if(postfix[i] == '+' || postfix[i] == '-' || postfix[i] == '*' || postfix[i] == '/'){
                            has_condition = true;
                        }
                    }
                    if(has_condition){
                        relocation_count = machine_templates("mov_addr_rbx", "0", 0, relocation_count);
                        for (int j = 0; *(postfix + j); j++) {
                            if (postfix[j] != '+' && postfix[j] != '-' && postfix[j] != '*' && postfix[j] != '/' && postfix[j] != ' ') {

                                stack_asm[index_stack][v_index] = postfix[j];
                                s_a[index_stack][v_index] = postfix[j];

                                was_val = true;
                                v_index++;
                            } else {
                                if(!was_val)
                                    index_stack--;
                                if (postfix[j] == '+') {
                                    if (strcmp(s_a[index_stack - 1], "") != 0 && strcmp(s_a[index_stack - 0], "") != 0) {

                                        relocation_count = machine_templates("mov_addr_rax", s_a[index_stack - 1], index_vars, relocation_count);
                                        relocation_count = machine_templates("mov_addr_rcx", s_a[index_stack], index_vars, relocation_count);

                                        relocation_count = machine_templates("add_rax_rcx", 0x00, index_vars, relocation_count);
                                        relocation_count = machine_templates("mov_addr_rax", "0", 0, relocation_count);

                                        memset(s_a[index_stack - 1], '\0', 1024);
                                        memset(s_a[index_stack - 0], '\0', 1024);
                                        index_stack = 0;

                                        relocation_count = machine_templates("add_rcx_rbx", 0x00, index_vars, relocation_count);
                                    } else {
                                        if (strcmp(s_a[index_stack], "") != 0) {
                                            relocation_count = machine_templates("add_addr_rbx", s_a[index_stack], index_vars, relocation_count);
                                            memset(s_a[index_stack - 0], '\0', 1024);
                                        }
                                    }

                                } else if (postfix[j] == '*') {

                                    if (strcmp(s_a[index_stack - 0], "") != 0 && strcmp(s_a[index_stack - 1], "") == 0) {
                                        relocation_count = machine_templates("mov_rbx_rax", 0x00, index_vars, relocation_count);
                                        relocation_count = machine_templates("mov_addr_rcx", s_a[index_stack - 0], index_vars, relocation_count);
                                        relocation_count = machine_templates("mul_rcx", 0x00, 0, relocation_count);
                                        relocation_count = machine_templates("mov_rax_rbx", 0x00, 0, relocation_count);
                                        relocation_count = machine_templates("mov_addr_rax", 0x00, 0, relocation_count);
                                        memset(s_a[index_stack - 0], '\0', 1024);
                                        memset(s_a[index_stack - 1], '\0', 1024);
                                    } else {
                                        if (strcmp(s_a[index_stack - 1], "") != 0) {
                                            relocation_count = machine_templates("mov_addr_rax", s_a[index_stack - 1], index_vars, relocation_count);
                                            memset(s_a[index_stack - 1], '\0', 1024);
                                        }
                                        if (strcmp(s_a[index_stack - 0], "") != 0) {
                                            relocation_count = machine_templates("mov_addr_rcx", s_a[index_stack - 0], index_vars, relocation_count);
                                            relocation_count = machine_templates("mul_rcx", 0x00, 0, relocation_count);
                                            relocation_count = machine_templates("mov_rax_rbx", 0x00, 0, relocation_count);
                                            relocation_count = machine_templates("mov_addr_rax", "0", 0, relocation_count);
                                            memset(s_a[index_stack - 0], '\0', 1024);
                                        }
                                        memset(s_a, '\0', 100);
                                        index_stack = 0;
                                    }

                                } else if (postfix[j] == '/') {
                                    if (strcmp(s_a[index_stack - 0], "") != 0 && strcmp(s_a[index_stack - 1], "") == 0) {
                                            relocation_count = machine_templates("mov_addr_rcx", s_a[index_stack - 0], index_vars, relocation_count);

                                            relocation_count = machine_templates("mov_rbx_rax", 0x00, index_vars, relocation_count);
                                            relocation_count = machine_templates("div_rcx", 0x00, index_vars, relocation_count);
                                            relocation_count = machine_templates("xor_rdx_rdx", 0x00, index_vars, relocation_count);
                                            relocation_count = machine_templates("mov_rax_rbx", 0x00, index_vars, relocation_count);
                                            relocation_count = machine_templates("mov_addr_rax", "0", index_vars, relocation_count);
                                            relocation_count = machine_templates("mov_addr_rcx", "0", index_vars, relocation_count);

                                    } else {
                                        if (strcmp(s_a[index_stack - 1], "") != 0) {
                                            relocation_count = machine_templates("mov_addr_rax", s_a[index_stack - 1], index_vars, relocation_count);
                                        }
                                        if (strcmp(s_a[index_stack - 0], "") != 0) {
                                            relocation_count = machine_templates("mov_addr_rcx", s_a[index_stack - 0], index_vars, relocation_count);
                                            relocation_count = machine_templates("xor_rdx_rdx", 0x00, index_vars, relocation_count);
                                            relocation_count = machine_templates("div_rcx", 0x00, index_vars, relocation_count);
                                            relocation_count = machine_templates("mov_rax_rbx", 0x00, 0, relocation_count);

                                        }
                                        memset(s_a[index_stack - 0], '\0', sizeof(s_a[index_stack - 0]));
                                        memset(s_a[index_stack - 1], '\0', sizeof(s_a[index_stack - 1]));
                                        index_stack = 0;
                                    }

                                    memset(s_a[index_stack - 0], '\0', sizeof(s_a[index_stack - 0]));
                                    memset(s_a[index_stack - 1], '\0', sizeof(s_a[index_stack - 1]));

                                } else if (postfix[j] == '-') {

                                    if (strcmp(s_a[index_stack - 0], "") != 0 && strcmp(s_a[index_stack - 1], "") == 0) {

                                        relocation_count = machine_templates("mov_addr_rax", s_a[index_stack - 0], index_vars, relocation_count);
                                        relocation_count = machine_templates("sub_rax_rbx", 0x00, index_vars, relocation_count);
                                        relocation_count = machine_templates("mov_addr_rax", "0", index_vars, relocation_count);

                                    } else {
                                        if (strcmp(s_a[index_stack - 1], "") != 0) {
                                            relocation_count = machine_templates("mov_addr_rax", s_a[index_stack - 1], index_vars, relocation_count);
                                        }
                                        if (strcmp(s_a[index_stack - 0], "") != 0) {
                                            relocation_count = machine_templates("mov_addr_rcx", s_a[index_stack - 0], index_vars, relocation_count);
                                            relocation_count = machine_templates("sub_rcx_rax", 0x00, index_vars, relocation_count);
                                            relocation_count = machine_templates("mov_rax_rbx", 0x00, 0, relocation_count);
                                            relocation_count = machine_templates("mov_addr_rax", "0", index_vars, relocation_count);
                                        }
                                        memset(s_a[index_stack - 0], '\0',sizeof(s_a[index_stack - 0]));
                                        memset(s_a[index_stack - 1], '\0',sizeof(s_a[index_stack - 1]));
                                        index_stack = 0;
                                    }
                                    memset(s_a[index_stack - 0], '\0',sizeof(s_a[index_stack - 0]));
                                    memset(s_a[index_stack - 1], '\0',sizeof(s_a[index_stack - 1]));
                                }

                                index_stack++;
                                was_val = false;
                                v_index = 0;
                            }
                        }

                        relocation_count = machine_templates("mov_rbx_addr", variable, index_vars, relocation_count);
                        relocation_count = machine_templates("mov_addr_rbx", "0", 0, relocation_count);

                        memset(stack, '\0', sizeof(stack));
                        memset(infix, '\0', sizeof(infix));
                        memset(postfix, '\0', sizeof(postfix));
                        tokens = strtok(NULL, ";");
                    }else{
                        // TODO NEED TO FIX
//                        relocation_count = machine_templates("mov_addr_rax", postfix, index_vars, relocation_count);
//                        relocation_count = machine_templates("mov_rax_addr", variable, index_vars, relocation_count);
//                        relocation_count = machine_templates("mov_addr_rax", "0", 0, relocation_count);
                    }
                }
                get_buffer_from_token(tokens, buffer[0], buffer[1]);
                if(strcmp(words[atoi(buffer[0])-1][atoi(buffer[1])-1], "true") == 0){
                    strcat(infix, "1");
                }
                else if(strcmp(words[atoi(buffer[0])-1][atoi(buffer[1])-1], "false") == 0){
                    strcat(infix, "0");
                }else{
                    int true_value = 0;
                    if(strcmp(buffer[0], "3") ==0 || strcmp(buffer[0], "4") ==0){
                        true_value = atoi(buffer[1])-0;
                    }else{
                        true_value = atoi(buffer[1])-1;
                    }
                    strcat(infix, words[atoi(buffer[0])-1][true_value]);
//                    printf("infizzzz: %s\n",infix);
//                    printf("buffer[0]: %d\n",atoi(buffer[0])-1);
//                    printf("buffer[1]: %d\n",true_value);
                }


            }else{
                //                  as
                if(strcmp(tokens, "1,9")!=0) {
                    get_buffer_from_token(tokens, buffer[0], buffer[1]);
                    strcpy(variable, words[atoi(buffer[0])-1][atoi(buffer[1])]);
                }
            }
//                              as
            if (strcmp(tokens, "1,9") == 0) {
                is_condition_start = true;
                memset(infix, '\0', sizeof(infix));
                memset(postfix, '\0', sizeof(postfix));
            }
            //                   if                             for                         while
            if (strcmp(tokens, "1,4") == 0 || strcmp(tokens, "1,13") == 0 || strcmp(tokens, "1,5") == 0) {
                has_oper = true;
                printf("if detect\n");
                //                  for
                if(strcmp(tokens, "1,13") == 0)
                    cycle_for = true;
                //                  while
                if(strcmp(tokens, "1,5") == 0){
                    while_relocation = getFileSize(TEMP_OBJ_FILE_NAME);
                    cycle_while = true;
                }
            }

            //                      end                             next
            if(!cycle_for) {
                char hex[10]; //      }                             }
                if (strcmp(tokens, "2,22") == 0 || strcmp(tokens, "2,22") == 0) {

                    if(cycle_while){
                        code_align+=5;
                        int local_relocation =  (while_relocation - getFileSize(TEMP_OBJ_FILE_NAME)) - 5;
                        relocation_count = machine_templates("jmp", 0x00, index_vars, relocation_count);
                        memset(hex, '\0', 10);
                        sprintf(hex,"%d", local_relocation);
                        write_bytes_to_file_position(TEMP_OBJ_FILE_NAME, getFileSize(TEMP_OBJ_FILE_NAME), hex, 4);

                        cycle_while = false;
                    }

                    code_align = (getFileSize(TEMP_OBJ_FILE_NAME) - temporary_code_size) - 4;
                    memset(hex, '\0', 10);
                    sprintf(hex,"%d", code_align);

                    write_bytes_to_file_position(TEMP_OBJ_FILE_NAME, temporary_code_size, hex, 4);

                    code_align = (getFileSize(TEMP_OBJ_FILE_NAME) - temporary_code_size_else) - 4;
                    memset(hex, '\0', 10);
                    sprintf(hex,"%d", code_align);

                    write_bytes_to_file_position(TEMP_OBJ_FILE_NAME, temporary_code_size_else, hex, 4);

                }//                else
                if(strcmp(tokens, "1,8") == 0){
                    int local_tmp_code_size_else = temporary_code_size_else;

                    relocation_count = machine_templates("jmp", 0x00, index_vars, relocation_count);
                    temporary_code_size_else = getFileSize(TEMP_OBJ_FILE_NAME);
                    hex_local = 0x00000000;
                    appendToFile(TEMP_OBJ_FILE_NAME, &hex_local, 4);

                    code_align = (getFileSize(TEMP_OBJ_FILE_NAME) - local_tmp_code_size_else) - 4;
                    memset(hex, '\0', 10);
                    sprintf(hex,"%d", code_align);

                    write_bytes_to_file_position(TEMP_OBJ_FILE_NAME, local_tmp_code_size_else, hex, 4);
                }
            }

            if(has_oper){
                get_buffer_from_token(tokens, buffer[0], buffer[1]);
                //             then   }          do                }
                if((strcmp(tokens, "1,12")==0 || strcmp(tokens, "1,6")==0) && !cycle_for){ // THEN OR DO
//                if((strcmp(tokens, "2,21")==0 || strcmp(tokens, "2,21")==0) && !cycle_for){ // {
                    if(not_flag == 1){
                        relocation_count = machine_templates("not_rbx", 0x00, index_vars, relocation_count);
                    }
                    relocation_count = machine_templates("cmp_rbx_rax", 0x00, index_vars, relocation_count);
                    if(cmp_flag!=0) {
                        switch (cmp_flag) {
                            case 2: // ==
                                relocation_count = machine_templates("je", 0x00, index_vars, relocation_count);
                                break;
                            case 1: // !=
                                relocation_count = machine_templates("jne", 0x00, index_vars, relocation_count);
                                break;
                            case 6: // >=
                                relocation_count = machine_templates("jge", 0x00, index_vars, relocation_count);
                                break;
                            case 4: // =<
                                relocation_count = machine_templates("jle", 0x00, index_vars, relocation_count);
                                break;
                            case 3:  // <
                                relocation_count = machine_templates("jl", 0x00, index_vars, relocation_count);
                                break;
                            case 5:  // >
                                relocation_count = machine_templates("jg", 0x00, index_vars, relocation_count);
                                break;
                        }
                    }

                    hex_local = 0x0A;
                    appendToFile(TEMP_OBJ_FILE_NAME, &hex_local, 1);

                    relocation_count = machine_templates("jmp", 0x00, index_vars, relocation_count);
                    temporary_code_size_else = getFileSize(TEMP_OBJ_FILE_NAME);
                    hex_local = 0x00000000;
                    appendToFile(TEMP_OBJ_FILE_NAME, &hex_local, 4);

                    relocation_count = machine_templates("jmp", 0x00, index_vars, relocation_count);
                    temporary_code_size = getFileSize(TEMP_OBJ_FILE_NAME);
                    hex_local = 0x00000000;
                    appendToFile(TEMP_OBJ_FILE_NAME, &hex_local, 4);

                    has_oper = false;
                }
                if(!cycle_for) {
                        if (strcmp(buffer[0], "4") == 0 || strcmp(buffer[0], "3") == 0) {
                            if(!has_next) {
                                relocation_count = machine_templates("mov_addr_rax", words[atoi(buffer[0])-1][atoi(buffer[1])], index_vars, relocation_count);
                                has_next = true;
                            }else{
                                relocation_count = machine_templates("mov_addr_rbx", words[atoi(buffer[0])-1][atoi(buffer[1])], index_vars, relocation_count);
                                has_next = false;
                            }
                        }else if(strcmp(buffer[0], "2") == 0 && strcmp(buffer[1], "35") != 0){
                            cmp_flag = atoi(buffer[1]);
                            printf("cmp flag %d\n", cmp_flag);
                        }else if(strcmp(buffer[0], "1") == 0 && strcmp(buffer[1], "35") == 0){
                            not_flag = 1;
                        }
//                    }
                }else{
                    //                   to
                    if (strcmp(tokens, "1,7") == 0) { // DETECT to

                        tokens = strtok(NULL, ";");
                        get_buffer_from_token(tokens, buffer[0], buffer[1]);

                        temporary_code_size = getFileSize(TEMP_OBJ_FILE_NAME);

                        if (strcmp(buffer[0], "4") == 0 || (strcmp(buffer[0], "3") == 0)) {
                            relocation_count = machine_templates("mov_addr_rbp", words[atoi(buffer[0])-1][atoi(buffer[1])], index_vars, relocation_count);
                        }


                        strcpy(variable_for, variable);

                        relocation_count = machine_templates("mov_addr_rax", variable, index_vars, relocation_count);
                        relocation_count = machine_templates("mov_addr_rbx", "1", index_vars, relocation_count);
                        relocation_count = machine_templates("add_rbx_rax", 0x00, index_vars, relocation_count);

                        relocation_count = machine_templates("mov_rax_addr", variable, index_vars, relocation_count);

                    }//                   }
                    if (strcmp(tokens, "2,22") == 0) { // DETECT NEXT

                        relocation_count = machine_templates("cmp_addr_rbp", variable_for, index_vars, relocation_count);
                        relocation_count = machine_templates("jle", 0x00, index_vars, relocation_count);
                        hex_local = 0x05;
                        appendToFile(TEMP_OBJ_FILE_NAME, &hex_local, 1);

                        relocation_count = machine_templates("jmp", 0x00, index_vars, relocation_count);

                        code_align = (temporary_code_size - getFileSize(TEMP_OBJ_FILE_NAME)) + 3;
                        char hex[10];
                        memset(hex, '\0', 10);
                        sprintf(hex,"%d", code_align);

                        write_bytes_to_file_position(TEMP_OBJ_FILE_NAME, getFileSize(TEMP_OBJ_FILE_NAME), hex, 4);
                        cycle_for = false;
                        has_oper = false;
                    }
                }
            }
        //               display func
        if(strcmp(tokens, "1,11")==0){
            has_displ = true;
        }
        //                  ;
        if(strcmp(tokens, "2,13")==0){
            has_displ = false;
        }
        if(has_displ){
            bool valid = false;
            get_buffer_from_token(tokens, buffer[0], buffer[1]);

            int true_value = 0;
            if(strcmp(buffer[0], "3") ==0 || strcmp(buffer[0], "4") ==0){
                true_value = atoi(buffer[1])-0;
            }else{
                true_value = atoi(buffer[1])-1;
            }

            if(strcmp(buffer[0], "4") == 0){
                valid = true;
                relocation_count = machine_templates("mov_addr_rdx", words[atoi(buffer[0])-1][true_value], index_vars, relocation_count);

            }else if(strcmp(buffer[0], "3") == 0){
                valid = true;
                relocation_count = machine_templates("mov_addr_rdx", words[atoi(buffer[0])-1][true_value], index_vars, relocation_count);
            }
            if(valid) {
                relocation_count = machine_templates("mov_addr_rcx", "0", 0, relocation_count);

                for(int variable_format = 0; variable_format < index_vars; variable_format++){
                    if(strcmp(def_vars[variable_format].name, words[2][true_value])==0){
                        if(strcmp(def_vars[variable_format].type, "19")==0){ // @ -> float
                            relocation_count = machine_templates("lea_addr_double", 0x00, index_vars, relocation_count);
                        }else if(strcmp(def_vars[variable_format].type, "20")==0){ // & -> bool
                            relocation_count = machine_templates("lea_addr_bool", words[atoi(buffer[0])-1][true_value], index_vars, relocation_count);
                        }else{
                            relocation_count = machine_templates("lea_addr_int", 0x00, index_vars, relocation_count);
                        }
                        break;
                    }
                    relocation_count = machine_templates("lea_addr_int", 0x00, index_vars, relocation_count);
                }

                relocation_count = machine_templates("mov_rax_rcx", 0x00, 0, relocation_count);
                relocation_count = machine_templates("print", 0x00, index_vars, relocation_count);
            }
        }
            get_buffer_from_token(tokens, buffer[0], buffer[1]);

//            strcat(stack, words[atoi(buffer[0])][atoi(buffer[1])]);
            strcat(stack, tokens);
            strcat(stack, ";");
        }
        tokens = strtok(NULL, ";");
    }

    long code_count_text = 0;
    code_count_text = (getFileSize(TEMP_OBJ_FILE_NAME));

    append_bytes_from_file(TEMP_OBJ_FILE_NAME_END, TEMP_OBJ_FILE_NAME);
    competition(TEMP_OBJ_FILE_NAME, 0x90);

    SymbolEntry symbolEntry = {
            .s_name = ".file",
            .s_value = 0x00000000,
            .s_section = 0xfffe,
            .s_type = 0x0000,
            .s_storage_class = 0x67,
            .s_num_aux = 0x01
    };

    add_symbol_table(symbolEntry);

    SymbolEntry symbolEntry_fake = {
            .s_name = "fake",
            .s_value = 0x00000000,
            .s_section = 0x0000,
            .s_type = 0x0000,
            .s_storage_class = 0x00,
            .s_num_aux = 0x00
    };

    add_symbol_table(symbolEntry_fake);


    if(index_vars > 1) {
        for (int k = 0; k < index_vars; k++) {
            SymbolEntry symbolEntryVar;
            strcpy(symbolEntryVar.s_name, hex_values_l[k].name);
            symbolEntryVar.s_value = hex_values_l[k].addr;
            symbolEntryVar.s_section = 0x0002;
            symbolEntryVar.s_type = 0x0000,
                    symbolEntryVar.s_storage_class = 0x03;
            symbolEntryVar.s_num_aux = 0x00;
            add_symbol_table(symbolEntryVar);

            add_data_section(hex_values_l[k]);
        }
    }else{
        SymbolEntry symbolEntryVar;
        strcpy(symbolEntryVar.s_name, hex_values_l[0].name);
        symbolEntryVar.s_value = hex_values_l[0].addr;
        symbolEntryVar.s_section = 0x0002;
        symbolEntryVar.s_type = 0x0000,
        symbolEntryVar.s_storage_class = 0x03;
        symbolEntryVar.s_num_aux = 0x00;
        add_symbol_table(symbolEntryVar);

        add_data_section(hex_values_l[0]);
    }


    char* format_data = "%f\n\0\0\0\0\0";
    appendToFile_little_en(TEMP_OBJ_FILE_NAME_DATA, format_data, 8);
    format_data = "%d\n\0\0\0\0\0";
    appendToFile_little_en(TEMP_OBJ_FILE_NAME_DATA, format_data, 8);
    format_data = "true\n\0\0\0";
    appendToFile_little_en(TEMP_OBJ_FILE_NAME_DATA, format_data, 8);
    format_data = "false\n\0\0";
    appendToFile_little_en(TEMP_OBJ_FILE_NAME_DATA, format_data, 8);
    competition(TEMP_OBJ_FILE_NAME_DATA, 0x00);

    SymbolEntry symbolEntry_text = {
            .s_name = ".text",
            .s_value = 0x00000000,
            .s_section = 0x0001,
            .s_type = 0x0000,
            .s_storage_class = 0x03,
            .s_num_aux = 0x01
    };

    add_symbol_table(symbolEntry_text);

    SymbolEntry symbolEntry_bd = {
            .s_name = "bd",
            .s_value = 0x00000000,
            .s_section = 0x0000,
            .s_type = 0x0000,
            .s_storage_class = 0x00,
            .s_num_aux = 0x00
    };

    add_symbol_table(symbolEntry_bd);

    SymbolEntry symbolEntry_data = {
            .s_name = ".data",
            .s_value = 0x00000000,
            .s_section = 0x0002,
            .s_type = 0x0000,
            .s_storage_class = 0x03,
            .s_num_aux = 0x01
    };

    add_symbol_table(symbolEntry_data);

    SymbolEntry symbolEntry_dd = {
            .s_name = "1d",
            .s_value = 0x00000000,
            .s_section = 0x0000,
            .s_type = 0x0000,
            .s_storage_class = 0x00,
            .s_num_aux = 0x00
    };

    add_symbol_table(symbolEntry_dd);

    SymbolEntry symbolEntry_bss = {
            .s_name = ".bss",
            .s_value = 0x00000000,
            .s_section = 0x0003,
            .s_type = 0x0000,
            .s_storage_class = 0x03,
            .s_num_aux = 0x01
    };

    add_symbol_table(symbolEntry_bss);

    SymbolEntry symbolEntry_null = {
            .s_name = 0x0000000000000000,
            .s_value = 0x00000000,
            .s_section = 0x0000,
            .s_type = 0x0000,
            .s_storage_class = 0x00,
            .s_num_aux = 0x00
    };

    add_symbol_table(symbolEntry_null);


    SymbolEntry symbolEntry_main = {
            .s_name = "main",
            .s_value = 0x00000003,
            .s_section = 0x0001,
            .s_type = 0x0000,
            .s_storage_class = 0x02,
            .s_num_aux = 0x00
    };

    add_symbol_table(symbolEntry_main);

    uint8_t machine_code[] = {
            0x70, 0x72, 0x69, 0x6E, 0x74, 0x66, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00
    };

    SymbolEntry symbolEntry_print;
    memcpy(&symbolEntry_print, machine_code, sizeof(SymbolEntry));

    add_symbol_table(symbolEntry_print);

    uint32_t hex_local_l = 0x00000004;
    appendToFile(TEMP_OBJ_FILE_SYMBOLS, &hex_local_l, 4);

    competition(TEMP_OBJ_FILE_NAME, 0x00);
    competition(TEMP_OBJ_FILE_RELOCATIONS, 0x00);
    competition(TEMP_OBJ_FILE_SYMBOLS, 0x00);

    long code_count_data = 0;
    code_count_data = getFileSize(TEMP_OBJ_FILE_NAME_DATA);

    long code_count_relocations = 0;
    code_count_relocations = getFileSize(TEMP_OBJ_FILE_RELOCATIONS);
    raw_data = getFileSize(TEMP_OBJ_FILE_NAME);
    append_bytes_from_file(TEMP_OBJ_FILE_NAME_DATA, TEMP_OBJ_FILE_NAME);

    fileHeader.Machine = 0x8664;
    fileHeader.NumberOfSections = 0x0003;
    fileHeader.TimeDateStamp = 0x00000000;
    fileHeader.PointerToSymbolTable =  raw_data + code_count_data + code_count_relocations - 4 + (48*3);
    fileHeader.NumberOfSymbols =  index_vars + 10;
    fileHeader.SizeOfOptionalHeader = 0x0000;
    fileHeader.Characteristics = 0x0005;

    add_coff_header();

    text.physical_address = 0x00000000;
    text.virtual_address  = 0x00000000;
    text.size_of_raw_data = raw_data;
    text.file_pointer_to_raw_data =  0x0000008c; // !!!!
    text.file_pointer_to_relocation_table = 0x0000008c+raw_data+code_count_data;
    text.file_pointer_to_line_numbers = 0x00000000;
    text.number_of_relocations = relocation_count;
    text.number_of_line_numbers = 0x0000;
    text.flags = 0x60500020;

    hex_local_l = 0x2E; // name .
    appendToFile(OBJ_FILE_NAME, &hex_local_l, 1);
    hex_local_l = 0x74786574; // name text
    appendToFile(OBJ_FILE_NAME, &hex_local_l, 4);

    hex_local_l = 0x000000; // name ____
    appendToFile(OBJ_FILE_NAME, &hex_local_l, 3);

    add_section_header(text);


    data.physical_address = 0x00000000;
    data.virtual_address  = 0x00000000;
    data.size_of_raw_data = code_count_data;
    data.file_pointer_to_raw_data = 0x0000008c + raw_data; // !!!!
    data.file_pointer_to_relocation_table = 0x00000000;
    data.file_pointer_to_line_numbers = 0x00000000;
    data.number_of_relocations = 0x0000;
    data.number_of_line_numbers = 0x0000;
    data.flags = 0xC0500040;

    hex_local_l = 0x2E; // name .
    appendToFile(OBJ_FILE_NAME, &hex_local_l, 1);
    hex_local_l = 0x61746164; // name data
    appendToFile_little_en(OBJ_FILE_NAME, &hex_local_l, 4);
    hex_local_l = 0x000000; // name ____
    appendToFile(OBJ_FILE_NAME, &hex_local_l, 3);

    add_section_header(data);

    bss.physical_address = 0x00000000;
    bss.virtual_address  = 0x00000000;
    bss.size_of_raw_data = 0x00000000;
    bss.file_pointer_to_raw_data =  0x00000000; // !!!!
    bss.file_pointer_to_relocation_table = 0x00000000;
    bss.file_pointer_to_line_numbers = 0x00000000;
    bss.number_of_relocations = 0x0000;
    bss.number_of_line_numbers = 0x0000;
    bss.flags = 0xC0500080;

    hex_local_l = 0x2E; // name .
    appendToFile(OBJ_FILE_NAME, &hex_local_l, 1);
    hex_local_l = 0x737362; // name bss
    appendToFile(OBJ_FILE_NAME, &hex_local_l, 3);

    hex_local_l = 0x00000000; // name ____
    appendToFile(OBJ_FILE_NAME, &hex_local_l, 4);

    add_section_header(bss);


    append_bytes_from_file(TEMP_OBJ_FILE_NAME, OBJ_FILE_NAME);
    append_bytes_from_file(TEMP_OBJ_FILE_RELOCATIONS, OBJ_FILE_NAME);
    append_bytes_from_file(TEMP_OBJ_FILE_SYMBOLS, OBJ_FILE_NAME);

    competition(OBJ_FILE_NAME, 0x00);

    fclose(fopen("asm.s", "w"));
    file_write("asm.s", result);

    printf("\nИдентификаторы:\n\n");
    for(int var = 0; var < index_vars; var++){
        printf("Номер идентификатора: %d\nИдентификатор: %s\nЗначение: %llx\nадрес: %x\n\n", var, hex_values_l[var].name, hex_values_l[var].value, hex_values_l[var].addr);
    }

    for(int tb = 0; tb < 13; tb++){
        printf("Номер числа: %d Значение: '%s'\n", tb, words[0][tb]);
    }
    printf("\nDELIMITERS:\n\n");
    for(int tb = 0; tb < 25; tb++){
        printf("Номер разделителя: %d Значение: '%s'\n", tb, words[1][tb]);
    }

    return true;
}