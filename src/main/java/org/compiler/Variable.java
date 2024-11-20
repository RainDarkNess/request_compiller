package org.compiler;

public class Variable {
    public String name;
    public String value;
    public boolean init;
    public String type;

    public String getValue(){return value;}
    public boolean getInit(){
        return init;
    }
    public String getType(){return type;}
}
