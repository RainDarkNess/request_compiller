package org.compiler;

import org.compiler.instrument.CustomException;
import org.compiler.syntax.Syntax;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.ArrayList;

public class Main {
    public static void main(String[] args) throws IOException, CustomException {
        Lexer lexer = new Lexer();
        ArrayList<ArrayList> result = lexer.lexerStart();
        Syntax syntax = new Syntax();
        syntax.start(result);

        // FORMING COFF FILE
        String cFileName = "lib.c";
        String executableName = "program";

        try {
            ProcessBuilder compileProcessBuilder = new ProcessBuilder("gcc", cFileName, "-o", executableName);
            Process compileProcess = compileProcessBuilder.inheritIO().start();
            int compileExitCode = compileProcess.waitFor();
            if (compileExitCode != 0) {
                System.out.println("Compilation failed.");
                return;
            }
            System.out.println("Compilation successful.");

            ProcessBuilder runProcessBuilder = new ProcessBuilder("./" + executableName); // Используйте executableName на Windows
            Process runProcess = runProcessBuilder.start();

            BufferedReader reader = new BufferedReader(new InputStreamReader(runProcess.getInputStream()));
            String line;
            while ((line = reader.readLine()) != null) {
                System.out.println(line);
            }

            int runExitCode = runProcess.waitFor();
            System.out.println("Program exited with code: " + runExitCode);
        } catch (IOException | InterruptedException e) {
            e.printStackTrace();
        }
    }
}