proc main begin
b = 2.1
c = 8.3
temp0 = b / c
temp1 = temp0 / a
temp2 = temp1 * b
temp3 = a + temp2
temp4 = temp3 > 0.0
temp6 = !temp4
if (temp6) goto Label1
temp5 = a - 1.0
a = temp5
goto Label0
Label1:
Label0:
proc main end

