int m;

int f(int n, int m){
    int ret, k;

    if(n == 0){
        ret = 1;
        return ret;
    }
    else{
        ret = 2;
    }

    k = n - 1;
    ret = f(k, k);
    ret = n * ret;

    return ret;
}

void main(){
    int a;
    int b;

    b = 5;
    a = f(b, b);
    print a;

    m = 2;
    print m;
}