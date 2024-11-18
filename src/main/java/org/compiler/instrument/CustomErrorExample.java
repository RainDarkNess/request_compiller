package org.compiler.instrument;

public class CustomErrorExample {
    public static void main(String[] args) {
        try {
            checkValue(-1);
        } catch (CustomException e) {
            System.out.println("Ошибка: " + e.getMessage());
        }
    }

    public static void checkValue(int value) throws CustomException {
        if (value < 0) {
            throw new CustomException("Значение не может быть отрицательным: " + value);
        }
        System.out.println("Значение: " + value);
    }
}
