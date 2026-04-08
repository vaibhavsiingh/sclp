proc main begin
a = 2
b = 4
c = 10
d = 3
e = 8
f = 7
temp0 = b + c
temp1 = a >= temp0
temp2 = e - a
temp3 = d < temp2
temp4 = temp1 && temp3
temp5 = f != d
temp6 = temp4 || temp5
temp10 = !temp6
if (temp10) goto Label1
temp7 = d * e
temp8 = temp7 / f
temp9 = a + temp8
a = temp9
goto Label0
Label1:
b = 1
Label0:
proc main end

