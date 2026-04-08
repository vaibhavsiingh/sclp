proc main begin
a = 2.3
b = 3.4
c = 4.5
d = 5.6
e = 6.7
temp0 = b + c
temp1 = a >= temp0
temp2 = e * d
temp3 = temp2 / c
temp4 = d > temp3
temp5 = temp1 && temp4
temp8 = !temp5
if (temp8) goto Label1
temp6 = a * d
temp7 = temp6 / e
a = temp7
goto Label0
Label1:
temp9 = e + d
e = temp9
Label0:
proc main end

