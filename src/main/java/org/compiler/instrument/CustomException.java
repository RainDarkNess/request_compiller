package org.compiler.instrument;

// Класс для кастомного исключения
public class CustomException extends Exception {
    public CustomException(String message) {
        super(message);
    }
}
