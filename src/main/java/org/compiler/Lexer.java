package org.compiler;

import org.compiler.instrument.CustomException;

import java.io.*;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;

import static org.compiler.instrument.DelimiterWordHelp.readDelimitersWords;
import static org.compiler.instrument.DigitCheck.convertValue;

public class Lexer {

    static String words = "";
    static String delimiters = "";
    static ArrayList<String> table_vars = new ArrayList<String>();
    static ArrayList<String> table_values = new ArrayList<String>();
    static ArrayList<ArrayList> result = new ArrayList<ArrayList>();
    public static ArrayList<ArrayList> readFile(String fileName) throws IOException {
        Lexer.words = readDelimitersWords("words");
        Lexer.delimiters = readDelimitersWords("delimiters");
        InputStream inputStream = Lexer.class.getClassLoader().getResourceAsStream(fileName);
        assert inputStream != null;
        InputStreamReader streamReader = new InputStreamReader(inputStream, StandardCharsets.UTF_8);
        BufferedReader reader = new BufferedReader(streamReader);

        StringBuilder buffer = new StringBuilder();

        int ind_count = 0;
        int value_count = 0;

        // Write temporary table file
        FileWriter clear = new FileWriter("./table", false);
        BufferedWriter writer = new BufferedWriter(new FileWriter("./table", true));

        try {
            int character;
            String[] words = Lexer.words.split(" ");
            String[] delimiters = Lexer.delimiters.split(" ");

            boolean comment_fist = false;
            boolean comment_start = false;
            boolean comment_pre_end = false;
            boolean comment_end = false;

            while ((character = reader.read()) != -1) {
                if(character != '\n' && character != '\t' && character != '\r') {

                    if(character == '/'){
                        if(comment_pre_end)
                            comment_end = true;
                        comment_fist = true;
                        continue;
                    }
                    if(character == '*'){
                        if(comment_start)
                            comment_pre_end = true;

                        if(comment_fist)
                            comment_start = true;
                        else
                            throw new CustomException("Ошибка форматирования комментария отсуствует '/'");
                        continue;
                    }
                    if(comment_end){
                        comment_fist = false;
                        comment_start = false;
                        comment_pre_end = false;
                        comment_end = false;
                    }

                    if(comment_pre_end)
                        throw new CustomException("Ошибка форматирования комментария отсустсвует '/'");

                    if(comment_start)
                        continue;

                    if(comment_fist)
                        throw new CustomException("Ошибка форматирования комментария отсустсвует '*'");



                    if((char) character == ';' || (char) character == ' ' || (char) character == ')' || (char) character == ','){
                        if (!buffer.isEmpty()) {
                            char firstChar = buffer.charAt(0);
                            if(Character.isDigit(firstChar) || firstChar == 't' || firstChar == 'f'){
                                writer.append("4,");
                                writer.append(String.valueOf(value_count)).append(";");
                                System.out.println("Значение 4,"+value_count+" v:"+buffer);
                                buffer = new StringBuilder(convertValue(String.valueOf(buffer)));
                                if(buffer.toString().equals("false_verification")){
                                    throw new CustomException("Неверно введеное число");
                                }
                                table_vars.add(String.valueOf(buffer));
                                value_count++;
                            }else{
                                writer.append("3,");
                                writer.append(String.valueOf(ind_count)).append(";");
                                System.out.println("Идентификатор 3,"+ind_count+" v:"+buffer);
                                table_values.add(String.valueOf(buffer));
                                ind_count++;
                            }
                        }
                        buffer = new StringBuilder();
                        if((char) character == ' '){
                            continue;
                        }
                    }

                    buffer.append((char) character);

                    int index = 1;
                    // Read of delimiters
                    for (String delimiter : delimiters) {

                        if (String.valueOf(buffer).equals(delimiter)) {
                            System.out.println("Разделитель 2,"+index+" v:"+buffer);
                            buffer = new StringBuilder();
                            writer.append("2,");
                            writer.append(String.valueOf(index)).append(";");
                            continue;
                        }
                        index++;
                    }

                    index = 1;
                    // Read of system words
                    for (String word : words) {
                        if (String.valueOf(buffer).equals(word)) {
                            System.out.println("Служ слово 1,"+index+" v:"+buffer);
                            buffer = new StringBuilder();
                            writer.append("1,");
                            writer.append(String.valueOf(index)).append(";");

                            continue;
                        }
                        index++;
                    }

                }
            }
        } catch (IOException ignored) {} catch (CustomException e) {
            throw new RuntimeException(e);
        }
        writer.close();
        result.add(table_vars);
        result.add(table_values);
        return result;
    }

    public ArrayList<ArrayList> lexerStart() throws IOException {

        result = readFile("examples/code");
        return result;
    }
}
