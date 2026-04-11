int l1, l2;

void func1();
float fl1, fl2;

int func2(int a, float b, int c);
float func3(int p, float q, float r);

int func4(){
    l1 = 4;
    return l1 + l1;
}

void func1(){
    l2 = 5;
    fl2 = 5.0;
}

int func2(int a, float b, int l1){
    a = a + 10;
    l1 = 5;
    return a + l1 + l2 + 2;
}

float func3(int a, float b, float c){
    b = b * fl2;
    c = c + 10.5;
    return b + c;
}

void main(){
    func1();

    // print l2; print fl2;
    l2 = func4();

    // print l1; print l2;
    l2 = func2(l1, fl2, l2);

    // print l1; print l2;
    fl2 = func3(l1, fl1, fl2);

    print fl1;
    print fl2;
}