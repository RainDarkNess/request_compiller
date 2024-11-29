# Простой компилятор модального языка
Простой компилятор в объектный файл для x8086 windows исходного модального языка поддающийся компиляции в exe с помощью gcc

## Языки
- java
- c

## Использование
Чтобы запустить приложение нужно:

Поместить исходный модальный язык по пути resources/examples/code

Пример языка
```typescript
{
    dim a, b #;
    dim i, r, h @;
    dim bool &;

    a:=1.23E+3;
    b:=5;

    if(a NE 1) begin
    a := a min 1;
    end

}
```
Далее запустить main.java

### Итог работы
Выходной скомпилированный объектный файл: myobj.o

### Компиляция в exe
```cmd
gcc myobj.o -o myprog
```
### how to update

```cmd
git fetch
git pull
```

### Зачем?
Просто.

## Команда проекта
я.
- [rain (github)](https://github.com/RainDarkNess) — разраб
