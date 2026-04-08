proc main begin
temp0 = -2.2
a = temp0
temp1 = a + 3.2
b = temp1
temp2 = a + b
temp3 = a - b
temp4 = temp2 != temp3
temp7 = !temp4
if (temp7) goto Label0
temp5 = a * b
stemp0 = temp5
goto Label1
Label0:
temp6 = a - b
stemp0 = temp6
Label1:
d = stemp0
proc main end

