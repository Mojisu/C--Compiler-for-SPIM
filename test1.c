int c[5];
int func(int d[]){
	int k;
	k = 2;
	output(d[k]);
	return d[k];
}
int f(int x) { return x; }
int g(int x[]) { return x[0]; }

void main(void){
	int a;
	int b;
	
	c[0] = 10;
	c[1] = 1;
	c[2] = 2;
	c[3] = 3;
	c[4] = 4;
	a = func(c);
	output(a);
	
	a = 3;
	{
		int d[5];
		int b;
		b = 5;
		output(a);
		output(b);
		{
			int k[3];
			int j;
			k[0] = 10;
			j = 111;
			output(a);
			output(b);
			output(k[0]);
			output(j);
		}
		if (b > 0) {
			int a[6];
			a[0] = 0;
			a[1] = 1;
			a[2] = 2;
			a[3] = 5;
			b = b - 1;
			a[b] = 1;
			
			a[a[b]] = 4;
			output(a[a[b]]);
			a[f(b)] = 3;
			a[g(a)] = 2;
			output(a[a[b]]);
			output(a[f(b)]);
			output(a[g(a)]);
		}
	}
}
