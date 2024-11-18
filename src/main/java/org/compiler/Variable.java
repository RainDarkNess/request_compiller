package org.compiler;

public class Variable {
    public String name;
    public int value;
    public boolean init;
    public int type;

    public int getValue(){return value;}
    public boolean getInit(){
        return init;
    }
    public int getType(){return type;}
}
