package org.compiler.syntax;

import org.compiler.Variable;
import org.compiler.instrument.CustomException;

import java.io.*;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Objects;

import static org.compiler.instrument.DelimiterWordHelp.readDelimitersWords;
import static org.compiler.instrument.FileCreator.clearFile;
import static org.compiler.instrument.FileCreator.createFileWithString;

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
    boolean varOperation = false;
    boolean hasOperator = false;
    boolean programStart = false;
    boolean typeCheck = false;
    boolean assignVar = false;

    // End.
    boolean hasEnd = false;
    boolean hasDot = false;

    // Condition booleans
    boolean chkVar = false;
    boolean chkOperator = false;

    // If booleans
    boolean isIf = false;
    int conditionBody = 0;
    boolean thenDetect = false;

    // While booleans
    boolean isWhile = false;
    boolean doDetect = false;

    // For booleans
    boolean isFor = false;
    boolean forDone = false;
    boolean variableCreation = false;
    String variableIdTmp = "";
    boolean asDetect = false;
    boolean toDetect = false;
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
        clearFile("./varibles");
        for(Variable var: variables){
            createFileWithString("./varibles", var.name);
            createFileWithString("./varibles", ";");
            createFileWithString("./varibles", var.value);
            createFileWithString("./varibles", ";");
            createFileWithString("./varibles", var.type);
            createFileWithString("./varibles", ";");
        }
        clearFile("./values");
        for(int i = 0; i < tableForAll.get(0).size(); i++){
            createFileWithString("./values", (String) tableForAll.get(0).get(i));
            createFileWithString("./values", ";");
        }
        clearFile("./ind");
        for(int i = 0; i < tableForAll.get(1).size(); i++){
            createFileWithString("./ind", (String) tableForAll.get(1).get(i));
            createFileWithString("./ind", ";");
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
        int tableNum = Integer.parseInt(buffer.split(",")[0]);
        if(buffer.equals("2,13")){
            varAction = false;
            return;
        }
        for(String var: tmp_var_name) {
            for(Variable variable : variables){
                if (Objects.equals(variable.name, String.valueOf(var))) {
                    if(variable.init){
                        return;
                    }
                    if(tableNum == 3){
                        ArrayList arr = tableForAll.get(1);
                        String var_tmp_name = tableForAll.get(1).get(Integer.parseInt(buffer.split(",")[1])).toString();
                        for(Variable _variable_ : variables){
                            if(Objects.equals(_variable_.name, var_tmp_name)){
                                if(Objects.equals(variable.type, _variable_.type)){
                                    variable.value = _variable_.value;
                                }else{
                                    throw new CustomException("Ошибка! У переменных разные типы данных" +  variable.value + " и " +_variable_.value);
                                }
                            }
                        }
                    }else if(tableNum == 4){
                        String type = valueCheck(tableForAll.get(0).get(Integer.parseInt(buffer.split(",")[1])).toString());
                        if(Objects.equals(type, variable.type)){
                            variable.value = ((String) tableForAll.get(0).get(Integer.parseInt(buffer.split(",")[1])));
                        }else{
                            throw new CustomException("Ошибка! У значения и переменной разные типы данных " +  variable.type + " и " +type);
                        }
                    }
                    variable.init = true;
                    debug.show("Переменной " + String.valueOf(var) + " присвоено значение " + variable.value);
                    varOperation = true;
                    hasOperator = false;
                    return;
                }
            }
        }
        if(tableNum == 4){
            debug.show("Найдено значение в выражении " + writeMessageByBuffer(buffer));
            varOperation = true;
            hasOperator = false;
            return;
        }//                umn                     del                     &&                      add
        if(buffer.equals("2,7") || buffer.equals("2,8") || buffer.equals("2,9") || buffer.equals("2,24")){
            debug.show("Найден оператор действия над переменной " + writeMessageByBuffer(buffer));
            hasOperator = true;
            if(varOperation)
                varOperation = false;
            else
                throw new CustomException("Неожиданный символ " + writeMessageByBuffer(buffer));
            return;
        }
        throw new CustomException("Ошибка! Переменная не существует " + writeMessageByBuffer(buffer));
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
    public String valueCheck(String buffer){
            String typeData = "18"; // int

            if (buffer.equals("true") || buffer.equals("false")) {
                typeData = "20"; // bool
                return typeData;
            }

            double test = 0.0;

            if (buffer.length() >= 19) {
                try {
                    test = ieee754ToDouble(Long.parseUnsignedLong(buffer));
                    String testValue = String.format("%f", test);
                    boolean hasDot = false;

                            System.out.printf("Double detected: %f%n", test);

                    for (int i = 0; i < testValue.length(); i++) {
                        if (hasDot) {
                            if (testValue.charAt(i) != '0') {
                                return "19"; // float
                            }
                        }
                        if (testValue.charAt(i) == ',') {
                            hasDot = true;
                        }
                    }
                    return "18"; // int
                } catch (NumberFormatException e) {
                    // Обработка ошибки преобразования
                    return typeData;
                }
            }

            if (buffer.equals("true") || buffer.equals("false")) {
                return "20";
            }

            return typeData;
        }

        private static double ieee754ToDouble(long value) {
            return Double.longBitsToDouble(value);
    }

    public void condition(String buffer) throws CustomException {
        boolean valid = false;
        int tableNum = Integer.parseInt(buffer.split(",")[0]);
        int tableVal = Integer.parseInt(buffer.split(",")[1]);
        if(!chkVar){
            if(tableNum == 3){
                valid = varCheck(buffer);
            }else if(tableNum == 4){
//                valid = valueCheck(buffer);
                valid = true;
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

    public void varCreate(String buffer){
        Variable variable = new Variable();
        variable.name = tableForAll.get(1).get(Integer.parseInt(buffer.split(",")[1])).toString();
        variable.init = true;
        variable.value = "0";
        variable.type = "";
        variables.add(variable);
    }

    private void forBodyCreate(String buffer) throws CustomException {
        int tableNum = Integer.parseInt(buffer.split(",")[0]);
        int tableVal = Integer.parseInt(buffer.split(",")[1]);
        if(toDetect){
            if (tableNum == 3) {
                debug.show("Найдена переменная в for для to: " + writeMessageByBuffer(buffer));
                forDone = true;
                varCheck(buffer);
                return;
            }else if(tableNum == 4){
                debug.show("Найдено значение в for для to: " + writeMessageByBuffer(buffer));
                forDone = true;
                valueCheck(buffer);
                return;
            }
            if(!doDetect){
                if (buffer.equals("1,6")) {
                    debug.show("Найдено do в for");
                    doDetect = true;
                    toDetect = false;
                    asDetect = false;
                    variableIdTmp = "";
                    variableCreation = false;
                    isFor = false;
                    return;
                }else{
                    throw new CustomException("Ошибка! Неожиданный символ в for в конце объявления цикла!." + writeMessageByBuffer(buffer));
                }
            }
            return;
        }
        if(!variableCreation) {
            if (tableNum == 3) {
                variableCreation = true;
                variableIdTmp = buffer;
                debug.show("Найдена переменная для for " + writeMessageByBuffer(buffer));

            } else {
                throw new CustomException("Ошибка! В for нет объявляемой переменной!." + writeMessageByBuffer(buffer));
            }
        }else{
            if(!asDetect) {
                if (buffer.equals("1,9")) {
                    asDetect = true;
                    debug.show("Найден as в for ");

                }else{
                    throw new CustomException("Ошибка! В for нет обязательного слова as!." + writeMessageByBuffer(buffer));
                }
            }else{
                if (tableNum == 3) {
                    debug.show("Найдена переменная в for " + writeMessageByBuffer(buffer));
                    varCheck(buffer);
                }else if(tableNum == 4){
                    debug.show("Найдено значение в for " + writeMessageByBuffer(buffer));
                    valueCheck(buffer);
                }else if(!toDetect){
                    if(buffer.equals("1,7")){
                        debug.show("Найдено to в for ");
                        toDetect = true;
                        variableCreation = false;
                    }else{
                        throw new CustomException("Ошибка! Неожиданный символ в for!." + writeMessageByBuffer(buffer));
                    }
                }else{
                    throw new CustomException("Ошибка! Неожиданный символ в for!." + writeMessageByBuffer(buffer));
                }
                varCreate(variableIdTmp);
                tmp_var_name.add((String) tableForAll.get(1).get(Integer.parseInt(variableIdTmp.split(",")[1])));
                varAssign(buffer);
            }
        }

    }

    public void syntaxCodeBody(String buffer)throws CustomException{
        String[] words = Syntax.words.split(" ");
        String[] delimiters = Syntax.delimiters.split(" ");

        if(Objects.equals(buffer, "1,3")){
            hasEnd = true;
            return;
        }
        if(Objects.equals(buffer, "2,12")){
            if(hasEnd){
                debug.show("Программа остановлена");
                if(conditionBody>0){
                    throw new CustomException("Ошибка! Незакрытая скобка составного оператора" + writeMessageByBuffer(buffer));
                }
            }else{
                throw new CustomException("Ошибка! Случайностоящая точка" + writeMessageByBuffer(buffer));
            }
        }
        if(hasEnd && !Objects.equals(buffer, "2,12")){
            throw new CustomException("Ошибка! незавершенный " + writeMessageByBuffer(buffer));
        }
        if(Integer.parseInt(buffer.split(",")[0]) == 3){
            varAction = true;
        }

        if (Objects.equals(buffer, "2,21")) {
            if(!thenDetect && !doDetect){
                throw new CustomException("Ошибка! Неожиданная открывающаяся скобка!." + writeMessageByBuffer(buffer));
            }
            debug.show("Открыта скобка");
            conditionBody++;
            thenDetect = false;
            doDetect = false;
        }else if((thenDetect || doDetect) && !Objects.equals(buffer, "2,21")){
            throw new CustomException("Ошибка! Открывающаяся скобка в составном операторе не найдена!." + writeMessageByBuffer(buffer));
        }

        if (Objects.equals(buffer, "2,22")) {
            debug.show("Закрыта скобка");
            if(conditionBody > 0 ){
                conditionBody--;
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
            chkVar = false;
        }
        if (Objects.equals(buffer, "1,8")) {
            thenDetect = true;
            debug.show("Найден else. Тело if продолжается");
            isIf = false;
        }

        if(isFor){
            forBodyCreate(buffer);
            return;
        }

        if (Objects.equals(buffer, "1,13")) {
            debug.show("Найден for");
            isFor = true;
        }

        if (Objects.equals(buffer, "1,6")) {
            if(chkOperator){
                throw new CustomException("Ошибка! Неверное условие в цикле." + writeMessageByBuffer(buffer));
            }
            doDetect = true;
            if(isFor) {
                if (forDone) {
                    debug.show("Найден do. Тело while начинается");
                    forDone = false;
                } else {
                    throw new CustomException("Ошибка! Неверное условие в цикле." + writeMessageByBuffer(buffer));
                }
            }

            isWhile = false;
            chkVar = false;
        }

        if(isIf){
            condition(buffer);
            return;
        }
        if(isWhile){
            condition(buffer);
            return;
        }


        if (Objects.equals(buffer, "1,4")) {
            debug.show("Найден if");
            isIf = true;
        }

        if(Objects.equals(buffer, "1,5")){
            debug.show("Найден while");
            isWhile = true;
        }
        int tableNum = Integer.parseInt(buffer.split(",")[0]);
        if(varAction){
            if (Objects.equals(buffer, "2,23")) {
                debug.show("Найдено несколько переменных");
                varOperation = false;
            } else if (Integer.parseInt(buffer.split(",")[0]) == 3) {
                if(!varOperation){
                    debug.show("Идет операция над переменной: " + writeMessageByBuffer(buffer));
                    tmp_var_name.add((String) tableForAll.get(1).get(Integer.parseInt(buffer.split(",")[1])));
                    varOperation = true;
                }else{
                    throw new CustomException("Отсуствие обязательного слово 'as'" + writeMessageByBuffer(buffer));
                }
            } else if (Integer.parseInt(buffer.split(",")[0]) == 4) {
                if(!varOperation){
                    debug.show("Идет операция с значением: " + writeMessageByBuffer(buffer));
                    varOperation = true;
                }else{
                    throw new CustomException("Отсуствие обязательного разделительного слова. Найдено: " + writeMessageByBuffer(buffer));
                }
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
                    var_add.value = "0";
                    var_add.type = buffer.split(",")[1];
                    var_add.init = false;
                    variables.add(var_add);
                }
                tmp_var_name.clear();
            }

            if(assignVar){
                varAssign(buffer);
                tmp_var_name.clear();
            }
            //                              as
            if (Objects.equals(buffer, "1,9")) {
                debug.show("Идет присвоение переменной: " + tmp_var_name.get(0));
                assignVar = true;
                varOperation = false;
            }
        }
        //                              ;
        if(Objects.equals(buffer, "2,13")){
            if(typeCheck && programStart){
                throw new CustomException("Ошибка! В данном модальном языке может быть объявлен только 1 тип данных.");
            }
            if(hasOperator)
                throw new CustomException("Ошибка! Ошибка в выражении.");

            varAction = false;
            programStart = true;
            typeCheck = false;
            assignVar = false;
            varOperation = false;
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
