package org.compiler.instrument;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;

public class FileCreator {
    public static void createFileWithString(String filePath, String content) {
        BufferedWriter writer = null;
        try {
            File file = new File(filePath);
            if (!file.exists()) {
                file.createNewFile();
            }
            writer = new BufferedWriter(new FileWriter(file, true));
            writer.write(content);
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            try {
                if (writer != null) {
                    writer.close();
                }
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    public static void clearFile(String filePath) {
        FileWriter writer = null;
        try {
            File file = new File(filePath);
            if (file.exists()) {
                writer = new FileWriter(file);
                writer.write("");
                System.out.println("Содержимое файла очищено: " + filePath);
            } else {
                writer = new FileWriter(file);
                System.out.println("Файл был создан: " + filePath);
            }
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            try {
                if (writer != null) {
                    writer.close();
                }
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }
}
