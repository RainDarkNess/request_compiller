package org.compiler.syntax;

import org.compiler.Variable;
import org.compiler.instrument.CustomException;

import java.io.*;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Objects;

import static org.compiler.instrument.DelimiterWordHelp.readDelimitersWords;

public class Syntax {
    static String words = "";
    static String delimiters = "";

    Debug debug = new Debug();

    ArrayList<ArrayList> tableForAll;
    ArrayList<Variable> variables = new ArrayList<>();

    // SYNTAX FLAGS AND COUNTERS
    int startTokens = 0;
    boolean isStarted = false;
    boolean varAction = false;
    boolean programStart = false;
    boolean typeCheck = false;
    boolean assignVar = false;

    // Condition booleans
    boolean chkVar = false;
    boolean chkOperator = false;

    // If booleans
    boolean isIf = false;
    int ifBody = 0;
    boolean thenDetect = false;

    int tmp_type = 0;
    ArrayList<String> tmp_var_name = new ArrayList<>();

    public void start(ArrayList<ArrayList> table) throws IOException, CustomException {
        tableForAll = table;

        // read generated file
        BufferedReader reader = new BufferedReader(new InputStreamReader(new FileInputStream("./table"), StandardCharsets.UTF_8));
        int c;
        Syntax.words = readDelimitersWords("words");
        Syntax.delimiters = readDelimitersWords("delimiters");
        StringBuilder builder = new StringBuilder();
        while((c = reader.read()) != -1) {
            char character = (char) c;

            if((char)character != ';'){
                builder.append(character);
            }else{
                syntaxStart(builder.toString());
                builder = new StringBuilder();
                continue;
            }
        }
    }
    public String writeMessageByBuffer(String buffer){
        int tableNum = Integer.parseInt(buffer.split(",")[0]);
        int tableVal = Integer.parseInt(buffer.split(",")[1])-1;
        return switch (tableNum) {
            case 1 -> Syntax.words.split(" ")[tableVal];
            case 2 -> Syntax.delimiters.split(" ")[tableVal];
            case 3 -> tableForAll.get(1).get(tableVal+1).toString();
            case 4 -> tableForAll.get(0).get(tableVal+1).toString();
            default -> "";
        };
    }
    public void varAssign(String buffer) throws CustomException{
        for(String var: tmp_var_name) {
            for(Variable variable : variables){
                if (Objects.equals(variable.name, String.valueOf(var))) {
                    if(variable.init){
                        return;
                    }
                    variable.value = Integer.parseInt((String) tableForAll.get(0).get(Integer.parseInt(buffer.split(",")[1])));
                    variable.init = true;
                    debug.show("Переменной " + String.valueOf(var) + " присвоено значение " + variable.value);
                    return;
                }
            }
        }
        throw new CustomException("Ошибка! Переменная не существует ");
    }

    public boolean varCheck(String buffer){
        debug.show("Провекра переменной " + tableForAll.get(1).get(Integer.parseInt(buffer.split(",")[1])));
        for(Variable variable: variables){
            if(variable.name.equals(tableForAll.get(1).get(Integer.parseInt(buffer.split(",")[1])))){
                debug.show("Проверка переменной пройдена");
                return variable.init;
            }
        }
        return false;
    }
    public boolean valueCheck(String buffer){
        debug.show("Найдено значение: " + writeMessageByBuffer(buffer));

        return true;
    }

    public void condition(String buffer) throws CustomException {
        boolean valid = false;
        int tableNum = Integer.parseInt(buffer.split(",")[0]);
        int tableVal = Integer.parseInt(buffer.split(",")[1]);
        if(!chkVar){
            if(tableNum == 3){
                valid = varCheck(buffer);
            }else if(tableNum == 4){
                valid = valueCheck(buffer);
            }
            if(!valid){
                throw new CustomException("Неожиданный символ или переменная не объявлена " + writeMessageByBuffer(buffer));
            }
            chkVar = valid;
            chkOperator = false;
        }else if(!chkOperator){
            if(tableNum == 2){
                if (tableVal > 0 && tableVal < 7){
                    debug.show("Найден оператор присвоения " + writeMessageByBuffer(buffer));
                }
                chkVar = false;
                chkOperator = true;
            }else {
                throw new CustomException("Неожиданный идентификатор " + writeMessageByBuffer(buffer));
            }
        }

    }

