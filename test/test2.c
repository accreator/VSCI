int max()
{
int x[10];
int i;
int ret;
i=0; while(i<10) {x[i] = (i-7)*(i-7); i=i+1;}
i=0; ret=-10; while(i<10) {if(x[i]>ret) ret=x[i];i=i+1;}
return ret;
}
/*
test2
*/