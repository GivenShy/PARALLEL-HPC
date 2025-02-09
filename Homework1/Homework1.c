#include <stdio.h>
#include <stdlib.h>


void Assignment1(){
    int a  = 1;
    int *pointer = &a;
    printf("Variable Value: %d\n",a);
    printf("Variable Address: %p\n",&a);
    printf("Variable Address using pointer: %p \n",pointer);
    *pointer = 11;
    printf("Modified value: %d\n",a);

}

void Assignment2(){
    int integers[5] = {1,2,3,4,5};
    int *p = integers;
    for(int i = 0; i<5;i++){
        printf("Array element: %d\n",*(p+i));
        *(p+i) = *(p+i)*10;
    }
    for(int i = 0;i<5;i++){
        printf("Modified Array element using pointer: %d\n",*(p+i));
        printf("Modified Array element using array name: %d\n",integers[i]);
    }
}

void swap(int *a, int *b){
    int temp = *a;
    *a = *b;
    *b = temp;
}

void Assignment3(){
    int a = 10;
    int b = 20;
    printf("Before modification: %d\n",a);
    printf("Before modification: %d\n",b);
    swap(&a,&b);
    printf("After modification: %d\n",a);
    printf("After modification: %d\n",b);
}

void Assignment4(){
    int a = 100;
    int *ptr = &a;
    int **pptr = &ptr;
    printf("Integer Value: %d\n",a);
    printf("Value using the pointer: %d\n",*ptr);
    printf("Value using the double pointer: %d\n",**pptr);
}

void Assignment5(){
    int *dynamicMemory = (int *)malloc(sizeof(int));
    if (dynamicMemory == NULL){
        printf("Memory allocation failed\n");
        return;
    }
    *dynamicMemory = 100;
    int *dynamicArray = (int *)malloc(5*sizeof(int));
    for(int i = 0; i<5;i++){
        *(dynamicArray+i) = i*15;
    }
    for (int i = 0; i<5; i++){
        printf("Array Value:%d\n",*(dynamicArray+i));
    }
    free(dynamicMemory);
    free(dynamicArray);

}

int str_length(char *str){
    int i = 0;

    while (*str++)
    {
        i++;
    }
    
    return i;
}

void Assignment6(){
    char *str = "Hello World!";
    printf("My string address: %d\n",str);
    for(char *s = str;*s;s++){
        printf("%c",*s);
    }
    printf("\n");
}

void Assignment7(){
    char *strings[] = {
        "First String",
        "Second String",
        "Third String",
        "Forth String",
        "Fifth String"
    };
    for(int i = 0;i<5;i++){
        printf("%s\n",*(strings+i));
    }
    printf("\n");
    strings[3] = "Modified third string";
    for(int i = 0;i<5;i++){
        printf("%s\n",*(strings+i));
    }
}

int main() {
    Assignment1();
    Assignment2();
    Assignment3();
    Assignment4();
    Assignment5();
    Assignment6();
    char user_input[100];
    printf("Enter a random string: ");
    scanf("%[^\n]",user_input);
    printf("%s\n", user_input);

    printf("Length of the entered string: %d",str_length(user_input));
    Assignment7();
    return 0;
}