    public void syntaxCodeBody(String buffer)throws CustomException{
        String[] words = Syntax.words.split(" ");
        String[] delimiters = Syntax.delimiters.split(" ");

        if(Integer.parseInt(buffer.split(",")[0]) == 3){
            varAction = true;
        }

        if (Objects.equals(buffer, "2,21")) {
            if(!thenDetect){
                throw new CustomException("Ошибка! Неожиданная открывающаяся скобка!." + writeMessageByBuffer(buffer));
            }
            debug.show("Открыта скобка if");
            ifBody++;
            thenDetect = false;
        }else if(thenDetect && !Objects.equals(buffer, "2,21")){
            throw new CustomException("Ошибка! Открывающаяся скобка в составном операторе не найдена!." + writeMessageByBuffer(buffer));
        }

        if (Objects.equals(buffer, "2,22")) {
            debug.show("Закрыта скобка if");
            if(ifBody > 0 ){
                ifBody--;
            }else{
                throw new CustomException("Ошибка! Неожиданная закрывающаяся скобка!." + writeMessageByBuffer(buffer));
            }
        }


        if (Objects.equals(buffer, "1,12")) {
            if(chkOperator){
                throw new CustomException("Ошибка! Неверное условие в if." + writeMessageByBuffer(buffer));
            }
            thenDetect = true;
            debug.show("Найден then. Тело if начинается");
            isIf = false;
        }
        if (Objects.equals(buffer, "1,8")) {
            thenDetect = true;
            debug.show("Найден else. Тело if продолжается");
            isIf = false;
        }

        if(isIf){
            condition(buffer);
            return;
        }

        if (Objects.equals(buffer, "1,4")) {
            debug.show("Найден if");
            isIf = true;
        }

        if(varAction){
            if (Objects.equals(buffer, "2,23")) {
                debug.show("Найдено несколько переменных");
            } else if (Integer.parseInt(buffer.split(",")[0]) == 3) {
                debug.show("Идет операция над переменной: " + writeMessageByBuffer(buffer));
                tmp_var_name.add((String) tableForAll.get(1).get(Integer.parseInt(buffer.split(",")[1])));
            }

            //                            int                                 float                                 bool
            if(Objects.equals(buffer, "2,18") || Objects.equals(buffer, "2,19") || Objects.equals(buffer, "2,20")){
                debug.show("Найден тип данных: " + delimiters[Integer.parseInt(buffer.split(",")[1])-1]);
                typeCheck = true;
                tmp_type = Integer.parseInt(buffer.split(",")[1]);
                for(String var: tmp_var_name){
                    for (Variable variable : variables) {
                        if (Objects.equals(variable.name, String.valueOf(var))) {
                            throw new CustomException("Данное имя переменной уже занято!. " + String.valueOf(var));
                        }
                    }
                    Variable var_add = new Variable();
                    var_add.name = String.valueOf(var);
                    var_add.value = 0;
                    var_add.type = Integer.parseInt(buffer.split(",")[1]);
                    var_add.init = false;
                    variables.add(var_add);
                }
                tmp_var_name.clear();
            }

            if(assignVar){
                if(Integer.parseInt(buffer.split(",")[0]) == 4){
                    varAssign(buffer);
                }
                tmp_var_name.clear();
            }
            //                              as
            if (Objects.equals(buffer, "1,9")) {
                debug.show("Идет присвоение переменной: " + tmp_var_name.get(0));
                assignVar = true;
            }
        }
        //                              ;
        if(Objects.equals(buffer, "2,13")){
            if(typeCheck && programStart){
                throw new CustomException("Ошибка! В данном модальном языке может быть объявлен только 1 тип данных.");
            }
            varAction = false;
            programStart = true;
            typeCheck = false;
            assignVar = false;
        }

    }

    public void syntaxStart(String buffer) throws CustomException {
        if(isStarted){
            if(startTokens != 2){
                throw new CustomException("Ошибка! Отсуствует обзательное слово вначале программы. Найдено " + writeMessageByBuffer(buffer));
            }
            syntaxCodeBody(buffer);
        }else{
            if(Objects.equals(buffer, "1,1")){
                startTokens++;
                debug.show("Найден begin");
            }
            if(Objects.equals(buffer, "1,2")){
                if(startTokens==1) {
                    startTokens++;
                    isStarted = true;
                    debug.show("Найден var");
                }else{
                    throw new CustomException("Ошибка! Отсуствует обзательное слово вначале программы. Найдено " + writeMessageByBuffer(buffer));
                }
            }
        }
    }
}
