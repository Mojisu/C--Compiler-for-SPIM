int sum(int a){
	if(a == 1)
		return 1;
	else
		return a + sum(a-1);
}

void main(void){
	int i;
	int o;

	i = input();
	o = sum(i);
	output(o);
}
