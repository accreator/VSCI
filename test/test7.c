float pi2()
{
int a = 1103515245;
int c = 12345;
int x = 84575325;
int i = 0;
int u = 0, v = 0;
while(i<500000) {
x = a*x+c;
float r1 = x/2147483647.0; //(x%32768)/32768.0;
x = a*x+c;
float r2 = x/2147483647.0; //(x%32768)/32768.0;
if(r1*r1+r2*r2 <= 1) u = u + 1;
v = v + 1;
i = i + 1;
}
return 4.0*u/v;
}
