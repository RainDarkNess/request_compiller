package org.compiler;

import org.compiler.instrument.CustomException;
import org.compiler.syntax.Syntax;

import java.io.IOException;
import java.util.ArrayList;

public class Main {
    public static void main(String[] args) throws IOException, CustomException {
        Lexer lexer = new Lexer();
        ArrayList<ArrayList> result = lexer.lexerStart();
        Syntax syntax = new Syntax();
        syntax.start(result);
    }
}