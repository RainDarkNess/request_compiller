package org.compiler.instrument;

import java.util.Objects;

public class DigitCheck {
    private static final char[] LATIN_HEX_ALPHABET = {'A', 'B', 'C', 'D', 'E', 'F', 'a', 'b', 'c', 'd', 'e', 'f', 'x'};

    public static String convertValue(String value) {
        boolean debug = true;
        boolean exponentialWritePlus = false;
        boolean exponentialWriteExp = false;
        boolean isHex = false;
        boolean isOct = false;
        boolean isBin = false;
        boolean wasDot = false;
        boolean isDec = false;
        boolean valid = true;
        boolean isReal = false;

        if(Objects.equals(value, "false")){
            return "-1";
        }
        if(Objects.equals(value, "true")){
            return "0";
        }

        StringBuilder tmpValue = new StringBuilder();
        int strLenValue = (!Character.isDigit(value.charAt(value.length() - 1)) && value.length() > 1) ? value.length() - 1 : value.length();

        for (int i = 0; i < strLenValue; i++) {
            char currentChar = value.charAt(i);
            tmpValue.append(currentChar);

            if (currentChar == 'E' || currentChar == 'e') {
                if (debug) System.out.println("Exponential read?");
                exponentialWriteExp = true;
                if(wasDot){
                    return "false_verification";
                }
            }else{
                wasDot = false;
            }
            if (currentChar == '+' || currentChar == '-') {
                if (debug) System.out.println("Plus or Minus find...Exponential read.");
                if (exponentialWriteExp) {
                    exponentialWritePlus = true;
                } else {
                    System.err.printf("Error. In exp writing not found 'E' %s%n", value);
                    return "false_verification";
                }
            }
            if (!exponentialWritePlus && !exponentialWriteExp) {
                isHex = false;
                if (!Character.isDigit(currentChar)) {
                    boolean forRealHexCheck = false;
                    for (char hexChar : LATIN_HEX_ALPHABET) {
                        if (currentChar == hexChar) {
                            isHex = true;
                            forRealHexCheck = true;
                            break;
                        }
                    }
                    if (currentChar == '.') {
                        wasDot = true;
                        if (isReal) {
                            System.err.println("Error. Find double dot");
                            return "false_verification";
                        }
                        if (isHex) {
                            System.err.println("Error. hex cannot be real");
                            return "false_verification";
                        }
                        isReal = true;
                        continue;
                    } else if (!isHex) {
                        System.err.println("Error. Find letter but it not in hex table");
                        valid = false;
                        break;
                    }
                    if (isReal && !forRealHexCheck) {
                        System.err.println("Error. Real format number not valid");
                        valid = false;
                        break;
                    }
                } else {
                    int digit = Character.getNumericValue(currentChar);
                    if (digit >= 0 && digit <= 1) {
                        isBin = true;
                    }
                    if (digit >= 2 && digit <= 7) {
                        isBin = false;
                        isOct = true;
                    }
                    if (digit >= 8 && digit <= 9) {
                        isBin = false;
                        isOct = false;
                        isDec = true;
                    }
                }
            } else {
                if ((currentChar == '.' || !Character.isDigit(currentChar)) && currentChar != 'E' && currentChar != '+' && currentChar != '-') {
                    System.err.println("Error. Find not digit value after exp");
                    return "false_verification";
                }
            }
        }

        if (exponentialWriteExp && !exponentialWritePlus) {
            if (debug) System.out.println("In exp writing not found '+'");
        }

        if (valid) {
            double answerDouble = 0;
            float answerFloat = 0;
            int answerInt = 0;

            char lastChar = value.charAt(value.length() - 1);
            if(!Character.isDigit(lastChar) && lastChar != 'H' && lastChar != 'h' && lastChar != 'D' && lastChar != 'd' && lastChar != 'B' && lastChar != 'b' && lastChar != 'O' && lastChar != 'o'){
                return "false_verification";
            }
            if (lastChar == 'H' || lastChar == 'h') {
                if (isReal) {
                    System.err.println("Error. hex cannot be real");
                    return "false_verification";
                }
                if (debug) System.out.println("Digit is hex");
                int hexDigit = Integer.parseInt(tmpValue.toString(), 16);
                tmpValue.setLength(0);
                tmpValue.append(hexDigit);
            } else if (lastChar == 'B' || lastChar == 'b') {
                if (isOct || isDec || isHex) {
                    System.err.printf("Error. Find litter %c, in another type%n", lastChar);
                    return "false_verification";
                }
                int decimalValue = binaryToDecimal(value.substring(0, value.length() - 1));
                tmpValue.setLength(0);
                tmpValue.append(decimalValue);
                if (debug) System.out.println("Digit is binary");
            } else if (lastChar == 'O' || lastChar == 'o') {
                if (isDec || isHex) {
                    System.err.printf("Error. Find litter %c, in another type%n", lastChar);
                    return "false_verification";
                }
                if (debug) System.out.println("Digit is oct");
                int octal = Integer.parseInt(tmpValue.toString());
                int decimalValue = 0;
                int octDigit = 1;
                while (octal > 0) {
                    int lastDigit = octal % 10;
                    octal /= 10;
                    decimalValue += lastDigit * octDigit;
                    octDigit *= 8;
                }
                tmpValue.setLength(0);
                tmpValue.append(decimalValue);
            } else if (lastChar == '.') {
                System.err.println("Error. Real format number not valid");
                valid = false;
            } else if (lastChar == 'D' || lastChar == 'd') {
                if (isHex) {
                    System.err.printf("Error. Find litter %c, in another type%n", lastChar);
                    return "false_verification";
                }
            } else {
                if (exponentialWriteExp) {
                    answerFloat = Float.parseFloat(tmpValue.toString());
                    String ans = doubleToIEEE754String(answerFloat);
                    tmpValue.setLength(0);
                    tmpValue.append(ans);
                } else if (isReal) {
                    answerFloat = Float.parseFloat(tmpValue.toString());
                    String ans = doubleToIEEE754String(answerFloat);
                    tmpValue.setLength(0);
                    tmpValue.append(ans);
                } else {
                    answerInt = Integer.parseInt(tmpValue.toString());
                    tmpValue.setLength(0);
                    tmpValue.append(answerInt);
                }
            }
        }

        return valid ? tmpValue.toString() : "false_verification";
    }

    public static String doubleToIEEE754String(float value) {
        long bits = Double.doubleToLongBits(value);
        return Long.toString(bits);
    }

    public static String intToIEEE754Decimal(int value) {
        return String.valueOf(Float.intBitsToFloat(value));
    }


    private static int binaryToDecimal(String binary) {
        return Integer.parseInt(binary, 2);
    }
}
