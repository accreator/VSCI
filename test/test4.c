/*
test4
*/
int sort() {
int x[7]  = {7,1,3,4,6,5,2};
int i=0;
while(i<7) {
int j=i;
while(j<7) {
if(x[i]>x[j]) {
int t = x[i];
x[i] = x[j];
x[j] = t;
}
j = j + 1;
}
i = i + 1;
}
i = 0;
int ret=0; ///*
while(i<7) {
ret=ret*10+x[i];
i = i+1;
} //*/
return ret; /* meow~ */
}