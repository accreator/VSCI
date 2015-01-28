float pi() {
float ret = 0;
int i = 1;
while(i<500000) {
ret = ret + 1.0/i/i;
i = i + 1;
}
ret = ret*6;
float x=0, s = 1;
while(ret-x*x>0.00001) {
while(ret-(x+s)*(x+s)<0) {
s = s/2;
}
x = x+s;
}
ret = x;
return ret;
}