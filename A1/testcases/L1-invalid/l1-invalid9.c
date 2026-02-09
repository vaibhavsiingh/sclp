void main(string a,string b){
	
	print a; // print the garbage value because it is a local variable and not initialized
	print b; // produces null with memory out of bound exception
}
