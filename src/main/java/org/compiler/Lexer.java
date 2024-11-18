package org.compiler;

import java.io.*;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;

import static org.compiler.instrument.DelimiterWordHelp.readDelimitersWords;

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

            while ((character = reader.read()) != -1) {
                if(character != '\n' && character != '\t' && character != '\r') {

                    if((char) character == ';' || (char) character == ' ' || (char) character == ')' || (char) character == ','){
                        if (!buffer.isEmpty()) {
                            char firstChar = buffer.charAt(0);
                            if(Character.isDigit(firstChar)){
                                writer.append("4,");
                                writer.append(String.valueOf(value_count)).append(";");
                                System.out.println("VALUE 4,"+value_count+" v:"+buffer);
                                table_vars.add(String.valueOf(buffer));
                                value_count++;
                            }else{
                                writer.append("3,");
                                writer.append(String.valueOf(ind_count)).append(";");
                                System.out.println("VAR 3,"+ind_count+" v:"+buffer);
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
                            System.out.println("DELIMITER 2,"+index+" v:"+buffer);
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
                            System.out.println("WORD 1,"+index+" v:"+buffer);
                            buffer = new StringBuilder();
                            writer.append("1,");
                            writer.append(String.valueOf(index)).append(";");

                            continue;
                        }
                        index++;
                    }

                }
            }
        } catch (IOException ignored) {}
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
