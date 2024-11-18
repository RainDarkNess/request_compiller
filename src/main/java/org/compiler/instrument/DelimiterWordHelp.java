package org.compiler.instrument;

import org.compiler.Lexer;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;

public class DelimiterWordHelp {
    public static String readDelimitersWords(String filename) throws IOException {
        ArrayList<String> result = new ArrayList<>();
        InputStream inputStream = Lexer.class.getClassLoader().getResourceAsStream("files/"+filename);
        assert inputStream != null;
        InputStreamReader streamReader = new InputStreamReader(inputStream, StandardCharsets.UTF_8);
        BufferedReader reader = new BufferedReader(streamReader);
        int character;
        StringBuilder sb = new StringBuilder();
        while ((character = reader.read()) != -1) {
            sb.append((char)character);
        }
        return sb.toString();
    }
}
