package org.compiler.syntax;

public class Debug {
    public boolean debugFlag = true;
    public void show(String string){
        if(debugFlag){
            System.out.println(string);
        }
    }
}
